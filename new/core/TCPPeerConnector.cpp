
#include "StdAfx.h"

#include "TCPPeerConnector.h"
#include "common/UDPSender.h"
#include "common/AppModule.h"
#include "PeerConnector.h"
#include "PeerManagerImpl.h"
#include "common/IpPool.h"
#include "PeerConnectionInfo.h"
#include "common/BaseInfo.h"
#include "common/GloalTypes.h"
#include "common/PacketSender.h"

#include "framework/socket.h"
#include "framework/timer.h"
#include <ppl/net/asio/message_tcp_socket.h>

#include <synacast/protocol/PacketHeadIO.h>
#include <synacast/protocol/PacketObfuscator.h>
#include <synacast/protocol/PeerPacket.h>

#include <ppl/data/stlutils.h>
#include <ppl/data/strings.h>
#include "util/testlog.h"

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/noncopyable.hpp>
#include "ppl/util/time_counter.h"
using namespace ppl::util::detail;
/// 正在进行连接或握手的peer
class TCPPendingPeer : public pool_object, private boost::noncopyable, private tcp_socket_listener, public boost::enable_shared_from_this<TCPPendingPeer>
{
public:
	explicit TCPPendingPeer( TCPPeerConnector& owner, boost::shared_ptr<TCPPeerConnectionInfo> connInfo ) 
		: m_owner( owner )
		, m_connInfo( connInfo )
		, m_PeerInformation( owner.GetOwner().GetPeerInformation() )
		, m_ErrorCode( 0 )
		, m_IsHandshaking( false )
	{
	//	LIVE_ASSERT(m_connInfo->Socket.unique());
		m_connInfo->Socket->set_listener(this);
		m_KeyAddress = m_connInfo->KeyPeerAddress.ToTCPAddress();
		m_timer.set_callback(boost::bind(&TCPPendingPeer::OnTimeout, this));
	}
	virtual ~TCPPendingPeer()
	{
	//	TCPClientSocketListener* listener = this;
		//	VIEW_INFO("delete TCPPendingPeer " << this->m_connInfo->RemoteAddress << " with " << listener);
		//TRACE("TCPPendingPeer::~TCPPendingPeer() close %p %p\n", this, m_connInfo->Socket.get());
	}

	/// 获取peer地址
	const SimpleSocketAddress& GetKeyAddress() const { return m_KeyAddress; }

	bool IsHandshaking() const { return m_IsHandshaking; }

	/// 获取连接信息
	boost::shared_ptr<TCPPeerConnectionInfo> GetConnectionInfo() { return m_connInfo; }

	bool Refuse( UINT32 refuseReason )
	{
		m_owner.GetOwner().GetStatistics().TCPInfo.RefuseErrors[refuseReason]++;
		boost::scoped_ptr<serializable> errorInfo;
		if ( PP_REFUSE_NO_DEGREE == refuseReason )
		{
			PPErrorRefuseNoDegree* noDegree = new PPErrorRefuseNoDegree;
			errorInfo.reset( noDegree );
			m_owner.GetPeerManager().GetBestPeersForRedirect( noDegree->RedirectInfo.Peers, 30, this->m_SocketAddress.GetIP() );
		}
		else
		{
			errorInfo.reset( new PPErrorRefuse( refuseReason ) );
		}
		return this->Disconnect( PP_LEAVE_REFUSE, errorInfo.get() );
	}
	bool Disconnect( UINT16 errcode, const serializable* errorInfo = NULL )
	{
		m_owner.GetOwner().GetStatistics().TCPInfo.DisconnectErrors[errcode]++;
		m_ErrorCode = errcode;
		VIEW_INFO( "TCP Disconnect " << *this->m_connInfo->Socket << strings::format( " 0x%04X ", errcode ) << " " << m_StartTime.elapsed32());
		//	if (errcode != PP_LEAVE_NO_DEGREE)
		{
			bool res = DoDisconnect( errcode, errorInfo );
			this->CloseSocket();
			return res;
		}
	}
	bool DoDisconnect(UINT16 errcode, const serializable* errorInfo)
	{
		if ( PP_LEAVE_NETWORK_ERROR == errcode || PP_LEAVE_INVALID_PACKET == errcode || PP_LEAVE_HANDSHAKE_ERROR == errcode )
			return false;
		PPErrorPacket leavePacket( errcode, errorInfo );
		return m_owner.GetOwner().GetTCPPacketSender()->Send(leavePacket, m_connInfo->Socket, m_SocketAddress) > 0;
	}


