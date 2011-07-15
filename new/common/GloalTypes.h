
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
 * @brief 包含一些基本的结构体的定义(如LIVE_INFO)
 */


/// tracker is ok
const BYTE TRACKER_ERROR_SUCCESS = 0;
/// 频道不存在
const BYTE TRACKER_ERROR_NO_CHANNEL = 1;
/// 地域限制
const BYTE TRACKER_ERROR_ZONE_DENIED = 2;

/// 过期的peer版本
const BYTE TRACKER_ERROR_OBSOLETE_PEER_VERSION = 3;

enum ExtraProxyEnum
{
	EXTRA_PROXY_NONE = 0, 
	EXTRA_PROXY_PEER = 1, 
	EXTRA_PROXY_NTS = 2, 
};



#pragma pack(push, 1)


/// live_param中对tracker列表进行扩展的部分
struct LIVE_PARAM_TRACKER_EXTENSION
{
	///扩展的　Tracker列表大小
	DWORD ExTrackerCount;
	///扩展的　Tracker列表
	TRACKERADDR ExTrackerList[64];
};


/// 定期更新tracker列表
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
//           以下是一些 公共的结构
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


/// 本地播放的一些统计数据
struct LOCAL_STATISTICS
{
	/// 资源范围的最小值
	UINT32	MinIndex;

	/// 资源范围的最大值
	UINT32	MaxIndex;

	///Skip指针 = MinIndex + BufferSize
	UINT32	SkipIndex;

	/// 当前播放到的位置
	UINT32	PlayToIndex;

	///开始下载的Piece编号
	UINT32  StartIndex;

	///已经下载的Piece个数
	UINT32	TotalPushedCount;

	///缓冲区的大小
	UINT32	BufferSize;

	///缓冲区内已经下载的个数
	UINT32	PieceCount;

	///缓冲区的时间长度，以毫秒计算
	UINT32	BufferTime;

	/// 将结构体清零
	void Clear()
	{
		FILL_ZERO(*this);
	}
};




/// 流量信息，包含总的字节数和最近5秒内各秒的字节数
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

/// 流量统计数据，包括下载和上传的流量
struct FLOW_STATISTICS
{
	/// 下行流量信息
	FLOW_INFO	Download;

	/// 上行流量信息
	FLOW_INFO	Upload;

	/// 平均qos
	UINT16 AverageQos;

	/// 平均跳帧率
	UINT8 AverageSkipPercent;

	/// 保留字段2
	UINT8 Reserved2;

	/// 开始的TickCount计数
	UINT32 StartTickCount;

	/// 重置记录信息
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



/// 配额数组的大小
const int MAX_DATA_REQUESTS = 8;


/// 基本的PEER信息
struct PEER_INFORMATION
{
	/// 用于标识本Peer的全局唯一的ID
	GUID PeerGUID;

	/// peer网络信息
	PEER_NET_INFO NetInfo;

	/// 外部看到的Peer的IP
	UINT32 DetectedIP;

	/// 状态信息
	PEER_STATUS_INFO StatusInfo;

	/// 流量信息
	FLOW_STATISTICS Flow;

	/// 当前剩余的配额值
	UINT32 CurrentLeftQuota;

	/// 最大的配额值
	UINT32 MaxQuota;

	/// 失败次数
	UINT32 FailedCount;

	/// 正在下载的piece索引的数组
	UINT32 DownloadingPieces[MAX_DATA_REQUESTS];

	/// peer的版本号
	//UINT32 AppVersion;
};

/// PEER_INFORMATION的封装类
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


/// 详细的PEER信息
struct PEER_DETAILED_INFO : public PEER_INFORMATION
{
	///P2P状态：0：未开始 1：登录TRACKER中 2：登录TRACKER成功
	WORD LIVE_STATE;

	///缓冲区状态
	WORD BUF_STATE;

	///跳帧状态
	WORD SKIP_STATE;
	WORD Reserved;

	/// peer的缓冲区统计数据
	LOCAL_STATISTICS Statistics;
};

#pragma pack(push, 1)

/// 频道信息
struct LIVE_CHANNEL_INFO 
{
	///从Tracker获得的SourceMinMax
	PEER_MINMAX	SourceMinMax;
	///从Tracker获得的SourceMinMax时的时间
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
	

