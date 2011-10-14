#include "StdAfx.h"

#include "PeerManagerImpl.h"

#include "common/AppModule.h"
#include "PeerConnection.h"
#include "common/StreamBuffer.h"
#include "common/IpPool.h"
#include "common/MediaStorage.h"
#include "common/GloalTypes.h"

#include "Downloader.h"
#include "Uploader.h"
#include "PeerConnector.h"
#include "UDPPeerConnector.h"
#include "TCPPeerConnector.h"
#include "common/PeerInfoItem.h"
#include "common/AppParam.h"
#include "common/BaseInfo.h"
#include "util/HuffmanCoding.h"
#include "util/testlog.h"

#include <synacast/protocol/PeerPacket.h>
#include <synacast/protocol/PacketHead.h>
#include <synacast/protocol/data/PeerAddressUtil.h>

#include <ppl/data/stlutils.h>
#include <ppl/data/numeric.h>
#include <ppl/io/stdfile.h>
#include <ppl/os/paths.h>
#include <ppl/os/file_system.h>
#include <ppl/util/ini_file.h>

#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/classification.hpp>

#include <ppl/text/json/writer.h>

/// 发给framework的消息：禁止某个ip来的连接请求
const UINT UM_ADD_BAN_IP = UM_USER+2000;




bool TCPSyncProblemChecker::Check()
{
	if (m_peerType == NORMAL_PEER && m_connectTime.elapsed() > 1000)
	{
		APP_ERROR("change transfer method to udp " << m_connectTime.elapsed());
		m_liveInfo.TransferMethod = TRANSFER_UDP;
		return false;
	}
	return true;
}

TCPSyncProblemChecker::~TCPSyncProblemChecker()
{
	this->Check();
}

bool HistoricalRateMeasure::Record(size_t size)
{
	this->Rate.Record((UINT)size);
	UINT newSpeed = this->Rate.GetRate();
	if (newSpeed <= this->MaximumSpeed)
		return false;
	this->MaximumSpeed = newSpeed;
	return true;
}


/// 默认值：PeerConnection的资源范围中估计的密集资源的位置－－用于FindStartPosition中定位起始的下载位置
//const UINT PEER_DEFAULT_DENSE_RESOURCE_POSITION = 75; //!起始的下载位置由33/100改为33/100

/// 默认值：tcp连接的超时时间
//const UINT PEER_DEFAULT_TCP_CONNECT_TIMEOUT = 10;

/// 默认值：p2p连接的超时时间
//const UINT PEER_DEFAULT_P2P_CONNECT_TIMEOUT = 10;



void P2PResearchConfig::Clear()
{
	this->FixedUploadSpeed = 0;
	this->UseFixedUploadSpeed = false;
	this->ConnectionControllInterval = 5;
}

void P2PResearchConfig::Load( ini_file& ini )
{
	ini.set_section( _T("connection") );
	PeerAddressUtil::ParseAddressList( this->DeniedPeers, ini.get_string( _T("deny"), "" ) );
	PeerAddressUtil::ParseAddressList( this->PreferredPeers, ini.get_string( _T("prefer"), "" ) );
	PeerAddressUtil::ParseIPList( this->DeniedIPs, ini.get_string( _T("denyip"), "" ) );
	tstring fixedQOS = ini.get_string( _T("FixedUploadSpeed"), "" );
	this->UseFixedUploadSpeed = numeric<UINT16>::try_parse( fixedQOS, this->FixedUploadSpeed );
	this->ConnectionControllInterval = ini.get_int( _T("ConnectionControllInterval"), this->ConnectionControllInterval );
}

PeerManagerConfig::PeerManagerConfig()
{
	this->DenseResourcePosition = 60;
	this->LocateFromMaxTimeSpan = 40;
	this->StartupReferPeerCount = 4;
	this->StartupReferTime = 4;
	this->RequestStoppedTimeout = 60;
	this->PrelocateInterval = 30;
	this->MaxDownloadSpeed = 0;

	this->AnnounceInterval = 3;
	this->ReservedDegree = 10;
	this->ReservedDegreeMin = 3;
	this->ReservedDegreeMax = 10;
	this->MinPeerCountForGoodStatus = 8;
	this->LANMinPeerCountForGoodStatus = 20;

	//		this->InitialPeerProtectionTime = 40;
	this->InitialPeerProtectionTime = 15;
	this->MinListInterval = 5;
	this->MaxListInterval = 65;
	this->MaxOnceKickPeerCount = 3;

	this->WANMinLocalPeerCount = 20;
	this->WANMinLocalPeerPercent = 10;
	this->LANMinLocalPeerCount = 30;

	this->ConnectingPeerPercent = 33;

	this->NoConnectOut = false;

}

void PeerManagerConfig::Load( ini_file& ini )
{
	this->MaxDownloadSpeed = ini.get_int(TEXT("MaxDownloadSpeed"), this->MaxDownloadSpeed);

	this->DenseResourcePosition = ini.get_int(TEXT("StartPosition"), this->DenseResourcePosition);
	LIMIT_MIN_MAX(this->DenseResourcePosition, 0, 100);

	this->LocateFromMaxTimeSpan = ini.get_int(TEXT("LocateFromMaxTimeSpan"), this->LocateFromMaxTimeSpan);
	this->StartupReferPeerCount = ini.get_int(TEXT("StartupReferPeerCount"), this->StartupReferPeerCount);
	this->StartupReferTime = ini.get_int(TEXT("StartupReferTime"), this->StartupReferTime);

	this->RequestStoppedTimeout = ini.get_int(TEXT("RequestStoppedTimeout"), this->RequestStoppedTimeout);
	this->PrelocateInterval = ini.get_int(TEXT("PrelocateInterval"), this->PrelocateInterval);

	ini.set_section(TEXT("peermgr"));
	this->AnnounceInterval = ini.get_int(TEXT("AnnounceInterval"), this->AnnounceInterval);
	LIMIT_MIN(this->AnnounceInterval, 1);
	this->ReservedDegree = ini.get_int(TEXT("ReservedDegree"), this->ReservedDegree);
	LIMIT_MIN_MAX(this->ReservedDegree, 0, 100);
	this->ReservedDegreeMin = ini.get_int(TEXT("ReservedDegreeMin"), this->ReservedDegreeMin);
	this->ReservedDegreeMax = ini.get_int(TEXT("ReservedDegreeMax"), this->ReservedDegreeMax);

	this->MinPeerCountForGoodStatus = ini.get_int(TEXT("MinPeerCountForGoodStatus"), this->MinPeerCountForGoodStatus);
	this->InitialPeerProtectionTime = ini.get_int(TEXT("InitialPeerProtectionTime"), this->InitialPeerProtectionTime);

	this->MinListInterval = ini.get_int(TEXT("MinListInterval"), this->MinListInterval);
	this->MaxListInterval = ini.get_int(TEXT("MaxListInterval"), this->MaxListInterval);

	this->MaxOnceKickPeerCount = ini.get_int(TEXT("MaxOnceKickPeerCount"), this->MaxOnceKickPeerCount);

	this->WANMinLocalPeerCount = ini.get_int(TEXT("WANMinLocalPeerCount"), this->WANMinLocalPeerCount);
	this->WANMinLocalPeerPercent = ini.get_int(TEXT("WANMinLocalPeerPercent"), this->WANMinLocalPeerPercent);
	LIMIT_MIN_MAX(this->WANMinLocalPeerPercent, 0, 100);
	this->LANMinLocalPeerCount = ini.get_int(TEXT("LANMinLocalPeerCount"), this->LANMinLocalPeerCount);

	this->ConnectingPeerPercent = ini.get_int(TEXT("ConnectingPeerPercent"), this->ConnectingPeerPercent);
	LIMIT_MIN_MAX(this->ConnectingPeerPercent, 0, 100);

	this->NoConnectOut = ini.get_bool(TEXT("NoConnectOut"), this->NoConnectOut);

	this->ConnectionConfig.Load(ini);
	this->Uploader.Load(ini);
	this->TunnelConfig.Load(ini);
	this->ConnectorConfig.Load(ini);
}


