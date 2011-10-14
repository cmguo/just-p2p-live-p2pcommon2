/********************************************************************
Copyright(c) 2004-2005 PPLive.com All rights reserved.                                                 
	filename: 	PeerConnection.h
	created:	2005-4-21   19:40
	author:		
	purpose:	
*********************************************************************/

#ifndef _LIVE_P2PCOMMON2_NEW_CORE_PEER_CONNECTION_H_
#define _LIVE_P2PCOMMON2_NEW_CORE_PEER_CONNECTION_H_



#include "util/BitMap.h"
#include "PeerConnectionInfo.h"

#include "util/flow.h"

#include <synacast/protocol/PeerError.h>
#include <synacast/protocol/piecefwd.h>
#include <synacast/protocol/DataIO.h>

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/noncopyable.hpp>

#include <map>
#include <list>
#include <iosfwd>

class SubPieceUnitRequestPacket;
class SubPiecePacket;

class PeerChannel;
class PeerTunnel;
class SourceResource;
class SubPieceUnit;

typedef boost::shared_ptr<PeerTunnel> PeerTunnelPtr;



enum ErrorTypeEnum
{
	ERROR_TYPE_PROTOCOL = 0, 
	ERROR_TYPE_NETWORK = 1, 
};



class PacketBase;
class PPAnnounce;
class PPHuffmanAnnounce;
class CStreamBuffer;
class Storage;
class CIPPool;
class PeerNetInfo;
class PeerStatusInfo;
struct PP_SUB_PIECE_DATA;
class Downloader;
class Uploader;
class PeerChannel;
class PeerInfoItemProvider;

class TCPConnectionPacketSender;
class UDPConnectionPacketSender;

struct NEW_UDP_PACKET_HEAD;
struct UDP_SESSION_INFO;


/// Request导致的下载超时消息，下载请求必须在10秒内得到回应
//const UINT32 REQUEST_MAX_TIMEOUT		= 1*1000;

/// 慢连接的配额界限
//const double PEER_SLOW_QUOTA = 0.1;

/// 紧急Request导致的下载超时
//const UINT32 URGENT_REQUEST_MAX_TIMEOUT	= 3000;

/// 最大配额值
//const double CONST_MAX_QUOTA = 20.0;


/// Peer Announce过来的bitmap前50个需要忽略，因为很有可能在我请求的时候就删除掉了
//const UINT32 MAYBE_FAILED_PIECE_COUNT	= 30;


/// 发出请求时通知界面
const UINT WM_SEND_REQUEST = WM_USER + 2001;

/// 请求超时时通知界面
const UINT WM_REQUEST_TIMEOUT = WM_USER + 2002;

/// 收到数据时通知界面
const UINT WM_RECV_DATA = WM_USER + 2003;


enum PeerConnectionTypeEnum
{
	PCT_NO_CONNECTION = 0,

	PCT_INNER_TO_INNER = 1,	// NAT Traversal
	PCT_INNER_TO_OUTER = 2,
	PCT_INNER_TO_UPNP = 3,
	PCT_INNER_TO_LAN = 4,		// LAN

	PCT_OUTER_TO_INNER = 5,	// callback
	PCT_OUTER_TO_OUTER = 6,
	PCT_OUTER_TO_UPNP = 7,

	PCT_UPNP_TO_INNER = 8,	// callback
	PCT_UPNP_TO_OUTER = 9,
	PCT_UPNP_TO_UPNP = 10,
	PCT_UPNP_TO_LAN = 11,
};


class CPeerManager;
class PeerConnectionInfo;
class PeerHandshakeInfo;
class CPeerInfoItem;
class CPeerInfo;
class AppModule;
class CDetailedPeerInfo;


/// piece请求的信息
struct PieceRequestInfo
{
	UINT	PiececIndex;
	time_counter	RequestTime;
	time_counter	RefferRequestTime;
	UINT	InternalTimeout;
	bool	IsRecordTimeout;
	UINT	LastSubPieceIndex;
	bool	IsSubPieceRequest;

	PieceRequestInfo() 
	{ 
		this->PiececIndex = 0;
		this->InternalTimeout = 0;
		RequestTime.sync();
		RefferRequestTime.sync();
		IsRecordTimeout = false;
		this->LastSubPieceIndex = 0;
		this->IsSubPieceRequest = false;
	}
	
	explicit PieceRequestInfo(UINT index,UINT timeout)
	{
		this->PiececIndex = index;
		this->InternalTimeout = timeout;
		RequestTime.sync();
		RefferRequestTime.sync();
		IsRecordTimeout = false;
		this->LastSubPieceIndex = 0;
		this->IsSubPieceRequest = false;
	}

