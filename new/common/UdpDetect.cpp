
#include "StdAfx.h"

#include "UdpDetect.h"
#include "BaseInfo.h"
#include "PacketSender.h"
#include "GloalTypes.h"
#include "IpPool.h"
#include "PeerManager.h"
#include "StreamBufferStatistics.h"

#include <synacast/protocol/PeerPacket.h>
#include <synacast/protocol/PacketHead.h>
#include <synacast/protocol/data/SimpleSocketAddress.h>
#include "util/testlog.h"

#include "AppModule.h"



//const UINT DETECT_TIMER_INTERVAL		= 1000;	//̽����
//const UINT DETECT_IP_COUNT				= 50;	//ÿ��̽����� 

/// ����̽�����
//const int MAX_DETECT_COUNT = 25;
/// һ����Ҫ��̽�����
//const int NORMAL_DETECT_COUNT = 10;


CUdpDetect::CUdpDetect(CIPPool& ipPool, PeerManager& peerManager, boost::shared_ptr<PeerInformation> peerInformation, boost::shared_ptr<UDPConnectionlessPacketSender> sender, const CLiveInfo& liveInfo) :
	m_PeerManager(peerManager), 
	m_LiveInfo(liveInfo), 
	m_ipPool(ipPool), 
	m_NetInfo(peerInformation->NetInfo), 
	m_StatusInfo(peerInformation->StatusInfo), 
	m_PacketSender(sender), 
	m_PeerInformation(peerInformation),
	m_RefuseDetect(false), 
	m_SentPeerExchangePacket( new PeerExchangePacket ), 
	m_ExchangedPeerTime(0)
	//m_OnceDetectCount(MAX_DETECT_COUNT)
{
	UDPDETECT_DEBUG("New CUdpDetect");
	m_needDetect = false;

	ini_file ini;
	m_PeerManager.GetAppModule().InitIni( ini );
	ini.set_section( _T("detector") );
	m_Config.Load( ini );
}

CUdpDetect::~CUdpDetect()
{
	UDPDETECT_DEBUG("Delete CUdpDetect");
}

void CUdpDetect::HandleDetect( data_input_stream& is, const NEW_UDP_PACKET_HEAD& head, const PACKET_PEER_INFO& packetPeerInfo, const SimpleSocketAddress& sockAddr )
{
	PPDetect parser;
	if (!parser.Read(is))
	{
		m_Statistics.Detect.Packets.Request.InvalidCount++;
		return;
	}

	m_Statistics.Detect.Packets.Request.ReceivedCount++;
	m_Statistics.DetectFlow.Download.Record( is.total_size() );

	if ( m_RefuseDetect )
	{
		return ;
	}

	//����Ƿ����Լ������Լ���
	if (packetPeerInfo.PeerGUID == m_PeerInformation->PeerGUID)
	{
		UDPDETECT_EVENT("Recv a self-self Hello Packet");
		return;
	}

	VIEW_INFO("UDPReceiveDetect " << is.total_size() << " " << sockAddr);

	VIEW_INFO( "HandleDetect " << sockAddr << " " << parser.KeyAddress << " " << parser.SendOffTime );

	// �յ�Detect���ĺ� ����Redetest����
	PPRedetect resp;
	resp.KeyAddress = parser.KeyAddress;
	resp.SendOffTime = parser.SendOffTime;
	size_t packetSize = m_PacketSender->Send(resp, head.TransactionID, sockAddr);

	// ����ͳ����Ϣ
	if ( packetSize > 0 )
	{
		m_Statistics.DetectFlow.Upload.Record( packetSize );
		m_Statistics.Detect.Packets.Response.SentCount++;
	}
	else
	{
		m_Statistics.Detect.Packets.Response.SendFailedCount++;
	}
}