PeerConnectorConfig::PeerConnectorConfig()
{
	this->DelayTimes = 0;
	this->Interval = 4;
	this->MaxConnectionInitiateEveryTime = 5;
	this->ShortConnectionTimeout = 3;
	this->NormalConnectionTimeout = 5;
	this->LongConnectionTimeout = 10;
	this->VIPConnectionTimeout = 60;
	this->HandshakeTimeout = 6000;
	this->UDPHandshakeTimeout = 4000;
	this->StartConnectIncrement = 3;
	this->IsRefuseUDPConnectionRequest = false;
	this->IfOnlyAcceptMDS = false;
	this->DisableDirectConnection = false;
}

void PeerConnectorConfig::Load(ini_file& ini)
{
	ini.set_section(TEXT("connector"));
	this->DelayTimes = ini.get_int(TEXT("DelayTimes"), this->DelayTimes);
	this->Interval = ini.get_int(TEXT("Interval"), this->Interval);
	this->MaxConnectionInitiateEveryTime = ini.get_int(TEXT("MaxConnection"), this->MaxConnectionInitiateEveryTime);
	this->ShortConnectionTimeout = ini.get_int(TEXT("ShortTimeout"), this->ShortConnectionTimeout);
	this->NormalConnectionTimeout = ini.get_int(TEXT("NormalTimeout"), this->NormalConnectionTimeout);
	this->LongConnectionTimeout = ini.get_int(TEXT("LongTimeout"), this->LongConnectionTimeout);
	this->VIPConnectionTimeout = ini.get_int(TEXT("VIPTimeout"), this->VIPConnectionTimeout);
	this->HandshakeTimeout = ini.get_int(TEXT("HandshakeTimeout"), this->HandshakeTimeout);
	this->UDPHandshakeTimeout = ini.get_int(TEXT("UDPHandshakeTimeout"), this->UDPHandshakeTimeout);
	this->StartConnectIncrement = ini.get_int(TEXT("StartConnectIncrement"), this->StartConnectIncrement);

	if (this->HandshakeTimeout == 0)
		this->HandshakeTimeout = 6000;
	if (this->UDPHandshakeTimeout == 0)
		this->UDPHandshakeTimeout = 4000;
	if (this->Interval == 0)
		this->Interval = 4;
	if (this->MaxConnectionInitiateEveryTime == 0)
		this->MaxConnectionInitiateEveryTime = 5;

	this->DisableDirectConnection = ini.get_bool(TEXT("DisableDirectConnection"), this->DisableDirectConnection);

	ini.set_section(TEXT("acceptor"));
	this->IsRefuseUDPConnectionRequest = ini.get_bool(TEXT("RefuseUDPConnectionRequest"), this->IsRefuseUDPConnectionRequest);
	this->IfOnlyAcceptMDS = ini.get_bool(TEXT("OnlyAcceptMDS"), this->IfOnlyAcceptMDS);
}

PeerTunnelConfig::PeerTunnelConfig()
{
	this->UsedTime = 3000;
	this->UdpWindowSizeMin = 2;
	this->TcpWindowSizeMin = 2;
	this->UdpWindowSizeMax = 50;
	this->TcpWindowSizeMax = 30;
	this->TcpWindowSizeCal = 4000;
	this->UdpIntTimeOutMin = 2500;
	this->UdpIntTimeOutMax = 5500;
	this->UdpUseArraySize = 50;
	this->ExTimeOutLevel = 3500;
	this->PieceTCPPeerTunnelUsedTime = 100*1000;
}
void PeerTunnelConfig::Load(ini_file& ini)
{
	ini.set_section(TEXT("download"));
	this->UsedTime = ini.get_int(TEXT("UsedTime"), this->UsedTime);
	this->UdpWindowSizeMin = ini.get_int(TEXT("UdpWindowSizeMin"), this->UdpWindowSizeMin);
	this->TcpWindowSizeMin = ini.get_int(TEXT("TcpWindowSizeMin"), this->TcpWindowSizeMin);
	this->UdpWindowSizeMax = ini.get_int(TEXT("UdpWindowSizeMax"), this->UdpWindowSizeMax);
	this->TcpWindowSizeMax = ini.get_int(TEXT("TcpWindowSizeMax"), this->TcpWindowSizeMax);
	this->TcpWindowSizeCal = ini.get_int(TEXT("TcpWindowSizeCal"), this->TcpWindowSizeCal);
	this->UdpIntTimeOutMin = ini.get_int(TEXT("UdpIntTimeOutMin"), this->UdpIntTimeOutMin);
	this->UdpIntTimeOutMax = ini.get_int(TEXT("UdpIntTimeOutMax"), this->UdpIntTimeOutMax);
	this->UdpUseArraySize = ini.get_int(TEXT("UdpUseArraySize"), this->UdpUseArraySize);
	this->ExTimeOutLevel = ini.get_int(TEXT("ExTimeOutLevel"), this->ExTimeOutLevel);
	this->PieceTCPPeerTunnelUsedTime = ini.get_int(TEXT("PieceTCPPeerTunnelUsedTime"), this->PieceTCPPeerTunnelUsedTime);
}

PeerConnectionConfig::PeerConnectionConfig()
{
	this->MaybeFailedPieceCount = 30;
	this->MaxIdleTime = 40;
	this->PrelocateDensePosition = 50;
	this->SourceMinMaxReferRangePercent = 50;
	this->SourceMinMaxReferGap = 30;
}

void PeerConnectionConfig::Load(ini_file& ini)
{
	ini.set_section(TEXT("connection"));
	this->MaybeFailedPieceCount = ini.get_int(TEXT("MaybeFailedPieceCount"), this->MaybeFailedPieceCount);
	this->MaxIdleTime = ini.get_int(TEXT("MaxIdleTime"), this->MaxIdleTime);

	this->PrelocateDensePosition = ini.get_int(TEXT("PrelocateDensePosition"), this->PrelocateDensePosition);
	LIMIT_MIN_MAX(this->PrelocateDensePosition, 0, 100);

	this->SourceMinMaxReferRangePercent = ini.get_int(TEXT("SourceMinMaxReferRangePercent"), this->SourceMinMaxReferRangePercent);
	LIMIT_MIN_MAX(this->SourceMinMaxReferRangePercent, 0, 100);
	this->SourceMinMaxReferGap = ini.get_int(TEXT("SourceMinMaxReferGap"), this->SourceMinMaxReferGap);
}

UploaderConfig::UploaderConfig()
{
	this->MaxUploadSpeed = 0;
	this->InitialMaxUploadTimesPerSubPiece = 1000;
}

void UploaderConfig::Load( ini_file& ini )
{
	ini.set_section(TEXT("uploader"));
	this->MaxUploadSpeed = ini.get_int(TEXT("MaxUploadSpeed"), this->MaxUploadSpeed * 8) / 8;
	this->InitialMaxUploadTimesPerSubPiece = ini.get_int(TEXT("InitialMaxUploadTimesPerSubPiece"), this->InitialMaxUploadTimesPerSubPiece);
	// 允许不上传的客户端
	// 		if( InitialMaxUploadTimesPerSubPiece == 0 )
	// 		{
	// 			InitialMaxUploadTimesPerSubPiece = 1;
	// 		}
	// LIVE_ASSERT(this->MaxUploadSpeed > 50);
	// LIVE_ASSERT(this->InitialMaxUploadTimesPerSubPiece > 0);
}



