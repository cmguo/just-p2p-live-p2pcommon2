
#include "StdAfx.h"

#include "UDPPeerConnector.h"
#include "common/UDPSender.h"
#include "common/AppModule.h"
#include "PeerConnector.h"
#include "PeerManagerImpl.h"
#include "common/IpPool.h"
#include "PeerConnection.h"
#include "PeerConnectionInfo.h"
#include "common/GloalTypes.h"

#include "common/PacketSender.h"
#include "common/BaseInfo.h"
#include <synacast/protocol/PacketBuilder.h>
#include <synacast/protocol/PeerPacket.h>

#include "util/StunModule.h"
#include "util/testlog.h"

#include <ppl/data/stlutils.h>
#include <ppl/data/strings.h>

#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>


class UDPPendingPeer : public pool_object, private boost::noncopyable, public boost::enable_shared_from_this<UDPPendingPeer>
{
public:
	enum HandshakeResultEnum { HANDSHAKE_OK = 1, HANDSHAKE_FAILED = 2, HANDSHAKE_PENDING = 3 };

	explicit UDPPendingPeer(UDPPeerConnector& owner, const SimpleSocketAddress& sockAddr, 
				boost::shared_ptr<UDPPeerConnectionInfo> connInfo, UINT16 connectionID) 
		: ConnectionInfo(connInfo)
		, m_owner(owner)
                , m_AppModule(owner.GetAppModule())
				, m_PeerManager(owner.GetPeerManager())
				, m_Owner(owner.GetOwner())
                , m_PeerInformation(owner.GetOwner().GetPeerInformation())
                , m_ErrorCode( 0 )                
                , m_RefuseReason( 0 )
		, m_HandshakeTransactionID( 0 )
	{
		const PeerConnectorConfig& config = m_owner.GetPeerManager().GetConnectorConfig();
		UINT extraConnectTimeout = 0;
		if ( m_owner.GetPeerManager().GetPeerCount() > m_owner.GetPeerManager().GetStableConnectionCount() )
		{
		//	extraConnectTimeout = 2000;
		}
		m_timer.set_callback(boost::bind(&UDPPendingPeer::OnTimeout, this));
		m_timer.start(config.UDPHandshakeTimeout + extraConnectTimeout );
		this->ConnectionInfo->RemoteSocketAddress = sockAddr;
	}
	virtual ~UDPPendingPeer()
	{

	}

	UINT16 GetErrorCode() const { return m_ErrorCode; }
	UINT32 GetRefuseReason() const { return m_RefuseReason; }

	/// ��ȡpeer��ַ
	const SimpleSocketAddress& GetSocketAddress() const { return this->ConnectionInfo->RemoteSocketAddress; }

	bool IsInitFromRemote() const { return this->ConnectionInfo->IsInitFromRemote; }

	void UpdateSocketAddress(const SimpleSocketAddress& sockAddr)
	{
		this->ConnectionInfo->UpdateSocketAddress(sockAddr);
	}
	bool SendPacket( const PacketBase& packet, UINT transactionID, const SimpleSocketAddress& sockAddr )
	{
		size_t packetSize = m_owner.GetOwner().GetUDPPacketSender()->Send( packet, transactionID, sockAddr );
		if ( packetSize > 0 )
		{
			m_owner.GetOwner().GetStatistics().UDPInfo.Flow.Upload.Record( packetSize );
		}
		return packetSize > 0;
	}


	/// �������ֵ���Ϣ
	void SaveHandshakeInfo(const HandshakePacket& handshake, const PACKET_PEER_INFO& packetPeerInfo, const SimpleSocketAddress& sockAddr)
	{
		LIVE_ASSERT( packetPeerInfo.Address.IsFullyValid() );
//		LIVE_ASSERT( packetPeerInfo.OuterAddress.IsAddressValid() );
		this->HandshakeInfo.Degrees = packetPeerInfo.Degrees;
		this->HandshakeInfo.ChannelGUID = packetPeerInfo.ChannelGUID;
		this->HandshakeInfo.PeerGUID = packetPeerInfo.PeerGUID;
		this->HandshakeInfo.AppVersion = packetPeerInfo.AppVersion;
		this->HandshakeInfo.Address = packetPeerInfo.Address;
		this->HandshakeInfo.OuterAddress = packetPeerInfo.OuterAddress;
		this->HandshakeInfo.DetectedRemoteAddress = packetPeerInfo.DetectedRemoteAddress;
		this->HandshakeInfo.IsEmbedded = ( 0 != packetPeerInfo.IsEmbedded );
		this->HandshakeInfo.CoreInfo = packetPeerInfo.CoreInfo;
		this->HandshakeInfo.MustBeVIP = handshake.MustBeVIP;
	}