	/// 内部记录是否p2p模块是活动的
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
	/// 保留字段，供内部调试使用
	char Reserved[104];
};

#pragma pack(pop)

/// IP Pool中的统计数据
struct IP_POOL_INFO 
{
//	DWORD	Wait_for_Connect_Count;		//等待探测的IP个数
//	DWORD	Succed_detected_Count;		//探测成功的IP个数

	/// 候选peer的个数
	WORD	CandidatePoolSize;
	/// 已探测peer的个数
	WORD	DetectedPoolSize;
	/// 备用peer的个数
	WORD	TotalPoolSize;
	/// 正在进行的连接个数(暂时放在这里)
	WORD	PendingConnectionCount;
};


/// PEER_DETAILED_INFO的封装类
class CDetailedPeerInfo : public PEER_DETAILED_INFO
{
public:
	CDetailedPeerInfo()
	{
		FILL_ZERO(*this);
		Flow.Reset();
		// 初始的跳帧率为100
		StatusInfo.Status.SkipPercent = 100;
		StatusInfo.Status.DegreeLeft = 60;
		StatusInfo.Status.UploadBWLeft = 1000;
	}

};


/// PEER_INFO中Peer的状态
enum PeerInfoState
{
	pis_unused = 0, 
	pis_used = 2, 
};

/// 代表一个PEER的PEER_INFO Slot
struct PEER_INFO_ITEM
{
	/// PEER信息项
	CPeerInfo PeerInfo;

	/// 此slot的状态，为PeerInfoState(unused, connecting, connected)
	UINT8 State : 2;
	UINT8 Reserved : 5;
	/// 是否通过nat穿越
	UINT8 IsThroughNAT : 1;

	/// IsInitFromRemote，连接是否由远程发起
	bool BaseConnectionFlags;

	/// IsVIP，是否为VIP连接
	UINT8 ConnectionFlags;

	UINT8 ConnectionType : 4;
	UINT8 NATType : 4;
};

/// PEER_INFO_ITEM的封装类
class CPeerInfoItem : public PEER_INFO_ITEM
{
public:
	// connection flags
	enum { CF_INIT_FROM_REMOTE = 0x01 };
	enum { CF_VIP = 0x01, CF_MULTI_CONNECTION = 0x02, CF_INNER = 0x04, CF_LAN = 0x08 };

	/// 是否已被占用
	bool IsUnused() const { return this->State == pis_unused; }

	/// 申请使用这个信息项
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

	/// 释放这个信息项
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



/// 最小的peer个数限制
const size_t PPL_MIN_PEER_COUNT_LIMIT = 256;

/// 最大Peer个数的限制
const size_t PPL_MAX_PEER_COUNT_LIMIT = 1024 * 8;


/// 最大tracker个数
const WORD MAX_TRACKER_CLIENT_COUNT	= 16;
const WORD TRACKER_CLIENT_COUNT_LIMIT	= 80;

enum TransferMethodEnum
{
	/// 默认，完全方案，进行完全传输， 先UDP再TCP再TCP80 的方案 （完全方案）带 UDP 洪泛 和 UDP 探测
	TRANSFER_ALL = 0, 

	/// UDP传输方案， 只进行UDP传输	带 UDP 洪泛 和 UDP 探测
	TRANSFER_UDP = 1, 

	/// TCP传输方案， 只进行TCP传输，先TCP再TCP80	带 UDP 洪泛 和 UDP 探测
	TRANSFER_TCP = 2, 

	///  防止无线路由器断网方案， 只进行TCP传输，先TCP再TCP80,不带 UDP 洪泛 并且 也不带 UDP 探测
	TRANSFER_NO_DETECT = 3, 
};


/// 用于视频直播应用模块的共享内存结构体
struct LIVE_INFO
{
	/// 信息状态，当App应用结束时，置为FFFF，监视模块发现该值变为FFFF时，也结束监视
	WORD	APP_STATE;

	/// 说明信息(注意和上面的WORD补齐为16字节)
	char Comment[14];

	/// 没有延迟的source的minmax
	PEER_MINMAX UndelayedSourceMinMax;

	/// 码流率
	UINT BitRate;
	/// 流读取速度
	UINT StreamReadingSpeed;


	/// 访问的tracker服务器的总个数
	UINT16 TotalTrackerCount;
	BYTE TrackerErrorCode;
	BYTE Reserved2;
	char CommentReserved[80];

	/// 同时发起连接的参数，做为总的同时发起的连接数
	WORD MaxConnectingCount;