//////////////////////////////////////////////////////////////////////////
//	共有函数
//////////////////////////////////////////////////////////////////////////

CPeerManager::CPeerManager(AppModule * lpPeerModule) : 
		m_PeerModule(*lpPeerModule), 
		m_streamBuffer(lpPeerModule->GetStreamBuffer()), 
		m_storage(m_streamBuffer.GetStorage()), 
		m_ipPool(lpPeerModule->GetIPPool()), 
		m_NetInfo(lpPeerModule->GetPeerInformation()->NetInfo), 
		m_StatusInfo(lpPeerModule->GetPeerInformation()->StatusInfo), 
		m_LongTimeUploadFlow(15), m_LongTimeDownloadFlow(15), m_ConnectTimeout( 10 ), 
		m_HuffmanCoding( new HuffmanCoding )
{
	APP_DEBUG("New CPeerManager");
	LIVE_ASSERT(&m_streamBuffer != NULL);

	m_state = st_none;

	m_PeerInformation = m_PeerModule.GetPeerInformation();

	m_downloader.reset( DownloaderFactory::CreateTrivial() );
	m_connector.reset( new PeerConnector( *this, m_PeerInformation, m_PeerModule.GetTCPConnectionlessPacketSender(), m_PeerModule.GetUDPConnectionlessPacketSender() ) );
	m_huffmanAnnouncePacket.reset( new PPHuffmanAnnounce );
	m_huffmanAnnouncePacket->Status = m_StatusInfo->GetStatusEx();
	// 使用更实时的degree信息
	m_huffmanAnnouncePacket->Status.Degrees = m_statistics.Degrees.ToSimple();
	m_statistics.Degrees.Left = m_PeerModule.GetMaxConnectionCount();

	m_ConnectTimeout = m_config.ConnectorConfig.NormalConnectionTimeout;
}

CPeerManager::~CPeerManager()
{
	m_connector.reset();
	Stop();
	APP_DEBUG("Delete CPeerManager");
}

bool CPeerManager::DoStart()
{
	if (m_state == st_running)
	{
		MANAGER_WARN("CPeerManager already start!");
		return false;
	}
	m_state=st_running;

	MANAGER_EVENT("CPeerManager::Start() and m_state = "<<m_state);
	//m_IntervalTimer.Start(this, APP_TIMER_INTERVAL);
	this->LoadConfiguration();
	// 先加载配置，然后再创建uploader
	m_uploader.reset(new Uploader(*this, m_storage, m_config.Uploader));

	// Spark Test
	{
		PEER_ADDRESS addr;
		if (PeerAddressUtil::ParseAddress( addr, "124.207.162.78:11087:11933"/*"192.168.43.129:3013:3340"*/ ))
		{
			DialSpark(addr, 0);
		}
	}
	DialSparks();
	return true;
}

bool CPeerManager::Stop()
{
	if (m_state == st_none)
	{
		MANAGER_WARN("CPeerManager doesn't start yet ,so cannot stop!");
		return false;
	}
	//m_IntervalTimer.Stop();

	//断开和删除所有PeerConnection
	MANAGER_EVENT("CPeerManager::Stop begin " << m_Connections.size());
	this->Validate();
	// Added by Tady, 012211: Spark. See func. PeerDownloader.RemoveTunnel().
	m_downloader->Stop(); 

	while (false == m_Connections.empty())
	{
		DelPeer(*m_Connections.begin(), PP_LEAVE_QUIT);
	}
	LIVE_ASSERT(m_Connections.empty());
	m_Connections.clear();
	m_connectionIndex.clear();
	m_AddressIndex.clear();
	m_IPIndex.clear();
	m_SessionKeyIndex.clear();

	m_state = st_none;
	MANAGER_EVENT("CPeerManager::Stop()!!");
	return true;
}

//bool CPeerManager::CheckConnected( const GUID& PeerGuid, bool isVip ) const
//{
//	int maxCount = GetMaxRepeatedConnectionCount(isVip);
//	int count = 0;
//	STL_FOR_EACH_CONST(PeerConnectionCollection, m_Connections, itr)
//	{
//		PeerConnection *pc = *itr;
//		LIVE_ASSERT(pc != NULL);
//		if (pc->GetPeerGUID() == PeerGuid)
//		{
//			++count;
//			if (count >= maxCount)
//				return true;
//		}
//	}
//	return false;
//}
//
//bool CPeerManager::IsPeerConnected(const GUID& peerGuid, bool isVIP, bool isInitFromRemote) const
//{
//	if (!CheckConnected(peerGuid, isVIP))
//		return false;
//	if (isInitFromRemote)
//	{
//		// 2006-01-14 cmp 由服务器端主动踢掉重复连接
//		return true;
//	}
//	// 客户端等待服务器端踢连接
//	return false;
//}

bool CPeerManager::IsPeerConnected(const GUID& peerGuid, bool isVIP, bool isInitFromRemote) const
{
	return this->HasPeer(peerGuid);
}

bool CPeerManager::CanAcceptConnection( UINT ip ) const
{
	return true;
}

//////////////////////////////////////////////////////////////////////////
//	与PeerConnection生命周期相关的函数
//////////////////////////////////////////////////////////////////////////

int CPeerManager::GetDegreeLeft() const
{
	return m_PeerModule.GetMaxConnectionCount() - (int)m_Connections.size();
}

void CPeerManager::OnSocketAccept(tcp_socket_ptr newClient, const InetSocketAddress& addr)
{
	m_connector->GetTCPConnector()->OnSocketAccept(newClient, addr);
}

void CPeerManager::NotifyDeletePeer(PeerConnection* conn, long errcode)
{
	//TRACE( TEXT("CPeerManager::NotifyDeletePeer %p 0x%04x\n"), conn, errcode );
	this->DoDelPeer(conn, (UINT16)errcode);
}

bool CPeerManager::DoDelPeer( PeerConnection* pc, UINT16 errcode )
{
	this->Validate();
	MANAGER_EVENT("CPeerManager::DelPeer PeerHandle="<<make_pair(pc, strings::format( "0x%04x", errcode ))<<" "<<*pc);
	VIEW_INFO("DelPeer "<<pc->GetKeyAddress()<<" "<<strings::format( "0x%04x", errcode )<<" End");
	//TRACE( TEXT("CPeerManager::DoDelPeer %p 0x%04x\n"), pc, errcode );
	if (errcode == 0)
	{
		LIVE_ASSERT(false);
	}
	if (errcode == PP_LEAVE_INVALID_PACKET)
	{
		VIEW_ERROR("CPeerManager::DelPeer PP_LEAVE_INVALID_PACKET");
		//LIVE_ASSERT(false);
	}
	PeerConnectionCollection::iterator itr = m_Connections.find(pc);
	if (pc == NULL || itr == m_Connections.end())
	{
		MANAGER_ERROR("CPeerManager::DelPeer cannot found Peer to del!");
		LIVE_ASSERT( false );
		return false;
	}
	TEST_LOG_OUT_ONCE("delete peer " << *pc << " with error=" << errcode);

	m_downloader->RemoveTunnel( pc );

	//m_statistics.ErrorCodes[errcode]++;
	m_connectionIndex.erase(pc->GetPeerGUID());
	bool res = multimaps::erase( m_AddressIndex, pc->GetKeyAddress(), pc );
	LIVE_ASSERT(res);
	res = multimaps::erase( m_IPIndex, pc->GetKeyAddress().IP, pc );
	LIVE_ASSERT(res);
	if (pc->IsUDP()/* && UDPPeerConnectionInfo::IsValidSessionKey(pc->GetRemoteSessionKey())*/)
	{
		int erasedCount = m_SessionKeyIndex.erase(pc->GetRemoteSessionKey());
		LIVE_ASSERT( 1 == erasedCount );
		VIEW_DEBUG( "PeerManager::DelPeer session peer " << *pc << " " << pc->GetRemoteSessionKey() );
	}
	m_Connections.erase(itr);
	m_statistics.Degrees.Dec(pc->IsUDP(), pc->IsInitFromRemote(), pc->IsVIP(), m_PeerModule.GetMaxConnectionCount());
	SyncConnectionInfo();
	const FlowMeasure& flow = pc->GetFlow();
	m_ipPool.AddDisconnected(pc->GetKeyAddress(), (UINT)(pc->GetConnectionTime()), 
		errcode, flow.Download.GetAverageRate(), flow.Upload.GetAverageRate(), 
		flow.Download.GetTotalBytes(), flow.Upload.GetTotalBytes());
	//pc->Disconnect(errcode);

	// Added by Tady, 011511: Spark!
	if (pc->GetConnectionInfo().ConnectParam.IsSpark == true)
	{
		STL_FOR_EACH(std::vector<PeerConnection*>, m_sparkConnections, iter)
		{
			if (*iter == pc)
			{
				m_sparkConnections.erase(iter);
				break;
			}
		}
	}

	delete pc;
	this->Validate();
	return true;
}