void CUdpDetect::HandleRedetect( data_input_stream& is, const NEW_UDP_PACKET_HEAD& head, const PACKET_PEER_INFO& packetPeerInfo, const SimpleSocketAddress& sockAddr )
{
	PPDetect parser;
	if (!parser.Read(is))
	{
		m_Statistics.Detect.Packets.Response.InvalidCount++;
		return;
	}

	m_Statistics.Detect.Packets.Response.ReceivedCount++;
	m_Statistics.DetectFlow.Download.Record( is.total_size() );

	//����Ƿ����Լ������Լ���
	if (packetPeerInfo.PeerGUID == m_PeerInformation->PeerGUID)
	{
		UDPDETECT_EVENT("Recv a self-self Hello Packet");
		return;
	}

	VIEW_INFO("UDPReceiveRedetect " << is.total_size() << " " << sockAddr);
	//LIVE_ASSERT( parser->KeyAddress.ToUDPAddress() == sockAddr );
	CANDIDATE_PEER_INFO peerInfo;
	peerInfo.Address = parser.KeyAddress;
	peerInfo.CoreInfo = packetPeerInfo.CoreInfo;
	UINT rtt = time_counter::get_system_count32() - parser.SendOffTime;

	VIEW_INFO( "HandleRedetect " << sockAddr << " " << parser.KeyAddress << " " << parser.SendOffTime << " " << rtt );
	m_ipPool.AddDetected(peerInfo, rtt, true);
}

