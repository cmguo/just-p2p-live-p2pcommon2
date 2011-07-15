
#include "StdAfx.h"

#include "PeerConnection.h"
#include "common/GloalTypes.h"

#include "common/AppModule.h"
#include "PeerManagerImpl.h"
#include "common/StreamBuffer.h"
//#include "MediaStorage.h"
#include "PeerConnectionInfo.h"
#include "Downloader.h"
#include "Uploader.h"
#include "common/IpPool.h"
#include "common/PeerInfoItem.h"
#include "common/BaseInfo.h"

#include "PeerChannel.h"

#include "util/testlog.h"
#include "util/HuffmanCoding.h"

#include "framework/socket.h"
#include <synacast/protocol/PeerPacket.h>
#include <ppl/data/strings.h>


/// 最大的minmax长度
const int MAX_MINMAX_LENGTH = 8000;

PeerConnectionTypeEnum NetToConnectionTable[4][4] = {
	{ PCT_NO_CONNECTION, PCT_NO_CONNECTION,		PCT_NO_CONNECTION,	PCT_NO_CONNECTION},
	{ PCT_NO_CONNECTION, PCT_INNER_TO_INNER,	PCT_INNER_TO_OUTER,	PCT_INNER_TO_UPNP},
	{ PCT_NO_CONNECTION, PCT_OUTER_TO_INNER,	PCT_OUTER_TO_OUTER,	PCT_OUTER_TO_UPNP},
	{ PCT_NO_CONNECTION, PCT_UPNP_TO_INNER,		PCT_UPNP_TO_OUTER,	PCT_UPNP_TO_UPNP},
};


std::ostream& operator<<(std::ostream& os, const PeerConnection& pc)
{
	return os << "PeerConnection (" << (pc.IsUDP() ? "UDPT" : "TCP") << ") " << pc.GetKeyAddress();
}

std::ostream& operator<<(std::ostream& os, const DEGREE_COUNTER& degrees)
{
	return os << strings::format( "(Degree:Left=%d,All=%d,%d,UDP=%d,%d)", degrees.Left, degrees.All.In, degrees.All.Out, degrees.UDPT.In, degrees.UDPT.Out );
}



/// 代表一个peer-peer的连接，也可以认为是代表一个远端的Peer
class TCPPeerConnection : public PeerConnection
{
public:
	TCPPeerConnection(CPeerManager& peerManager, boost::shared_ptr<PeerInfoItemProvider> peerInfoItem, boost::shared_ptr<TCPPeerConnectionInfo> connInfo, const PeerHandshakeInfo& handshakeInfo)
		: PeerConnection(peerManager, peerInfoItem, connInfo, handshakeInfo, SimpleSocketAddress( connInfo->Socket->remote_address() ), false), m_TCPConnectionInfo( connInfo )
	{
		UDPT_INFO("TCPPeerConnection: " << connInfo->RemoteAddress);
	}

	~TCPPeerConnection()
	{
	}

	bool IsInputSource() const { return m_TCPConnectionInfo->IsInputSource; }

protected:
	boost::shared_ptr<TCPPeerConnectionInfo> m_TCPConnectionInfo;
};


class UDPTSessionPeerConnection : public PeerConnection
{
public:
	UDPTSessionPeerConnection( CPeerManager& peerManager, boost::shared_ptr<PeerInfoItemProvider> peerInfoItem, boost::shared_ptr<UDPPeerConnectionInfo> connInfo, const PeerHandshakeInfo& handshakeInfo ) 
		: PeerConnection( peerManager, peerInfoItem, connInfo, handshakeInfo, connInfo->RemoteSocketAddress, true), m_UDPConnectionInfo( connInfo )
	{
		m_LocalSessionKey = connInfo->LocalSessionKey;
		m_RemoteSessionKey = connInfo->RemoteSessionKey;
		UDPT_DEBUG("construct UDPSessionPeerConnection " << connInfo->RemoteAddress << " " << GetKeyAddress() << " " << make_tuple(m_UDPConnectionInfo->RemoteSessionKey, m_UDPConnectionInfo->LocalSessionKey));
		assert( m_UDPConnectionInfo->IsBothSessionKeyValid() );
	}
	~UDPTSessionPeerConnection()
	{
		m_PeerManager.GetStatisticsData().TotalOldMaxSequenceID += m_Channel->GetTotalPacketCount();
		UDPT_DEBUG("destruct UDPSessionPeerConnection " << m_ConnectionInfo->RemoteAddress << " " << make_tuple(m_UDPConnectionInfo->RemoteSessionKey, m_UDPConnectionInfo->LocalSessionKey));
	}

public:
	virtual UINT32 GetRemoteSessionKey() const { return m_UDPConnectionInfo->RemoteSessionKey; }

protected:
	boost::shared_ptr<UDPPeerConnectionInfo> m_UDPConnectionInfo;
};


PeerConnection* PeerConnectionFactory::CreateUDPSession( CPeerManager& peerManager, boost::shared_ptr<PeerInfoItemProvider> peerInfoItem, 
														 boost::shared_ptr<UDPPeerConnectionInfo> connInfo, const PeerHandshakeInfo& handshakeInfo, boost::shared_ptr<UDPConnectionPacketSender> packetSender)
{
	PeerConnection* pc = new UDPTSessionPeerConnection( peerManager, peerInfoItem, connInfo, handshakeInfo );
	PeerChannel* channel = PeerChannelFactory::CreateUDP( *pc, packetSender, connInfo->RemoteSocketAddress, connInfo->LocalSessionKey, connInfo->RemoteSessionKey );
	pc->SetChannel(channel);
// 	if (pc->IsInitFromRemote() == false)
// 		pc->Echo(true);
	return pc;
}


PeerConnection* PeerConnectionFactory::CreateTCP( CPeerManager& peerManager, boost::shared_ptr<PeerInfoItemProvider> peerInfoItem, 
												 boost::shared_ptr<TCPPeerConnectionInfo> connInfo, const PeerHandshakeInfo& handshakeInfo, boost::shared_ptr<TCPConnectionPacketSender> packetSender )
{
	PeerConnection* pc = new TCPPeerConnection(peerManager, peerInfoItem, connInfo, handshakeInfo);
	PeerChannel* channel = PeerChannelFactory::CreateTCP(*pc, packetSender, connInfo->Socket);
	pc->SetChannel(channel);
// 	if (pc->IsInitFromRemote() == false)
// 		pc->Echo(true);
	return pc;
}