int CPeerManager::GetPeerCount() const
{
	return (int)m_Connections.size();
}

int CPeerManager::GetLocalPeerCount() const
{
	int count = 0;
	STL_FOR_EACH_CONST( PeerConnectionCollection, m_Connections, itr)
	{
		PeerConnection* pc = *itr;
		if( pc->IsInitFromRemote() == false )
			count ++;
	}
	return count;
}

int CPeerManager::GetRemotePeerCount() const
{
	int count = 0;
	STL_FOR_EACH_CONST( PeerConnectionCollection, m_Connections, itr)
	{
		PeerConnection* pc = *itr;
		if( pc->IsInitFromRemote() == true )
			count ++;
	}
	return count;
}

void CPeerManager::LoadConfiguration()
{
//	m_config.TCPConnectTimeout = ini.GetInteger("param1", PEER_DEFAULT_TCP_CONNECT_TIMEOUT);
//	m_config.P2PConnectTimeout = ini.GetInteger("param2", PEER_DEFAULT_P2P_CONNECT_TIMEOUT);
//	APP_DEBUG("CPeerManager::LoadConfiguration " << make_tuple(m_config.TCPConnectTimeout, m_config.P2PConnectTimeout));
//	LimitMinMax(m_config.TCPConnectTimeout, 1, 120);
//	LimitMinMax(m_config.P2PConnectTimeout, 1, 120);
//	m_config.P2PConnectTimeout *= 1000;

	ini_file ini;
	m_PeerModule.InitIni(ini);
	ini.set_section(_T("download"));
	m_config.Load(ini);

	if (m_config.ConnectorConfig.IsRefuseUDPConnectionRequest && m_PeerModule.IsSource())
	{
		m_PeerModule.GetInfo().TransferMethod = TRANSFER_TCP;
	}

	m_PeerModule.InitConfigFile( ini, _T("live_test.cfg") );
	m_config.ResearchConfig.Load( ini );

	m_StatusInfo->UseFixedQOS = m_config.ResearchConfig.UseFixedUploadSpeed;
	m_StatusInfo->FixedQOS = m_config.ResearchConfig.FixedUploadSpeed;
}


double CPeerManager::GetInitialQuota(bool isVip) const
{
	return isVip ? 1: 0.2;
}

int CPeerManager::GetMaxRepeatedConnectionCount(bool isVip) const
{
	return 1;
}



// Modified by Tady, 011511: Alway return true, so moved into header-file;
// bool CPeerManager::CanDownload(const PeerConnection* conn) const
// {
// //	return conn->IsVIP();
// 		return true;
// }

bool CPeerManager::CanKick(const PeerConnection* conn) const
{
	// 条件是：非vip、已经握手 并且 不是source
	return !conn->IsVIP() && !conn->MaybeSource();
}

bool CPeerManager::NeedResource(const PeerConnection* pc) const
{
	return true;
}

void CPeerManager::SyncConnectionInfo()
{
	m_statistics.ConnectionCount = (int)m_Connections.size();

	const DEGREE_COUNTER& degrees = m_statistics.Degrees;
	m_PeerInformation->StatusInfo->Degrees = degrees.ToSimple();

	LIVE_ASSERT((size_t)(degrees.Left + degrees.All.GetTotal()) == m_PeerModule.GetMaxConnectionCount());
	LIVE_ASSERT(degrees.Left + m_Connections.size() == (size_t)m_PeerModule.GetMaxConnectionCount());
	LIVE_ASSERT(degrees.Left == GetDegreeLeft());
	LIVE_ASSERT(degrees.All.GetTotal() == (int)m_Connections.size());
	VIEW_DEBUG("PeerManager:DegreeInfo " << make_tuple(degrees.Left, degrees.All.In, degrees.All.Out) << m_PeerModule.GetMaxConnectionCount());
}

UINT CPeerManager::GetAnnounceInterval() const
{
	// peer端发送announce的间隔为5秒
	return m_config.AnnounceInterval;
}

/*
void CPeerManager::OnTimerElapsed(Timer* sender)
{
	LIVE_ASSERT(&m_IntervalTimer == sender);
	OnAppTimer(m_IntervalTimer.GetTimes());
}
*/

void CPeerManager::DoAnnounce(PPHuffmanAnnounce* announcePacket)
{
	PeerConnectionCollection::const_iterator iter = m_Connections.begin();
	while (iter != m_Connections.end())
	{
		PeerConnection* pc = *iter;
		++iter; // 先推进迭代器，这样HandlePeer删除conn也没有关系
		pc->Announce(announcePacket);
	}
}

void CPeerManager::DoStatistics()
{
	double qos = 0;
	double skipPercent = 0;
	PeerConnectionCollection::const_iterator iter = m_Connections.begin();
	while (iter != m_Connections.end())
	{
		PeerConnection* pc = *iter;
		++iter; // 先推进迭代器，这样HandlePeer删除conn也没有关系
		skipPercent += pc->GetPeerInfo().StatusInfo.Status.SkipPercent;
		qos += pc->GetPeerInfo().StatusInfo.Status.Qos;
	}
	if (m_Connections.size() > 0)
	{
		skipPercent /= m_Connections.size();
		qos /= m_Connections.size();
	}
	m_statistics.AverageSkipPercent = static_cast<UINT16>(skipPercent);
	m_statistics.AverageQos = static_cast<UINT>(qos);
}

