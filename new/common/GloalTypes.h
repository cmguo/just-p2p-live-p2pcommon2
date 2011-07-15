
#ifndef _LIVE_P2PCOMMON2_NEW_COMMON_GLOAL_TYPES_H_
#define _LIVE_P2PCOMMON2_NEW_COMMON_GLOAL_TYPES_H_

#include <synacast/protocol/data/PeerMinMax.h>
#include <synacast/protocol/data/PeerStatus.h>
#include <synacast/protocol/data/TrackerAddress.h>
#include <synacast/protocol/data/NetInfo.h>
#include <ppl/data/tstring.h>
#include <ppl/util/time_counter.h>

/**
 * @file
 * @brief ����һЩ�����Ľṹ��Ķ���(��LIVE_INFO)
 */


/// tracker is ok
const BYTE TRACKER_ERROR_SUCCESS = 0;
/// Ƶ��������
const BYTE TRACKER_ERROR_NO_CHANNEL = 1;
/// ��������
const BYTE TRACKER_ERROR_ZONE_DENIED = 2;

/// ���ڵ�peer�汾
const BYTE TRACKER_ERROR_OBSOLETE_PEER_VERSION = 3;

enum ExtraProxyEnum
{
	EXTRA_PROXY_NONE = 0, 
	EXTRA_PROXY_PEER = 1, 
	EXTRA_PROXY_NTS = 2, 
};



#pragma pack(push, 1)


/// live_param�ж�tracker�б������չ�Ĳ���
struct LIVE_PARAM_TRACKER_EXTENSION
{
	///��չ�ġ�Tracker�б��С
	DWORD ExTrackerCount;
	///��չ�ġ�Tracker�б�
	TRACKERADDR ExTrackerList[64];
};


/// ���ڸ���tracker�б�
struct PARAM_UPDATE_TRACKERS
{
	GUID ChannelGUID;
	DWORD TrackerCount;
	TRACKERADDR Trackers[1];
};

#pragma pack(pop)


//#pragma pack(push, 1)

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//           ������һЩ �����Ľṹ
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


/// ���ز��ŵ�һЩͳ������
struct LOCAL_STATISTICS
{
	/// ��Դ��Χ����Сֵ
	UINT32	MinIndex;

	/// ��Դ��Χ�����ֵ
	UINT32	MaxIndex;

	///Skipָ�� = MinIndex + BufferSize
	UINT32	SkipIndex;

	/// ��ǰ���ŵ���λ��
	UINT32	PlayToIndex;

	///��ʼ���ص�Piece���
	UINT32  StartIndex;

	///�Ѿ����ص�Piece����
	UINT32	TotalPushedCount;

	///�������Ĵ�С
	UINT32	BufferSize;

	///���������Ѿ����صĸ���
	UINT32	PieceCount;

	///��������ʱ�䳤�ȣ��Ժ������
	UINT32	BufferTime;

	/// ���ṹ������
	void Clear()
	{
		FILL_ZERO(*this);
	}
};




/// ������Ϣ�������ܵ��ֽ��������5���ڸ�����ֽ���
struct FLOW_INFO
{
	enum { RESERVED_UNITS = 4 };
	UINT32	Total;
	UINT32	Speed;
	UINT32	Reserved[RESERVED_UNITS];

	void SetFlow(UINT total, UINT speed)
	{
		SetSingleFlow(total, speed);
		for (int i = 0; i < RESERVED_UNITS; ++i)
		{
			this->Reserved[i] = speed;
		}
	}
	void SetSingleFlow(UINT total, UINT speed)
	{
		this->Total = total;
		this->Speed = speed;
	}
};

/// ����ͳ�����ݣ��������غ��ϴ�������
struct FLOW_STATISTICS
{
	/// ����������Ϣ
	FLOW_INFO	Download;

	/// ����������Ϣ
	FLOW_INFO	Upload;

	/// ƽ��qos
	UINT16 AverageQos;

	/// ƽ����֡��
	UINT8 AverageSkipPercent;

	/// �����ֶ�2
	UINT8 Reserved2;

	/// ��ʼ��TickCount����
	UINT32 StartTickCount;

	/// ���ü�¼��Ϣ
	void Reset()
	{
		FILL_ZERO(*this);
		StartTickCount = time_counter::get_system_count32();
	}

	UINT GetAverageDownloadSpeed() const
	{
		DWORD usedTime = time_counter::get_system_count32() - this->StartTickCount;
		return (UINT) ( (usedTime == 0) ? 0 : this->Download.Total * 1000.0 / usedTime );
	}