PeerConnection::PeerConnection(CPeerManager& peerManager, boost::shared_ptr<PeerInfoItemProvider> peerInfoItem, boost::shared_ptr<PeerConnectionInfo> connInfo, 
		const PeerHandshakeInfo& handshakeInfo, const SimpleSocketAddress& socketAddr, bool isUDPT)
	: 
	m_PeerManager(peerManager),
    m_PeerModule(m_PeerManager.GetAppModule()),
    m_Storage(m_PeerModule.GetStreamBuffer().GetStorage()),
    m_IPPool(m_PeerModule.GetIPPool()),
    m_Downloader(peerManager.GetDownloader()),
    m_Uploader(peerManager.GetUploader()),
    m_NetInfo(peerManager.GetAppModule().GetPeerInformation()->NetInfo),
    m_StatusInfo(peerManager.GetAppModule().GetPeerInformation()->StatusInfo),
    m_SourceResource(peerManager.GetAppModule().GetPeerInformation()->StatusInfo->GetSourceResource()),
    m_PeerInfoItemProvider(peerInfoItem), 
	m_LocalInfo(m_PeerManager.GetAppModule().GetInfo().LocalPeerInfo),
    m_ItemPeerInfo(*peerInfoItem->GetItem()),
	m_PeerInfo(peerInfoItem->GetItem()->PeerInfo),
   	m_ConnectionInfo(connInfo),
	m_LongTimeFlow( 60 ), 
	m_SocketAddress(socketAddr), 
	m_Degrees( handshakeInfo.Degrees ), 
	m_HandshakeInfo( handshakeInfo ), 
	m_LastAnnounceTime(0), 
	m_IsClosed(false), 
	m_IsUDP(isUDPT), 
    m_IsLANConnection(false),
    m_IsUPNP( PNT_UPNP == handshakeInfo.CoreInfo.PeerNetType ),
	m_IsInner( false ),
	m_MaybeSource(false),
    m_IsResourceGood(false),
    m_TotalRequestedSubPieces(0),
	m_receiveUnusedDataSubPieceCount(0),    
    m_AnnounceTimesWhenReqesting(0)   	
     
{
	m_LocalSessionKey = 0;
	m_RemoteSessionKey = 0;

	m_PeerInfo.Flow.Reset();

	m_receivedSubPieceCount = 0;
	m_receiveUnusedSubPieceCount = 0;

	assert( m_HandshakeInfo.Address.IsFullyValid() );
//	assert( m_HandshakeInfo.OuterAddress.IsAddressValid() );
	assert( INADDR_BROADCAST != m_HandshakeInfo.Address.IP );
//	assert( 0 != m_HandshakeInfo.OuterAddress.IP );
	assert( INADDR_BROADCAST != m_HandshakeInfo.OuterAddress.IP );

	// 将socket的RemoteAddress写入到PeerInfo中
	m_PeerInfo.DetectedIP = m_SocketAddress.GetIP();
	m_PeerInfo.NetInfo.Address = handshakeInfo.Address;
	m_PeerInfo.StatusInfo.Status.DegreeLeft = m_Degrees.Left;
	m_PeerInfo.PeerGUID = handshakeInfo.PeerGUID;
	assert(m_HandshakeInfo.AppVersion != 0xB72A33A1);


	// 公网或者upnp节点的IsInner都是false
	if ( m_SocketAddress.IP != m_HandshakeInfo.Address.IP )
	{
		// 内网peer，非局域网连接
		m_IsInner = true;
		m_IsLANConnection = false;
		if ( m_IsUPNP )
		{
			m_IsInner = false;
		}
	}
	else
	{
		// 公网或者内网对内网
		bool isInnerIP = IsPrivateIP( m_SocketAddress.IP );
		m_IsInner = isInnerIP;
		m_IsLANConnection = isInnerIP;
	}


	if (IsInitFromRemote())
	{	// 外来的连接
		APP_DEBUG("Peer:" << *this << " New PeerConnection Established from " << m_SocketAddress );
		m_IPPool.AddConnected( GetKeyAddress(), m_HandshakeInfo.Address.TcpPort, IPPOOL_CONNECT_NONE, m_IsLANConnection);
	}
	else
	{	// 本地的连接
		APP_DEBUG("Peer:" << *this << " New PeerConnection Established to " << m_SocketAddress);
		UINT connectFlags = IPPOOL_CONNECT_NONE;
		if ( IsUDP() )
		{
			connectFlags = IPPOOL_CONNECT_UDPT;
		}
		else if (m_ConnectionInfo->RealPort == 80)
		{
			connectFlags = IPPOOL_CONNECT_80;
		}
		else
		{
			connectFlags = IPPOOL_CONNECT_NORMAL;
		}
		m_IPPool.AddConnected( GetKeyAddress(), m_ConnectionInfo->RealPort, (UINT8)connectFlags, m_IsLANConnection);
	}
	m_ConnectionType = SelectConnectionType( m_HandshakeInfo );
/*	if ( m_ConnectionInfo->IsInitFromRemote && m_SocketAddress.IP != m_HandshakeInfo.Address.IP )
	{
		// 远程发起的连接，并且socket ip不等于汇报的ip
		// 此peer是内网peer，并且跟我不在同一内网
		m_IsInner = true;
		m_IsLANConnection = false;
		if ( false == m_NetInfo->IsExternalIP() )
		{
			assert( m_ConnectionInfo->RemoteAddress.IP == m_SocketAddress.IP );
		}
	}
	else
	{
		// 其它情况，1，公网peer发起的连接； 2，局域网peer发起的连接；3，我发起的连接（包括公网和局域网的）
		bool isInnerIP = IsPrivateIP( m_SocketAddress.IP );
		m_IsInner = isInnerIP;
		m_IsLANConnection = isInnerIP;
		if ( isInnerIP )
		{
			// 对于既有公网ip又有内网ip的peer，可能会遇到这种情况？
			assert( false == m_NetInfo->IsExternalIP() );
			assert( m_SocketAddress.IP == m_HandshakeInfo.Address.IP );
		}
		else
		{
		}
	}*/

	if ( PNT_UPNP == m_HandshakeInfo.CoreInfo.PeerNetType )
	{
		// 检查
		assert( m_IsUPNP );
		//assert( ( m_ConnectionInfo->IsInitFromRemote && m_SocketAddress.IP != m_HandshakeInfo.Address.IP ) || m_IsInner );
		assert( m_HandshakeInfo.OuterAddress.IsValid() );
	}

	if ( false == m_IsInner && false == m_IsUPNP )
	{
		//assert( m_SocketAddress.IP == m_HandshakeInfo.Address.IP );
		assert( false == IsPrivateIP( m_HandshakeInfo.Address.IP ) );
	}


	/// 检查此连接是否局域网内部的连接
/*	if (m_NetInfo->IsExternalIP())
	{
		m_IsLANConnection = false;
	}
	else
	{
		// 自己是内网peer，如果两个ip的net id相同，则表示是同一内网
		const UINT net_id_mask = 0x00FFFFFF;
		UINT myNetID = m_NetInfo->InternalIP & net_id_mask;
		UINT remoteNetID = m_SocketAddress.IP & net_id_mask;
		m_IsLANConnection = (myNetID == remoteNetID);
	}*/

	//UpdateDownloadingInfo();
	m_ItemPeerInfo.Init( m_ConnectionInfo->IsInitFromRemote, m_ConnectionInfo->ConnectParam.IsVIP, m_IsInner, m_IsLANConnection, static_cast<UINT8>( m_ConnectionType ), m_HandshakeInfo.CoreInfo.NATType, m_ConnectionInfo->IsThroughNAT );
	m_PeerInfo.NetInfo.CoreInfo = m_HandshakeInfo.CoreInfo;

//	m_PeerInfo.DownloadingPieces[MAX_DATA_REQUESTS - 2] = m_ConnectTime;
	m_PeerInfo.DownloadingPieces[MAX_DATA_REQUESTS - 1] = m_HandshakeInfo.AppVersion;

	VIEW_INFO("Peer:" << GetKeyAddress() << " PeerVersion " << m_HandshakeInfo.AppVersion << " END");
	VIEW_DEBUG("new PeerConnection " << GetKeyAddress() << " PeerVersion " << strings::format("0x%X", m_HandshakeInfo.AppVersion) << " " << m_PeerManager.GetStatistics().Degrees << " IsInitFromRemote " << IsInitFromRemote() );

	// Added by Tady, 081108: For MRP. 
	m_multiRequestsPacket.reset(new SubPieceUnitRequestPacket);
	m_bIsMRPSuported = false;

	if ( m_ConnectionInfo->IsThroughNAT )
	{
		if ( m_ConnectionInfo->IsInitFromRemote )
		{
			GetPeerManager().GetStatisticsData().SucceededReceivedNATConnections++;
		}
		else
		{
			GetPeerManager().GetStatisticsData().SucceededInitiatedNATConnections++;
		}
	}

	if ( m_ConnectionInfo->IsInitFromRemote )
	{
		TEST_LOG_OUT_ONCE("incoming connection to peer " << *this);
	}
	else
	{
		TEST_LOG_OUT_ONCE("outgoing connection from peer " << *this);
	}

}