void CPeerManager::OnAppTimer(UINT times)
{
	// 每0.25s 触动一次
	int seconds = times / APP_TIMER_TIMES_PER_SECOND;
	MANAGER_DEBUG("CPeerManager::OnAppTimer " << make_tuple(times, seconds));
	if (times % APP_TIMER_TIMES_PER_SECOND == 0)
	{	// 默认情况 APP_TIMER_TIMES_PER_SECOND==4 ，相当于每秒执行一次
		ListPeers();
		KickConnections(seconds);
		ControllConnections();

		if ( seconds % 5 == 3 )
		{
			// 每60秒钟重新读取一次Uploader配置
			this->LoadConfiguration();
			this->DenyPeers();
			CheckMDS();
		}

		if ( seconds % 10 == 7 )
		{
			TEST_LOG_OUT("peer connections: " << m_Connections.size());
			size_t index = 0;
			STL_FOR_EACH_CONST( PeerConnectionCollection, m_Connections, iter )
			{
				PeerConnection* pc = *iter;
				(void)pc;
                                TEST_LOG_OUT("connection stat: " << index << " " << *pc 
					<< " (download,upload)=" << make_tuple( pc->GetFlow().Download.GetRate(), pc->GetFlow().Upload.GetRate() ) 
					<< " minmax=" << pc->GetRealBitmap().GetMinMax() );
				++index;
			}
			TEST_LOG_FLUSH();
		}
	}
	{
		m_uploader->OnAppTimer(times);
	}
	UINT maxConnectingCount = m_PeerModule.GetInfo().MaxConnectingCount;
	if (maxConnectingCount == 0)
	{
		maxConnectingCount = 30;
	}

	// 每0.25s都会决定发起连接
	InitiateConnections(times, m_PeerModule.GetSysInfo().MaxConnectPendingCount, maxConnectingCount);

	if (times % APP_TIMER_TIMES_PER_SECOND == 0)
	{
		m_LongTimeUploadFlow.Update();
		m_LongTimeDownloadFlow.Update(); 
		// 2006-02-21 cmp StreamBuffer改为只生成资源位图，然后由PeerManager来生成报文
	//	PEER_MINMAX minmax = { sb->GetMinIndex(), sb->GetMaxIndex() };

		UINT minIndex = m_storage.GetMinIndex();
		UINT maxIndex = m_storage.GetMaxIndex();
		LIVE_ASSERT(maxIndex >= minIndex);
		UINT minmaxRange = m_storage.GetBufferSize();
		LIVE_ASSERT(minmaxRange < USHRT_MAX);
		LIMIT_MAX(minmaxRange, USHRT_MAX);

		DoStatistics();

        //	const PEER_STATUS& status = m_StatusInfo->Status;

		UINT announceInterval = GetAnnounceInterval();
		if (seconds % announceInterval == 0)
		{
			BitMap totalResource = m_streamBuffer.BuildTotalBitmap();
			LIVE_ASSERT(totalResource.GetMinIndex() == minIndex);
			m_huffmanAnnouncePacket->Status = m_StatusInfo->GetStatusEx();
			m_huffmanAnnouncePacket->Status.Degrees = m_statistics.Degrees.ToSimple();
			m_huffmanAnnouncePacket->MinIndex = totalResource.GetMinIndex();
			m_huffmanAnnouncePacket->HuffmanTimes = totalResource.GetHuffmanString( m_huffmanAnnouncePacket->ResourceData, *m_HuffmanCoding );
			DoAnnounce(m_huffmanAnnouncePacket.get());
		}


		PeerConnectionCollection::const_iterator iter = m_Connections.begin();
		while (iter != m_Connections.end())
		{
			PeerConnection* pc = *iter;
			++iter; // 先推进迭代器，这样HandlePeer删除conn也没有关系
			pc->OnAppTimer(seconds);
		}

		PEER_MINMAX minmax = { minIndex, maxIndex };
                (void)minmax;
		MANAGER_EVENT("Send Announce To " << m_Connections.size() << " Peer, My Range is " << minmax);

		// 更新流量信息，必须是每秒钟一次
		m_flow.Update();
		m_protocolFlow.Update();
		m_externalDownloadRate.Update();
		m_externalUploadRate.Update();

		m_statistics.UpdateFlow();
		m_statistics.SyncFlow();

		VIEW_INFO("FadeInfoRate " << m_flow.Download.GetRate() << " " << m_flow.Upload.GetRate() << " " 
			<< m_protocolFlow.Download.GetRate() << " " << m_protocolFlow.Upload.GetRate() << " End");
		VIEW_INFO("FadeInfo " << m_flow.Download.GetTotalBytes() << " " << m_flow.Upload.GetTotalBytes() << " " 
			<< m_protocolFlow.Download.GetTotalBytes() << " " << m_protocolFlow.Upload.GetTotalBytes() << " End");


		CalcMaxPFPSBandWidth(seconds);
	}

	// Added by Tady, 081408: For MRP repeat. The step is about 250ms.
	for (PeerConnectionCollection::const_iterator iter = m_Connections.begin(); iter != m_Connections.end(); iter++)
	{
		(*iter)->OnTimerForMRP();
	}

}

int CPeerManager::GetMaxReservedDegree() const
{
	int maxConnectionCount = m_PeerModule.GetMaxConnectionCount();
	// 默认情况下 ReservedDegree = 10
	int maxReservedDegree = maxConnectionCount * m_config.ReservedDegree / 100;
	// 默认情况下 ReservedDegreeMin = 3
	LIMIT_MIN(maxReservedDegree, (int)m_config.ReservedDegreeMin);
	// 默认情况下 ReservedDegreeMax = 10
	LIMIT_MAX(maxReservedDegree, (int)m_config.ReservedDegreeMax);
	return maxReservedDegree;
}

bool CPeerManager::CheckConnected( UINT Ip, unsigned short TcpPort, bool isVip ) const
{
	//int maxCount = GetMaxRepeatedConnectionCount(isVip);
	int maxCount = 1;
	int count = 0;
	STL_FOR_EACH_CONST(PeerConnectionCollection, m_Connections, itr)
	{
		PeerConnection *pc = *itr;
		LIVE_ASSERT(pc != NULL);
		//未连接的也可以查找，需要根据RemoteAddress，而不是握手时汇报过来的地址来进行检查
		//const PEER_ADDRESS& thisAddr = pc->GetHandshakeInfo().OuterAddress;
		const PEER_ADDRESS& thisAddr = pc->GetKeyAddress();
		//LIVE_ASSERT(thisAddr.IsFullyValid());
		//const PEER_ADDRESS& thisAddr = pc->GetPeerInfo()->LocalAddress;
		if ((thisAddr.IP == Ip) && (thisAddr.TcpPort == TcpPort))
		{
			++count;
			if (count >= maxCount)
			{
				VIEW_ERROR("CPeerManager::CheckConnected Yes " << IPAddress(Ip) << " " << TcpPort << " " << isVip);
				return true;
			}
		}
	}
	VIEW_ERROR("CPeerManager::CheckConnected No " << IPAddress(Ip) << " " << TcpPort << " " << isVip);
	return false;
}

void CPeerManager::DoAddPeer(PeerConnection* conn)
{
	LIVE_ASSERT( false == containers::contains( m_Connections, conn ) );
	m_Connections.insert(conn);
	pair<PeerConnectionIndex::iterator, bool> res = m_connectionIndex.insert(make_pair(conn->GetPeerGUID(), conn));
	LIVE_ASSERT(res.second);
	m_AddressIndex.insert( make_pair( conn->GetKeyAddress(), conn ) );
	m_IPIndex.insert( make_pair( conn->GetKeyAddress().IP, conn ) );
	m_statistics.Degrees.Inc(conn->IsUDP(), conn->IsInitFromRemote(), conn->IsVIP(), m_PeerModule.GetMaxConnectionCount());
	SyncConnectionInfo();
	m_downloader->AddTunnel( conn );
	conn->Announce(m_huffmanAnnouncePacket.get());

	LIMIT_MIN( m_statistics.MaxConnectedPeerCount, m_Connections.size() );

	// Added by Tady, 011611: Spark!
	if (conn->GetConnectionInfo().ConnectParam.IsSpark == true)
	{
		m_sparkConnections.push_back(conn);
	}
}

