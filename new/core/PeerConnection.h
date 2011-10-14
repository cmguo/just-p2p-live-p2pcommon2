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


/// Request���µ����س�ʱ��Ϣ���������������10���ڵõ���Ӧ
//const UINT32 REQUEST_MAX_TIMEOUT		= 1*1000;

/// �����ӵ�������
//const double PEER_SLOW_QUOTA = 0.1;

/// ����Request���µ����س�ʱ
//const UINT32 URGENT_REQUEST_MAX_TIMEOUT	= 3000;

/// ������ֵ
//const double CONST_MAX_QUOTA = 20.0;


/// Peer Announce������bitmapǰ50����Ҫ���ԣ���Ϊ���п������������ʱ���ɾ������
//const UINT32 MAYBE_FAILED_PIECE_COUNT	= 30;


/// ��������ʱ֪ͨ����
const UINT WM_SEND_REQUEST = WM_USER + 2001;

/// ����ʱʱ֪ͨ����
const UINT WM_REQUEST_TIMEOUT = WM_USER + 2002;

/// �յ�����ʱ֪ͨ����
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


/// piece�������Ϣ
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
/// ����һ��peer-peer�����ӣ�Ҳ������Ϊ�Ǵ���һ��Զ�˵�Peer
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

	/// ����echo
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

	/// �Ƿ���Զ�˷�������
	bool IsInitFromRemote() const
	{
		LIVE_ASSERT( reinterpret_cast<UINT>( m_ConnectionInfo.get() ) != 0xFFFFFFFF );
		return m_ConnectionInfo->IsInitFromRemote;
	}

	/// �Ƿ�Ϊ�̶������ӣ����̶������Ӳ��ᱻPeerManager�޳������ߺ���Զ�����
	bool IsVIP() const { return m_ConnectionInfo->ConnectParam.IsVIP; }

	/// �Ƿ�������ip
	bool IsInnerIP() const { return m_IsInner; }

	/// �Ƿ��Ǿ������ڲ�����
	bool IsLANConnection() const { return m_IsLANConnection; }

	bool IsUPNP() const { return m_IsUPNP; }

	virtual bool HandleUDPSessionPacket(data_input_stream& is, const NEW_UDP_PACKET_HEAD& head, const UDP_SESSION_INFO& sessionInfo, const SimpleSocketAddress& sockAddr);

	/// ��ȡPeerInfo�д洢�ĵ�ַ��Ϣ(����IP�Ͷ˿�)
	//const PEER_ADDRESS& GetPeerAddress() const { return m_HandshakeInfo.Address; }

	/// ��ȡ��peer��socket��ַ
	const SimpleSocketAddress& GetSocketAddress() const { return m_SocketAddress; }

	/// ��ȡIPPool�������������ĵ�ַ
	const PEER_ADDRESS GetKeyAddress() const { return m_ConnectionInfo->KeyPeerAddress; }

	/// ��ȡ�����ڴ���
	const CPeerInfo& GetPeerInfo() const { return m_PeerInfo; }

	/// ��ȡpeer��guid
	const GUID& GetPeerGUID() const { return m_HandshakeInfo.PeerGUID; }

	/// ��peer�Ƿ���ָ����piece
	bool HasPiece(UINT index) const
	{
		if (index < GetMinIndex())
			return false;
		else
			return m_PeerResource[index];
	}

	/// ��ȡRelocateʱ�� ��Դ�ܼ���λ��
	UINT GetDenseMinIndex() const;
	
	/// ��ȡPrelocateʱ�� ��Դ�ܼ���λ��
	UINT GetPrelocateDenseMinIndex() const;

	/// ��ȡ��Դ��Χ
	UINT GetResourceRange() const
	{
		UINT minIndex = GetMinIndex();
		UINT maxIndex = GetMaxIndex();
		LIVE_ASSERT(maxIndex >= minIndex);
		return maxIndex - minIndex;
	}

	/// ��ȡ��Դ����ʼ��
	UINT GetMinIndex() const;
	UINT GetRealMinIndex() const
	{
		return m_PeerResource.GetMinIndex();
	}
	const BitMap& GetRealBitmap() const
	{
		return m_PeerResource;
	}
	/// ��ȡ��Դ�������
	UINT GetMaxIndex() const
	{
		if (m_PeerResource.GetSize() == 0)
			return 0;
		else
			return m_PeerResource.GetMaxIndex();
	}

	/// �Ƿ������source
	bool MaybeSource() const { return m_MaybeSource; }

	/// ��Դ״���Ƿ�Ϻã�����Դ�����Ƿ����SourceMinMax��һ��
	bool IsResourceGood() const { return m_IsResourceGood; }

	const PeerHandshakeInfo& GetHandshakeInfo() const { return m_HandshakeInfo; }

	/// ����pieceÿ�ҵ��Ĵ�����
	void SendPieceNotFound(UINT32 pieceIndex);
	void SendSubPieceNotFound(UINT32 pieceIndex, UINT8 subPieceIndex);
	PeerConnectionTypeEnum GetConnectionType() const { return m_ConnectionType; }