PeerConnection::~PeerConnection()
{
//	VIEW_INFO("~PeerConnection "<<GetKeyAddress()<<" End");
//	STL_FOR_EACH_CONST(PieceRequestInfoCollection, m_DownloadingTimeoutMap, iter)
//	{
//		m_downloader.OnPieceReceiveFailed(iter->first, this);
//		VIEW_INFO("ReceiveFailed "<<GetKeyAddress()<<" "<<iter->first<<" End");
//		m_LocalInfo.MaxQuota++;
//	}
	m_LocalInfo.MaxQuota += (UINT32)m_Downloader.RemoveRequests(this);
	m_Downloader.CheckRequestByConnection( this );

	const PEER_STATUS& status = m_LocalInfo.StatusInfo.Status;
        (void)status;
	APP_DEBUG("PeerConnection Closed. PeerInfo: " << *this << ", PeerName: " << m_SocketAddress 
		<< ", AppVersion=" << std::hex << m_HandshakeInfo.AppVersion << std::dec 
		<< ", Degree=" << make_tuple((int)status.DegreeLeft, status.OutDegree, status.InDegree) 
		<< ", AverageSpeed=" << make_tuple(m_Flow.Download.GetAverageRate(), m_Flow.Upload.GetAverageRate()) 
		<< ", RecentSpeed=" << make_tuple(m_Flow.Download.GetRate(), m_Flow.Upload.GetRate()) 
		<< ", Total=" << make_tuple(m_Flow.Download.GetTotalBytes(), m_Flow.Upload.GetTotalBytes()));

}


/// 获取资源的起始置
UINT PeerConnection::GetMinIndex() const
{
	if (m_PeerResource.GetSize() == 0)
		return 0;
	UINT index = m_PeerResource.GetMinIndex();
	// 如果大于MAYBE_FAILED_PIECE_COUNT * 2，则推进MAYBE_FAILED_PIECE_COUNT个Piece
	if (m_PeerResource.GetSize() > m_PeerManager.GetConfig().ConnectionConfig.MaybeFailedPieceCount * 2)
		index += m_PeerManager.GetConfig().ConnectionConfig.MaybeFailedPieceCount;
	return index;
}


void PeerConnection::SendPieceNotFound(UINT32 pieceIndex)
{
	this->SendSubPieceNotFound( pieceIndex, 0xFF );
}

void PeerConnection::SendSubPieceNotFound(UINT32 pieceIndex, UINT8 subPieceIndex)
{
	PPErrorNoPiece packet( SubPieceUnit( pieceIndex, subPieceIndex ) );
	this->DoSendError( PP_ERROR_NO_PIECE, &packet );
}

bool PeerConnection::RequestHeaderPiece(UINT32 index, UINT timeout )
{
	PPL_TRACE("RequestHeaderPiece " << index);

	SubPieceUnit subPiece(index);
	SubPieceUnitRequestPacket req(subPiece);
	if (!this->SendPacket(req))
	{
		//assert(!"Send Data Request Packet failed.");
		PEERCON_ERROR("Send Data Request Packet failed " << index << " with " << *this);
		return false;
	}
	//PieceRequestInfo info(index, timeout);

	//m_PeerManager.AddRequest(index, this);
	VIEW_DEBUG("Peer:" << *this << " RequestHeader "<<index);

	UpdateDownloadingInfo();
	//m_PeerModule.NotifyGUI(WM_SEND_REQUEST, m_PeerInfo.DetectedIP, index);
	m_AnnounceTimesWhenReqesting = 0;
	return true;
}

bool PeerConnection::CheckLongIdle()
{
	if (m_LastTimeRecvPacket.elapsed() <= m_PeerManager.GetConfig().ConnectionConfig.MaxIdleTime * 1000)//如果距离上一次收到包已经很久了
		return false;

	PEERCON_ERROR("Peer "<<*this<<" recv Packet timeout ,so del this Peer.");
	DoClose( PP_LEAVE_LONG_IDLE );
	return true;
}