	UINT GetAverageUploadSpeed() const
	{
		DWORD usedTime = time_counter::get_system_count32() - this->StartTickCount;
		return (UINT) ( (usedTime == 0) ? 0 : this->Upload.Total * 1000.0 / usedTime );
	}

	UINT GetRecentDownloadSpeed() const { return this->Download.Speed; }
	UINT GetRecentUploadSpeed() const { return this->Upload.Speed; }
};



/// �������Ĵ�С
const int MAX_DATA_REQUESTS = 8;


/// ������PEER��Ϣ
struct PEER_INFORMATION
{
	/// ���ڱ�ʶ��Peer��ȫ��Ψһ��ID
	GUID PeerGUID;

	/// peer������Ϣ
	PEER_NET_INFO NetInfo;

	/// �ⲿ������Peer��IP
	UINT32 DetectedIP;

	/// ״̬��Ϣ
	PEER_STATUS_INFO StatusInfo;

	/// ������Ϣ
	FLOW_STATISTICS Flow;

	/// ��ǰʣ������ֵ
	UINT32 CurrentLeftQuota;

	/// �������ֵ
	UINT32 MaxQuota;

	/// ʧ�ܴ���
	UINT32 FailedCount;

	/// �������ص�piece����������
	UINT32 DownloadingPieces[MAX_DATA_REQUESTS];

	/// peer�İ汾��
	//UINT32 AppVersion;
};

/// PEER_INFORMATION�ķ�װ��
class CPeerInfo : public PEER_INFORMATION
{
public:
	CPeerInfo()
	{
		Clear();
	}

	void Clear()
	{
		FILL_ZERO(*this);
		Flow.Reset();
	}


};


/// ��ϸ��PEER��Ϣ
struct PEER_DETAILED_INFO : public PEER_INFORMATION
{
	///P2P״̬��0��δ��ʼ 1����¼TRACKER�� 2����¼TRACKER�ɹ�
	WORD LIVE_STATE;

	///������״̬
	WORD BUF_STATE;

	///��֡״̬
	WORD SKIP_STATE;
	WORD Reserved;

	/// peer�Ļ�����ͳ������
	LOCAL_STATISTICS Statistics;
};

#pragma pack(push, 1)

/// Ƶ����Ϣ
struct LIVE_CHANNEL_INFO 
{
	///��Tracker��õ�SourceMinMax
	PEER_MINMAX	SourceMinMax;
	///��Tracker��õ�SourceMinMaxʱ��ʱ��
	DWORD	GetSourceMinMaxTime;

	DWORD BaseIndex;
	DWORD BaseTimeStamp;
	DWORD MinTimeStamp;
	DWORD MaxTimeStamp;
	DWORD SkipTimeStamp;

	UINT FirstErrorPiece;
	UINT FirstErrorPieceTimeStamp;

	WORD DetectIndexSize;
	WORD ConnectIndexSize;
	WORD ActiveIndexSize;

	WORD ConnectionPoolSize;
	WORD UnconnectablePoolSize;

	WORD OnceHelloCount;

	WORD HandshakingPeerCount;

	WORD AutoMaxAppPeerCount;

	WORD DetectCheckCount;
	WORD ConnectCheckCount;
	WORD DeleteCheckCount;

	DEGREE_PAIR TCPDegree;
	DEGREE_PAIR UDPTDegree;

	WORD RealOnceHelloCount;

	UINT TotalReceivedSubPieceCount;
	UINT TotalReceivedUnusedSubPieceCount;

	WORD RealOnceDetectCount;

	WORD HelloIndexSize;

	WORD IPPoolDetectingCount;

	WORD Reserved0;


	PEER_MINMAX UndelayedSourceMinMax;
	

	/// �ڲ���¼�Ƿ�p2pģ���ǻ��
	UINT32 InternalAliveRecord;

	UINT64 TrackerDownloadBytes;
	UINT64 DetectDownloadBytes;
	UINT64 OtherUDPDownloadBytes;
	UINT64 TotalDownloadBytes;
	UINT64 TotalUploadBytes;

	UINT64 TotalDownloadMediaBytes;
	UINT64 TotalUploadMediaBytes;


	UINT TotalRequestedSubPieceCount;
	UINT TotalSubPieceRequestsReceived;
	UINT TotalSubPiecesUploaded;

	//UINT32 LeaveReasons[5];
	/// �����ֶΣ����ڲ�����ʹ��
	char Reserved[104];
};

#pragma pack(pop)