	bool HandshakeFirst(UINT32 newSessionKey)
	{
		VIEW_INFO("udp handshake first " << newSessionKey << " " << GetSocketAddress());
		const PeerConnectParam& param = this->ConnectionInfo->ConnectParam;
		bool needNAT = false;
		if ( false == param.IsAccessible() )
		{
			// ��nat֮����Ҫ����Խ
			const PeerItem& peer = param;
			if ( peer.Info.Address.IsUDPValid() && peer.StunServerAddress.IsUDPValid() )
			{
				// ip��udp�˿���Ч������ip��������ip������stun server address����Ч��udp��ַ
				needNAT = true;
			}
		}

		this->m_HandshakeTime.sync();
		this->ConnectionInfo->RemoteSessionKey = newSessionKey;
		if ( false == this->HandshakeSession(0, newSessionKey, 0, needNAT) )
			return false;
		m_HandshakeTransactionID = m_owner.GetOwner().GetUDPPacketSender()->GetBuilder()->GetTransactionID()->Current();
		if ( needNAT )
		{
			m_owner.GetPeerManager().GetStatisticsData().InitiatedNATConnections++;
		}
		return true;
	}
	bool HandshakeSecond(UINT32 transactionID, UINT32 localSessionKey, UINT32 newSessionKey)
	{
		VIEW_INFO("udp handshake second " << make_tuple(localSessionKey, newSessionKey) << " " << GetSocketAddress());
		this->m_HandshakeTime.sync();
		m_HandshakeTransactionID = transactionID;

		this->ConnectionInfo->LocalSessionKey = localSessionKey;
		this->ConnectionInfo->RemoteSessionKey = newSessionKey;
		LIVE_ASSERT( this->ConnectionInfo->IsBothSessionKeyValid() );
		return this->HandshakeSession(transactionID, localSessionKey, newSessionKey, false);
	}
	bool HandshakeThird(UINT32 transactionID)
	{
		VIEW_INFO("udp handshake third " << GetSocketAddress());
		LIVE_ASSERT(this->ConnectionInfo->LocalSessionKey != 0);
		LIVE_ASSERT( this->ConnectionInfo->IsBothSessionKeyValid() );
		return this->HandshakeSession(transactionID, this->ConnectionInfo->LocalSessionKey, SESSION_KEY_END, false);
	}

	bool HandshakeSession(UINT32 transactionID, UINT32 sessionKeyA, UINT32 sessionKeyB, bool needNAT)
	{
		HandshakePacket packet;
		packet.MustBeVIP = false;
		packet.SessionKeyA = sessionKeyA;
		packet.SessionKeyB = sessionKeyB;
		bool res = SendPacket( packet, transactionID, GetSocketAddress() );
		if ( false == res )
			return false;
		StunModule* stunModule = m_AppModule.GetStunModule();
		if ( needNAT && stunModule )
		{
			UDPConnectionlessPacketBuilder& packetBuilder = *m_owner.GetOwner().GetUDPPacketSender()->GetBuilder();
			stunModule->Transmit(packetBuilder.GetData(), packetBuilder.GetSize(), GetSocketAddress(), 
				this->ConnectionInfo->ConnectParam.StunServerAddress.ToUDPAddress());
			VIEW_ERROR("nat handshake to " << GetSocketAddress() << " via " << this->ConnectionInfo->ConnectParam.StunServerAddress.ToUDPAddress());
			this->ConnectionInfo->IsThroughNAT = true;
		}
		return true;
	}