bool PeerConnection::OnAppTimer(UINT seconds)
{
	// 更新流量信息数据，未连接的也可以更新
	UpdateFlowStatics();
	m_PeerInfo.Flow.Download.Reserved[0] = (UINT32)m_TotalRequestedSubPieces;

	// 以下是为了PPMonitor里面的值来设定的
	m_PeerInfo.DownloadingPieces[4] = m_receivedSubPieceCount;
	m_PeerInfo.DownloadingPieces[6] = m_receiveUnusedSubPieceCount;
	m_PeerInfo.DownloadingPieces[1] = MAKEWORD(m_IsUDP, m_Tunnel->GetWindowSize());
	if ( CheckLongIdle() )
	{
		// 长时间没有收到报文，断开连接
		return false;
	}
	m_Channel->OnAppTimer(seconds);
	
	UpdateDownloadingInfo();
	
	// Added by Tady, 082908: To del detect-process.
// 	if (this->IsInitFromRemote() == false && seconds % 20 == 0)
// 	{
// 		this->Echo(true);
// 	}

	return true;
}

void PeerConnection::UpdateFlowStatics()
{
	// 更新流量信息并保存到共享内存
//	m_PeerManager.GetFlow().Update();
//	SaveFlowInfo(m_LocalInfo.Flow, m_PeerManager.GetFlow());
	m_Flow.Update();
	m_LongTimeFlow.Update();
	//m_recentUploadFlow.Update();
	SaveFlowInfo(m_PeerInfo.Flow, m_Flow);
	assert(m_PeerInfo.DownloadingPieces[7] == m_HandshakeInfo.AppVersion);
//	VIEW_INFO("PeerConnection::UpdateFlowStatics Flow: " << make_tuple(m_flow.Download.GetRate(), m_PeerManager.GetFlow().Download.GetRate()) 
//		<< make_tuple(m_PeerInfo.Flow.Download.Recent[0], m_LocalInfo.Flow.Download.Recent[0]));
}

void PeerConnection::UpdateDownloadingInfo()
{
/*	const int max_data_records = 1;
	int index = 0;
	//for (PieceRequestInfoCollection::const_iterator iter = m_DownloadingTimeoutMap.begin();iter != m_DownloadingTimeoutMap.end();iter++,index++)
	//{
	//	if (index >= max_data_records) break;
	//	m_PeerInfo.DownloadingPieces[index] = iter->first;
	//}
//	m_DownloadingTimeoutMap
	m_PeerInfo.MaxQuota = m_lastUsedTime;
	
	m_PeerInfo.CurrentLeftQuota = MAKE_DWORD(0, m_AnnounceTimesWhenReqesting);
	*/

	//const int max_data_records = 1;
	int index = 0;

	//for (PieceRequestInfoCollection::const_iterator iter = m_DownloadingTimeoutMap.begin();iter != m_DownloadingTimeoutMap.end();iter++,index++)
	//{
	//	if (index >= max_data_records) break;
	//	m_PeerInfo.DownloadingPieces[index] = iter->first;
	//}
	m_PeerInfo.DownloadingPieces[index] = (UINT32)m_Tunnel->GetTaskQueueSize();

	//m_lastUsedTime = m_Tunnel->GetUsedTime();
	m_PeerInfo.MaxQuota = m_Tunnel->GetRealUsedTime();

	m_PeerInfo.CurrentLeftQuota = MAKE_DWORD(m_Tunnel->GetRequestCount(), m_AnnounceTimesWhenReqesting);
}

bool PeerConnection::SendPacket(const PacketBase& packet)
{
	assert(IsValidPeerConnectionActionForSend(packet.GetAction()));
	size_t totalSize = DoSendPacket(packet);
	if (totalSize == 0)
	{
//		PPL_TRACE("PeerConnection::SendPacket " << *this << " " << (int)packet.GetAction());
//#pragma message("------ do not close the socket when PeerConnection::SendPacket failed for udpt peer connection")
		//if (false == IsUDP())
		{
			DoClose(PP_LEAVE_NETWORK_ERROR);
		}
		return false;
	}
	this->RecordUploadFlow(totalSize, packet.GetAction());
	return true;
}

void PeerConnection::RecordUploadFlow(size_t size, UINT8 action)
{
	m_Flow.Upload.Record((UINT)size);
	m_LongTimeFlow.Upload.Record((UINT)size);
	SaveFlowInfo(m_PeerInfo.Flow.Upload, m_Flow.Upload);
	m_PeerManager.RecordUploadFlow(size, action, this);
}

void PeerConnection::RecordDownloadFlow(size_t size, UINT8 action)
{
	VIEW_INFO("PeerConnection::RecordDownloadFlow sync m_LastTimeRecvPacket " << *this << " " << make_tuple(m_LocalSessionKey, m_RemoteSessionKey));
	m_LastTimeRecvPacket.sync();
	m_Flow.Download.Record((UINT)size);
	m_LongTimeFlow.Download.Record((UINT)size);
	SaveFlowInfo(m_PeerInfo.Flow.Download, m_Flow.Download);
	m_PeerManager.RecordDownloadFlow(size, action, this);
}

void PeerConnection::DoClose(UINT16 reason)
{
	if (m_IsClosed)
	{
		PEERCON_ERROR("Peer:" << *this << " PeerConnection::Close again " << strings::format( "0x%04x", reason ));
		return;
	}
	m_IsClosed = true;
	long errcode = reason;
	if (errcode == 0)
	{
		assert(false);
	}
	this->Disconnect( reason, NULL );

	CPeerInfo& peerInfo = m_PeerInfo;
	const PEER_STATUS& status = peerInfo.StatusInfo.Status;
	VIEW_DEBUG("Peer:" << *this << " PeerConnection::Close " << strings::format( "0x%04x", reason ) << " " << make_tuple(m_LocalSessionKey, m_RemoteSessionKey));
	(void)status;
        PEERCON_ERROR("PeerConnection::Close: " << *this 
		<< ", Degree=" << make_tuple((int)status.DegreeLeft, status.OutDegree, status.InDegree) 
		<< ", Socket: " << m_SocketAddress 
		<< ", AverageSpeed=" << make_tuple(m_Flow.Download.GetAverageRate(), m_Flow.Upload.GetAverageRate()) 
		<< ", RecentSpeed=" << make_tuple(m_Flow.Download.GetRate(), m_Flow.Upload.GetRate()) 
		<< ", Total=" << make_tuple(m_Flow.Download.GetTotalBytes(), m_Flow.Upload.GetTotalBytes()));
	m_PeerManager.NotifyDeletePeer(this, errcode);
}

UINT PeerConnection::GetQOS() const
{
	// 如果是老peer，则qos为0，以AverageUploadSpeed来计算
	UINT qos = m_PeerInfo.StatusInfo.Status.Qos;
	if (qos > 0)
		return qos * 1000;
	return m_Flow.Upload.GetAverageRate();
}