/// IP Pool�е�ͳ������
struct IP_POOL_INFO 
{
//	DWORD	Wait_for_Connect_Count;		//�ȴ�̽���IP����
//	DWORD	Succed_detected_Count;		//̽��ɹ���IP����

	/// ��ѡpeer�ĸ���
	WORD	CandidatePoolSize;
	/// ��̽��peer�ĸ���
	WORD	DetectedPoolSize;
	/// ����peer�ĸ���
	WORD	TotalPoolSize;
	/// ���ڽ��е����Ӹ���(��ʱ��������)
	WORD	PendingConnectionCount;
};


/// PEER_DETAILED_INFO�ķ�װ��
class CDetailedPeerInfo : public PEER_DETAILED_INFO
{
public:
	CDetailedPeerInfo()
	{
		FILL_ZERO(*this);
		Flow.Reset();
		// ��ʼ����֡��Ϊ100
		StatusInfo.Status.SkipPercent = 100;
		StatusInfo.Status.DegreeLeft = 60;
		StatusInfo.Status.UploadBWLeft = 1000;
	}

};


/// PEER_INFO��Peer��״̬
enum PeerInfoState
{
	pis_unused = 0, 
	pis_used = 2, 
};

/// ����һ��PEER��PEER_INFO Slot
struct PEER_INFO_ITEM
{
	/// PEER��Ϣ��
	CPeerInfo PeerInfo;

	/// ��slot��״̬��ΪPeerInfoState(unused, connecting, connected)
	UINT8 State : 2;
	UINT8 Reserved : 5;
	/// �Ƿ�ͨ��nat��Խ
	UINT8 IsThroughNAT : 1;

	/// IsInitFromRemote�������Ƿ���Զ�̷���
	bool BaseConnectionFlags;

	/// IsVIP���Ƿ�ΪVIP����
	UINT8 ConnectionFlags;

	UINT8 ConnectionType : 4;
	UINT8 NATType : 4;
};

/// PEER_INFO_ITEM�ķ�װ��
class CPeerInfoItem : public PEER_INFO_ITEM
{
public:
	// connection flags
	enum { CF_INIT_FROM_REMOTE = 0x01 };
	enum { CF_VIP = 0x01, CF_MULTI_CONNECTION = 0x02, CF_INNER = 0x04, CF_LAN = 0x08 };

	/// �Ƿ��ѱ�ռ��
	bool IsUnused() const { return this->State == pis_unused; }

	/// ����ʹ�������Ϣ��
/*	void Acquire(bool isInitFromRemote, bool isVip)
	{
		assert(IsUnused());
		this->State = pis_used;
		this->IsInitFromRemote = isInitFromRemote;
		this->ConnectionFlags = isVip;
		this->PeerInfo.Clear();
	}*/
	
	void Acquire()
	{
		assert(IsUnused());
		this->State = pis_used;
		this->BaseConnectionFlags = 0;
		this->ConnectionFlags = 0;
		this->NATType = 0;
		this->Reserved = 0;
		this->IsThroughNAT = 0;
		this->PeerInfo.Clear();
	}

	void Init( bool isInitFromRemote, bool isVIP, bool isInner, bool isLAN, UINT8 connectionType, UINT8 natType, bool throughNAT )
	{
		assert( connectionType < 16 );
		this->BaseConnectionFlags = 0;
		this->ConnectionFlags = 0;
		this->ConnectionType = connectionType;
		this->NATType = natType;
		this->IsThroughNAT = throughNAT;
		if ( isVIP )
			this->ConnectionFlags |= CF_VIP;
		if ( isInner )
			this->ConnectionFlags |= CF_INNER;
		if ( isLAN )
			this->ConnectionFlags |= CF_LAN;
		if ( isInitFromRemote )
			this->BaseConnectionFlags |= CF_INIT_FROM_REMOTE;
	}

	bool IsVIP() const
	{
		return (this->ConnectionFlags & CF_VIP) != 0;
	}
	bool IsInner() const
	{
		return (this->ConnectionFlags & CF_INNER) != 0;
	}
	bool IsLAN() const
	{
		return (this->ConnectionFlags & CF_LAN) != 0;
	}
	bool IsInitFromRemote() const
	{
		return (this->BaseConnectionFlags & CF_INIT_FROM_REMOTE) != 0;
	}

	/// �ͷ������Ϣ��
	void Release()
	{
		assert(!IsUnused());
		this->State = pis_unused;
		this->BaseConnectionFlags = 0;
		this->ConnectionFlags = 0;
		this->ConnectionType = 0;
		this->NATType = 0;
		this->Reserved = 0;
		this->IsThroughNAT = 0;
		this->PeerInfo.Clear();
	}
};