	bool DoHandshake()
	{
		PEERCON_INFO("Send TcpHello to " << *m_connInfo->Socket);
		HandshakePacket packet;
		packet.MustBeVIP = false;
		packet.SessionKeyA = 0;
		packet.SessionKeyB = 0;
		size_t packetSize = m_owner.GetOwner().GetTCPPacketSender()->Send( packet, m_connInfo->Socket, m_SocketAddress);
		if ( packetSize > 0 )
		{
			m_owner.GetOwner().GetStatistics().TCPInfo.Flow.Upload.Record( packetSize );
		}
		return packetSize > 0 ;
	}


	bool Handshake()
	{
		if ( false == DoHandshake() )
		{
			m_ErrorCode = PP_LEAVE_NETWORK_ERROR;
			return false;
		}
		return true;
	}

	bool Connect()
	{
		// 检查tcp连接操作变为同步方式的问题
		TCPSyncProblemChecker syncChecker( m_owner.GetAppModule().GetInfo(), m_PeerInformation->NetInfo->CoreInfo.PeerType );
		return m_connInfo->Socket->connect( m_connInfo->RemoteAddress.IP, m_connInfo->RealPort );
	}

	void ChangeToHandshaking()
	{
		m_connInfo->HandshakeStartTime.sync();
	//	const PeerConnectorConfig& config = m_owner.GetPeerManager().GetConnectorConfig();
		UINT HandshakeTimeout = m_owner.GetPeerManager().GetConnectorConfig().HandshakeTimeout;
		UINT extraConnectTimeout = 0;
		if ( m_owner.GetPeerManager().GetPeerCount() > m_owner.GetPeerManager().GetStableConnectionCount() )
		{
		//	extraConnectTimeout = 2000;
		}
		m_timer.start(HandshakeTimeout + extraConnectTimeout);
		m_connInfo->Socket->receive();
		m_IsHandshaking = true;
	}

	bool CheckConnectionRequest()
	{
		boost::shared_ptr<TCPPeerConnectionInfo> connInfo = this->GetConnectionInfo();
		tcp_socket_ptr sock = connInfo->Socket;
		InetSocketAddress sockAddr = sock->remote_address();
		if ( m_owner.GetPeerManager().IfRefuseTCPConnectionRequest() )
		{
			this->Refuse( PP_REFUSE_TCP );
			return false;
		}
		if (m_owner.GetAppModule().GetInfo().TransferMethod == TRANSFER_UDP)
		{
			PEERCON_DEBUG("udp transfer mode, refuse tcp connection " << sock);
			this->Refuse( PP_REFUSE_TCP );
			return false;
		}

		if ( false == connInfo->ConnectParam.IsVIP && false == connInfo->IsInputSource )
		{
			// 如果剩余度不够，或者有其它原因导致此ip的连接请求不能被接收
			int minDegree = -2;
// 			if ( IsPrivateIP( sockAddr.GetIP() ) )
// 				minDegree = -5;
			if ( m_owner.GetPeerManager().GetDegreeLeft() < minDegree )
			{
				this->Refuse( PP_REFUSE_NO_DEGREE );
				return false;
			}

			if ( false == m_owner.GetPeerManager().CanAcceptConnection(sockAddr.GetIP()))
			{
				PEERCON_DEBUG("AddPeer, CanAcceptConnection is false! so Refuse Connect! " << sockAddr);
				VIEW_INFO( "AddPeer, CanAcceptConnection is false! so Refuse Connect! "<<sockAddr);
				this->Refuse( PP_REFUSE_INNER_PEER );
				return false;
			}
		}

		if ( false == Handshake() )
		{
			// do nothing
			return false;
		}
		return true;
	}

