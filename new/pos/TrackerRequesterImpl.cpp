
#include "StdAfx.h"

#include "TrackerRequesterImpl.h"
#include "TrackerClient.h"
#include "TrackerRequesterListener.h"
#include "common/BaseInfo.h"
#include <synacast/protocol/PacketHeadIO.h>
#include <synacast/protocol/base.h>
#include <synacast/protocol/PacketStream.h>


const UINT PPL_TRACKER_CLIENT_COUNT_LIMIT = 256;


/// ���ڼ���Ƿ�������(ͨ�����ͱ����ʵ��)������һ��list
const UINT TRACKER_KEEP_RUNNING_INTERVAL	= 5 * 60 * 1000; // 5����



/// TrackerClient������������ʧ�ܴ���
const WORD MAX_TRACKER_FAILED_TIMES	= 1;







TrackerRequester* TrackerRequesterFactory::PeerCreate()
{
	return new PeerTrackerRequester();
}

TrackerRequester* TrackerRequesterFactory::MCCCreate()
{
	return new SourceTrackerRequester();
}

TrackerRequester* TrackerRequesterFactory::MDSCreate()
{
	return new MDSTrackerRequester();
}

TrackerRequester* TrackerRequesterFactory::SimpleMDSCreate()
{
	return new SimpleMDSTrackerRequester();
}

TrackerRequester* TrackerRequesterFactory::SparkMDSCreate()
{
	return new SparkMDSTrackerRequester();
}




/*
void TrackerRequesterImpl::Start(UINT count, const TRACKER_LOGIN_ADDRESS addrs[])
{
	DoStart(count, addrs);
	if (!m_KeepRunningTimer.IsStarted())
	{
		m_KeepRunningTimer.Start(this, m_KeepRunningInterval);
	}
}
*/
void TrackerRequesterImpl::Start( TrackerRequesterListener& listener, UINT count, const TRACKER_LOGIN_ADDRESS addrs[], boost::shared_ptr<PeerInformation> peerInformation, boost::shared_ptr<TrackerPacketSender> packetSender )
{
	LIVE_ASSERT(!m_isStarted);
	m_listener = &listener;
	m_PeerInformation = peerInformation;
	m_PacketSender = packetSender;
	DoStart(count, addrs);
	if (false == m_KeepRunningTimer.is_started())
	{
		m_KeepRunningTimer.start(m_KeepRunningInterval);
	}
}


TrackerRequesterImpl::TrackerRequesterImpl() 
	: m_listener( NULL ), m_isStarted(false)
{
	TRACKER_DEBUG("Start keep running timer at interval " << TRACKER_KEEP_RUNNING_INTERVAL);
	m_KeepRunningInterval = TRACKER_KEEP_RUNNING_INTERVAL;
	//m_PeerInformation = m_listener->GetData().GetPeerInformation();

	m_KeepRunningTimer.set_callback(boost::bind(&TrackerRequesterImpl::OnKeepRunningTimer, this));
}

TrackerRequesterImpl::~TrackerRequesterImpl()
{
	DeleteClients();
}

void TrackerRequesterImpl::DoStart(size_t trackerCount, const TRACKER_LOGIN_ADDRESS addrs[])
{
	LIVE_ASSERT(!m_isStarted);
	LIVE_ASSERT(trackerCount > 0 && trackerCount <= PPL_TRACKER_CLIENT_COUNT_LIMIT);
	CreateClients(trackerCount, addrs);
	DoStartClients();
	m_isStarted = true;
}