UINT16 PeerConnection::UpdateMinMax()
{
	m_LastAnnounceTime.sync();
	m_PeerInfo.DownloadingPieces[MAX_DATA_REQUESTS - 3] = m_LastAnnounceTime;

	// 记录历史最高的qos值
	m_PeerInfo.StatusInfo.MinMax = m_PeerResource.GetMinMax();

	if (IsVIP())
	{
		const PEER_MINMAX& minmax = m_PeerInfo.StatusInfo.MinMax;
		if (m_PeerModule.SaveVIPMinMax(minmax))
		{
			PEERCON_EVENT("Peer:" << *this << " Update Source MinMax use vip resource " << minmax );
		}
	}
	VIEW_INFO("PeerBitmap "<<GetKeyAddress()<<" "<<m_PeerResource.GetMinIndex()<<" "<<m_PeerResource<<" End");
	const PEER_STATUS& status = m_PeerInfo.StatusInfo.Status;
	(void)status;
        VIEW_INFO("PeerInfo " << GetKeyAddress() << " " << status.InDegree << " " << status.OutDegree << " " << (int)status.DegreeLeft << " " << status.Qos << " End");
	CheckResourceStatus();
	return 0;
}


void PeerConnection::CheckResourceStatus()
{
	PEER_MINMAX minmax = m_PeerResource.GetMinMax();
	int sourceMinMaxLength = m_SourceResource->GetLength();
	bool isResourceFull = m_PeerResource.CheckIfAllBitsSet(); // 资源是否是满的
	int minmaxLength = m_PeerResource.FindSkipIndex();

	// 资源长度大于source资源长度一半的算作资源状况较好
	m_IsResourceGood = (minmaxLength > sourceMinMaxLength * (int)m_PeerManager.GetConfig().ConnectionConfig.SourceMinMaxReferRangePercent / 100);
	VIEW_INFO("PeerGood "<<GetKeyAddress()<<" "<<m_IsResourceGood<<" End" );

	// 如果资源不是全满，也不是source
	if (!isResourceFull)
	{
		m_MaybeSource = false;
		VIEW_INFO("PeerSource "<<*this<<" false End" );
		return;
	}
	// 检查minmax长度跟SourceMinMax是否相符
	if ((UINT)abs(minmaxLength - sourceMinMaxLength) > m_PeerManager.GetConfig().ConnectionConfig.SourceMinMaxReferGap)
	{
		m_MaybeSource = false;
		VIEW_INFO("PeerSource "<<*this<<" false End" );
		return;
	}
	// 如果MaxIndex不超过记录的SourceMinMax.MaxIndex，则不会是Source
	if (minmax.MaxIndex <= m_SourceResource->GetMaxIndex())
	{
		m_MaybeSource = false;
		VIEW_INFO("PeerSource "<<*this<<" false End" );
		return;
	}
	m_MaybeSource = true;
	VIEW_INFO("PeerSource "<<*this<<" true End" );
}

void PeerConnection::Announce(const PPHuffmanAnnounce* huffmanAnnouncePacket)
{
	if (huffmanAnnouncePacket != NULL)
	{
		VIEW_INFO("Peer:" << *this << " PeerConnection::AnnounceHuffman " << huffmanAnnouncePacket->GetSize());
		this->SendPacket(*huffmanAnnouncePacket);
	}
}

void PeerConnection::RequestFirstByTS(UINT64 inTS)
{
	SubPieceUnitRequestPacket reqPacket(inTS);
	this->SendPacket(reqPacket);
}

UINT16 PeerConnection::HandleOldPacket(data_input_stream& is, UINT8 action)
{
	UINT16 errcode = 0;
	switch (action)
	{
	case PPT_HUFFMAN_ANNOUNCE:
		errcode = HandleHuffmanAnnounce(is);
		break;

	case PPT_SUB_PIECE_DATA:
		errcode = HandleSubPieceData(is);
		break;

	case PPT_SUB_PIECE_REQUEST:
		errcode = HandleSubPieceRequest(is);
		break;

	case PPT_PEER_EXCHANGE:
		errcode = HandlePeerExchange(is);
		break;

	case PPT_ERROR:
		errcode = HandleError(is);
		break;

	default:
		VIEW_ERROR("unsupported p2p action " << action << is.available());
	}
	return errcode;
}

void PeerConnection::Echo(bool isResponse)
{
	UDPT_INFO("Peer:" << *this << " PeerConnection::Echo ");

	PeerExchangePacket packet;
	packet.SendOffTime = time_counter::get_system_count32();
	packet.IsResponse = isResponse;

	const UINT max_echo_peer_count = 60; // 避免echo报文超过1K字节
	size_t count = m_PeerManager.FillPeerAddresses(packet.Peers, max_echo_peer_count, m_PeerInfo.DetectedIP);
	assert(count <= max_echo_peer_count);
	for (size_t i = 0; i < count; ++i)
	{
		VIEW_INFO("Peer:" << *this << " TCP SendEchoPeer " << packet.Peers[i].Address << " to " << m_HandshakeInfo.OuterAddress << " " << *this);
	}
	UDPT_INFO("Send TcpEcho to " << m_SocketAddress << " , size=" << count);
	this->SendPacket(packet);
}

UINT16 PeerConnection::HandleHuffmanAnnounce(data_input_stream& is)
{
	VIEW_INFO("Peer:" << *this << " PeerConnection::HandleHuffmanAnnounce " << is.available());

	PPHuffmanAnnounce packet;
	if (!packet.Read(is))
		return PP_LEAVE_INVALID_PACKET;

	PEER_STATUS& status = m_PeerInfo.StatusInfo.Status;
	status.Qos = packet.Status.Qos;
	status.UploadBWLeft = packet.Status.UploadBWLeft;
	status.SkipPercent = packet.Status.SkipPercent;
	m_Degrees = packet.Status.Degrees;
	status.DegreeLeft = m_Degrees.Left;
	status.InDegree = m_Degrees.All.In;
	status.OutDegree = m_Degrees.All.Out;

	BitMap tempBitmap(packet.MinIndex, packet.ResourceData);
	VIEW_INFO("HaffmanPeerBitmap "<<GetKeyAddress()<<" "<<packet.MinIndex<<" "<<tempBitmap<<" End");
	// 使用huffman解码得出原始的资源位图
	m_PeerResource = BitMap(packet.MinIndex, m_PeerManager.GetHuffmanCoding().Decode(packet.ResourceData, packet.HuffmanTimes));
	VIEW_INFO("HaffmanRate "<<GetKeyAddress()<<" "<<tempBitmap.GetSize()<<" "<<m_PeerResource.GetSize()<<" End");
	return UpdateMinMax();
}