public:
	/// ���ڣ�����false��ʾPeerConnection��ɾ����
	bool OnAppTimer(UINT seconds);

	/// ��ȡ��peer�ķ�������
	UINT GetQOS() const;

	/// ��ȡ������Ϣ
	const FlowMeasure& GetFlow() const { return m_Flow; }

	/// ��ȡƽ�����ϴ��ٶ�
	UINT GetAverageUploadSpeed() const { return m_Flow.Upload.GetAverageRate(); }


	/// �Ͽ�����
	void Disconnect( UINT16 leaveReason, const serializable* errorInfo );

	/// ��ȡ���ֳɹ���ʱ��
	const time_counter& GetStartTime() const { return m_StartTime; }

	/// ��ȡ���ֳɹ������ڵ�ʱ��
	UINT64 GetConnectionTime() const { return m_StartTime.elapsed(); }

	/// ��ȡ���һ���յ����ĵ�ʱ��
	UINT64 GetLastReceivePacketTime() const { return m_LastTimeRecvPacket.elapsed(); }

	/// ����header
	bool RequestHeaderPiece(UINT32 index, UINT timeout);

	/// ����announce����
	void Announce(const PPHuffmanAnnounce* huffmanAnnouncePacket);
	
	/// ����RequestByTS���ģ�RequestSubPiece��
	void RequestFirstByTS(UINT64 inTS);

	/// ��¼��������
	void RecordDownloadFlow(size_t size, UINT8 action);

	/// ��¼�ϴ�����
	void RecordUploadFlow(size_t size, UINT8 action);

	UINT16 HandleOldPacket(data_input_stream& is, UINT8 action);

	const PeerConnectionInfo& GetConnectionInfo() const { return *m_ConnectionInfo; }

protected:
	/// �����Դ״��
	void CheckResourceStatus();

	/// ��¼ʵ��ʹ�õ�ʱ��
	void RecordRealUsedTime();

	/// ���ɴ�����
	void DispatchPacket(const BYTE* buf, int size);

	///��������ͳ����Ϣ
	void UpdateDownloadingInfo();

	/// ������㷢��ԭʼ����
	size_t DoSendPacket(const PacketBase& packet);

	/// ������㷢��ԭʼ����(packet��ֻ����������)
	bool SendPacket(const PacketBase& packet);

	//bool DoRequestOnePiece(UINT32 pieceIndex);


	bool DoSendError( UINT16 errcode, const serializable* errorInfo );