bool TrackerRequesterImpl::TryHandleResponse( data_input_stream& is, const InetSocketAddress& sockAddr, BYTE proxyType )
{
	TRACKER_ADDRESS addr;
	addr.IP = sockAddr.GetIP();
	addr.Port = sockAddr.GetPort();
	addr.Type = proxyType;
	addr.ReservedStatusCode = 0;
	TrackerClientIndex::const_iterator iter = m_clientIndex.find( addr );
	if ( iter == m_clientIndex.end() )
		return false;
	TrackerClientPtr client = iter->second;
	LIVE_ASSERT(client);

	// ��Ҫ����ͷ��
	OLD_UDP_PACKET_HEAD udpHead;
	GUID channelGUID;
	is >> udpHead >> channelGUID;
	if ( !is )
	{
		// ����ͷ��ʧ��
		LIVE_ASSERT(false);
		return false;
	}
	if ( channelGUID != m_PeerInformation->ChannelGUID )
	{
		// channel guid��һ��
		return false;
	}

	if ( false == client->HandleResponse( is, udpHead ) )
	{
		// ������û�б�trackerclientʶ�𣬷���false��������ģ�鴦��
		TRACKER_DEBUG("TrackerRequesterImpl::HandleResponse no client " << addr);
		m_Statistics.TotalIgnoredResponse++;
		return false;
	}
	m_Statistics.Flow.Download.Record( is.total_size() );
	return true;
}

bool TrackerRequesterImpl::TryHandleSecureResponse( data_input_stream& is, NEW_UDP_PACKET_HEAD& head, const InetSocketAddress& sockAddr, BYTE proxyType, bool isPeerProxy )
{
	TRACKER_ADDRESS addr;
	addr.IP = sockAddr.GetIP();
	addr.Port = sockAddr.GetPort();
	addr.Type = proxyType;
	addr.ReservedStatusCode = 0;
	// ���ݵ�ַѰ��tracker
	TrackerClientIndex::const_iterator iter = m_clientIndex.find( addr );
	if ( iter == m_clientIndex.end() )
		return false; // û���ҵ���˵������peer-tracker���ģ�����false��������ģ�鴦��
	TrackerClientPtr client = iter->second;
	LIVE_ASSERT(client);

	GUID channelGUID, peerGUID;
	is >> channelGUID >> peerGUID;
	if ( !is )
	{
		// ����ͷ��ʧ��
		LIVE_ASSERT(false);
		return false;
	}
	if ( channelGUID != m_PeerInformation->ChannelGUID || peerGUID != m_PeerInformation->PeerGUID )
	{
		// channel guid��һ��
		return false;
	}

	if ( false == client->HandleSecureResponse( is, head, isPeerProxy ) )
	{
		// ������û�б�trackerclientʶ�𣬷���false��������ģ�鴦��
		TRACKER_DEBUG("TrackerRequesterImpl::HandleResponse no client " << head.Action << " from " << addr);
		m_Statistics.TotalIgnoredResponse++;
		return false;
	}
	m_Statistics.Flow.Download.Record( is.total_size() );
	return true;
}


void TrackerRequesterImpl::CreateClients(size_t count, const TRACKER_LOGIN_ADDRESS addrs[])
{
	LIVE_ASSERT(count > 0 && count <= PPL_TRACKER_CLIENT_COUNT_LIMIT);
	LIVE_ASSERT(m_clients.empty());
	m_clients.resize(count);
	m_clientIndex.clear();
	for (size_t i = 0; i < count; ++i)
	{
		m_clients[i].reset( CreateClient() );
		LIVE_ASSERT(m_clients[i] != NULL);
		TRACKER_ADDRESS trackerAddr = addrs[i].ServerAddress;
		trackerAddr.ReservedStatusCode = 0;
		m_clients[i]->Init( *this, addrs[i], m_PeerInformation, m_PacketSender );
		m_clientIndex[ trackerAddr ] = m_clients[i];
	}
}
void TrackerRequesterImpl::DoStartClients()
{
	for (size_t i = 0; i < m_clients.size(); ++i)
	{
		m_clients[i]->Start();
	}
}
void TrackerRequesterImpl::DeleteClients()
{
	STL_FOR_EACH(TrackerClientCollection, m_clients, iter)
	{
		TrackerClientPtr client = *iter;
		client->Stop();
	}
	m_clients.clear();
	m_clientIndex.clear();
}
void TrackerRequesterImpl::OnKeepRunningTimer()
{
	for (size_t i = 0; i < m_clients.size(); ++i)
	{
		m_clients[i]->KeepAlive();
	}
}