	bool HandleHandshakeRequest( const HandshakePacket& packet, const NEW_UDP_PACKET_HEAD& head, const PACKET_PEER_INFO& packetPeerInfo )
	{
		VIEW_INFO( "udp handshake request from " << this->GetSocketAddress() );
		const SimpleSocketAddress& sockAddr = this->GetSocketAddress();
		if ( m_AppModule.IsSource() && m_PeerManager.GetConnectorConfig().IsRefuseUDPConnectionRequest)
		{
			this->Refuse( PP_REFUSE_UDP, head.TransactionID );
			return false;
		}
		if (packetPeerInfo.PeerGUID == m_PeerInformation->PeerGUID)
		{
			this->Disconnect( head.TransactionID, PP_LEAVE_SELF_TO_SELF );
			return false;
		}
		int minDegree = -2;
// 		if ( IsPrivateIP( sockAddr.IP ) )
// 			minDegree = -10;
		if (m_PeerManager.GetDegreeLeft() < minDegree)
		{
			this->Refuse( PP_REFUSE_NO_DEGREE, head.TransactionID );
			return false;
		}
		if (m_AppModule.GetInfo().TransferMethod == TRANSFER_TCP )
		{
			this->Refuse( PP_REFUSE_UDP, head.TransactionID );
			return false;
		}
		if (m_Owner.IfOnlyAcceptMDS())
		{
			// ���ֻ����mds����ô��Ҫ�ȼ��handshake�����Ƿ������°汾��Ȼ����PeerType�Ƿ���mds
			// PeerType����mds����Ҫ�ܾ�
			bool needRefuse = false;
			if (packetPeerInfo.CoreInfo.PeerType != MDS_PEER)
				needRefuse = true;
			if (needRefuse)
			{
				this->Refuse( PP_REFUSE_PEER, head.TransactionID );
				return false;
			}
		} 
		this->SaveHandshakeInfo(packet, packetPeerInfo, sockAddr);
		// sessionЭ�������
		if (!UDPPeerConnectionInfo::IsValidSessionKey(packet.SessionKeyA) || packet.SessionKeyB != 0)
		{
			this->Disconnect( head.TransactionID, PP_LEAVE_INVALID_PACKET );
			return false;
		}
		if (!this->HandshakeSecond(head.TransactionID, packet.SessionKeyA, m_owner.GetNewSessionKey()))
		{
			this->Disconnect( head.TransactionID, PP_LEAVE_NETWORK_ERROR );
			return false;
		}
		return true;
	}
	/// ����Զ�˵Ļظ�������false��ʾ���ԣ�true��ʾ������
	bool HandleResponse( data_input_stream& is, const NEW_UDP_PACKET_HEAD& head, const PACKET_PEER_INFO& packetPeerInfo )
	{
		//static UINT stMaxRTT = 0;
		//static UINT rtt10b = 0;
		//static UINT rtt15b = 0;
		//static UINT rtt20b = 0;
		//static UINT rtt50b = 0;
		//static UINT rtt100b = 0;
		//static UINT rtt200b = 0;
		//static UINT rtt300b = 0;
		//static UINT rtt400b = 0;
		//static UINT rtt500b = 0;
		//static UINT rtt500o = 0;
		//static UINT rtt100up= 0;
		if ( head.TransactionID == m_HandshakeTransactionID )
		{
			// ��handshake�Ļ�Ӧ�����Լ�¼RTTֵ
			if ( this->ConnectionInfo->ConnectParam.IsConnectForDetect )
			{
				//LIVE_ASSERT( UINT_MAX == this->ConnectionInfo->ConnectParam.RTT );
				UINT rtt = this->m_HandshakeTime.elapsed32();
				LIVE_ASSERT( rtt < 10000 );
				this->ConnectionInfo->ConnectParam.RTT = rtt;
				CANDIDATE_PEER_INFO peerInfo;
				peerInfo.Address = this->ConnectionInfo->KeyPeerAddress;
				peerInfo.CoreInfo = packetPeerInfo.CoreInfo;
				this->m_owner.GetAppModule().GetIPPool().AddDetected(peerInfo, rtt, true );
				/*
				if (stMaxRTT < rtt)
				{
					stMaxRTT = rtt;
				}
				if (rtt < 100)
				{
					rtt100b++;
				}
				else if (rtt < 200)
				{
					rtt200b++;
				}
				else if (rtt < 300)
				{
					rtt300b++;
				}
				else if (rtt < 400)
				{
					rtt400b++;
				}
				else if (rtt < 500)
				{ 
					rtt500b++;
				}
				else
					rtt500o++;*/
 
				if ( 0)//rtt > 150 ) // Added by Tady, 101408: Just use the strong peer.
				{
//					rtt100up++; 
					this->Disconnect( 0, PP_LEAVE_HANDSHAKE_TIMEOUT );
					this->OnUDPTHandshakeError();
					return true;
				}
			}
		}
		if ( this->IsInitFromRemote() )
		{
			LIVE_ASSERT( GUID_NULL != this->HandshakeInfo.PeerGUID );
			if ( packetPeerInfo.PeerGUID != this->HandshakeInfo.PeerGUID )
			{
				// peer guid���ԣ����Դ˱���
				LIVE_ASSERT( !"bad peer guid for udp handshaking" );
				return false;
			}
		}
		const SimpleSocketAddress& sockAddr = this->ConnectionInfo->RemoteSocketAddress;
		if ( head.Action == PPT_HANDSHAKE )
		{
			HandshakeResultEnum res = this->HandleHandshakeResponse( is, head, packetPeerInfo );
			if ( HANDSHAKE_OK == res )
			{
				// ���ֳɹ�
				m_owner.OnUDPTPeerHandshaked( this->shared_from_this() );
			}
			else if ( HANDSHAKE_FAILED == res )
			{
				LIVE_ASSERT(m_ErrorCode != 0);
				this->OnUDPTHandshakeError();
			}
			else
			{
				LIVE_ASSERT( HANDSHAKE_PENDING == res );
				return false;
			}
		}
		else if (head.Action == PPT_ERROR)
		{
			HandleUDPTError(is, head, packetPeerInfo, sockAddr);
		}
		else
		{
			// �����ֲ�ʶ�𣬺��Դ˱���
			VIEW_DEBUG( "UDPPeerConnector::HandleResponse invalid action " << head.Action << " " << this->ConnectionInfo->RemoteSocketAddress );
			LIVE_ASSERT( !"invalid packet action for udp handshaking" );
			return false;
		}
		return true;
	}