	explicit PieceRequestInfo(UINT index, UINT timeout, UINT lastSubPiece)
	{
		this->PiececIndex = index;
		this->InternalTimeout = timeout;
		RequestTime.sync();
		RefferRequestTime.sync();
		IsRecordTimeout = false;
		this->LastSubPieceIndex = lastSubPiece;
		this->IsSubPieceRequest = true;
	}
};

typedef std::map<UINT32, PieceRequestInfo>	PieceRequestInfoCollection;

//class SubPieceUnitRequestPacket; // Added by Tady, 081108: For MRP.
/// 代表一个peer-peer的连接，也可以认为是代表一个远端的Peer
class PeerConnection : public pool_object, private boost::noncopyable
{
public:
	PeerConnection(CPeerManager& peerManager, boost::shared_ptr<PeerInfoItemProvider> peerInfoItem, boost::shared_ptr<PeerConnectionInfo> connInfo, 
		const PeerHandshakeInfo& handshakeInfo, const SimpleSocketAddress& socketAddr, bool isUDPT);
	virtual ~PeerConnection();

	PeerTunnel * GetTunnel() { return m_Tunnel.get(); }
	void SetTunnel( PeerTunnelPtr tunnel ) { m_Tunnel = tunnel; }

	CPeerManager& GetPeerManager() { return m_PeerManager; }

	bool OnPeerChannelData(data_input_stream& is, UINT8 action);
	bool OnPeerChannelError(long errcode, ErrorTypeEnum errType);

	void SetChannel(PeerChannel* channel);

	int GetSendPending();

	/// 发送echo
	void Echo(bool isResponse); // param is 0 means request.

	virtual bool RequestSubPiece(SubPieceUnit subPieceUnit);
	
	// Added by Tady, 080808: For Multi-subpieces-request.
	bool AddSubPieceIntoMRP(SubPieceUnit subPieceUnit);
	bool FlushOutMRP();
	bool SendMRP(bool bNeedRepeatMRP);
	bool IsMRPFree();
	bool IsLastRequestFinished();
	void ClearMRP();
	bool IsMRPSuported();
	void RepeatMRP();
	void OnTimerForMRP(); // For MRP repeat. The step is about 250ms.

	bool SendSubPiece(SubMediaPiecePtr subPiece);

	const DEGREE_INFO& GetDegrees() const { return m_Degrees; }

	const FlowMeasure& GetLongTimeFlow() const { return m_LongTimeFlow; }

	AppModule& GetAppModule() { return m_PeerModule; }

public:
	virtual UINT32 GetRemoteSessionKey() const { LIVE_ASSERT( false ); return 0; }
	bool IsUDP() const { return m_IsUDP; }

	/// 是否由远端发起连接
	bool IsInitFromRemote() const
	{
		LIVE_ASSERT( reinterpret_cast<UINT>( m_ConnectionInfo.get() ) != 0xFFFFFFFF );
		return m_ConnectionInfo->IsInitFromRemote;
	}

	/// 是否为固定的连接－－固定的连接不会被PeerManager剔除，断线后会自动重连
	bool IsVIP() const { return m_ConnectionInfo->ConnectParam.IsVIP; }

	/// 是否是内网ip
	bool IsInnerIP() const { return m_IsInner; }

	/// 是否是局域网内部连接
	bool IsLANConnection() const { return m_IsLANConnection; }

	bool IsUPNP() const { return m_IsUPNP; }

	virtual bool HandleUDPSessionPacket(data_input_stream& is, const NEW_UDP_PACKET_HEAD& head, const UDP_SESSION_INFO& sessionInfo, const SimpleSocketAddress& sockAddr);

	/// 获取PeerInfo中存储的地址信息(包括IP和端口)
	//const PEER_ADDRESS& GetPeerAddress() const { return m_HandshakeInfo.Address; }

	/// 获取此peer的socket地址
	const SimpleSocketAddress& GetSocketAddress() const { return m_SocketAddress; }

	/// 获取IPPool里面用来索引的地址
	const PEER_ADDRESS GetKeyAddress() const { return m_ConnectionInfo->KeyPeerAddress; }

	/// 获取共享内存项
	const CPeerInfo& GetPeerInfo() const { return m_PeerInfo; }

	/// 获取peer的guid
	const GUID& GetPeerGUID() const { return m_HandshakeInfo.PeerGUID; }