void CUdpDetect::DoDetect()
{	
	// �˺�������Timer�����ģ�ÿ��0.25��ͻᱻ����
	//return;
	//LIVE_ASSERT(m_OnceDetectCount >= NORMAL_DETECT_COUNT && m_OnceDetectCount <= MAX_DETECT_COUNT);
	
	if (m_LiveInfo.TransferMethod == TRANSFER_NO_DETECT)
	{
		return;
	}

	
	int	OnceHelloCount = 0, RealOnceHelloCount = 0;		// Once һ��Ӧ�÷�����PeerExchange�������� RealOnce��ʾһ��ʵ�ʷ�����PeerExchange������
	int OnceDetectCount = 0, RealOnceDetectCount = 0;	// Once һ��Ӧ�÷�����Detect�������� RealOnce��ʾһ��ʵ�ʷ�����Detect������

	UINT bufferTime = m_StatusInfo->BufferTime;		// ����Լ��Ĳ��Ż���ʱ��
	int IpPoolMaxCount = 1100;	
	// IpPoolMaxCount ��ʾIPPool�ĸ������ֵ��
	//		�� IPPool��ʵ�ʸ��� > IpPoolMaxCount ��ʱ������ֹͣ�鷶
	//		�� IPPool��ʵ�ʸ��� < IpPoolMaxCount ��ʱ�����Ǵ򿪺鷶
	// ����ϣ�����Ƴ� �鷶�Ĵ������ǽ�����һ����ѧģ��
	//		���ں���û���ܺõ�ʱ������ ���Ӻ鷶
	//		���ں��� �ܺ� �� �ȶ� ��ʱ�� ���Ǽ��ٺ鷶
	// ������ѧģ�ͼ����������
	if( bufferTime < 20*1000 )
	{
		IpPoolMaxCount = 1100;
	}
	else if( bufferTime > 70*1000 )
	{
		IpPoolMaxCount = 100;
	}
	else
	{
		//   bufferTime - 20*1000        1100 - IpPoolMaxCount
		// ------------------------- = -------------------------  ==>
		//    70*1000  - 20*1000             1100 - 100 
		//
		// 1000*(bufferTime - 30*1000) = 50*1000*(1100 - IpPoolMaxCount)  ==> 
		// 
		//   bufferTime - 20*1000
		// ------------------------- = 1100 - IpPoolMaxCount ==>
		//           50
		//                            bufferTime
		// IpPoolMaxCount = 1100 - ------------------ + 400
		//                               50
		//
		//                            bufferTime
		// IpPoolMaxCount = 1500 - ------------------
		//                               50
		IpPoolMaxCount = 1500 - bufferTime / 50;
		LIVE_ASSERT( IpPoolMaxCount >= 100 );
		LIVE_ASSERT( IpPoolMaxCount <= 1100 );
	}

	IPPOOL_INFO( "IpPoolMaxCount "<<IpPoolMaxCount );

	if( (int)m_ipPool.GetSize() > IpPoolMaxCount )
	{	// ��IpPool��ʵ�ʸ��� > IpPoolMaxCount ���ǲ��鷶��
		OnceHelloCount = 0;
//		OnceDetectCount = 0;
	}
	else
	{	
		OnceHelloCount = (IpPoolMaxCount - (int)m_ipPool.GetSize()) / 100; 
//		LIMIT_MIN_MAX( OnceHelloCount, 0, 10);
		LIMIT_MIN_MAX( OnceHelloCount, 1, 10);
// 		OnceDetectCount = (IpPoolMaxCount - (int)m_ipPool.GetSize()) / 100; 
// 		LIMIT_MIN_MAX( OnceDetectCount, 3, 10);
	}

	if( m_ipPool.GetSize() <= 1100 )
	{	// ���IPPool�ĸ���С��1000,���ۻ���ʱ����ô�����������ٱ���1��
		//! why? ������ ���ǵ�û�к鷶Ҳ���� ���� ֻҪС��1100�ĺ� ������������1���鷶
		LIMIT_MIN( OnceHelloCount, 1);
//		LIMIT_MIN( OnceDetectCount, 3);
	}

 	// ʵ�ʷ���鷶�����Ƕ�ÿ��CandidatePeer��˵��ͬһ��Peer�鷶������һ��ʱ��
 	{
 		PeerItem peer;
 		for( int i = 0; i < OnceHelloCount; i ++) 
 		{
 			if (!m_ipPool.GetForHello(peer))
 				break;
 			const PEER_ADDRESS& addr = peer.Info.Address;
 			if ( addr.UdpPort != 0 )
 			{
 				DoDetectPeer(peer);		//! why? ����鷶��ʱ��һ��ͬʱ����һ��̽��
 				RealOnceDetectCount ++;
 				this->ExchangePeers( addr.ToUDPAddress(), false, time_counter::get_system_count32(), 0 );
 				RealOnceHelloCount ++;
 				m_Statistics.PeerExchange.Packets.Request.SentCount++;
 			}
 		}
 	}

	// ����̽��Ĳ��� ����֤ÿ�����۷���3��̽�⡿

	OnceDetectCount = 3 - RealOnceHelloCount;

	// ʵ�ʷ���鷶�����Ƕ�ÿ��CandidatePeer��˵��ͬһ��Peer ̽�������һ��ʱ��
	{
		PeerItem peer;
		for (int i = 0; i < OnceDetectCount; i++)
		{
			if (!m_ipPool.GetForDetect(peer))
				break;
			const PEER_ADDRESS& addr = peer.Info.Address;
			if ( addr.UdpPort != 0 )
			{
				DoDetectPeer(peer);
				RealOnceDetectCount ++;
			}
		}
	}

	m_Statistics.OnceHelloCount = OnceHelloCount;
	m_Statistics.RealOnceDetectCount = RealOnceDetectCount;
	m_Statistics.RealOnceHelloCount = RealOnceHelloCount;
}


bool CUdpDetect::DoDetectPeer(const PeerItem& peer)
{
	const PEER_ADDRESS& addr = peer.Info.Address;
	UDPDETECT_EVENT("Send Detect Packet to " << addr);
	if ( m_PeerManager.ConnectForDetect(peer) )
		return true;
	PPDetect packet;
	packet.KeyAddress = addr;
	VIEW_INFO("UDPSendDetect " << addr);
	size_t packetSize = m_PacketSender->Send(packet, 0, addr.ToUDPAddress());
	if ( packetSize > 0 )
	{
		UDPT_INFO("SendUDPPacket For Detect "<<addr);
		//m_statistics.DetectTimes++;
		m_Statistics.Detect.Packets.Request.SentCount++;
		m_Statistics.DetectFlow.Upload.Record( packetSize );
	}
	else
	{
		m_Statistics.Detect.Packets.Request.SendFailedCount++;
	}
	return true;
}