	void HandleUDPTError( data_input_stream& is, const NEW_UDP_PACKET_HEAD& head, const PACKET_PEER_INFO& packetPeerInfo, const SimpleSocketAddress& sockAddr )
	{
		PPErrorInfo errorInfo;
		if ( is >> errorInfo && is.available() >= errorInfo.ErrorLength )
		{
			m_ErrorCode = errorInfo.ErrorCode;
			LIVE_ASSERT(m_ErrorCode != 0);
			m_owner.GetOwner().GetStatistics().UDPInfo.DisconnectedErrors[errorInfo.ErrorCode]++;
			PacketInputStream errorDataIS( is.get_buffer() + is.position(), errorInfo.ErrorLength );
			if ( errorInfo.ErrorCode == PP_LEAVE_REFUSE )
			{
				UINT32 refuseReason = 0;
				if ( errorDataIS >> refuseReason )
				{
					// ��refuse���ģ���ɾ��pending peer���ٴ�����ܵ�redirect����
					m_owner.GetOwner().GetStatistics().UDPInfo.RefusedErrors[refuseReason]++;
					VIEW_DEBUG( "UDP Disconnected refused " << sockAddr << strings::format( " 0x%04X", errorInfo.ErrorCode ) << " " << refuseReason );
					m_RefuseReason = refuseReason;
					this->OnUDPTHandshakeError();
					if ( PP_REFUSE_NO_DEGREE == refuseReason )
					{
						m_Owner.HandleNoDegree( errorDataIS, packetPeerInfo, false );
					}
					return;
				}
			}
			else
			{
				VIEW_DEBUG( "UDP Disconnected " << sockAddr << strings::format( " 0x%04X", errorInfo.ErrorCode ) );
			}
			this->OnUDPTHandshakeError();
		}
	}
	/// ����session���ģ���ֹhandshake response���Ķ�ʧ��������ʧ�ܣ�����false��ʾ����ʧ�ܣ���Ҫɾ����peer
	bool HandleSessionPacket( data_input_stream& is, const NEW_UDP_PACKET_HEAD& head, const UDP_SESSION_INFO& sessionInfo )
	{
		if ( head.Action == (PPT_HUFFMAN_ANNOUNCE | PPL_P2P_CONNECTION_ACTION_FLAG) )
		{
			if ( this->IsInitFromRemote() 
				&& this->ConnectionInfo->IsBothSessionKeyValid() 
				&& this->ConnectionInfo->RemoteSessionKey == sessionInfo.SessionKey)
			{
				LIVE_ASSERT( this->HandshakeInfo.ChannelGUID == m_PeerInformation->ChannelGUID );
				LIVE_ASSERT( this->HandshakeInfo.PeerGUID != m_PeerInformation->PeerGUID );
				if ( m_PeerManager.IsPeerConnected( HandshakeInfo.PeerGUID, ConnectionInfo->ConnectParam.IsVIP, ConnectionInfo->IsInitFromRemote ) )
				{
					this->Disconnect( 0, PP_LEAVE_DUPLICATE_CONNECTION );
					return false;
				}
				return true;
			}
		}
		else
		{
			// �����ֲ�ʶ�𣬺��Դ˱���
			VIEW_DEBUG( "UDPPeerConnector::HandleResponse invalid session action " << strings::format( "0x%04x", head.Action ) << " " << sessionInfo.SessionKey << " " << this->ConnectionInfo->RemoteSocketAddress );
			//LIVE_ASSERT( !"invalid packet action for udp session" );
			return false;
		}
		return false;
	}



	HandshakeResultEnum HandleHandshakeResponse( data_input_stream& is, const NEW_UDP_PACKET_HEAD& head, const PACKET_PEER_INFO& packetPeerInfo )
	{
		HandshakePacket packet;
		if ( false == packet.Read( is ) )
		{
			this->Disconnect( head.TransactionID, PP_LEAVE_INVALID_PACKET );
			LIVE_ASSERT(m_ErrorCode != 0);
			return HANDSHAKE_FAILED;
		}
		if ( this->ConnectionInfo->IsInitFromRemote )
		{
			LIVE_ASSERT( this->HandshakeInfo.PeerGUID != GUID_NULL );
			//? �������handshake��PeerGUID����ͬ���Ƿ��ǰ������ߵ�
			LIVE_ASSERT( packetPeerInfo.PeerGUID == this->HandshakeInfo.PeerGUID );
		}

		UDPPendingPeer* peer = this;
		const SimpleSocketAddress& sockAddr = peer->GetSocketAddress();
		if ( packetPeerInfo.PeerGUID == m_PeerInformation->PeerGUID )
		{
			this->Disconnect( head.TransactionID, PP_LEAVE_SELF_TO_SELF );
			LIVE_ASSERT(m_ErrorCode != 0);
			return HANDSHAKE_FAILED;
		}

		// ���SessionKeyA�Ƿ���Ч
		if ( false == UDPPeerConnectionInfo::IsValidSessionKey( packet.SessionKeyA ) )
		{
			//*|| peer->ConnectionInfo.RemoteSessionKey != handshakePacketInfo.SessionKeyA) 
			this->Disconnect( head.TransactionID, PP_LEAVE_INVALID_PACKET );
			LIVE_ASSERT(m_ErrorCode != 0);
			return HANDSHAKE_FAILED;
		}

		// �Ѿ����ڣ���ʾ�Ѿ�����Ԥ����״̬������תΪ����״̬
		UDPT_DEBUG( "PeerConnector::PreHandleUDPTPacket handshake2 " << sockAddr );

		peer->SaveHandshakeInfo( packet, packetPeerInfo, sockAddr );
		if ( peer->IsInitFromRemote() )
		{
			// Զ�̷���ģ�2��session keyӦ�ö�����
			LIVE_ASSERT( peer->ConnectionInfo->IsBothSessionKeyValid() );
			if ( packet.SessionKeyB != SESSION_KEY_END )
			{
				//this->Disconnect( head.TransactionID, PP_LEAVE_INVALID_PACKET );
				//return HANDSHAKE_FAILED; 
				return HANDSHAKE_PENDING;
			}
		}
		else
		{
			// �������������
			if ( UDPPeerConnectionInfo::IsValidSessionKey(packet.SessionKeyB) )
			{
				// handshake�����е�session key a������local peer�����remote peer��
				if ( packet.SessionKeyA != peer->ConnectionInfo->RemoteSessionKey )
				{
					//this->Disconnect( head.TransactionID, PP_LEAVE_INVALID_PACKET );
					//return HANDSHAKE_FAILED;
					return HANDSHAKE_PENDING;
				}
				peer->ConnectionInfo->LocalSessionKey = packet.SessionKeyB;
			}
			else
			{
				// remote peerǡ�÷���һ��handshake request��������ĳ���Ϊ�ǶԷ�����ģ������ϴη�����Է���session key������Ϊ���ӽ���
				if ( packet.SessionKeyB != SESSION_KEY_START )
				{
					//this->Disconnect( head.TransactionID, PP_LEAVE_INVALID_PACKET );
					//return HANDSHAKE_FAILED;
					return HANDSHAKE_PENDING;
				}
				peer->ConnectionInfo->IsInitFromRemote = false;
				if ( false == peer->HandshakeSecond( head.TransactionID, packet.SessionKeyA, peer->ConnectionInfo->RemoteSessionKey ) )
				{
					this->Disconnect( head.TransactionID, PP_LEAVE_NETWORK_ERROR );
					LIVE_ASSERT(m_ErrorCode != 0);
					return HANDSHAKE_FAILED;
				}
				return HANDSHAKE_PENDING;
			}
		}
		LIVE_ASSERT(peer->ConnectionInfo->IsBothSessionKeyValid());
		if (!peer->IsInitFromRemote())
		{
			// �����������handshake
			if (!peer->HandshakeThird(head.TransactionID))
			{
				this->Disconnect( head.TransactionID, PP_LEAVE_NETWORK_ERROR );
				LIVE_ASSERT(m_ErrorCode != 0);
				return HANDSHAKE_FAILED;
			}
		}
		LIVE_ASSERT( this->HandshakeInfo.ChannelGUID == m_PeerInformation->ChannelGUID );
		LIVE_ASSERT( this->HandshakeInfo.PeerGUID != m_PeerInformation->PeerGUID );
		if ( m_PeerManager.IsPeerConnected( HandshakeInfo.PeerGUID, ConnectionInfo->ConnectParam.IsVIP, ConnectionInfo->IsInitFromRemote ) )
		{
			this->Disconnect( head.TransactionID, PP_LEAVE_DUPLICATE_CONNECTION );
			LIVE_ASSERT(m_ErrorCode != 0);
			return HANDSHAKE_FAILED;
		}
		return HANDSHAKE_OK;
	}