	/// 此peer是否有指定的piece
	bool HasPiece(UINT index) const
	{
		if (index < GetMinIndex())
			return false;
		else
			return m_PeerResource[index];
	}

	/// 获取Relocate时候 资源密集的位置
	UINT GetDenseMinIndex() const;
	
	/// 获取Prelocate时候 资源密集的位置
	UINT GetPrelocateDenseMinIndex() const;

	/// 获取资源范围
	UINT GetResourceRange() const
	{
		UINT minIndex = GetMinIndex();
		UINT maxIndex = GetMaxIndex();
		LIVE_ASSERT(maxIndex >= minIndex);
		return maxIndex - minIndex;
	}

	/// 获取资源的起始置
	UINT GetMinIndex() const;
	UINT GetRealMinIndex() const
	{
		return m_PeerResource.GetMinIndex();
	}
	const BitMap& GetRealBitmap() const
	{
		return m_PeerResource;
	}
	/// 获取资源的最大置
	UINT GetMaxIndex() const
	{
		if (m_PeerResource.GetSize() == 0)
			return 0;
		else
			return m_PeerResource.GetMaxIndex();
	}

	/// 是否可能是source
	bool MaybeSource() const { return m_MaybeSource; }

	/// 资源状况是否较好，即资源长度是否大于SourceMinMax的一半
	bool IsResourceGood() const { return m_IsResourceGood; }

	const PeerHandshakeInfo& GetHandshakeInfo() const { return m_HandshakeInfo; }

	/// 发送piece每找到的错误报文
	void SendPieceNotFound(UINT32 pieceIndex);
	void SendSubPieceNotFound(UINT32 pieceIndex, UINT8 subPieceIndex);
	PeerConnectionTypeEnum GetConnectionType() const { return m_ConnectionType; }

public:
	/// 定期，返回false表示PeerConnection被删掉了
	bool OnAppTimer(UINT seconds);

	/// 获取此peer的服务质量
	UINT GetQOS() const;

	/// 获取流量信息
	const FlowMeasure& GetFlow() const { return m_Flow; }

	/// 获取平均的上传速度
	UINT GetAverageUploadSpeed() const { return m_Flow.Upload.GetAverageRate(); }


	/// 断开连接
	void Disconnect( UINT16 leaveReason, const serializable* errorInfo );

	/// 获取握手成功的时间
	const time_counter& GetStartTime() const { return m_StartTime; }

	/// 获取握手成功后到现在的时间
	UINT64 GetConnectionTime() const { return m_StartTime.elapsed(); }

	/// 获取最近一次收到报文的时间
	UINT64 GetLastReceivePacketTime() const { return m_LastTimeRecvPacket.elapsed(); }

	/// 请求header
	bool RequestHeaderPiece(UINT32 index, UINT timeout);

	/// 发送announce报文
	void Announce(const PPHuffmanAnnounce* huffmanAnnouncePacket);
	
	/// 发送RequestByTS报文（RequestSubPiece）
	void RequestFirstByTS(UINT64 inTS);

	/// 记录下载流量
	void RecordDownloadFlow(size_t size, UINT8 action);

	/// 记录上传流量
	void RecordUploadFlow(size_t size, UINT8 action);

	UINT16 HandleOldPacket(data_input_stream& is, UINT8 action);

	const PeerConnectionInfo& GetConnectionInfo() const { return *m_ConnectionInfo; }

protected:
	/// 检查资源状况
	void CheckResourceStatus();

	/// 记录实际使用的时间
	void RecordRealUsedTime();

	/// 分派处理报文
	void DispatchPacket(const BYTE* buf, int size);

	///更新下载统计信息
	void UpdateDownloadingInfo();

	/// 向网络层发送原始报文
	size_t DoSendPacket(const PacketBase& packet);

	/// 向网络层发送原始报文(packet中只包含报文体)
	bool SendPacket(const PacketBase& packet);

	//bool DoRequestOnePiece(UINT32 pieceIndex);


	bool DoSendError( UINT16 errcode, const serializable* errorInfo );

protected:
	/// 关闭此连接，reaason为关闭的原因
	/// 注意，以前是向PeerManager发消息通知PeerManager到时候删除peer，现改为直接删除，以减少对消息队列的依赖(2007-04-06 chenmp)
	void DoClose(UINT16 reason);

	/// 检查是否长时间没有收包
	bool CheckLongIdle();

	/// 初始化连接信息
	void InitConnectionData();

	/// 更新minmax
	UINT16 UpdateMinMax();

	/// 定期调用将更新流量统计
	void UpdateFlowStatics();