void CUdpDetect::Start(bool doDetect)
{
	m_needDetect = doDetect;
}


void CUdpDetect::OnAppTimer(UINT times)
{
	if ( times % APP_TIMER_TIMES_PER_SECOND == 0 )
	{
		m_Statistics.UpdateFlow();
		m_Statistics.SyncFlow();
	}

	if (!m_needDetect)
		return;
	DoDetect();
}

bool CUdpDetect::HandleUDPPacket( data_input_stream& is, const NEW_UDP_PACKET_HEAD& head, const PACKET_PEER_INFO& packetPeerInfo, const SimpleSocketAddress& sockAddr )
{
	LIVE_ASSERT(packetPeerInfo.ChannelGUID == m_PeerInformation->ChannelGUID);
	switch (head.Action)
	{
	case PPT_PEER_EXCHANGE:
		this->HandlePeerExchange( is, head, packetPeerInfo, sockAddr );
		break;
	case PPT_DETECT:
		this->HandleDetect( is, head, packetPeerInfo, sockAddr );
		break;
	case PPT_REDETECT:
		this->HandleRedetect( is, head, packetPeerInfo, sockAddr );
		break;
	default:
		return false;
	}
	return true;
}

void CUdpDetect::SetRefuseDetect( bool refuse )
{
	m_RefuseDetect = refuse;	
}

bool CUdpDetect::GetRefuseDetect()
{
	return m_RefuseDetect;
}

bool CUdpDetect::ExchangePeers( const SimpleSocketAddress& sockAddr, bool isResponse, UINT32 sendOffTime, UINT32 transactionID )
{
	VIEW_INFO("ExchangePeers " << sockAddr << " request type is: " << isResponse);
	PeerExchangePacket& packet = *m_SentPeerExchangePacket;
	packet.SendOffTime = sendOffTime;
	packet.IsResponse = isResponse;
	const UINT max_udp_echo_peer_count = 60; // ����echo���ĳ���1K�ֽڣ��Ҳ��ܳ���PeerExchangePacket���������63��
	if ( m_ExchangedPeerTime.elapsed32() > 100 )
	{
		// peer��ַ�б���100����֮�����
		size_t count = m_PeerManager.FillPeerAddresses( packet.Peers, max_udp_echo_peer_count, sockAddr.IP );
		LIVE_ASSERT( count == packet.Peers.size() );
		m_ExchangedPeerTime.sync();
	}
	size_t packetSize = m_PacketSender->Send( packet, transactionID, sockAddr );

	SINGLE_REQUEST_COUNTER& counter = isResponse ? m_Statistics.PeerExchange.Packets.Response : m_Statistics.PeerExchange.Packets.Request;

	if ( packetSize > 0 )
	{
		counter.SentCount++;
		m_Statistics.PeerExchangeFlow.Upload.Record( packetSize );
	}
	else
	{
		counter.SendFailedCount++;
	}
	m_Statistics.LastSentExchangedPeerCount = static_cast<UINT16>( packet.Peers.size() );
	m_Statistics.TotalSentExchangedPeerCount += packet.Peers.size();

	return true;
}