	bool DoDisconnect( const SimpleSocketAddress& sockAddr, UINT transactionID, UINT16 errcode, const serializable* errorInfo )
	{
		if ( PP_LEAVE_NETWORK_ERROR == errcode || PP_LEAVE_INVALID_PACKET == errcode || PP_LEAVE_HANDSHAKE_ERROR == errcode )
			return false;
		PPErrorPacket leavePacket( errcode, errorInfo );
		return this->SendPacket( leavePacket, transactionID, sockAddr );
	}

	bool Disconnect( UINT32 transactionID, UINT16 errcode, serializable* errorInfo = NULL )
	{
		LIVE_ASSERT( errcode != 0 );
		m_owner.GetOwner().GetStatistics().UDPInfo.DisconnectErrors[errcode]++;
		VIEW_INFO( "UDP Disconnect " << this->ConnectionInfo->RemoteSocketAddress << strings::format( " 0x%04X ", errcode ) << " " << m_StartTime.elapsed32());
		m_ErrorCode = errcode;
		return DoDisconnect( this->ConnectionInfo->RemoteSocketAddress, transactionID, errcode, errorInfo );
	}
	bool Refuse( UINT32 refuseReason, UINT32 transactionID )
	{
		m_owner.GetOwner().GetStatistics().UDPInfo.RefusedErrors[refuseReason]++;
		m_RefuseReason = refuseReason;
		boost::scoped_ptr<serializable> errorInfo;
		if ( PP_REFUSE_NO_DEGREE != refuseReason )
		{
			errorInfo.reset( new PPErrorRefuse( refuseReason ) );
		}
		else
		{
			PPErrorRefuseNoDegree* noDegree = new PPErrorRefuseNoDegree;
			errorInfo.reset( noDegree );
			m_PeerManager.GetBestPeersForRedirect( noDegree->RedirectInfo.Peers, 30, this->GetSocketAddress().IP );
		}
		return this->Disconnect( transactionID, PP_LEAVE_REFUSE, errorInfo.get() );
	}

	void OnDuplicateConnection(const PACKET_PEER_INFO& packetPeerInfo, const NEW_UDP_PACKET_HEAD& head)
	{
		this->Disconnect( head.TransactionID, PP_LEAVE_DUPLICATE_CONNECTION );
		if ( this->ConnectionInfo->IsThroughNAT )
		{
			m_owner.GetAppModule().GetIPPool().DeleteDuplicatedNAT(this->ConnectionInfo->RemoteSocketAddress, this->ConnectionInfo->KeyPeerAddress, packetPeerInfo);
		}
	}

protected:
	void OnTimeout()
	{
		// ��ʱ��֪ͨconnectorɾ����peer
		this->Disconnect( 0, PP_LEAVE_HANDSHAKE_TIMEOUT );
		this->OnUDPTHandshakeError();
	}

	void OnUDPTHandshakeError()
	{
		LIVE_ASSERT( m_ErrorCode != 0 );
		m_owner.OnUDPTHandshakeError( this->shared_from_this(), m_ErrorCode, m_RefuseReason );
	}

public:
	/// ������Ϣ
	boost::shared_ptr<UDPPeerConnectionInfo> ConnectionInfo;
	PeerHandshakeInfo HandshakeInfo;

	boost::shared_ptr<const UDPPeerConnectionInfo> GetConnectionInfo() const
	{
		return this->ConnectionInfo;
	}

	boost::shared_ptr<UDPPeerConnectionInfo> GetConnectionInfo()
	{
		return this->ConnectionInfo;
	}

private:
	/// ������
	UDPPeerConnector& m_owner;
	AppModule& m_AppModule;
	CPeerManager& m_PeerManager;
	PeerConnector& m_Owner;

