
#include "StdAfx.h"

#include "PeerConnector.h"
#include "PeerManagerImpl.h"
#include "common/IpPool.h"
#include "PeerConnection.h"
#include "TCPPeerConnector.h"
#include "UDPPeerConnector.h"
#include "common/AppModule.h"
#include "PeerConnectionInfo.h"
#include "common/PeerManagerStatistics.h"
#include "common/BaseInfo.h"
#include "common/GloalTypes.h"

#include "framework/socket.h"

#include <synacast/protocol/PeerPacket.h>
#include <synacast/protocol/PacketHead.h>





PeerConnector::PeerConnector(CPeerManager& peerManager, boost::shared_ptr<PeerInformation> peerInformation, 
							 boost::shared_ptr<TCPConnectionlessPacketSender> tcpPacketSender, boost::shared_ptr<UDPConnectionlessPacketSender> udpPacketSender) 
	: m_PeerManager(peerManager)
       	, m_LiveInfo(m_PeerManager.GetAppModule().GetInfo())
        , m_AppModule(peerManager.GetAppModule())
	, m_ipPool(peerManager.GetAppModule().GetIPPool())
        , m_TCPPacketSender( tcpPacketSender )
        , m_UDPPacketSender( udpPacketSender )	
        , m_statistics( peerManager.GetStatisticsData().ConnectorData )
        , m_maxConnectionID(0)
        , m_PeerInformation(peerInformation)
{
	m_NetInfo = m_PeerInformation->NetInfo;
	m_TCPConnector.reset(new TCPPeerConnector(*this));
	m_UDPConnector.reset(new UDPPeerConnector(*this));
}

PeerConnector::~PeerConnector()
{
}


bool PeerConnector::IfOnlyAcceptMDS() const
{
	return m_AppModule.IsSource() && m_PeerManager.GetConnectorConfig().IfOnlyAcceptMDS;
}

bool PeerConnector::IfRefusePeer(const PEER_CORE_INFO& coreInfo) const
{
	if (IfOnlyAcceptMDS() && coreInfo.PeerType != MDS_PEER)
		return true;
	return false;
}

bool PeerConnector::Connect(const PEER_ADDRESS& addr, const PeerConnectParam& param)
{
	if (param.ConnectFlags == IPPOOL_CONNECT_NORMAL)
	{
		return ConnectTCP(addr, 0, param);
	}
	else if (param.ConnectFlags == IPPOOL_CONNECT_80)
	{
		return ConnectTCP(addr, 80, param);	
	}
	else if (param.ConnectFlags == IPPOOL_CONNECT_UDPT)
	{
		return ConnectUDP(addr, param);
	}
	else
	{	// 如果默认的话，就直接转向 IPPOOL_CONNECT_
		assert(param.ConnectFlags == IPPOOL_CONNECT_NONE);
		return ConnectUDP(addr, param);
	}
}

bool PeerConnector::ConnectUDP(const PEER_ADDRESS& addr, const PeerConnectParam& param)
{
	if (m_LiveInfo.TransferMethod == TRANSFER_TCP)
	{
		return ConnectTCP(addr, 0, param); // 用于测试纯tcp的情况
	}
	if ( addr.UdpPort == 0 )
		return false;


	if (CheckConnected(addr, param.IsVIP))
	{
		VIEW_INFO("CPeerManager::ConnectToRemote CheckConnected " << addr << " failed");
		return false;
	}
	const PEER_ADDRESS& localAddr = m_NetInfo->Address;
	if (addr.IP == localAddr.IP && addr.UdpPort == localAddr.UdpPort)
	{
		//assert(false);
		VIEW_ERROR("ConnectUDP ignore myself " << addr);
		return false;
	}

	return m_UDPConnector->Connect(addr, param);
}