PeerConnection* CPeerManager::AddUDPTPeer(boost::shared_ptr<UDPPeerConnectionInfo> connInfo, const PeerHandshakeInfo& handshakeInfo)
{
	this->Validate();
	LIVE_ASSERT(!HasPeer(handshakeInfo.PeerGUID));
//	double quota = GetInitialQuota(connInfo->ConnectParam.IsVIP);

	boost::shared_ptr<PeerInfoItemProvider> peerInfoItemProvider;
	CPeerInfoItem* peerInfoItem = m_PeerModule.GetInfo().FindUnusedItem();
	if (peerInfoItem == NULL)
	{
		peerInfoItemProvider.reset(PeerInfoItemProviderFactory::CreateDynamic());
	}
	else
	{
		peerInfoItemProvider.reset(PeerInfoItemProviderFactory::CreateStatic(peerInfoItem));
	}
	PeerConnection* conn = NULL;
	LIVE_ASSERT(connInfo->IsBothSessionKeyValid());
	conn = PeerConnectionFactory::CreateUDPSession(*this, peerInfoItemProvider, connInfo, handshakeInfo, m_PeerModule.GetUDPConnectionPacketSender());
	//TRACE( TEXT("AddUDPPeer %p\n"), conn );
	pair<PeerSessionKeyIndex::iterator, bool> res = m_SessionKeyIndex.insert(make_pair(connInfo->RemoteSessionKey, conn));
	VIEW_DEBUG( "PeerManager::AddUDPPeer " << *conn << " " << connInfo->RemoteSessionKey);
	LIVE_ASSERT(res.second);
	DoAddPeer(conn);
	this->Validate();
	return conn;
}


bool CPeerManager::AddPeer(boost::shared_ptr<TCPPeerConnectionInfo> connInfo, const PeerHandshakeInfo& handshakeInfo)
{
	this->Validate();
	LIVE_ASSERT(!HasPeer(handshakeInfo.PeerGUID));
//	double quota = GetInitialQuota(connInfo->ConnectParam.IsVIP);

	boost::shared_ptr<PeerInfoItemProvider> peerInfoItemProvider;
	CPeerInfoItem* peerInfoItem = m_PeerModule.GetInfo().FindUnusedItem();
	if (peerInfoItem == NULL)
	{
		peerInfoItemProvider.reset(PeerInfoItemProviderFactory::CreateDynamic());
	}
	else
	{
		peerInfoItemProvider.reset(PeerInfoItemProviderFactory::CreateStatic(peerInfoItem));
	}
	PeerConnection* conn = PeerConnectionFactory::CreateTCP(*this, peerInfoItemProvider, connInfo, handshakeInfo, m_PeerModule.GetTCPConnectionPacketSender());
	//TRACE( TEXT("AddPeer %p\n"), conn );
	DoAddPeer(conn);
	this->Validate();
	return true;
}

size_t CPeerManager::FillPeerAddresses(std::vector<PeerExchangeItem>& peers, size_t maxCount, UINT targetIP)
{
//	m_PeerInformation->NetInfo->PublicHosts.clear();

	VIEW_EVENT( "PeerManager::ExchangePeers " << IPAddress( targetIP ) );
	bool isTargetInner = IsPrivateIP( targetIP );
	STL_FOR_EACH_CONST(PeerConnectionCollection, m_Connections, iter)
	{
		if ( peers.size() >= maxCount )
			break;

		PeerConnection* pc = *iter;

		// vip peer略去
		if (pc->IsVIP())
			continue;

		const PeerHandshakeInfo& handshakeInfo = pc->GetHandshakeInfo();
		PEER_ADDRESS peerAddress;
		// LAN，IP是内网IP
		// 连接地址与IP相同
		if ( 
			pc->GetSocketAddress().IP == targetIP || 
			( pc->IsLANConnection() && isTargetInner )
			)
		{
			peerAddress = handshakeInfo.Address;
		}	
		else if ( handshakeInfo.CoreInfo.PeerNetType == PNT_UPNP || handshakeInfo.CoreInfo.PeerNetType == PNT_OUTER )
		{
			peerAddress = handshakeInfo.OuterAddress;
		}



/*		if ( false == pc->IsInnerIP() )
		{
			// 公网peer，返回peer地址
			peerAddress = handshakeInfo.Address;
			LIVE_ASSERT( false == IsPrivateIP( peerAddress.IP ) );
		}
		else if ( pc->IsUPNP() )
		{
			// 启用upnp的peer，返回OuterAddress;
			peerAddress = handshakeInfo.OuterAddress;
			LIVE_ASSERT( false == IsPrivateIP( peerAddress.IP ) );
		}
		else
		{
			// 内网peer
			if ( pc->IsLANConnection() )
			{
				// 局域网连接的peer
				if ( isTargetInner )
				{
					// target和peer都和我在同一局域网
					peerAddress = handshakeInfo.Address;
					LIVE_ASSERT( IsPrivateIP( peerAddress.IP ) );
				}
				else
				{
					continue;
				}
			}
			else
			{
				// 非局域网连接的peer
				if ( pc->GetSocketAddress().IP == targetIP )
				{
					// target和peer在同一内网，但不是我所在的局域网
					peerAddress = handshakeInfo.Address;
					//continue;
				}
				else if ( false == pc->IsInitFromRemote() )
				{
					// 本地发起的连接，并且不是局域网连接，返回KeyAddress
					//? 可能是遗漏的upnp节点，或者是其它一些异常的情况？
					peerAddress = pc->GetKeyAddress();
					LIVE_ASSERT( false == IsPrivateIP( peerAddress.IP ) );
				}
				else
				{
					continue;
				}
			}
		}*/
		if ( false == peerAddress.IsValid() )
		{
			continue;
		}
		//LIVE_ASSERT( CheckIPValid( peerAddress.IP ) );


		PeerExchangeItem info;
		info.PeerType = handshakeInfo.CoreInfo.PeerType;
		info.Address = peerAddress;
		peers.push_back(info);

		//if ( PNT_INNER != this->m_PeerInformation->NetInfo->CoreInfo.PeerNetType 
		//	 && this->m_PeerInformation->NetInfo->PublicHosts.size() < 3 )
		//{
		//	if ( handshakeInfo.CoreInfo.PeerNetType == PNT_UPNP || handshakeInfo.CoreInfo.PeerNetType == PNT_OUTER )
		//	{
		//		this->m_PeerInformation->NetInfo->PublicHosts.push_back( handshakeInfo.OuterAddress );
		//	}
		//}

		VIEW_EVENT( "PeerManager::ExchangePeers do " << info.Address << " " << *pc );
	}
	return peers.size();
}

bool CPeerManager::HandleUDPPacket( data_input_stream& is, const NEW_UDP_PACKET_HEAD& head, const PACKET_PEER_INFO& packetPeerInfo, const SimpleSocketAddress& sockAddr, ExtraProxyEnum extraProxy )
{
	return m_connector->GetUDPConnector()->HandlePacket( is, head, packetPeerInfo, sockAddr, extraProxy );
}

bool CPeerManager::HandleUDPSessionPacket(data_input_stream& is, const NEW_UDP_PACKET_HEAD& head, const UDP_SESSION_INFO& sessionInfo, const SimpleSocketAddress& sockAddr)
{
	// 先检查已经连接的peer中有没有地址为sockAddr的
	PeerSessionKeyIndex::const_iterator iter = m_SessionKeyIndex.find(sessionInfo.SessionKey);
	if (iter != m_SessionKeyIndex.end())
	{
		PeerConnection* pc = iter->second;
		if (pc->IsUDP())
		{
			pc->HandleUDPSessionPacket(is, head, sessionInfo, sockAddr);
			return true;
		}
	}
	return m_connector->GetUDPConnector()->HandleSessionPacket(is, head, sessionInfo, sockAddr);
}