UINT16 PeerConnection::HandleError(data_input_stream& is)
{
	UINT16 errcode = 0;
	UINT16 errorLength = 0;
	is >> errcode >> errorLength;

	if ( !is )
	{
		return PP_LEAVE_INVALID_PACKET;
	}

	PEERCON_DEBUG("Peer:" << *this << " PeerConnection::HandleError: ErrorIndex="<< strings::format( "0x%04x", errcode ) 
		<< ", ErrorInfoLength=" << errorLength);
	if ( errcode & PP_ERROR_CLOSE_FLAG )
	{
		// 需要断开连接
	}
	else
	{
		if ( errcode == PP_ERROR_NO_PIECE )
		{
			SubPieceUnit subPieceUnit;
			if ( errorLength >= SubPieceUnit::object_size && is >> subPieceUnit )
			{
				if ( 0xFF == subPieceUnit.SubPieceIndex )
				{
					VIEW_DEBUG( "PeerConnection::OnPieceNotFound " << subPieceUnit.PieceIndex << " " << *this );
					m_Tunnel->OnPieceNotFound( subPieceUnit.PieceIndex );
				}
				else
				{
					VIEW_DEBUG( "PeerConnection::OnSubPieceNotFound " << subPieceUnit << " " << *this );
					m_Tunnel->OnSubPieceNotFound( subPieceUnit );
				}
			}
		}
		// 不需要断连接的错误报文
		errcode = 0;
	}
	return errcode;
}

UINT16 PeerConnection::HandlePeerExchange(data_input_stream& is)
{
	PeerExchangePacket packet;
	if ( false == packet.Read( is ) )
		return PP_LEAVE_INVALID_PACKET;
	TEST_LOG_OUT(packet->PeerCount << " peers echoed from conn " << *this);
	if ( packet.Peers.size() > 0 )
	{
		m_IPPool.AddExchangedPeers( packet.Peers.size(), &packet.Peers[0], CANDIDATE_FROM_TCP_ECHO );
	}
	TEST_LOG_FLUSH();

	//m_ipPool.AddEchoDetected(localPeer, usedTime, false);
	if ( false == packet.IsResponse )
	{
		this->Echo(true);
	}

	return 0;
}

UINT16 PeerConnection::HandleSubPieceRequest(data_input_stream& is)
{
	SubPieceUnitRequestPacket packet;
	if (!packet.Read(is))
		return PP_LEAVE_INVALID_PACKET;
	VIEW_DEBUG("Session: HandleSubPieceRequest " << packet.SubPieces.size() << " " << *this);
	m_PeerManager.GetStatisticsData().UploaderData.TotalSubPieceRequestsReceived++;

	if (packet.SubPieces.size() != 0)
	{
		for (UINT8 index = 0; index < packet.SubPieces.size(); ++index)
		{
			m_Uploader.UploadSubPiece(packet.SubPieces[index], this);		
		}
	}
	else
	{
		for (UINT8 index = 0; index < packet.TSOfPiece.size(); ++index)
		{
			m_Uploader.UploadSubPiece(packet.TSOfPiece[index], this);
		}
	}

	return 0;
}

UINT16 PeerConnection::HandleSubPieceData(data_input_stream& is)
{
	SubPiecePacket subPiecePacket;
	if ( false == subPiecePacket.Read( is ) )
		return PP_LEAVE_INVALID_PACKET;
	SubMediaPiecePtr subPiece = subPiecePacket.SubPiece;
	m_AnnounceTimesWhenReqesting = 0;
	VIEW_INFO("Peer: " << *this << " SubPieceReceived " << subPiece->PieceInfo.PieceIndex 
		<<" "<< subPiece->SubPieceInfo.SubPieceCount<<" "<< subPiece->SubPieceInfo.SubPieceIndex);
	if (subPiece->PieceInfo.PieceIndex == 0x0201ab98)
	{
		assert(false);
	}
	if (subPiece->SubPieceInfo.SubPieceCount >= 64)
	{
		VIEW_ERROR("Peer:" << *this << " sub piece invalid " << make_tuple(subPiece->PieceInfo.PieceIndex, subPiece->SubPieceInfo.SubPieceCount, subPiece->SubPieceInfo.SubPieceIndex));
		assert(false);
		return PP_LEAVE_INVALID_PACKET;
	}

	SubPieceUnit subPieceUnit( subPiece->PieceInfo.PieceIndex, subPiece->SubPieceInfo.SubPieceIndex );
	TEST_LOG_OUT_ONCE("sub piece " << subPieceUnit << " received from " << *this << " subpiece.DataSize=" << subPiece->SubPieceData.size() << " PieceLength=" << subPiece->GetPieceLength());
	PPL_TRACE("subpiece received " << *this << " " << subPieceUnit);

	m_PeerManager.GetStatisticsData().DownloaderData.SubPieceResponseRate.Record(1);

	m_receivedSubPieceCount++;
	m_PeerModule.GetInfo().ChannelInfo.TotalReceivedSubPieceCount++;
	bool isRedundence = false;
//-- For Debug. Tady --
// 	static bool stbStorageHasPiece = false;
// 	static int iCounterStorage = 0;
// 	static bool stbBeforeMinIndex = false;
// 	static int iCounterBeforeMin = 0;
// 	static bool stbDownloaderHasSubPiece = false;
// 	static int iCounterDownloader = 0;
// 	bool bStorageHasPiece = m_Storage.HasPiece(parser->PieceInfo.PieceIndex);
// 	bool bBeforeMinIndex = parser->PieceInfo.PieceIndex < m_Storage.GetMinIndex();
// 	bool bDownloaderHasSubPiece = m_Downloader.HasSubPiece(parser->PieceInfo.PieceIndex, parser->SubPieceInfo.SubPieceIndex);
// 	if (bStorageHasPiece)
// 	{
// 		stbStorageHasPiece = bStorageHasPiece;
// 		iCounterStorage++;
// 	}
// 	if (bBeforeMinIndex)
// 	{
// 		stbBeforeMinIndex = bBeforeMinIndex;
// 		iCounterBeforeMin++;
// 	}
// 	if (bDownloaderHasSubPiece)
// 	{
// 		stbDownloaderHasSubPiece = bDownloaderHasSubPiece;
// 		iCounterDownloader++;
// 	}
// 
	if ( m_Downloader.NeedSubPiece( subPiece->PieceInfo.PieceIndex, subPiece->SubPieceInfo.SubPieceIndex ) )
	{
		m_receiveUnusedSubPieceCount++;
		m_PeerModule.GetInfo().ChannelInfo.TotalReceivedUnusedSubPieceCount++;
		m_PeerManager.GetStatisticsData().DownloaderData.TotalDuplicateSubPieces++;
		isRedundence = true;
	}
	m_PeerManager.GetStatisticsData().DownloaderData.RedundentFlow.Record(isRedundence ? 1 : 0);

	if ( subPiece->SubPieceInfo.PieceLevel < 255 )
	{
		subPiece->SubPieceInfo.PieceLevel++;
	}
	//SubPieceDataPacketPtr subPiece(new SubPieceDataPacket(parser->PieceInfo, pieceLevel, parser->SubPieceInfo.SubPieceCount, 
	//	parser->SubPieceInfo.SubPieceIndex, parser->SubPieceLength, parser->SubPieceData));
	//m_PeerManager.RecordDownloadMedia(subPiece->GetSubPieceDataSize());
	m_PeerManager.GetStatisticsData().DownloaderData.MediaFlow.Record( subPiece->SubPieceData.size() );
	m_Tunnel->OnReceiveSubPiece(subPiece);
	return 0;
}
//
//void PeerConnection::OnReceiveSubPiece(SubPieceDataPacketPtr subPiece)
//{
//	assert(false);
//}