	/************************************************************************/
	/* 内部消息处理与映射函数                                   */
	/************************************************************************/
protected:
	UINT16 HandleHuffmanAnnounce(data_input_stream& is);
	UINT16 HandleError(data_input_stream& is);
	UINT16 HandleSubPieceData(data_input_stream& is);

	UINT16 HandleSubPieceRequest(data_input_stream& is);
	UINT16 HandlePeerExchange(data_input_stream& is);

	/// 检查发送报文的命令字类型是否有效
	bool IsValidPeerConnectionActionForSend(UINT8 action);

	
	PeerConnectionTypeEnum SelectConnectionType( PeerHandshakeInfo & m_HandshakeInfo ) const;

protected:
	/// 所属的PeerManager
	CPeerManager& m_PeerManager;

	/// 所属的AppModule
	AppModule& m_PeerModule;

	/// 数据缓冲区
	const Storage& m_Storage;

	/// ip池
	CIPPool& m_IPPool;
	Downloader& m_Downloader;
	Uploader& m_Uploader;

	boost::shared_ptr<PeerNetInfo> m_NetInfo;

	boost::shared_ptr<const PeerStatusInfo> m_StatusInfo;
	boost::shared_ptr<const SourceResource> m_SourceResource;

	boost::shared_ptr<PeerInfoItemProvider> m_PeerInfoItemProvider;

	/// 本地信息项
	CDetailedPeerInfo& m_LocalInfo;

	/// 对方的基本信息和统计信息
	CPeerInfoItem& m_ItemPeerInfo;

	/// peer信息项
	CPeerInfo& m_PeerInfo;

	/// 连接信息
	boost::shared_ptr<PeerConnectionInfo> m_ConnectionInfo;

	/// 流量计算
	FlowMeasure m_Flow;
	FlowMeasure m_LongTimeFlow;

	/// peer标识
	//GUID m_peerGUID;

	SimpleSocketAddress m_SocketAddress;

	//PEER_ADDRESS m_KeyPeerAddress;

	boost::scoped_ptr<PeerChannel> m_Channel;

	DEGREE_INFO m_Degrees;

	PeerTunnelPtr m_Tunnel;

	PeerHandshakeInfo m_HandshakeInfo;

	/// 最近一次收包的时间
	time_counter m_LastTimeRecvPacket;

	/// 握手成功的时间
	time_counter m_StartTime;

	/// 最近一次announce的时间
	time_counter m_LastAnnounceTime;

	/// 是否已经调用过Close
	bool m_IsClosed;

	/// 是否是udpt虚拟连接
	bool m_IsUDP;

	// 连接模型
	PeerConnectionTypeEnum m_ConnectionType;

	/// 是否是局域网内部的连接
	bool m_IsLANConnection;

	/// 是否是upnp的内网节点
	bool m_IsUPNP;

	/// peer是否是内网节点
	bool m_IsInner;

	/// 是否可能是source
	bool m_MaybeSource;

	/// 资源状况是否是好的，即资源长度是否大于SourceMinMax的一半
	bool m_IsResourceGood;


	/// 此peer的资源位图
	BitMap m_PeerResource;

	UINT m_receivedSubPieceCount;
	UINT m_receiveUnusedSubPieceCount;
	size_t m_TotalRequestedSubPieces;
	UINT m_receiveUnusedDataSubPieceCount;

	/// 每次正在请求时累积的announce次数
	int m_AnnounceTimesWhenReqesting;

	UINT m_LocalSessionKey;
	UINT m_RemoteSessionKey;


private:
//	int  m_iMRPSendTimes;
	bool m_bIsMRPSuported;
	boost::shared_ptr<SubPieceUnitRequestPacket> m_multiRequestsPacket;
};


std::ostream& operator<<(std::ostream& os, const PeerConnection& pc);


class PeerConnectionFactory
{
public:
	static PeerConnection* CreateTCP( CPeerManager& peerManager, boost::shared_ptr<PeerInfoItemProvider> peerInfoItem, boost::shared_ptr<TCPPeerConnectionInfo> connInfo, 
		const PeerHandshakeInfo& handshakeInfo, boost::shared_ptr<TCPConnectionPacketSender> packetSender );

	static PeerConnection* CreateUDPSession( CPeerManager& peerManager, boost::shared_ptr<PeerInfoItemProvider> peerInfoItem, boost::shared_ptr<UDPPeerConnectionInfo> connInfo, 
		const PeerHandshakeInfo& handshakeInfo, boost::shared_ptr<UDPConnectionPacketSender> packetSender );

};



#endif