	bool HandlePacket( BYTE* data, size_t size )
	{
		TCPPendingPeer* peer = this;
		boost::shared_ptr<TCPPeerConnectionInfo> connInfo = peer->GetConnectionInfo();
		tcp_socket_ptr sock = connInfo->Socket;

		if ( size < 3 )
		{
			peer->Disconnect( PP_LEAVE_INVALID_PACKET );
			return false;
		}
		int unshuffleLen = PacketObfuscator::UnObfuscate( reinterpret_cast<BYTE*>( data ), size );
		if ( unshuffleLen <= 0 )
		{
			VIEW_ERROR( "TCPPeerConnector unshuffle failed 0 " << make_tuple( size, unshuffleLen ) << *sock << " action=" << strings::format( "0x%02X%02X 0x%02X", data[0], data[1], data[2] ) );
			peer->Disconnect( PP_LEAVE_INVALID_PACKET );
			//LIVE_ASSERT( false );
			return false;
		}
		size_t len = static_cast<size_t>( unshuffleLen );
		if ( size < len )
		{
			VIEW_ERROR( "TCPPeerConnector unshuffle failed 1 " << make_tuple( size, len ) << *sock << " action=" << strings::format( "0x%02X%02X 0x%02X", data[0], data[1], data[2] ) );
			peer->Disconnect( PP_LEAVE_INVALID_PACKET );
			return false;
		}
		data += len;
		size -= len;

		PacketInputStream is( data, size );
		TCP_PACKET_HEAD head;
		PACKET_PEER_INFO packetPeerInfo;
		is >> head >> packetPeerInfo;
		if ( !is )
		{
			// invalid packet
			VIEW_ERROR( "TCPPeerConnector unshuffle failed 2 " << is << *sock << " action=" << strings::format( "0x%02X%02X 0x%02X", data[0], data[1], data[2] ) );
			peer->Disconnect( PP_LEAVE_INVALID_PACKET );
			return false;
		}

		if ( head.ProtocolVersion < SYNACAST_VERSION_3 )
		{
			VIEW_ERROR( "TCPPeerConnector unshuffle failed 2 " << head.ProtocolVersion );
			peer->Disconnect( PP_LEAVE_VERSION_NOT_SUPPORT );
			return false;
		}

		if ( head.Action == PPT_HANDSHAKE )
		{
			return HandleHandshakePacket( is, head, packetPeerInfo );
		}

		if ( head.Action == PPT_ERROR )
		{
			return HandleErrorPacket( is, head, packetPeerInfo );
		}
		peer->Disconnect( PP_LEAVE_INVALID_PACKET );
		return false;
	}

	bool HandleErrorPacket( data_input_stream& is, const TCP_PACKET_HEAD& head, const PACKET_PEER_INFO& packetPeerInfo )
	{
		PPErrorInfo errorInfo;
		is >> errorInfo;
		if ( !is )
		{
			Disconnect( PP_LEAVE_INVALID_PACKET );
			return false;
		}
		m_owner.GetOwner().GetStatistics().TCPInfo.DisconnectedErrors[errorInfo.ErrorCode]++;
		PacketInputStream errorDataIS( is.get_buffer() + is.position(), errorInfo.ErrorLength );
		if ( errorInfo.ErrorCode == PP_LEAVE_REFUSE )
		{
			UINT32 refuseReason = 0;
			if ( errorDataIS >> refuseReason )
			{
				m_owner.GetOwner().GetStatistics().TCPInfo.RefusedErrors[refuseReason]++;
				VIEW_DEBUG( "TCP Disconnected refused " << *m_connInfo->Socket << strings::format( " 0x%04X", errorInfo.ErrorCode ) << " " << refuseReason );
				if ( PP_REFUSE_NO_DEGREE == refuseReason )
				{
					this->m_owner.GetOwner().HandleNoDegree( errorDataIS, packetPeerInfo, true );
				}
			}
			else
			{
				VIEW_DEBUG( "TCP Disconnected invalid refuse packet " << *m_connInfo->Socket << strings::format( " 0x%04X", errorInfo.ErrorCode ) );
			}
		}
		else
		{
			VIEW_DEBUG( "TCP Disconnected " << *m_connInfo->Socket << strings::format( " 0x%04X", errorInfo.ErrorCode ) );
		}
		Disconnect( PP_LEAVE_HANDSHAKE_ERROR );
		return false;
	}