protected:
	/// �رմ����ӣ�reaasonΪ�رյ�ԭ��
	/// ע�⣬��ǰ����PeerManager����Ϣ֪ͨPeerManager��ʱ��ɾ��peer���ָ�Ϊֱ��ɾ�����Լ��ٶ���Ϣ���е�����(2007-04-06 chenmp)
	void DoClose(UINT16 reason);

	/// ����Ƿ�ʱ��û���հ�
	bool CheckLongIdle();

	/// ��ʼ��������Ϣ
	void InitConnectionData();

	/// ����minmax
	UINT16 UpdateMinMax();

	/// ���ڵ��ý���������ͳ��
	void UpdateFlowStatics();


	/************************************************************************/
	/* �ڲ���Ϣ������ӳ�亯��                                   */
	/************************************************************************/
protected:
	UINT16 HandleHuffmanAnnounce(data_input_stream& is);
	UINT16 HandleError(data_input_stream& is);
	UINT16 HandleSubPieceData(data_input_stream& is);

	UINT16 HandleSubPieceRequest(data_input_stream& is);
	UINT16 HandlePeerExchange(data_input_stream& is);

	/// ��鷢�ͱ��ĵ������������Ƿ���Ч
	bool IsValidPeerConnectionActionForSend(UINT8 action);

	
	PeerConnectionTypeEnum SelectConnectionType( PeerHandshakeInfo & m_HandshakeInfo ) const;

protected:
	/// ������PeerManager
	CPeerManager& m_PeerManager;

	/// ������AppModule
	AppModule& m_PeerModule;

	/// ���ݻ�����
	const Storage& m_Storage;

	/// ip��
	CIPPool& m_IPPool;
	Downloader& m_Downloader;
	Uploader& m_Uploader;

	boost::shared_ptr<PeerNetInfo> m_NetInfo;

	boost::shared_ptr<const PeerStatusInfo> m_StatusInfo;
	boost::shared_ptr<const SourceResource> m_SourceResource;

	boost::shared_ptr<PeerInfoItemProvider> m_PeerInfoItemProvider;

	/// ������Ϣ��
	CDetailedPeerInfo& m_LocalInfo;

	/// �Է��Ļ�����Ϣ��ͳ����Ϣ
	CPeerInfoItem& m_ItemPeerInfo;

	/// peer��Ϣ��
	CPeerInfo& m_PeerInfo;

	/// ������Ϣ
	boost::shared_ptr<PeerConnectionInfo> m_ConnectionInfo;

	/// ��������
	FlowMeasure m_Flow;
	FlowMeasure m_LongTimeFlow;

	/// peer��ʶ
	//GUID m_peerGUID;

	SimpleSocketAddress m_SocketAddress;

	//PEER_ADDRESS m_KeyPeerAddress;

	boost::scoped_ptr<PeerChannel> m_Channel;

	DEGREE_INFO m_Degrees;

	PeerTunnelPtr m_Tunnel;

	PeerHandshakeInfo m_HandshakeInfo;

	/// ���һ���հ���ʱ��
	time_counter m_LastTimeRecvPacket;

	/// ���ֳɹ���ʱ��
	time_counter m_StartTime;

	/// ���һ��announce��ʱ��
	time_counter m_LastAnnounceTime;

	/// �Ƿ��Ѿ����ù�Close
	bool m_IsClosed;

	/// �Ƿ���udpt��������
	bool m_IsUDP;

	// ����ģ��
	PeerConnectionTypeEnum m_ConnectionType;

	/// �Ƿ��Ǿ������ڲ�������
	bool m_IsLANConnection;

	/// �Ƿ���upnp�������ڵ�
	bool m_IsUPNP;

	/// peer�Ƿ��������ڵ�
	bool m_IsInner;

	/// �Ƿ������source
	bool m_MaybeSource;

	/// ��Դ״���Ƿ��Ǻõģ�����Դ�����Ƿ����SourceMinMax��һ��
	bool m_IsResourceGood;


	/// ��peer����Դλͼ
	BitMap m_PeerResource;

	UINT m_receivedSubPieceCount;
	UINT m_receiveUnusedSubPieceCount;
	size_t m_TotalRequestedSubPieces;
	UINT m_receiveUnusedDataSubPieceCount;

	/// ÿ����������ʱ�ۻ���announce����
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