/// ��С��peer��������
const size_t PPL_MIN_PEER_COUNT_LIMIT = 256;

/// ���Peer����������
const size_t PPL_MAX_PEER_COUNT_LIMIT = 1024 * 8;


/// ���tracker����
const WORD MAX_TRACKER_CLIENT_COUNT	= 16;
const WORD TRACKER_CLIENT_COUNT_LIMIT	= 80;

enum TransferMethodEnum
{
	/// Ĭ�ϣ���ȫ������������ȫ���䣬 ��UDP��TCP��TCP80 �ķ��� ����ȫ�������� UDP �鷺 �� UDP ̽��
	TRANSFER_ALL = 0, 

	/// UDP���䷽���� ֻ����UDP����	�� UDP �鷺 �� UDP ̽��
	TRANSFER_UDP = 1, 

	/// TCP���䷽���� ֻ����TCP���䣬��TCP��TCP80	�� UDP �鷺 �� UDP ̽��
	TRANSFER_TCP = 2, 

	///  ��ֹ����·�������������� ֻ����TCP���䣬��TCP��TCP80,���� UDP �鷺 ���� Ҳ���� UDP ̽��
	TRANSFER_NO_DETECT = 3, 
};


/// ������Ƶֱ��Ӧ��ģ��Ĺ����ڴ�ṹ��
struct LIVE_INFO
{
	/// ��Ϣ״̬����AppӦ�ý���ʱ����ΪFFFF������ģ�鷢�ָ�ֵ��ΪFFFFʱ��Ҳ��������
	WORD	APP_STATE;

	/// ˵����Ϣ(ע��������WORD����Ϊ16�ֽ�)
	char Comment[14];

	/// û���ӳٵ�source��minmax
	PEER_MINMAX UndelayedSourceMinMax;

	/// ������
	UINT BitRate;
	/// ����ȡ�ٶ�
	UINT StreamReadingSpeed;


	/// ���ʵ�tracker���������ܸ���
	UINT16 TotalTrackerCount;
	BYTE TrackerErrorCode;
	BYTE Reserved2;
	char CommentReserved[80];

	/// ͬʱ�������ӵĲ�������Ϊ�ܵ�ͬʱ�����������
	WORD MaxConnectingCount;

	/** 
	 * @brief p2p���ݴ���ķ�ʽ
	 * 0 - All
	 * 1 - UDP
	 * 2 - TCP
	 */
	BYTE TransferMethod;

	/// ���÷�����
	bool EnableProtectDisconnect;

	/// �������ӿ���
	bool IntelligentConnectionControlDisabled;

	/**
	 * @brief Graphʹ�÷�ʽ
	 * 0 -  ѡ��1 (�����渽¼)
	 * 1 -  ѡ��2 (�����渽¼)
	 * 2 -  ѡ��3 (�����渽¼)
     * �ں�һ���������������ֵ���Ϳ����ж���ʲô��ʽ�����������
	 */
	BYTE GraphMode;

	/// ���ڲ���
	WORD Reserved;

	/// ��pfps���ص�������(��λ���ֽ�/��)
	DWORD MaxPFPSBandWidth;

	/// ���ڸ����潻���Ĵ��ھ��
	HWND NotifyWindow;

	///Ӧ������
	int		AppType;

	///��ԴGUID
	GUID	ResourceGUID;

	///Ƶ��ID�ţ�����ԴGUID��Ӧ
	WORD	Channel_ID;
	///ICP��ţ���������������
	WORD	ICP_ID;
	///�û���ICP�����û���
	char	ICPUSER_ID[32];
	///�û�״̬��ICP����
	WORD	ICPUSER_STATE;
	WORD	Reserved3;
	///cooikeֵ
	char	Cookie[32];
	
	char	Username[64];
	char	Password[64];

	/// �ϴι۲��ʱ��
	DWORD	ObservTime;
	/// ����ʹ�õĶ˿�
	WORD	PlayPort;
	/// ý�����ͣ�0: NULL 1: Windows Media; 2: Real Media
	WORD	MediaType;

	/// Ƶ����Ϣ
	LIVE_CHANNEL_INFO	ChannelInfo;

	/// ����Peer��Ϣ(����Peer�Ļ�����Ϣ�ͱ��ػ�������ģ�������)
	CDetailedPeerInfo	LocalPeerInfo;