inline bool AddRedirectPeerInfo( std::vector<REDIRECT_PEER_INFO>& peers, size_t maxCount, const PeerConnection* pc )
{
	if ( peers.size() > maxCount )
		return false;
	REDIRECT_PEER_INFO peer;
	peer.Address = pc->GetHandshakeInfo().Address;
	peer.OuterAddress = pc->GetHandshakeInfo().OuterAddress;
	if ( 0 == peer.OuterAddress.IP )
	{
		peer.OuterAddress = pc->GetKeyAddress();
		LIVE_ASSERT( false );
	}
//	LIVE_ASSERT( peer.OuterAddress.IsAddressValid() );
	peer.AppVersion = pc->GetHandshakeInfo().AppVersion;
	peer.ConnectionType = pc->IsUDP();
	peer.DegreeLeft = pc->GetDegrees().Left;
	peer.PeerGUID = pc->GetPeerGUID();
	peer.PeerType = static_cast<PeerTypeEnum>( pc->GetHandshakeInfo().CoreInfo.PeerType );
	peer.IsEmbedded = pc->GetHandshakeInfo().IsEmbedded;
	peer.PeerNetType = static_cast<PeerNetTypeEnum>( pc->GetHandshakeInfo().CoreInfo.PeerNetType );
	//peer.DetectedAddress.IP = pc->GetSocketAddress().GetIP();
	//peer.DetectedAddress.Port = pc->GetSocketAddress().GetPort();
	//peer.DetectedAddress.Clear();
	//peer.Status = pc->GetPeerInfo().StatusInfo.Status;
	LIVE_ASSERT( peer.Address.IsFullyValid() );
//	LIVE_ASSERT( peer.OuterAddress.IsAddressValid() );
	LIVE_ASSERT( CheckIPValid( peer.Address.IP ) );
//	LIVE_ASSERT( CheckIPValid( peer.OuterAddress.IP ) );
	//LIVE_ASSERT(peer.DetectedAddress.IP != 0);
	//LIVE_ASSERT(peer.DetectedAddress.Port != 0);
	peers.push_back( peer );
	VIEW_EVENT( "PeerManager::Redirect do " << peer.Address << " " << peer.OuterAddress << " " << *pc );
	return true;
}

size_t CPeerManager::GetBestPeersForRedirect(std::vector<REDIRECT_PEER_INFO>& peers, size_t maxCount, UINT targetIP) const
{
	VIEW_EVENT( "PeerManager::Redirect " << IPAddress( targetIP ) );
	peers.clear();
	peers.reserve(3);

//	bool isTargetInner = IsPrivateIP( targetIP );
	typedef std::multimap<int, PeerConnection*, std::greater<int> > DegreeLeftIndexedPeerCollection;
	DegreeLeftIndexedPeerCollection coll;
	STL_FOR_EACH_CONST(PeerConnectionCollection, m_Connections, iter)
	{
		PeerConnection* pc = *iter;
		const DEGREE_INFO& degrees = pc->GetDegrees();
		const PEER_STATUS& status = pc->GetPeerInfo().StatusInfo.Status;
		if ( pc->IsVIP() 
			|| status.OutDegree <= 0 
			|| degrees.Left <= 0 
			|| degrees.All.Out + degrees.All.In <= 30 
			|| status.Qos <= 10 )
		{
			// 过滤掉不合适的节点
			continue;
		}

		UINT8 peerType = pc->GetHandshakeInfo().CoreInfo.PeerType;
		if ( MAS_PEER == peerType || MDS_PEER == peerType )
		{
			coll.insert( make_pair( status.DegreeLeft, pc ) );
		}
		else
		{
			if ( 
				( pc->IsLANConnection() && IsPrivateIP(targetIP) ) ||
				  pc->GetSocketAddress().IP == targetIP
				)
			{
				// 同一内网，直接添加
				AddRedirectPeerInfo( peers, maxCount, pc );
			}	
			else if ( pc->GetHandshakeInfo().CoreInfo.PeerNetType == PNT_UPNP || pc->GetHandshakeInfo().CoreInfo.PeerNetType == PNT_OUTER )
			{
				coll.insert( make_pair( degrees.Left, pc ) );
			}
			// 其它情况下的内网peer不会被exchange
		}
	}
	size_t preCount = peers.size();
	// 再从剩下的里面选择剩余度最大的3个
	STL_FOR_EACH_CONST(DegreeLeftIndexedPeerCollection, coll, iterPC)
	{
		PeerConnection* pc = iterPC->second;
		if ( peers.size() > preCount + 3 )
			break;
		AddRedirectPeerInfo( peers, maxCount, pc );
	}
	return peers.size();
}


bool CPeerManager::IsVIP(UINT ip) const
{
	return m_vips.find(ip) != m_vips.end();
}

void CPeerManager::LoadVIP(const PeerAddressCollection& vips)
{
	m_vips.clear();
	STL_FOR_EACH_CONST(PeerAddressCollection, vips, iter)
	{
		const PEER_ADDRESS& addr = *iter;
		m_vips.insert(addr.IP);
		APP_EVENT("AddVIP " << addr);
	}
}

bool CPeerManager::ConnectToRemote(const PEER_ADDRESS& addr, const PeerConnectParam& param)
{
	if ( this->m_config.NoConnectOut )
		return false;
	return m_connector->Connect(addr, param);
}

void CPeerManager::DialMDS()
{
	DialPeers(m_PeerModule.GetMDSPeers());
}

void CPeerManager::DialSpark(const PEER_ADDRESS& peerAddr, int connectFlags = IPPOOL_CONNECT_NONE)
{
	VIEW_EVENT("DialPeer " << peerAddr);
	PeerItem peer;
	peer.Init();
	peer.CanDetect = true;
	peer.Info.Address = peerAddr;
	PeerConnectParam param(peer, false, m_config.ConnectorConfig.LongConnectionTimeout, false, true);
	param.ConnectFlags = connectFlags;
	ConnectToRemote(peerAddr, param);
}

void CPeerManager::DialSparks()
{
	int connectFlags = IPPOOL_CONNECT_NONE;
	if (m_PeerModule.GetAppCreateParam().SparkTranstype == TEXT("tcp"))
	{
		connectFlags = IPPOOL_CONNECT_NORMAL;
	}

	STL_FOR_EACH_CONST(PeerAddressArray, m_PeerModule.GetAppCreateParam().Sparks, iter)
	{
		DialSpark(*iter, connectFlags);
	}
}

void CPeerManager::DialPeer(const PEER_ADDRESS& peerAddr)
{
	VIEW_EVENT("DialPeer " << peerAddr);
	PeerItem peer;
	peer.Init();
	peer.CanDetect = true;
	peer.Info.Address = peerAddr;
	PeerConnectParam param(peer, false, m_config.ConnectorConfig.LongConnectionTimeout, true);
	ConnectToRemote(peerAddr, param);
}

void CPeerManager::DialPeers(const PeerAddressCollection& peers)
{
	STL_FOR_EACH_CONST(PeerAddressCollection, peers, iter)
	{
		DialPeer(*iter);
	}
}


void CPeerManager::RecordUploadFlow(size_t size, UINT8 action, PeerConnection* pc)
{
	m_LongTimeUploadFlow.Record((UINT)size);
	RecordFlow(size, action, pc, m_flow.Upload, m_protocolFlow.Upload, m_externalUploadRate, m_PeerModule.GetInfo().LocalPeerInfo.Flow.Upload);
/*	if (!pc->IsUDPT())
	{
		// tcp的peer上传
		m_PeerModule.RecordUpload(size);
	}*/

	if ( PPT_SUB_PIECE_DATA == action )
	{
		// uploader上传数据
		m_statistics.UploaderData.Flow.Upload.Record( size );
	}
	else if ( PPT_SUB_PIECE_REQUEST == action )
	{
		// downloader发送请求
		m_statistics.DownloaderData.Flow.Upload.Record( size );
	}
	else if ( PPT_HUFFMAN_ANNOUNCE == action )
	{
		// announce流量
		m_statistics.AnnounceFlow.Upload.Record( size );
	}

	UINT32 qos = m_flow.Upload.GetRate() / 1000;
	m_PeerModule.GetInfo().LocalPeerInfo.StatusInfo.Status.Qos = (UINT16)min(qos, static_cast<UINT32>(USHRT_MAX));
}