int TrackerRequesterImpl::GetMaxKeepAliveTimeoutTimes() const
{
	return 3;
}

size_t TrackerRequesterImpl::GetClientCount() const
{
	return m_clients.size();
}

TrackerClientPtr TrackerRequesterImpl::GetClient(size_t index)
{
	if (index >= m_clients.size())
		return TrackerClientPtr();
	return m_clients[index];
}



void TrackerRequesterImpl::OnTrackerClientLogin(TrackerClient* sender)
{
	m_listener->OnLoginComplete(sender->GetServerAddress(), true);
}

void TrackerRequesterImpl::OnTrackerClientLoginFailed(TrackerClient* sender, long errcode)
{
}

void TrackerRequesterImpl::OnTrackerClientKeepAlive(TrackerClient* sender)
{
}

void TrackerRequesterImpl::OnTrackerClientKeepAliveFailed(TrackerClient* sender, long errcode)
{
}

void TrackerRequesterImpl::OnTrackerClientList(TrackerClient* sender, size_t count, const CANDIDATE_PEER_INFO peers[], size_t lanPeerCount, const INNER_CANDIDATE_PEER_INFO lanPeers[], UINT64 sourceTimeStamp)
{
	m_listener->OnPeerListed( sender->GetServerAddress(), static_cast<UINT8>( count ), peers, static_cast<UINT8>( lanPeerCount ), lanPeers, sourceTimeStamp );
}

void TrackerRequesterImpl::OnTrackerClientListFailed(TrackerClient* sender, long errcode)
{
}

bool TrackerRequesterImpl::SaveSourceMinMax(const PEER_MINMAX& minmax)
{
	return m_listener->SaveSourceMinMax(minmax);
}

void TrackerRequesterImpl::SaveDetectedAddress(UINT detectedIP, UINT16 detectedUDPPort, TrackerClient* sender)
{
	m_listener->SaveDetectedAddress(detectedIP, detectedUDPPort, sender->GetServerAddress());
}









//////////////////////////////////////////////////////////////////////////
// PeerTracker��Ա����
//////////////////////////////////////////////////////////////////////////

PeerTrackerRequester::PeerTrackerRequester()
{
	m_CurrentClient = 0;
	// Modified by Tady, 091310
//	m_UseTCP = false;
	m_UseTCP = true;
}
PeerTrackerRequester::~PeerTrackerRequester()
{
}

void PeerTrackerRequester::Restart()
{
	TrackerClientPtr client = GetCurrentClient();
	if (client)
	{
		// ���client�Ѿ���½�ϣ�����Ҫ���µ�½
		//client->Stop();
		client->Restart();
	}
}

void PeerTrackerRequester::DoStart(size_t trackerCount, const TRACKER_LOGIN_ADDRESS addrs[])
{
	TrackerRequesterImpl::DoStart(trackerCount, addrs);
	ListPeers();
}

void PeerTrackerRequester::ListPeers()
{
	if (m_clients.empty())
		return;
	bool useTcp = !GetCurrentClient()->IsUDP(); // ��ǰʹ�õ���TCP tracker
	for (size_t i = 0; i < m_clients.size(); ++i)
	{
		TrackerClientPtr client = m_clients[i];
		//const TRACKER_ADDRESS& addr = client->GetServerAddress();
		//(void)addr;
        if (client->IsUDP() || m_UseTCP || useTcp)
		{
			client->ListPeers();
		}
		else
		{
			TRACKER_DEBUG("DoList: ignore tcp tracker " << client->GetServerAddress() /*addr*/);
		}
	}
}