	bool HandleHandshakePacket( data_input_stream& is, const TCP_PACKET_HEAD& head, const PACKET_PEER_INFO& packetPeerInfo )
	{
		boost::shared_ptr<TCPPeerConnectionInfo> connInfo = GetConnectionInfo();
		tcp_socket_ptr sock = connInfo->Socket;
		HandshakePacket packet;
		if ( false == packet.Read( is ) )
		{
			Disconnect( PP_LEAVE_INVALID_PACKET );
			return false;
		}
		PeerHandshakeInfo handshakeInfo;

		if ( packetPeerInfo.ChannelGUID != m_PeerInformation->ChannelGUID )
		{
			Disconnect( PP_LEAVE_BAD_CHANNEL );
			return false;
		}
		if ( packetPeerInfo.PeerGUID == m_PeerInformation->PeerGUID )
		{
			Disconnect( PP_LEAVE_SELF_TO_SELF );
			return false;
		}

		handshakeInfo.ChannelGUID = packetPeerInfo.ChannelGUID;
		handshakeInfo.PeerGUID = packetPeerInfo.PeerGUID;
		handshakeInfo.Address = packetPeerInfo.Address;
		handshakeInfo.OuterAddress = packetPeerInfo.OuterAddress;
		handshakeInfo.DetectedRemoteAddress = packetPeerInfo.DetectedRemoteAddress;
		handshakeInfo.Degrees = packetPeerInfo.Degrees;
		LIVE_ASSERT( handshakeInfo.Address.IsFullyValid() );
		//	LIVE_ASSERT( handshakeInfo.OuterAddress.IsAddressValid() );
		{
			handshakeInfo.AppVersion = packetPeerInfo.AppVersion;
			LIVE_ASSERT( handshakeInfo.AppVersion != 0xB72A33A1 );

			handshakeInfo.IsEmbedded = packetPeerInfo.IsEmbedded > 0;
			handshakeInfo.CoreInfo = packetPeerInfo.CoreInfo;
			handshakeInfo.MustBeVIP = packet.MustBeVIP;
		}

		SimpleSocketAddress simpleSockAddr( sock->remote_address() );
		// 对于source，如果设置了只允许mds连接上，则拒绝非mds的连接请求
		if (m_owner.GetOwner().IfRefusePeer(handshakeInfo.CoreInfo))
		{
			Refuse( PP_REFUSE_PEER );
			return false;
		}
		else if (m_owner.GetPeerManager().IsPeerConnected(handshakeInfo.PeerGUID, connInfo->ConnectParam.IsVIP, connInfo->IsInitFromRemote))
		{
			Disconnect( PP_LEAVE_DUPLICATE_CONNECTION );
			return false;
		}

		//InetSocketAddress sockAddr = sock->GetRemoteAddress();
		//bool isVIP = m_PeerManager.IsVIP(sockAddr.GetIP());
		//bool isInputSource = m_PeerManager.IsInputSourceIP(sockAddr.GetIP());
		//UINT16 realPort = sockAddr.GetPort();

		//TCPPeerConnectionInfo connInfo(sock, true, isVIP, handshakeInfo.PeerAddress, realPort, IPPOOL_CONNECT_NONE, 0);
		if ( connInfo->IsInitFromRemote )
		{
			//connInfo->RemoteAddress = handshakeInfo.PeerAddress;
			//connInfo->KeyPeerAddress = handshakeInfo.PeerAddress;
		}
		m_PeerInformation->NetInfo->SavePeerDetectedAddress( packetPeerInfo.DetectedRemoteAddress, simpleSockAddr, true, m_connInfo->IsInitFromRemote );
		m_owner.GetPeerManager().AddPeer( connInfo, handshakeInfo );
		return true;
	}

	void UpdateSocketAddress()
	{
		m_SocketAddress = m_connInfo->Socket->remote_address();
	}