bool PeerConnection::SendSubPiece(SubMediaPiecePtr subPiece)
{
	TEST_LOG_OUT_ONCE("send sub piece " << subPiece << " to " << *this);
	PEERCON_DEBUG("Peer:" << *this << " sub piece sent " << subPiece->GetSubPieceUnit());
	SubPiecePacket packet( subPiece );
	if (this->SendPacket(packet))
	{
		//m_PeerManager.RecordUploadMedia(subPiece->GetSubPieceDataSize());
		m_PeerManager.GetStatisticsData().UploaderData.MediaFlow.Record(subPiece->SubPieceData.size());
		m_PeerManager.GetStatisticsData().UploaderData.TotalSubPiecesUploaded++;
		return true;
	}
	return false;
}

UINT PeerConnection::GetDenseMinIndex() const
{
	double range = GetResourceRange();
	UINT pos = (UINT) ( GetMinIndex() + range * 1.0 * m_PeerManager.GetConfig().DenseResourcePosition / 100 );		// 1/3处开始资源重定位
	UINT locateFromMaxTimeSpan = m_PeerManager.GetConfig().LocateFromMaxTimeSpan;
	UINT locateFromMaxLength = m_SourceResource->GetLength() * locateFromMaxTimeSpan / 120;
	if (locateFromMaxLength > 0 && GetMaxIndex() > locateFromMaxLength)
	{
		UINT pos2 = GetMaxIndex() - locateFromMaxLength;
		if (pos2 > pos)
		{
			// 取pos和pos2两种条件中的最大值
			pos = pos2;
		}
	}
	PEERCON_DEBUG("Peer:" << *this << " PeerConnection::GetDenseMinIndex " 
		<< make_tuple(m_PeerManager.GetConfig().DenseResourcePosition, pos - GetMinIndex()) 
		<< make_tuple(GetMinIndex(), GetMaxIndex() - GetMinIndex()));
	return pos;
}

UINT PeerConnection::GetPrelocateDenseMinIndex() const
{
	double range = GetResourceRange();
	UINT pos = (UINT)(GetMinIndex() + range * m_PeerManager.GetConfig().ConnectionConfig.PrelocateDensePosition / 100);		// 1/2处开始资源重定位
	PEERCON_DEBUG("Peer:" << *this << " PeerConnection::GetDenseMinIndex " 
		<< make_tuple(m_PeerManager.GetConfig().DenseResourcePosition, pos - GetMinIndex()) 
		<< make_tuple(GetMinIndex(), GetMaxIndex() - GetMinIndex()));
	return pos;
}

bool PeerConnection::IsValidPeerConnectionActionForSend(UINT8 action)
{
	const UINT8 valid_actions[] = 
	{ 
		PPT_ERROR, 
		//PPT_SIMPLE_HELLO, 
		//PPT_ANNOUNCE, 
		//PPT_REQUEST, 
		//PPT_MEDIA, 
		//PPT_SUB_PIECE_REQUEST, 
		//PPT_SUB_PIECE_DATA, 
		//PPT_HUFFMAN_ANNOUNCE, 
		PPT_PEER_EXCHANGE, 
		PPT_SUB_PIECE_REQUEST	, 
		PPT_SUB_PIECE_DATA		, 
		PPT_HUFFMAN_ANNOUNCE	, 
	};
	const size_t action_count = SIZEOF_ARRAY(valid_actions);
	if (IsUDP())
		return true;
	return std::find(valid_actions, valid_actions + action_count, action) != valid_actions + action_count;
}

bool PeerConnection::OnPeerChannelData(data_input_stream& is, UINT8 action)
{
	action &= (~PPL_P2P_CONNECTION_ACTION_FLAG);
	m_PeerInfo.DownloadingPieces[2] = (UINT32)m_Channel->GetTotalPacketCount();
	m_PeerInfo.DownloadingPieces[3] = (UINT32)m_Channel->GetReceivedPacketCount();
	this->RecordDownloadFlow(is.total_size(), action);
	UINT16 errcode = HandleOldPacket(is, action);
	if (errcode == 0)
		return true;
	this->DoClose(errcode);
	return false;
}

bool PeerConnection::OnPeerChannelError(long errcode, ErrorTypeEnum errType)
{
	if (errType == ERROR_TYPE_PROTOCOL)
	{
		this->DoClose( static_cast<UINT16>( errcode ) );
	}
	else if (errType == ERROR_TYPE_NETWORK)
	{
		this->DoClose( PP_LEAVE_NETWORK_ERROR );
	}
	else
	{
		assert(false);
		this->DoClose( PP_LEAVE_NETWORK_ERROR );
	}
	return false;
}

void PeerConnection::Disconnect( UINT16 errcode, const serializable* errorInfo )
{
	// 设置关闭标志，避免DoSendError中发包失败再调用DoClose
	m_IsClosed = true;
	PEERCON_INFO( "PeerConnection::Disconnect - leave reason: " << strings::format( "0x%04x", errcode ) << " " << *this );

	m_PeerManager.GetStatisticsData().SentErrorCodes[errcode]++;
	if ( PP_LEAVE_NETWORK_ERROR != errcode )
	{
		this->DoSendError( errcode, errorInfo );
	}
	//if (IsUDPT())
	//{
	//	//UDPTFinishPacket finish(0, 0);
	//	//this->SendPacket(finish);
	//}
}

bool PeerConnection::RequestSubPiece( SubPieceUnit subPiece )
{
	PPL_TRACE("RequestSubPiece " << *this << " " << subPiece);
	// 发出SubPiece请求报文
	SubPieceUnitRequestPacket packet(subPiece);
	if (false == this->SendPacket(packet))
		return false;
	DownloaderStatistics& stats = m_PeerManager.GetStatisticsData().DownloaderData;
	if ( 0 == stats.StartRequestTime )
	{
		stats.StartRequestTime = stats.StartTime.elapsed32();
	}
	VIEW_INFO("PeerConnection::RequestSubPiece "<<subPiece.PieceIndex);
	m_TotalRequestedSubPieces++;
	m_PeerModule.GetInfo().ChannelInfo.TotalRequestedSubPieceCount++;
	m_PeerManager.GetStatisticsData().DownloaderData.SubPieceRequestRate.Record(1);
	m_AnnounceTimesWhenReqesting = 0;
	return true;
}