bool PeerTrackerRequester::HandleFail(TrackerClient& client)
{
	// ���ټ�¼FailedTimes��ֱ���л�����һ��
	client.FailedTimes++;
	if (client.FailedTimes < MAX_TRACKER_FAILED_TIMES)
		return true;

	TRACKER_DEBUG("Tracker " << client.GetServerAddress() << " failed times exceeds max " << MAX_TRACKER_FAILED_TIMES);
	++m_CurrentClient;
	m_Statistics.SwitchClientTimes++;
	if (m_CurrentClient >= m_clients.size())
	{
		m_CurrentClient = 0;
		m_UseTCP = true; // ȫ��ʧ�ܣ�����m_UseTCPΪtrue
		TRACKER_DEBUG("Enable TCP List");
	}
	DoStartClients();
	return false;
}
void PeerTrackerRequester::HandleSuccess(TrackerClient& client)
{
	GetCurrentClient()->FailedTimes = 0;
}

void PeerTrackerRequester::OnTrackerClientList(TrackerClient* sender, size_t count, const CANDIDATE_PEER_INFO peers[], size_t lanPeerCount, const INNER_CANDIDATE_PEER_INFO lanPeers[], UINT64 sourceTimeStamp)
{
	m_listener->OnPeerListed( sender->GetServerAddress(), static_cast<UINT8>( count ), peers, static_cast<UINT8>( lanPeerCount ), lanPeers, sourceTimeStamp );
	if (sender->IsUDP() && count > 0)
	{
		// �����udptracker�ɹ�list��peers���ͽ���tcp
		m_UseTCP = false;
	}
}
// void TrackerRequesterImpl::OnTrackerClientListFailed(TrackerClient* sender, long errcode)
// {
// }




TrackerClient* PeerTrackerRequester::CreateClient()
{
	return TrackerClientFactory::PeerCreate();
}
void PeerTrackerRequester::DoStartClients()
{
	GetCurrentClient()->Start();
}
void PeerTrackerRequester::OnKeepRunningTimer()
{
	ListPeers();
}

void PeerTrackerRequester::OnTrackerClientLogin(TrackerClient* sender)
{
	TrackerRequesterImpl::OnTrackerClientLogin(sender);
	// join�ɹ���������tcptracker������list�ɹ�
	//m_UseTCP = false;
/*	TrackerClient* currentClient = GetCurrentClient();
//	LIVE_ASSERT(false);
	if (currentClient->IsOnline())
	{
//		sender->Stop();
	}
	else
	{
		for (size_t i = 0; i < m_clients.size(); ++i)
		{
			if (m_clients[i] == sender)
			{
				m_CurrentClient = i;
				break;
			}
		}
	}*/
}

void PeerTrackerRequester::KeepAlive()
{
	TrackerClientPtr client = GetCurrentClient();
	if (client && client->IsOnline() )
	{
		client->KeepAlive();
	}
}




TrackerClient* SourceTrackerRequester::CreateClient()
{
	return TrackerClientFactory::MCCCreate();
}



TrackerClient* SimpleMDSTrackerRequester::CreateClient()
{
	return TrackerClientFactory::SimpleMDSCreate();
}

void SimpleMDSTrackerRequester::ListPeers()
{
	for (size_t i = 0; i < m_clients.size(); ++i)
	{
		TrackerClientPtr client = m_clients[i];
		if (client->IsUDP())
		{
			//TRACKER_DEBUG("DoList " << client->GetServerAddress());
			client->ListPeers();
		}
	}
}

void SimpleMDSTrackerRequester::OnKeepRunningTimer()
{
	ListPeers();
}


void SimpleMDSTrackerRequester::DoStart(size_t trackerCount, const TRACKER_LOGIN_ADDRESS addrs[])
{
	SourceTrackerRequester::DoStart(trackerCount, addrs);
	ListPeers();
}



void SparkMDSTrackerRequester::DoStart( size_t trackerCount, const TRACKER_LOGIN_ADDRESS addrs[] )
{
	LIVE_ASSERT(!m_isStarted);
	LIVE_ASSERT(trackerCount > 0 && trackerCount <= PPL_TRACKER_CLIENT_COUNT_LIMIT);
	CreateClients(trackerCount, addrs);
	// No JOIN, just LIST.
//	DoStartClients();
	m_isStarted = true;

	ListPeers();
}