bool PeerConnector::ConnectTCP(const PEER_ADDRESS& addr, UINT16 realPort, const PeerConnectParam& param)
{
	if (m_LiveInfo.TransferMethod == TRANSFER_UDP)
	{
		return ConnectUDP(addr, param); // 用于测试纯tcp的情况
	}

	if (realPort == 0)
		realPort = addr.TcpPort;

	if ( realPort == 0 )
		return false;

	if (CheckConnected(addr, param.IsVIP))
	{
		VIEW_INFO("CPeerManager::ConnectToRemote CheckConnected " << addr << " failed");
		return false;
	}
	const PEER_ADDRESS& localAddr = m_NetInfo->Address;
	if (addr.IP == localAddr.IP && addr.TcpPort == localAddr.TcpPort)
	{
		//assert(false);
		VIEW_ERROR("ConnectTCP ignore myself " << addr);
		return false;
	}

	return m_TCPConnector->Connect(addr, realPort, param);
}


bool PeerConnector::CheckConnected(const PEER_ADDRESS& addr, bool isVip) const
{
	if ( m_TCPConnector->Contains( addr ) || m_UDPConnector->Contains( addr.ToUDPAddress() ))
		return true;
	return m_PeerManager.CheckConnected(addr.IP, addr.TcpPort, isVip);
}

void PeerConnector::SyncPendingConnectionInfo()
{
	//m_LocalInfo.DownloadingPieces[6] = MAKELONG(m_statistics.TotalInitiateConnections, m_statistics.TotalSucceededConnections);
//	m_LocalInfo.Flow.Reserved = m_TotalSucceededP2PConnections;
	//m_LiveInfo.IPPoolInfo.PendingConnectionCount = GetTotalPendingPeerCount();
	//m_LiveInfo.ChannelInfo.HandshakingPeerCount = m_handshakingPeers.size();

	m_statistics.TCP.PendingPeerCount = (UINT32)m_TCPConnector->GetPendingPeerCount();
	m_statistics.UDP.PendingPeerCount = (UINT32)m_UDPConnector->GetPendingPeerCount();

	m_statistics.TCPConnectingPeerCount = (UINT32)m_TCPConnector->GetConnectingPeerCount();
	m_statistics.TCPHandshakingPeerCount = (UINT32)m_TCPConnector->GetHandshakingPeerCount();
}

bool PeerConnector::IsBusy() const
{
	int maxConn = m_AppModule.GetSysInfo().MaxConnectPendingCount;

#if 0
	maxConn = 10;
#pragma message("!!!!!使用固定的并发连接限制值10")
#endif

	int pendingCount = tcp_socket::pending_connection_count();
	APP_DEBUG("TCPConnectIsBusy: " << make_tuple(maxConn, pendingCount, GetTotalPendingPeerCount()));
	//? 是否需要考虑对maxConn加以限制
	LIMIT_MAX(maxConn, 100);
// 	if (pendingCount < (int)GetTotalPendingPeerCount())
// 	{
// 		// 取pendingCount和m_PendingConnections中个数的最大值
// 		pendingCount = (int)GetTotalPendingPeerCount();
// 	}
	if (pendingCount < (int)m_TCPConnector->GetConnectingPeerCount())
	{
		// 取pendingCount和m_PendingConnections中个数的最大值
		pendingCount = (int)m_TCPConnector->GetConnectingPeerCount();
	}
	return pendingCount >= maxConn;
}


/*
bool PeerConnector::Disconnect( TCPClientSocketPtr sock, long errcode )
{
	return m_TCPConnector->Disconnect(sock, errcode);	
}*/