	/// ����������ֳ�ʱ�Ķ�ʱ��
	once_timer m_timer;

	boost::shared_ptr<const PeerInformation> m_PeerInformation;

	UINT16 m_ErrorCode;
	UINT32 m_RefuseReason;

	time_counter m_HandshakeTime;
	UINT32 m_HandshakeTransactionID;
	time_counter m_StartTime;
};






UDPPeerConnector::UDPPeerConnector( PeerConnector& owner ) 
	: m_Owner(owner) 
	, m_AppModule(owner.GetAppModule())
        , m_PeerManager(owner.GetPeerManager())
	, m_IPPool(owner.GetAppModule().GetIPPool())
        , m_LiveInfo(owner.GetAppModule().GetInfo())
	, m_MySessionKey(1)
	, m_statistics( owner.GetStatistics() )
{
	m_MySessionKey = RandomGenerator().Next(30000) + 1;
}

bool UDPPeerConnector::Connect(const PEER_ADDRESS& addr, const PeerConnectParam& param)
{
	if (param.IsConnectForDetect)
	{
//		LIVE_ASSERT(UINT_MAX == param.RTT);
	}
	if ( addr.UdpPort == 0 )
		return false;

	if (param.IsConnectForDetect == false)
	{
		m_IPPool.AddConnecting(addr);
	}

	SimpleSocketAddress sockAddr = addr.ToUDPAddress();
	boost::shared_ptr<UDPPeerConnectionInfo> connInfo( new UDPPeerConnectionInfo( sockAddr, false, addr, param, addr ) );

	const PeerConnectorStatistics& statistics = m_statistics;
        (void)statistics;
	VIEW_INFO("CPeerManager::ConnectToRemote try to connect udp "<< addr << " vip=" << param.IsVIP << " Timeout:" << param.Timeout << " " 
		<< CalcRatio(statistics.TCP.TotalInitiatedConnections, statistics.TCP.TotalSucceededInitiatedConnections) 
		<< CalcRatio(statistics.UDP.TotalInitiatedConnections, statistics.UDP.TotalSucceededInitiatedConnections));
	UDPPendingPeerPtr peer = CreateUDPTPendingPeer(sockAddr, connInfo);
	if (!peer->HandshakeFirst(GetNewSessionKey()))
	{
		return false;
	}
	AddUDPTPendingPeer(peer);
	m_statistics.UDP.TotalInitiatedConnections++;
	UDPT_DEBUG("PeerConnector::ConnectUDP send first handshake " << addr);

	return true;
}


bool UDPPeerConnector::HandlePacket(data_input_stream& is, const NEW_UDP_PACKET_HEAD& head, const PACKET_PEER_INFO& packetPeerInfo, const SimpleSocketAddress& sockAddr, ExtraProxyEnum extraProxy)
{
	m_statistics.UDPInfo.Flow.Download.Record( is.total_size() );

	LIVE_ASSERT( packetPeerInfo.ChannelGUID == m_Owner.GetPeerInformation()->ChannelGUID);
	//LIVE_ASSERT( TRANSFER_UDP == m_LiveInfo.TransferMethod || TRANSFER_ALL == m_LiveInfo.TransferMethod );

	UDPPendingPeerPtr peer;
	UDPPendingPeerCollection::const_iterator iter = m_udptHandshakingPeers.find(sockAddr);
	if ( iter == m_udptHandshakingPeers.end() )
	{
		if ( head.Action != PPT_HANDSHAKE )
			return false;
		HandshakePacket handshakePacket;
		if ( false == handshakePacket.Read( is ) )
			return false;
		{
			// ����sockAddrû���ҵ���Ӧ��peer���ٳ��Ը���Handshake�����е�RemoteSessionKey����
			// handshake���ı��������������еĵڶ��λ��ߵ�����
			if ( UDPPeerConnectionInfo::IsValidSessionKey( handshakePacket.SessionKeyA ) 
				&& UDPPeerConnectionInfo::IsValidSessionKey(handshakePacket.SessionKeyB) )
			{
				UDPPendingPeerKeyIndex::const_iterator iterKey = m_IndexedPeers.find( handshakePacket.SessionKeyA );
				if ( iterKey != m_IndexedPeers.end() )
				{
					peer = iterKey->second;
					LIVE_ASSERT(peer->GetSocketAddress() != sockAddr);
					//LIVE_ASSERT(peer->GetSocketAddress().IP == sockAddr.IP);
					//LIVE_ASSERT(peer->GetSocketAddress().Port != sockAddr.Port);

					m_udptHandshakingPeers.erase(peer->GetSocketAddress());
					// ��¼�ϵ�KeyPeerAddress��Ȼ����ippool�м�¼��peer����ʧ�ܣ�Ȼ����µ�ַ��Ϣ����Ҫ�Ƕ˿ڲ�һ����
					PEER_ADDRESS oldKeyAddress = peer->GetConnectionInfo()->KeyPeerAddress;
					m_IPPool.AddConnectFailed(oldKeyAddress, false);

					peer->UpdateSocketAddress(sockAddr);
					LIVE_ASSERT(peer->GetSocketAddress() == sockAddr);
					m_udptHandshakingPeers[sockAddr] = peer;
					return peer->HandleResponse( is, head, packetPeerInfo );
				}
			}
		}
		m_statistics.UDP.TotalRequestedConnections++;

		PEER_ADDRESS keyAddr = packetPeerInfo.OuterAddress;
		if ( 0 == keyAddr.IP )
		{
			//LIVE_ASSERT( false );
			keyAddr.IP = sockAddr.IP;
		}
		PeerItem peerItem;
		peerItem.Init();
		peerItem.Info.Address = packetPeerInfo.OuterAddress;
		PeerConnectParam param(peerItem, false, m_PeerManager.GetConnectTimeout(), m_PeerManager.IsVIP(sockAddr.IP));
		boost::shared_ptr<UDPPeerConnectionInfo> connInfo( new UDPPeerConnectionInfo( sockAddr, true, packetPeerInfo.Address, param, packetPeerInfo.OuterAddress ) );
		if ( EXTRA_PROXY_NTS == extraProxy )
		{
			connInfo->IsThroughNAT = true;
			m_PeerManager.GetStatisticsData().ReceivedNATConnections++;
		}
		peer = CreateUDPTPendingPeer( sockAddr, connInfo );
		if ( peer->HandleHandshakeRequest( handshakePacket, head, packetPeerInfo ) )
		{
			AddUDPTPendingPeer(peer);
		}
		else
		{
			UINT16 errcode = peer->GetErrorCode();
			UINT32 refuseReason = peer->GetRefuseReason();
			this->HandleUDPTConnectFail( connInfo, errcode, refuseReason );
		}
	}
	else
	{
		peer = iter->second;
		return peer->HandleResponse( is ,head, packetPeerInfo );
	}

	return true;
}