void CUdpDetect::HandlePeerExchange(  data_input_stream& is, const NEW_UDP_PACKET_HEAD& head, const PACKET_PEER_INFO& packetPeerInfo, const SimpleSocketAddress& sockAddr  )
{
	PeerExchangePacket packet;
	if ( false == packet.Read( is ) )
	{
		m_Statistics.PeerExchange.Packets.Request.InvalidCount++;
		return;
	}

	
	if ( 0 == packet.IsResponse )
	{	// ���� ��������� PeerExchange
		m_Statistics.PeerExchange.Packets.Request.ReceivedCount++;
	}
	else
	{	// ���� ���ڻ�Ӧ�� PeerExchange
		m_Statistics.PeerExchange.Packets.Response.ReceivedCount++;
	}
	m_Statistics.PeerExchangeFlow.Download.Record( is.total_size() );
	m_Statistics.LastReceivedExchangedPeerCount = static_cast<UINT16>( packet.Peers.size() );
	m_Statistics.TotalReceivedExchangedPeerCount += packet.Peers.size();

	//LIVE_ASSERT( sockAddr.IP == packetPeerInfo.OuterAddress.IP || sockAddr.IP == packetPeerInfo.Address.IP );

	//����Ƿ����Լ������Լ���
	if (packetPeerInfo.PeerGUID == m_PeerInformation->PeerGUID)
	{
		UDPDETECT_EVENT("Recv a self-self Hello Packet");
		return;
	}

	VIEW_DEBUG( "HandlePeerExchange " << packet.Peers.size() );

	// ���鷺����peer�ӵ�IPPool��
	TEST_LOG_OUT(count << " peers echoed from udp peer " << sockAddr);
	if ( packet.Peers.size() > 0 )
	{
		m_ipPool.AddExchangedPeers( packet.Peers.size(), &packet.Peers[0], CANDIDATE_FROM_UDP_ECHO );
	}
	TEST_LOG_FLUSH();

	// ��öԷ���IPPool������Key�ĵ�ַ���������㷨: �����ͬһ�������ڵ㣬����������ַ������ǹ����ڵ㣬���ù�����ַ��
	PEER_ADDRESS keyAddress = GetProperUDPKeyPeerAddress( packetPeerInfo.Address, sockAddr );

	if ( false == packet.IsResponse )
	{
		// request����Ҫ�ظ�
		//���Է��Ƿ񾭹�NAT,���δ����NAT�����Լ��뵽����IPPool
		if (sockAddr.IP == packetPeerInfo.Address.IP)
		{
			UDPDETECT_EVENT("Hello Remote In same network , add to IP Pool!");
			m_ipPool.AddCandidate(keyAddress, packetPeerInfo.CoreInfo, CANDIDATE_FROM_UDP_HELLO);
		}
		this->ExchangePeers( sockAddr, true, packet.SendOffTime, head.TransactionID );
		return;
	}
	else
	{
		// response����Ҫ����rtt������ӵ�IPPool
		DWORD usedTime = time_counter::get_system_count32() - packet.SendOffTime;
		//���ݽ���Ϣ��RTT���뵽Candidate

		if (keyAddress.IP != packetPeerInfo.Address.IP)
		{	// �����ڵ� ��Ϊ���������ϵĽڵ�
			//! why? ��NAT��Խ�й�
			VIEW_INFO("IPNotEqual "<<keyAddress<<" "<<packetPeerInfo.Address<<" End");
			m_ipPool.AddCannotConnectNode( keyAddress.IP, keyAddress.TcpPort );
		}
		else if( false == m_ipPool.FindAddress(keyAddress) )
		{	// ��IPPool�� �Ҳ��������ַ��ӳ�� Ҳ  ��Ϊ���������ϵĽڵ�
			//!	why? ��NAT��Խ�й�
			VIEW_INFO("IPNotFound "<<keyAddress<<" "<<packetPeerInfo.Address<<" End");
			m_ipPool.AddCannotConnectNode( keyAddress.IP, keyAddress.TcpPort );
		}
		else
		{
			VIEW_INFO("IPAddDetected "<<keyAddress<<" "<<packetPeerInfo.Address<<" End");
			LIVE_ASSERT( keyAddress.IP == packetPeerInfo.Address.IP );
			CANDIDATE_PEER_INFO candidatePeer;
			candidatePeer.Address = packetPeerInfo.Address;
			candidatePeer.CoreInfo = packetPeerInfo.CoreInfo;

			//! ��������usedTime, �᲻���Detect��ͻ
			m_ipPool.AddEchoDetected(candidatePeer, usedTime, true);
		}
	}
}