bool PeerConnection::DoSendError( UINT16 errcode, const serializable* errorInfo )
{
	PPErrorPacket packet( errcode, errorInfo );
	return SendPacket( packet );
}

void PeerConnection::RecordRealUsedTime()
{
	this->m_PeerInfo.FailedCount = m_Tunnel->GetUsedTime();
}

bool PeerConnection::HandleUDPSessionPacket( data_input_stream& is, const NEW_UDP_PACKET_HEAD& head, const UDP_SESSION_INFO& sessionInfo, const SimpleSocketAddress& sockAddr )
{
	return m_Channel->HandleUDPSessionPacket(is, head, sessionInfo, sockAddr);
}

size_t PeerConnection::DoSendPacket( const PacketBase& packet )
{
	return m_Channel->SendPacket(packet);
}

int PeerConnection::GetSendPending()
{
	return m_Channel->GetSendPending();
}

void PeerConnection::SetChannel( PeerChannel* channel )
{
	m_Channel.reset(channel);
}

bool PeerConnection::AddSubPieceIntoMRP(SubPieceUnit subPieceUnit)
{
// 	if (!IsLastRequestFinished())
// 		return false;
// 	if (!m_multiRequestsPacket)
// 		m_multiRequestsPacket.reset(new SubPieceUnitRequestPacket);	

	m_multiRequestsPacket->AddOneSubPieceUnit(subPieceUnit);

	if (m_multiRequestsPacket->GetRequestCount() >= 15)
	{
		if (!FlushOutMRP())
			return false;
	}
	return true;
}

bool PeerConnection::FlushOutMRP()
{
// 	if (!IsLastRequestFinished())
// 		return false;
// 	if (m_iMRPSendTimes > 0)
// 		return false;
	if (m_multiRequestsPacket->GetRequestCount() > 0)
	{
		if (false == this->SendMRP(false))
			return false;

		DownloaderStatistics& stats = m_PeerManager.GetStatisticsData().DownloaderData;
		if ( 0 == stats.StartRequestTime )
		{
			stats.StartRequestTime = stats.StartTime.elapsed32();
		}
		//	VIEW_INFO("PeerConnection::RequestSubPiece "<<subPiece.PieceIndex);
		m_TotalRequestedSubPieces+= m_multiRequestsPacket->GetRequestCount();
		m_PeerModule.GetInfo().ChannelInfo.TotalRequestedSubPieceCount += m_multiRequestsPacket->GetRequestCount();
		m_PeerManager.GetStatisticsData().DownloaderData.SubPieceRequestRate.Record(m_multiRequestsPacket->GetRequestCount());

		m_AnnounceTimesWhenReqesting = 0;

		m_multiRequestsPacket.reset( new SubPieceUnitRequestPacket() );
	}
	return true;
}

bool PeerConnection::SendMRP(bool bNeedRepeatMRP)
{
	assert(m_multiRequestsPacket->GetRequestCount() > 0);
	assert(IsValidPeerConnectionActionForSend(m_multiRequestsPacket->GetAction()));

//	size_t totalSize = m_Channel->SendPacket(*m_multiRequestsPacket, bNeedRepeatMRP);
	size_t totalSize = m_Channel->SendPacket(m_multiRequestsPacket);
	if (totalSize == 0)
	{
//		PPL_TRACE("PeerConnection::SendPacket " << *this << " " << (int)packet.GetAction());
//#pragma message("------ do not close the socket when PeerConnection::SendPacket failed for udpt peer connection")
		//if (false == IsUDP())
		{
			DoClose(PP_LEAVE_NETWORK_ERROR);
		}
		return false;
	}

	this->RecordUploadFlow(totalSize, m_multiRequestsPacket->GetAction());

//	m_iMRPSendTimes++;
//	m_multiRequestsPacket->Clear();
	return true;
}

void PeerConnection::ClearMRP() 
{ m_multiRequestsPacket->Clear(); /*m_iMRPSendTimes = 0; */}

bool PeerConnection::IsMRPFree()
{
	if (m_multiRequestsPacket->GetRequestCount() > 0)
		return false;
	return true;
}

bool PeerConnection::IsLastRequestFinished()
{
// 	if (m_Channel->IsLastMRPResponseReceived() 
// 		|| m_iMRPSendTimes >= m_multiRequestsPacket.GetSize() || m_iMRPSendTimes >= 4)
// 	if (m_Channel->IsLastMRPResponseReceived())//|| m_iMRPSendTimes == 0 || m_iMRPSendTimes >= m_multiRequestsPacket->GetRequestCount())//m_iMRPSendTimes > 0)
// 	{
// 		if (m_multiRequestsPacket->GetRequestCount() > 0 )
// 			ClearMRP();
// 		return true;
// 	}
	return false;
}

bool PeerConnection::IsMRPSuported() 
{ 
	if (m_bIsMRPSuported == false && m_Channel->IsLastMRPResponseReceived())
	{
		m_bIsMRPSuported = true;
	}
	return m_bIsMRPSuported; 
}

void PeerConnection::RepeatMRP()
{
// 	if (!IsLastRequestFinished())
// 	{
// 		SendMRP(true);
// 	}
}

void PeerConnection::OnTimerForMRP() { m_Channel->OnTimerForMRP(); }

PeerConnectionTypeEnum PeerConnection::SelectConnectionType( PeerHandshakeInfo & m_HandshakeInfo ) const
{
	// 先处理内网直接连接 LAN
	if( m_SocketAddress.IP == m_HandshakeInfo.Address.IP &&
		::IsPrivateIP(m_SocketAddress.IP ) )
	{
		if ( m_NetInfo->CoreInfo.PeerNetType == PNT_INNER )
		{
			return PCT_INNER_TO_LAN;
		}
		else if ( m_NetInfo->CoreInfo.PeerNetType == PNT_UPNP )
		{
			return PCT_UPNP_TO_LAN;
		}
	}

	if ( IsInitFromRemote() )
	{
		return NetToConnectionTable[m_HandshakeInfo.CoreInfo.PeerNetType][m_NetInfo->CoreInfo.PeerNetType];
	}

	return NetToConnectionTable[m_NetInfo->CoreInfo.PeerNetType][m_HandshakeInfo.CoreInfo.PeerNetType];
}