bool UDPPeerConnector::HandleSessionPacket(data_input_stream& is, const NEW_UDP_PACKET_HEAD& head, const UDP_SESSION_INFO& sessionInfo, const SimpleSocketAddress& sockAddr)
{
	UDPPendingPeerCollection::iterator iter = m_udptHandshakingPeers.find(sockAddr);
	if (iter != m_udptHandshakingPeers.end())
	{
		UDPPendingPeerPtr peer = iter->second;
		if ( peer->HandleSessionPacket( is, head, sessionInfo ) )
		{
			// ����Զ�̷�������ӣ��յ�handshake�����յ�announceҲ�����ֳɹ�
			// ���ֳɹ�����Ԥ����״̬תΪ����״̬
			PeerConnection* pc = OnUDPTPeerHandshaked( peer );
			if ( pc != NULL )
			{
				pc->HandleUDPSessionPacket( is, head, sessionInfo, sockAddr );
			}
		}
		else
		{
			OnUDPTHandshakeError( peer, PP_LEAVE_HANDSHAKE_TIMEOUT, 0 );
		}
	}
	else
	{
		VIEW_ERROR("ignore session packet for udp connector " << make_tuple((int)head.Action, sessionInfo.SequenceID, sessionInfo.SessionKey) << " from " << sockAddr);
		return false;
	}
	return true;
}

void UDPPeerConnector::OnUDPTHandshakeError(UDPPendingPeerPtr peer, UINT16 errcode, UINT32 refuseReason)
{
	LIVE_ASSERT( peer );
#ifdef _DEBUG
	UDPPendingPeerCollection::iterator iter = m_udptHandshakingPeers.find(peer->GetSocketAddress());
	LIVE_ASSERT( iter != m_udptHandshakingPeers.end() );
	LIVE_ASSERT( peer == iter->second );
#endif

	LIVE_ASSERT( 0 != errcode );
	m_udptHandshakingPeers.erase( peer->GetSocketAddress() );
	m_IndexedPeers.erase(peer->GetConnectionInfo()->RemoteSessionKey);
	m_Owner.SyncPendingConnectionInfo();
	this->HandleUDPTConnectFail(peer->ConnectionInfo, errcode, refuseReason);
}

PeerConnection* UDPPeerConnector::OnUDPTPeerHandshaked(UDPPendingPeerPtr peer)
{
	LIVE_ASSERT( peer );
	UDPT_DEBUG("PeerConnector::HandleUDPTPacket handshake3 " << peer->GetSocketAddress());
	// ɾ��HandshakingPeer
	m_udptHandshakingPeers.erase( peer->GetSocketAddress() );
	m_IndexedPeers.erase(peer->GetConnectionInfo()->RemoteSessionKey);
	m_Owner.SyncPendingConnectionInfo();
	// ֪ͨPeerManager���peer
	LIVE_ASSERT( peer->HandshakeInfo.Address.IsFullyValid() );
//	LIVE_ASSERT( peer->HandshakeInfo.OuterAddress.IsAddressValid() );
	if ( false == peer->ConnectionInfo->IsInitFromRemote )
		m_statistics.UDP.TotalSucceededInitiatedConnections++;
	else
		m_statistics.UDP.TotalSucceededRequestedConnections++;
	PeerConnection* pc = m_PeerManager.AddUDPTPeer( peer->ConnectionInfo, peer->HandshakeInfo );
	LIVE_ASSERT( pc );
	return pc;
}