	void CloseSocket()
	{
		this->m_connInfo->Socket->close();
	}

private:
	void on_socket_connect(tcp_socket* sender)
	{
		m_SocketAddress = sender->remote_address();
		m_connInfo->ConnectUsedTime = m_connInfo->ConnectStartTime.elapsed32();
		PEERCON_DEBUG( "TCPPendingPeer::OnPeerConnected " << *sender);
		// 连接成功，发送握手报文
		if ( this->Handshake() )
		{
			// 进入握手阶段
			this->ChangeToHandshaking();
			m_owner.OnPeerConnected( this->shared_from_this() );
		}
		else
		{
			// 发送报文失败
			m_ErrorCode = PP_LEAVE_NETWORK_ERROR;
			this->CloseSocket();
			m_owner.HandleConnectError( this->shared_from_this(), PP_LEAVE_NETWORK_ERROR );
		}
	}
	void on_socket_connect_failed(tcp_socket* sender, int errcode)
	{
		VIEW_INFO("SocketConnectFailed "<<" End");
		PEERCON_DEBUG( "TCPPendingPeer::OnSocketConnectFailed " << *sender << " " << errcode << " " << m_connInfo->ConnectStartTime.elapsed());
		m_ErrorCode = PP_LEAVE_NETWORK_ERROR;
		this->CloseSocket();
		m_owner.HandleConnectError( this->shared_from_this(), errcode );
	}

	virtual void on_socket_receive(tcp_socket* sender, BYTE* data, size_t size)
	{
		m_owner.GetOwner().GetStatistics().TCPInfo.Flow.Download.Record( size );
		m_owner.GetAppModule().RecordDownload(size);
		bool res = this->HandlePacket( data, size );
		m_owner.HandlePeerHandshake( this->shared_from_this(), ( res ? 0 : m_ErrorCode ) );
	}
	virtual void on_socket_receive_failed(tcp_socket* sender, int errcode)
	{
		m_ErrorCode = PP_LEAVE_NETWORK_ERROR;
		this->CloseSocket();
		m_owner.HandlePeerHandshake( this->shared_from_this(), m_ErrorCode );
	}

	void OnTimeout()
	{
		m_ErrorCode = PP_LEAVE_HANDSHAKE_TIMEOUT;
		this->Disconnect( m_ErrorCode );
		m_owner.HandlePeerHandshake( this->shared_from_this(), m_ErrorCode );
	}

protected:
	/// 所有者
	TCPPeerConnector& m_owner;

	/// 连接信息
	boost::shared_ptr<TCPPeerConnectionInfo> m_connInfo;

	boost::shared_ptr<const PeerInformation> m_PeerInformation;

	/// 握手超时的定时器
	once_timer m_timer;

	UINT16 m_ErrorCode;

	SimpleSocketAddress m_KeyAddress;
	InetSocketAddress m_SocketAddress;

	bool m_IsHandshaking;

	time_counter m_StartTime;
};




TCPPeerConnector::TCPPeerConnector(PeerConnector& owner) 
	: m_Owner(owner)
        , m_AppModule(owner.GetAppModule())
        , m_PeerManager(owner.GetPeerManager())
		, m_IPPool(owner.GetAppModule().GetIPPool())
	    , m_LiveInfo(owner.GetAppModule().GetInfo())	
	    , m_statistics( owner.GetStatistics() )
{
}