void CPeerManager::RecordDownloadFlow(size_t size, UINT8 action, PeerConnection* pc)
{
	m_LongTimeDownloadFlow.Record((UINT)size);
	RecordFlow(size, action, pc, m_flow.Download, m_protocolFlow.Download, m_externalDownloadRate, m_PeerModule.GetInfo().LocalPeerInfo.Flow.Download);
	if (!pc->IsUDP())
	{
		// tcp的peer下载
		m_PeerModule.RecordDownload(size);
	}

	if ( PPT_SUB_PIECE_DATA == action )
	{
		// downloader收到数据
		m_statistics.DownloaderData.Flow.Download.Record( size );
	}
	else if ( PPT_SUB_PIECE_REQUEST == action )
	{

		// uploader收到请求
		m_statistics.UploaderData.Flow.Download.Record( size );
	}
	else if ( PPT_HUFFMAN_ANNOUNCE == action )
	{
		// announce流量
		m_statistics.AnnounceFlow.Download.Record( size );
	}
}

void CPeerManager::RecordFlow(size_t size, UINT8 action, const PeerConnection* pc, RateMeasure& flow, RateMeasure& protocolFlow, HistoricalRateMeasure& externalFlow, FLOW_INFO& flowInfo)
{
	flow.Record((UINT)size);
	// 将流量信息保存到共享内存中
	SaveFlowInfo(flowInfo, flow);
	if ( action != PPT_SUB_PIECE_DATA )
	{
		// 如果是非媒体报文，则计入下载的协议消耗
		protocolFlow.Record((UINT)size);
	}
	if (!pc->IsLANConnection())
	{
		externalFlow.Record(size);
	}
}

bool CPeerManager::DelPeer( PeerConnection* conn, UINT16 errcode, const serializable* errorInfo )
{
	conn->Disconnect( errcode, errorInfo );
	return this->DoDelPeer( conn, errcode );
}

void CPeerManager::KickPeer( PeerConnection* pc, UINT32 reason )
{
	MANAGER_ERROR("KickPeer for reason " << reason << " " << *pc);
//	CORE_SendObjectMessage(0, this, UM_ADD_BAN_IP, pc->GetPeerInfo().DetectedIP, 0, 0, 0);
	m_statistics.SentKickReasons[reason]++;
	PPErrorKick packet( 0, reason );
	DelPeer(pc, PP_LEAVE_KICKED, &packet );
}

int CPeerManager::GetMaxLocalPeerCount() const
{
	return 20;
}

int CPeerManager::GetStableConnectionCount() const
{
	int count = m_PeerModule.GetMaxConnectionCount() * 80 / 100;
	LIMIT_MIN_MAX( count, 40, 60 );
	return count;
}

void CPeerManager::DenyPeers()
{
	STL_FOR_EACH_CONST( PeerAddressCollection, m_config.ResearchConfig.DeniedPeers, iter )
	{
		this->DenyPeer( *iter );
	}
	STL_FOR_EACH_CONST( std::set<UINT32>, m_config.ResearchConfig.DeniedIPs, iterIP )
	{
		this->DenyPeer( *iterIP );
	}
}

size_t CPeerManager::DenyPeer( const PEER_ADDRESS& addr )
{
	size_t kickedCount = 0;
	for ( ;; )
	{
		PeerConnectionAddressIndex::iterator iter = m_AddressIndex.find( addr );
		if ( iter == m_AddressIndex.end() )
			break;
		this->KickPeer( iter->second, PP_KICK_DENY );
		++kickedCount;
	}
	VIEW_EVENT("DenyPeer by PeerAddress " << addr << " " << kickedCount);
	return kickedCount;
}

size_t CPeerManager::DenyPeer( UINT32 ip )
{
	std::pair<PeerConnectionIPIndex::iterator, PeerConnectionIPIndex::iterator> range = m_IPIndex.equal_range( ip );
	size_t kickedCount = 0;
	PeerConnectionIPIndex::iterator iter = range.first;
	while ( iter != range.second )
	{
		PeerConnection* pc = iter->second;
		// 注意，必须先推进迭代器，再删除pc，因为删除pc会让iter失效
		++iter;
		this->KickPeer( pc, PP_KICK_DENY );
		++kickedCount;
	}
	VIEW_EVENT("DenyPeer by ip " << IPAddress(ip) << " " << kickedCount);
	return kickedCount;	
}

void CPeerManager::ControllConnections()
{
	if ( m_LastConnectionControllTime.elapsed32() < m_config.ResearchConfig.ConnectionControllInterval * 1000 )
		return;
	m_LastConnectionControllTime.sync();
	if ( false == m_PeerInformation->IsNormalPeer() )
		return;

	tstring filename = ppl::os::paths::combine( m_PeerModule.GetAppCreateParam().ConfigDirectory, _T("live_testconn.cfg") );
	ppl::io::stdfile_reader fin;
	if ( false == fin.open_text( filename.c_str() ) )
		return;
	const size_t max_line_count = 512;
	size_t lineCount = 0;
	string line;

	while ( fin.read_line( line ) )
	{
		++lineCount;
		if ( lineCount > max_line_count )
			break;
		boost::trim_right_if(line, boost::is_any_of("\r\n"));
		if ( line.empty() )
			continue;
		std::pair<string, string> parts = strings::split_pair( line, ' ' );
		const string& ipstr = parts.first;
		const string& op = parts.second;
		if ( ipstr.empty() || op.empty() )
			break;
		if ( "connect" == op )
		{
			PEER_ADDRESS addr;
			if ( false == PeerAddressUtil::ParseAddress( addr, ipstr ) )
				return;
			DialPeer( addr );
		}
		else if ( "disconnect" == op )
		{
			IPAddress ipaddr( ipstr.c_str() );
			if ( ipaddr.IsNone() || ipaddr.IsBroadcast() || ipaddr.IsAny() )
			{
				// 非法ip，退出
				return;
			}
			DenyPeer( ipaddr.GetAddress() );
		}
		else
		{
			// 非法行，退出
			return;
		}
	}
	fin.close();
	// close then delete
	ppl::os::file_system::remove_file( filename.c_str() );
}

bool CPeerManager::SaveLog(JsonWriter& writer, UINT32 detectedIP, UINT32& totalPeercount, UINT32& savedPeercount) const
{
	const int max_ip_count = 100;
	UINT totalUp(0), totalDown(0), rTotalUp(0), rTotalDown(0);

	STL_FOR_EACH_CONST(PeerConnectionCollection , m_Connections, iter)
	{
		PeerConnection* pc = *iter;
		const FlowMeasure& flow = pc->GetFlow();
		// 转换为KB
		UINT downloadBytes = static_cast<UINT>( flow.Download.GetTotalBytes() );
		UINT uploadBytes = static_cast<UINT>( flow.Upload.GetTotalBytes() );
		totalDown += downloadBytes;
		totalUp   += uploadBytes;
		downloadBytes /= 1024;
		uploadBytes /= 1024;
		if ( downloadBytes > 0 || uploadBytes > 0 )
		{
			// 忽略小于1K的
			if (totalPeercount <(size_t) max_ip_count)
			{
				if ( savedPeercount > 0 )
				{
					writer.WriteComma();
				}

				UINT32 ip = ( pc->IsLANConnection() ) ? detectedIP : pc->GetPeerInfo().DetectedIP;
				{
					JsonWriter::ObjectWriterEntry objectEntry(writer.GetStream());
					writer.WriteJsonString("IP", AnsiFormatIPAddress(ip));
					writer.WriteComma();
					writer.WriteJsonNumber("Down", numeric<UINT>::format(uploadBytes));
					writer.WriteComma();
					writer.WriteJsonNumber("Up", numeric<UINT>::format(downloadBytes));

					rTotalUp += uploadBytes;
					rTotalDown += downloadBytes;
				}
				++savedPeercount;
			}
			++totalPeercount;
		}
	}

	return true;
}