void UDPPeerConnector::HandleUDPTConnectFail(boost::shared_ptr<UDPPeerConnectionInfo> connInfo, UINT16 errcode, UINT32 refuseReason)
{
	LIVE_ASSERT( 0 != errcode );
	if( m_LiveInfo.TransferMethod == TRANSFER_TCP )
	{
		if ( false == connInfo->IsInitFromRemote )
		{
			LIVE_ASSERT( ! "invalid situation for udp connect failure" );
		}
	}

	if (connInfo->ConnectParam.IsConnectForDetect == true)
	{// �����κδ���
		return;
	}

	if (connInfo->IsInitFromRemote == false)
	{
		if ( errcode == PP_LEAVE_SELF_TO_SELF )
			return;
		if ( errcode == PP_LEAVE_BAD_CHANNEL || errcode == PP_LEAVE_VERSION_NOT_SUPPORT || errcode == PP_LEAVE_DUPLICATE_CONNECTION || errcode == PP_LEAVE_SELF_TO_SELF )
		{
			// ��Ӧ���ٳ�������
			m_IPPool.AddConnectFailed(connInfo->KeyPeerAddress, false);
			return;
		}
		bool ifNoUDP = false;
		if ( errcode == PP_LEAVE_REFUSE )
		{
			if ( refuseReason == PP_REFUSE_NO_DEGREE || refuseReason == PP_REFUSE_PEER )
			{
				// ��Ӧ���ٳ�������
				m_IPPool.AddConnectFailed(connInfo->KeyPeerAddress, false);
				return;
			}
			if ( refuseReason == PP_REFUSE_UDP )
			{
				// ����Ҫ�ٳ���udp
				ifNoUDP = true;
			}
		}
		// ֻ�б��ط�������Ӳ��� ���Ի���
		if (connInfo->ConnectParam.ConnectFlags == IPPOOL_CONNECT_UDPT)
		{
			m_IPPool.AddConnectFailed(connInfo->KeyPeerAddress, false);
		}
		else if (connInfo->ConnectParam.ConnectFlags == IPPOOL_CONNECT_NONE)
		{
			// ����ʹ�� TCP ������ʽ ȥ��������
			if (m_LiveInfo.TransferMethod == TRANSFER_UDP)
			{
				// ��UDP����ģʽ��û�б�Ҫ��ȥ����TCP
				m_IPPool.AddConnectFailed(connInfo->KeyPeerAddress, false);
			}
			else
			{
				//ifNoUDP = true;
				UINT val = 0;
				if ( false == ifNoUDP )
				{
					RandomGenerator rnd;
					val = rnd.Next();
					if ( val % 3 != 0 )
					{
						// 2/3�Ļ���תΪtcp
						ifNoUDP = true;
					}
#if 1
//#pragma message("------ disable udp-switch-to-udp")
					ifNoUDP = true;
#endif
				}
				VIEW_INFO("ConnectChange UDPT --> TCP " << connInfo->KeyPeerAddress << " random number " << val << " IfNoUDP=" << ifNoUDP);
				if ( ifNoUDP )
				{
					if (m_Owner.IsBusy())
					{
						m_IPPool.AddConnectFailed(connInfo->KeyPeerAddress, false);
						return;
					}
					if ( this->m_Owner.ConnectTCP(connInfo->KeyPeerAddress, 0, connInfo->ConnectParam) )
					{
						m_Owner.GetStatistics().UDPSwitchToTCP++;
					}
				}
				else
				{
					if ( this->m_Owner.ConnectUDP(connInfo->KeyPeerAddress, connInfo->ConnectParam) )
					{
						m_Owner.GetStatistics().UDPSwitchToUDP++;
					}
				}
			}
		}
		else
		{
			LIVE_ASSERT(0);
		}
	}
}

UDPPendingPeerPtr UDPPeerConnector::CreateUDPTPendingPeer( const SimpleSocketAddress& sockAddr, boost::shared_ptr<UDPPeerConnectionInfo> connInfo )
{
	if ( connInfo->IsInitFromRemote )
	{
		TEST_LOG_OUT_ONCE("udp connection request from peer " << connInfo->KeyPeerAddress);
	}
	else
	{
		TEST_LOG_OUT_ONCE("initiate udp connection to peer " << connInfo->KeyPeerAddress);
	}

	LIVE_ASSERT( false == containers::contains(m_udptHandshakingPeers, sockAddr) );
	return UDPPendingPeerPtr(new UDPPendingPeer(*this, sockAddr, connInfo, (UINT16)m_ConnectionID.New()));
}

void UDPPeerConnector::AddUDPTPendingPeer( UDPPendingPeerPtr peer )
{
	VIEW_DEBUG( "UDPPeerConnector::AddUDPTPendingPeer " << peer->GetSocketAddress() );
	LIVE_ASSERT( TRANSFER_UDP == m_LiveInfo.TransferMethod || TRANSFER_ALL == m_LiveInfo.TransferMethod || TRANSFER_NO_DETECT == m_LiveInfo.TransferMethod );
	m_udptHandshakingPeers[peer->GetSocketAddress()] = peer;

	LIVE_ASSERT( false == containers::contains(m_IndexedPeers, peer->GetConnectionInfo()->RemoteSessionKey) );
	m_IndexedPeers[peer->GetConnectionInfo()->RemoteSessionKey] = peer;

	m_Owner.SyncPendingConnectionInfo();
}

bool UDPPeerConnector::Contains(const SimpleSocketAddress& sockAddr) const
{
	return containers::contains(m_udptHandshakingPeers, sockAddr);
}

UINT32 UDPPeerConnector::GetNewSessionKey()
{
	UINT32 sessionKey = m_MySessionKey;
	m_MySessionKey++;
	if (m_MySessionKey == SESSION_KEY_START || m_MySessionKey == SESSION_KEY_END)
	{
		LIVE_ASSERT(false);
		m_MySessionKey = SESSION_KEY_START + 1;
	}
	return sessionKey;
}