bool TCPPeerConnector::Connect(const PEER_ADDRESS& addr, UINT16 realPort, const PeerConnectParam& param)
{
	// 如果是代替探测的连接，则不能使用tcp方式
	// 但如果ConnectFlags是IPPOOL_CONNECT_NORMAL，并且rtt有效
	if ( param.IsConnectForDetect && (param.RTT > 8000 || param.ConnectFlags != IPPOOL_CONNECT_NORMAL) && m_IPPool.GetSize() > 10 )
		return false;
	if ( realPort == 0 )
		return false;

	TEST_LOG_OUT_ONCE("initiate tcp connection to peer " << addr << " with real port " << realPort);

	m_IPPool.AddConnecting( addr );
	tcp_socket_ptr sock( new message_tcp_socket );
	SimpleSocketAddress sockAddr = addr.ToTCPAddress();
	boost::shared_ptr<TCPPeerConnectionInfo> connInfo( new TCPPeerConnectionInfo( sock, sockAddr, false, addr, realPort, param ) );
	// 将socket对象关联到ConnectingPeer对象
	ConnectingPeerPtr peer( new TCPPendingPeer( *this, connInfo ) );

	sock->set_connect_timeout(param.Timeout);
	const PeerConnectorStatistics& statistics = m_statistics;
	(void)statistics;
        VIEW_INFO("CPeerManager::ConnectToRemote try to connect tcp "<< addr << " Timeout:" << param.Timeout << " " 
		<< CalcRatio(statistics.UDP.TotalInitiatedConnections, statistics.UDP.TotalSucceededInitiatedConnections) 
		<< CalcRatio(statistics.TCP.TotalInitiatedConnections, statistics.TCP.TotalSucceededInitiatedConnections));

	if ( false == peer->Connect() )
	{
		// 如果连接失败，auto_ptr会自动删除PeerSocket对象
		VIEW_INFO("CPeerManager::ConnectToRemote failed " << addr);

		HandleTCPConnectFail( connInfo );
		return false;
	}

	LIVE_ASSERT( false == containers::contains(m_ConnectingPeers, peer->GetKeyAddress()) );
	m_ConnectingPeers[peer->GetKeyAddress()] = peer;
	m_statistics.TCP.TotalInitiatedConnections++;
	m_Owner.SyncPendingConnectionInfo();
	UDPT_DEBUG("PeerConnector::ConnectTCP " << addr);
	return true;
}

void TCPPeerConnector::OnSocketAccept(tcp_socket_ptr sock, const InetSocketAddress& sockAddr)
{
	VIEW_INFO( "tcp socket connect request from " << sockAddr );
	MANAGER_DEBUG("PeerConnector::OnSocketAccept " << *sock);

	m_statistics.TCP.TotalRequestedConnections++;

	SimpleSocketAddress remoteSockAddr(sockAddr);
	PEER_ADDRESS peerAddr;
	peerAddr.IP = sockAddr.GetIP();
	peerAddr.TcpPort = sockAddr.GetPort();
	peerAddr.UdpPort = 0;

	bool isVIP = m_PeerManager.IsVIP(sockAddr.GetIP());
	bool isInputSource = m_PeerManager.IsInputSourceIP(sockAddr.GetIP());

	UINT16 realPort = sockAddr.GetPort();
	PeerItem peerItem;
	peerItem.Init();
	peerItem.Info.Address = peerAddr;
	PeerConnectParam param(peerItem, false, m_PeerManager.GetConnectTimeout(), isVIP);
	boost::shared_ptr<TCPPeerConnectionInfo> connInfo( new TCPPeerConnectionInfo( sock, remoteSockAddr, true, peerAddr, realPort, param ) );
	connInfo->IsInputSource = isInputSource;
	HandshakingPeerPtr peer( new HandshakingPeer( *this, connInfo ) );
	peer->UpdateSocketAddress();

	if ( false == peer->CheckConnectionRequest() )
	{
		sock->close();
		return;
	}
	TEST_LOG_OUT_ONCE("tcp connection request from peer " << sockAddr);

	if ( false == containers::contains( m_HandshakingPeers, peer->GetKeyAddress() ) )
	{
		// 进入握手阶段
		peer->ChangeToHandshaking();
		m_HandshakingPeers[peer->GetKeyAddress()]  = peer;
	}
	else
	{
		LIVE_ASSERT( false );
	}
	m_Owner.SyncPendingConnectionInfo();
}

void TCPPeerConnector::HandlePeerHandshake( TCPPendingPeerPtr peer, int errcode )
{
	LIVE_ASSERT( peer->IsHandshaking() );
	//PEERCON_DEBUG("PeerConnector::HandlePeerHandshake " << *peer->GetConnectionInfo()->Socket << " " << errcode);
	if ( 0 == errcode )
	{
		if ( false == peer->GetConnectionInfo()->IsInitFromRemote )
			m_statistics.TCP.TotalSucceededInitiatedConnections++;
		else
			m_statistics.TCP.TotalSucceededRequestedConnections++;
	}
	else if ( false == peer->GetConnectionInfo()->IsInitFromRemote || peer->GetConnectionInfo()->RemoteAddress.IsValid() )
	{
		m_IPPool.AddDisconnected( peer->GetConnectionInfo()->RemoteAddress, 0, errcode, 0, 0, 0, 0 );
	}
	size_t erasedCount = m_HandshakingPeers.erase(peer->GetKeyAddress());
	LIVE_ASSERT( erasedCount == 1 );
	m_Owner.SyncPendingConnectionInfo();
}