void PeerConnector::HandleNoDegree( data_input_stream& is, const PACKET_PEER_INFO& packetPeerInfo, bool isTCP )
{
	PPRedirectInfo redirectPacket;
	is >> redirectPacket;
	if ( !is )
		return;
	VIEW_DEBUG("HandleNoDegree " << make_tuple( packetPeerInfo.Address, packetPeerInfo.OuterAddress ) << " " << redirectPacket.Peers.size() );
	m_ipPool.AddConnectFailed(packetPeerInfo.OuterAddress, isTCP);
	PEER_CORE_INFO coreInfo;
	coreInfo.Clear();
	for (UINT8 index = 0; index < redirectPacket.Peers.size(); ++index)
	{
		const REDIRECT_PEER_INFO& peer = redirectPacket.Peers[index];
		//VIEW_INFO("Redirect " << peer.Address << " " << peer.Status.UploadBWLeft << " Qos=" << peer.Status.Qos << " DL=" << (int)peer.Status.DegreeLeft << " DO=" << peer.Status.OutDegree << " DI=" << peer.Status.InDegree << " SP=" << peer.Status.SkipPercent );
		PEER_ADDRESS targetAddr ;
		if ( peer.OuterAddress.IP == m_NetInfo->GetOuterIP() )	// 外部IP相同，取内网IP连接，局域网连接
		{
			targetAddr = peer.Address;
		}
		else if ( peer.OuterAddress.IsValid() )						// 外网地址可用，取外网地址
		{
			targetAddr = peer.OuterAddress;
		}
		else														// 没有外网地址可用，只好取内网地址
		{
			targetAddr = peer.Address;
		}
		assert( targetAddr.IsValid() );
		/*
		if ( false == peer.OuterAddress.IsValid() )
		{
			// 如果OuterAddress无效（可能是还没有获取到外部ip和端口），使用内部地址代替
			targetAddr = peer.Address;
			assert( false );
		}
		else if ( peer.OuterAddress.IP == m_NetInfo->OuterAddress.IP )
		{
			// 外部IP和我的外部IP相同，此peer跟我处于同一内网，使用内部地址连接
			targetAddr = peer.Address;
		}
		else
		{
			targetAddr = peer.OuterAddress;
		}
		assert( 0 != targetAddr.UdpPort || 0 != targetAddr.TcpPort );
		*/
		m_ipPool.AddCandidate(targetAddr, coreInfo, CANDIDATE_FROM_REDIRECT);
		// 检查剩余度是否不足
		if ( m_PeerManager.GetDegreeLeft() < 0 )
			return;
		// 检查出度是否已经过大
		if ( m_PeerManager.GetStatistics().Degrees.All.Out + 3 > m_PeerManager.GetMaxLocalPeerCount() )
			return;
		/// 检查pending连接是否已经过多
		if ( IsBusy() )
			return;
		PeerItem peerItem;
		peerItem.Init();
		peerItem.CanDetect = true;
		peerItem.Info.Address = targetAddr;
		PeerConnectParam param(peerItem, false, m_PeerManager.GetConnectTimeout(), false);
		if ( targetAddr.TcpPort == 0 && targetAddr.UdpPort == 0 )	// 两个端口都没有，错误
		{
			VIEW_ERROR( "PeerConnector::HandleNoDegree - targetAddr error. inner address: " << peer.Address << " outer address: " << peer.OuterAddress );
		}
		else if ( targetAddr.TcpPort == 0 )	// 没有TCP端口
		{
			this->ConnectUDP( targetAddr, param );
		}
		else if ( targetAddr.UdpPort == 0 )	// 没有UDP端口，只能连接TCP
		{
			this->ConnectTCP( targetAddr, 0, param );
		}
		else if (peer.ConnectionType == 0)	// 两个端口都存在
		{
			// tcp
			this->ConnectTCP( targetAddr, 0, param );
		}
		else if ( peer.ConnectionType == 1 )
		{
			// udp
			this->ConnectUDP( targetAddr, param );
		}
		else
		{
			VIEW_ERROR( "PeerConnector::HandleNoDegree invalid peer connection type " << peer.ConnectionType << " " << make_tuple( peer.Address, peer.OuterAddress, targetAddr ) << " " << packetPeerInfo.OuterAddress );
		}
	}
}

size_t PeerConnector::GetTotalPendingPeerCount() const
{
	return m_TCPConnector->GetConnectingPeerCount() + m_UDPConnector->GetConnectingPeerCount();
}

size_t PeerConnector::GetConnectingPeerCount()
{
	return m_TCPConnector->GetConnectingPeerCount();
}

size_t PeerConnector::GetHandshakingPeerCount()
{
	return m_TCPConnector->GetHandshakingPeerCount();
}