	/// ����IPPool��Ϣ
	IP_POOL_INFO		IPPoolInfo;

	/// Tracker��ַ����
	BYTE		TrackerCount;
	/// ��ǰ��¼��Tracker������
	BYTE		CurrentTracker;
	/// �Ƿ�����TCP Tracker
	BYTE		UseTCPTracker;
	BYTE		TrackerReserved;
	/// Tracker��ַ�б�������ַ��״̬
	TRACKER_LOGIN_ADDRESS Trackers[MAX_TRACKER_CLIENT_COUNT];

	/// ��ǰPEER�ĸ���
	WORD	RemotePeerCount;
	/// ���ֳɹ���Peer����
	WORD	HandshakingPeerCount;
	/// Peer��Ϣ�������
	CPeerInfoItem	Peers[PPL_MIN_PEER_COUNT_LIMIT];

//	DWORD	HuntIndex;
//	DWORD	SkipIndex;
//	DWORD	LocationIndex;
//	DWORD	StartIndex;
};

//#pragma pack(pop)

/// SYSINFO�ķ�װ��
class CSysInfo : public SYSINFO
{
public:
	/// ��ȡ��Ҫ�㱨��ȥ��TCP�˿�
//	WORD GetOuterTcpPort() const { return (ExtenalTcpPort > 0) ? ExtenalTcpPort : TcpPort; }

	/// ��ȡ��Ҫ�㱨��ȥ��UDP�˿�
//	WORD GetOuterUdpPort() const { return (ExtenalUdpPort > 0) ? ExtenalUdpPort : UdpPort; }

	/// ���upnp�Ƿ�����
//	bool IsUPNPEnabled() const { return this->ExtenalTcpPort > 0 || this->ExtenalUdpPort > 0; }
};


class IPPoolStatistics;
class StreamBufferStatistics;
class UDPDetectorStatistics;
class LiveAppModuleCreateParam;

/// LIVE_INFO�ķ�װ��
class CLiveInfo : public LIVE_INFO
{
public:
	void Init(const LiveAppModuleCreateParam& param, UINT appVersion);
	void SyncStreamBufferStatistics(const StreamBufferStatistics& statistics);
	void SyncIPPoolStatistics(const IPPoolStatistics& statistics);
	void SyncUDPDetectorStatistics(const UDPDetectorStatistics& statistics);

	void SyncSourceMinMax(const PEER_MINMAX& sourceMinMax);

	/// Ѱ��δʹ�õĿ�����
	CPeerInfoItem* FindUnusedItem()
	{
		for (unsigned int i = 0; i < PPL_MIN_PEER_COUNT_LIMIT; ++i)
		{
			if (Peers[i].IsUnused())
				return &Peers[i];
		}
		return NULL;
	}
};

class PeerAuthInfo;
void InitPeerAuthInfo(PeerAuthInfo& authInfo, const LiveAppModuleCreateParam& param);






class RateMeasure;
class FlowMeasure;


/// ��������Ϣ���浱�����ڴ����
void SaveFlowInfo(FLOW_INFO& flowInfo, const RateMeasure& measure);
/// ��������Ϣ���浱�����ڴ����
void SaveFlowInfo(FLOW_STATISTICS& flowInfo, const FlowMeasure& measure);

void SaveLocalFlowInfo(FLOW_STATISTICS& flowInfo, const FlowMeasure& measure);




class MediaServerListener;

//#if defined(_PPL_PLATFORM_LINUX) || defined(_PPL_USE_ASIO)
//MediaServerListener* CreateMediaServerListener(int netWriterMode);
//#elif defined(_PPL_PLATFORM_MSWIN)
MediaServerListener* CreateMediaServerListener(const CLiveInfo& liveInfo);
//#endif

class PeerNetInfo;
PeerNetInfo* CreatePeerNetInfo(const SYSINFO& sysInfo);


/*
/// peer���ͣ���Ϊ��ͨpeer��Source/SuperNode��peer
enum PeerTypeEnum
{
	NORMAL_PEER = 0, 
	SOURCE_PEER = 1, 
	MDS_PEER = 2, 
	MAS_PEER = 3, 
	PUBLISHER_PEER = 4,
};


inline bool IsNormalPeer(const PEER_CORE_INFO& coreInfo)
{
	return coreInfo.PeerType == NORMAL_PEER;
}
*/


#ifdef NEED_LOG

void LoadLogSetting(const tstring& baseDir);

#else

void LoadLogSetting(const tstring& /*baseDir*/);

#endif

#endif