void TCPPeerConnector::OnPeerConnected(TCPPendingPeerPtr peer)
{
	// peer 由正在连接状态转换到正在握手状态
	LIVE_ASSERT( peer );
	LIVE_ASSERT( peer->IsHandshaking() );
	LIVE_ASSERT( containers::contains( m_ConnectingPeers, peer->GetKeyAddress() ) );
	m_ConnectingPeers.erase( peer->GetKeyAddress() );
	if ( false == containers::contains( m_HandshakingPeers, peer->GetKeyAddress() ) )
	{
		m_HandshakingPeers[peer->GetKeyAddress()] = peer;
	}
	m_statistics.TCPConnectSucceededTimes++;
	m_Owner.SyncPendingConnectionInfo();
}

void TCPPeerConnector::HandleConnectError( TCPPendingPeerPtr peer, int errcode )
{
	LIVE_ASSERT( peer );
	LIVE_ASSERT( errcode != 0 );
	LIVE_ASSERT( false == peer->IsHandshaking() );
	PEERCON_DEBUG("PeerConnector::HandleConnectError " << peer->GetConnectionInfo()->KeyPeerAddress << " " << errcode);

	LIVE_ASSERT( containers::contains( m_ConnectingPeers, peer->GetKeyAddress() ) );
	boost::shared_ptr<TCPPeerConnectionInfo> connInfo = peer->GetConnectionInfo();
	size_t erasedCount = m_ConnectingPeers.erase(peer->GetKeyAddress());
	LIVE_ASSERT( erasedCount == 1 );

	this->HandleTCPConnectFail(connInfo);
	m_Owner.SyncPendingConnectionInfo();
}

void TCPPeerConnector::HandleTCPConnectFail(boost::shared_ptr<TCPPeerConnectionInfo> connInfo)
{
	if( m_LiveInfo.TransferMethod == TRANSFER_UDP )
	{
		if ( false == connInfo->IsInitFromRemote )
		{
			LIVE_ASSERT( ! "invalid situation for tcp connect failure" );
		}
	}

	if (connInfo->IsInitFromRemote == false)
	{	// 只有本地发起的连接才有 重试机制
		if (connInfo->RealPort == 80)
		{	// TCP 80 方式
			m_IPPool.AddConnectFailed(connInfo->KeyPeerAddress, true);
		}
		else if (connInfo->RealPort == connInfo->KeyPeerAddress.TcpPort)
		{	// TCP 正常 方式
			if (connInfo->ConnectParam.ConnectFlags == IPPOOL_CONNECT_NORMAL)
			{
				m_IPPool.AddConnectFailed(connInfo->KeyPeerAddress, true);
			}
			else if (connInfo->ConnectParam.ConnectFlags == IPPOOL_CONNECT_NONE)
			{	// 尝试使用 TCP 80 的方式去建立连接
				VIEW_INFO("ConnectChange TCP --> TCP80 " << connInfo->KeyPeerAddress << " " << connInfo->ConnectParam.CanDetect << " " << connInfo->ConnectParam.ConnectFlags);
				if ( this->m_Owner.ConnectTCP(connInfo->KeyPeerAddress, 80, connInfo->ConnectParam) )
				{
					m_Owner.GetStatistics().TCPSwitchTo80++;
				}
			}
		}
		else
		{
			LIVE_ASSERT(0);
		}
	}
}

bool TCPPeerConnector::Contains( const PEER_ADDRESS& addr ) const
{
	SimpleSocketAddress sockAddr = addr.ToTCPAddress();
	return containers::contains(m_ConnectingPeers, sockAddr) || containers::contains(m_HandshakingPeers, sockAddr);	
}