	/** 
	 * @brief p2p数据传输的方式
	 * 0 - All
	 * 1 - UDP
	 * 2 - TCP
	 */
	BYTE TransferMethod;

	/// 启用防断网
	bool EnableProtectDisconnect;

	/// 智能连接控制
	bool IntelligentConnectionControlDisabled;

	/**
	 * @brief Graph使用方式
	 * 0 -  选项1 (见下面附录)
	 * 1 -  选项2 (见下面附录)
	 * 2 -  选项3 (见下面附录)
     * 内核一启动，就设置这个值，就可以判断用什么方式的流来输出了
	 */
	BYTE GraphMode;

	/// 用于补齐
	WORD Reserved;

	/// 供pfps下载的最大带宽(单位：字节/秒)
	DWORD MaxPFPSBandWidth;

	/// 用于跟界面交互的窗口句柄
	HWND NotifyWindow;

	///应用类型
	int		AppType;

	///资源GUID
	GUID	ResourceGUID;

	///频道ID号，与资源GUID对应
	WORD	Channel_ID;
	///ICP编号，由启动参数传入
	WORD	ICP_ID;
	///用户在ICP处的用户名
	char	ICPUSER_ID[32];
	///用户状态：ICP定义
	WORD	ICPUSER_STATE;
	WORD	Reserved3;
	///cooike值
	char	Cookie[32];
	
	char	Username[64];
	char	Password[64];

	/// 上次观察的时间
	DWORD	ObservTime;
	/// 播放使用的端口
	WORD	PlayPort;
	/// 媒体类型：0: NULL 1: Windows Media; 2: Real Media
	WORD	MediaType;

	/// 频道信息
	LIVE_CHANNEL_INFO	ChannelInfo;

	/// 本地Peer信息(包含Peer的基本信息和本地缓冲区等模块的数据)
	CDetailedPeerInfo	LocalPeerInfo;

	/// 本地IPPool信息
	IP_POOL_INFO		IPPoolInfo;

	/// Tracker地址个数
	BYTE		TrackerCount;
	/// 当前登录的Tracker的索引
	BYTE		CurrentTracker;
	/// 是否启用TCP Tracker
	BYTE		UseTCPTracker;
	BYTE		TrackerReserved;
	/// Tracker地址列表，包含地址和状态
	TRACKER_LOGIN_ADDRESS Trackers[MAX_TRACKER_CLIENT_COUNT];

	/// 当前PEER的个数
	WORD	RemotePeerCount;
	/// 握手成功的Peer个数
	WORD	HandshakingPeerCount;
	/// Peer信息项的数组
	CPeerInfoItem	Peers[PPL_MIN_PEER_COUNT_LIMIT];

//	DWORD	HuntIndex;
//	DWORD	SkipIndex;
//	DWORD	LocationIndex;
//	DWORD	StartIndex;
};

//#pragma pack(pop)

/// SYSINFO的封装类
class CSysInfo : public SYSINFO
{
public:
	/// 获取需要汇报出去的TCP端口
//	WORD GetOuterTcpPort() const { return (ExtenalTcpPort > 0) ? ExtenalTcpPort : TcpPort; }

	/// 获取需要汇报出去的UDP端口
//	WORD GetOuterUdpPort() const { return (ExtenalUdpPort > 0) ? ExtenalUdpPort : UdpPort; }

	/// 检查upnp是否启用
//	bool IsUPNPEnabled() const { return this->ExtenalTcpPort > 0 || this->ExtenalUdpPort > 0; }
};


class IPPoolStatistics;
class StreamBufferStatistics;
class UDPDetectorStatistics;
class LiveAppModuleCreateParam;

/// LIVE_INFO的封装类
class CLiveInfo : public LIVE_INFO
{
public:
	void Init(const LiveAppModuleCreateParam& param, UINT appVersion);
	void SyncStreamBufferStatistics(const StreamBufferStatistics& statistics);
	void SyncIPPoolStatistics(const IPPoolStatistics& statistics);
	void SyncUDPDetectorStatistics(const UDPDetectorStatistics& statistics);

	void SyncSourceMinMax(const PEER_MINMAX& sourceMinMax);

	/// 寻找未使用的空闲项
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


/// 将速率信息保存当共享内存的中
void SaveFlowInfo(FLOW_INFO& flowInfo, const RateMeasure& measure);
/// 将流量信息保存当共享内存的中
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
/// peer类型，分为普通peer和Source/SuperNode的peer
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
