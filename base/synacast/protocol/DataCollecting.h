
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_COLLECTING_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_COLLECTING_H_

#include <synacast/protocol/data/PeerMinMax.h>
#include <synacast/protocol/data/PeerStatus.h>
#include <assert.h>
#include <ppl/util/time_counter.h>

enum DataCollectingType
{
	DCT_APP = 11,
	DCT_IPPOOL = 21,  
	DCT_DETECTOR = 31, 
	DCT_STREAMBUFFER = 41, 
	DCT_CRASHLOG = 51, 
	DCT_TRACKER = 61, 
	DCT_PEERMANAGER = 71, 
	DCT_CONNECTOR = 81, 
	DCT_DOWNLOADER = 91, 
	DCT_UPLOADER = 101, 
	DCT_MEDIASERVER = 111, 
};



struct DC_HEAD
{
	UINT16 DataType;
	UINT16 Reserved;
	UINT32 HeadExtraSize;
	UINT32 StructSize;

	void InitHead( DataCollectingType type, UINT32 dataSize )
	{
		this->DataType = static_cast<UINT16>( type );
		this->Reserved = 0;
		this->HeadExtraSize = 0;
		this->StructSize = dataSize;
	}
};

struct DC_CRASH_LOG : public DC_HEAD
{
	unsigned char Data[1];
};

struct SINGLE_RATE_COUNTER
{
	UINT32 StructSize;
	/// 总共数量
	UINT64 Total;
	/// 当前速率
	UINT32 Rate;
	/// 平均速率
	UINT32 AverageRate;
	/// 历史最高速率
	UINT32 MaxRate;

	void Clear()
	{
		FILL_ZERO( *this );
		this->StructSize = sizeof( *this );
	}
};

struct RATE_COUNTER
{
	UINT32 StructSize;

	SINGLE_RATE_COUNTER Bytes;
	SINGLE_RATE_COUNTER Packets;

	void Clear()
	{
		this->StructSize = sizeof( *this );
		this->Bytes.Clear();
		this->Packets.Clear();
	}
};


struct TRAFFIC_COUNTER
{
	RATE_COUNTER Upload;
	RATE_COUNTER Download;

	void Clear()
	{
		this->Upload.Clear();
		this->Download.Clear();
	}
};


struct SINGLE_REQUEST_COUNTER
{
	UINT32 StructSize;

	/// 发出的次数
	UINT64 SentCount;

	/// 收到的请求次数
	UINT64 ReceivedCount;

	/// 收到的无效报文的个数
	UINT64 InvalidCount;

	/// 发送失败的次数
	UINT64 SendFailedCount;

	void Clear()
	{
		FILL_ZERO( *this );
		this->StructSize = sizeof( *this );
	}
};



struct REQUEST_COUNTER
{
	UINT32 StructSize;

	SINGLE_REQUEST_COUNTER Request;
	SINGLE_REQUEST_COUNTER Response;

	void Clear()
	{
		this->StructSize = sizeof( *this );
		this->Request.Clear();
		this->Response.Clear();
	}
};


struct CODE2_COUNTER
{
	UINT16 Code;
	UINT32 Count;

	void Clear()
	{
		this->Code = 0;
		this->Count = 0;
	}
};

struct CODE4_COUNTER
{
	UINT32 Code;
	UINT32 Count;

	void Clear()
	{
		this->Code = 0;
		this->Count = 0;
	}
};

struct START_RECORD
{
	UINT32 DownloadSpeed;
	UINT32 ConnectionCount;

	void Clear()
	{
		this->DownloadSpeed = 0;
		this->ConnectionCount = 0;
	}
};

/// 开始20秒的统计数据
class START_STATS
{
public:
	// 不可以超过100
	enum { MAX_RECORDS = 20 };

	START_RECORD Records[MAX_RECORDS];

	void Clear()
	{
		for ( size_t index = 0; index < MAX_RECORDS; ++index )
		{
			this->Records[index].Clear();
		}
	};
};


struct DETECTOR_REQUEST_STATS
{
	UINT32 StructSize;

	REQUEST_COUNTER Packets;
	/// 流量
	TRAFFIC_COUNTER Traffic;

	void Clear()
	{
		this->StructSize = sizeof(*this);

		this->Packets.Clear();
		this->Traffic.Clear();
	}
};

struct DETECTOR_STATS : public DC_HEAD
{
	DETECTOR_REQUEST_STATS Detect;
	DETECTOR_REQUEST_STATS PeerExchange;

//	UINT64 IgnoredPeerExchangeTargets;

	UINT64 TotalReceivedExchangedPeerCount;
	UINT64 TotalSentExchangedPeerCount;

	UINT16 LastReceivedExchangedPeerCount;
	UINT16 LastSentExchangedPeerCount;

	UINT32 OnceHelloCount;
	UINT32 RealOnceHelloCount;
	UINT32 RealOnceDetectCount;

	void Clear()
	{
		FILL_ZERO( *this );
		this->InitHead( DCT_DETECTOR, sizeof( *this ) );

		this->Detect.Clear();
		this->PeerExchange.Clear();

//		this->IgnoredPeerExchangeTargets = 0;

		this->TotalReceivedExchangedPeerCount = 0;
		this->LastReceivedExchangedPeerCount = 0;

		this->TotalSentExchangedPeerCount = 0;
		this->LastSentExchangedPeerCount = 0;

		this->OnceHelloCount = 0;
		this->RealOnceHelloCount = 0;
		this->RealOnceDetectCount = 0;

	}

};


struct TRACKER_REQUEST_COUNTER
{
	UINT32 RequestTimes;
	UINT32 SucceededTimes;
	UINT32 FailedTimes;
};


struct TRACKER_STATS : public  DC_HEAD
{

	/// 报文流量和数量
	TRAFFIC_COUNTER Traffic;

	UINT32 SwitchClientTimes;

	TRACKER_REQUEST_COUNTER Register;
	TRACKER_REQUEST_COUNTER Join;
	TRACKER_REQUEST_COUNTER KeepAlive;
	TRACKER_REQUEST_COUNTER List;
	TRACKER_REQUEST_COUNTER TCPList;

	UINT32 LeaveTimes;


	/// 总共List到的peer个数
	UINT32 TotalListedPeers;
	UINT32 TCPTotalListedPeers;

	/// 上一次List到的peer个数
	UINT16 LastListedPeers;
	UINT16 TCPLastListedPeers;

	/// 总共List到的peer个数
	UINT32 TotalListedLANPeers;

	/// 上一次List到的peer个数
	UINT16 LastListedLANPeers;

	UINT32 TotalInvalidResponse;
	UINT32 TotalInvalid2Response;

	UINT32 TotalIgnoredResponse;


	/// join/keepalive/list失败的次数
	/// 切换tracker的次数
	/// tcp tracker的list次数和list到的peer个数

	void Clear()
	{
		FILL_ZERO( *this );
		this->InitHead( DCT_TRACKER, sizeof( *this ) );

		this->Traffic.Clear();
	}
};




struct APP_STATS : public  DC_HEAD
{

	/// PacketSender信息
	UINT32 StartTime;
	UINT32 CurrentTime;
	SYSTEMTIME StartDateTime;

	/// 总共的收包信息(下载)
	TRAFFIC_COUNTER TotalTraffic;


	/// 各种sender对应的流量
	TRAFFIC_COUNTER OldUDPTraffic; //? 跟TrackerFlow重复
	TRAFFIC_COUNTER TCPConnectionlessTraffic;
	TRAFFIC_COUNTER TCPConnectionTraffic;
	TRAFFIC_COUNTER UDPConnectionlessTraffic;
	TRAFFIC_COUNTER UDPConnectionTraffic;

	UINT64 TotalIgnoredPackets;
	UINT64 TotalInvalidPackets;

	/// unshuffle返回-1的报文
	UINT64 TotalTooShortPackets;

	/// 以0xe903开头的不识别的老报文
	UINT64 TotalOldInvalidPackets;

	UINT64 TotalSelfToSelfPackets;


	START_STATS StartData;

	void Clear()
	{
		FILL_ZERO( *this );
		this->StructSize = sizeof( *this );
		this->InitHead( DCT_APP, sizeof( *this ) );

		::GetLocalTime( &this->StartDateTime );
		this->StartTime = ppl::util::detail::GetTickCount();

		this->TotalTraffic.Clear();

		this->OldUDPTraffic.Clear();
		this->TCPConnectionlessTraffic.Clear();
		this->TCPConnectionTraffic.Clear();
		this->UDPConnectionlessTraffic.Clear();
		this->UDPConnectionTraffic.Clear();

		this->StartData.Clear();
	}

	UINT32 GetRunTime()
	{
		return this->CurrentTime - this->StartTime;
	}
};


struct UPLOADER_STATS : public DC_HEAD
{

	TRAFFIC_COUNTER Traffic;
	RATE_COUNTER MediaTraffic;

	/// 收到的请求数
	UINT64 TotalSubPieceRequestsReceived;
	/// 实际上传的subpiece数
	UINT64 TotalSubPiecesUploaded;
	/// 没有找到的subpiece数
	UINT64 TotalMissingSubPieces;
	/// 由于上传限速导致没有上传的subpiece数
	UINT64 TotalRefusedSubPieces;

	UINT32 MaxSubPieceUploadTimes;
	UINT32 MaxUploadSpeed;
	UINT32 CurrentUploadSpeed;

	void Clear()
	{
		FILL_ZERO( *this );
		this->StructSize = sizeof( *this );
		this->InitHead( DCT_UPLOADER, sizeof( *this ) );

		this->Traffic.Clear();
		this->MediaTraffic.Clear();
	}

};

struct DOWNLOADER_SUB_STATS
{
	UINT StructSize;

	UINT TotalRequestCount;
	UINT TotalRequestSuccedRate;
	UINT TotalWindowSize;

	UINT PeerTunnelCount;
	UINT UnusedTunnelCount;
	UINT HighConnectionCount;
	UINT DepletedTunnelCount;

	UINT MinUsedTime;
	UINT MaxUsedTime;

	UINT MinWindowSize;
	UINT MaxWindowSize;

	UINT MinRequestSuccedRate;
	UINT MaxRequestSuccedRate;

	UINT MaxTaskQueueSize;
	UINT MaxRequestCount;

	void Clear()
	{
		FILL_ZERO(*this);
		this->StructSize = sizeof(*this);
	}

	void ResetForTemp()
	{
		this->Clear();

		this->MinUsedTime = UINT_MAX;
		this->MinWindowSize = USHRT_MAX;

		this->MaxWindowSize = 0;
		this->MaxUsedTime = 0;

		this->MaxTaskQueueSize = 0;
		this->MaxRequestCount = 0;

		this->MinRequestSuccedRate = UINT_MAX;
		this->MaxRequestSuccedRate = 0;
	}

	double GetAverageSucceededRate() const
	{
		double averageSucceededRate = this->PeerTunnelCount == 0 ? 0.0 : this->TotalRequestSuccedRate / this->PeerTunnelCount;
		averageSucceededRate /= 100.0;
		return averageSucceededRate;
	}
};

struct DOWNLOADER_STATS : public DC_HEAD
{

	TRAFFIC_COUNTER Traffic;
	RATE_COUNTER MediaTraffic;

	PEER_MINMAX DownloadRange;
	UINT64 TotalRequsts;
	/// 重复的
	UINT64 TotalDuplicateSubPieces;
	/// 过期的，例如小于MinIndex
	UINT64 TotalExpiredSubPieces;

	UINT FirstStartIndex;
	UINT TotalRequestCount;
	UINT PeerTunnelCount;

	DOWNLOADER_SUB_STATS UDP;
	DOWNLOADER_SUB_STATS TCP;

	UINT RecentReceivedSubPieces;
	UINT RecentRedundentSubPieces;

	/// 开始定位请求的时间
	UINT StartLocateTime;

	/// 开始预分配下载任务的时间
	UINT StartAssignTime;

	/// 开始实际请求的时间
	UINT StartRequestTime;

	/// 收到第一个subpiece的时间
	UINT FirstSubPieceReceivedTime;

	/// 未完成的数据片
	UINT UnfinishedDataPieces;
	UINT32 UnfinishedSubPieceCount;

	void Clear()
	{
		FILL_ZERO( *this );
		this->InitHead( DCT_DOWNLOADER, sizeof( *this ) );

		this->UDP.Clear();
		this->TCP.Clear();
		this->Traffic.Clear();
		this->MediaTraffic.Clear();
	}

	double GetRecentRedundentSubPieceRate() const
	{
		double rate = this->RecentReceivedSubPieces == 0 ? 0.0 : this->RecentRedundentSubPieces * 100.0 / this->RecentReceivedSubPieces;
		return rate;
	}
};


const size_t STATS_MAX_ERROR_CODES = 20;



struct CONNECTOR_SUB_STATS
{
	UINT32 StructSize;

	TRAFFIC_COUNTER Traffic;

	/// 发起的连接请求数
	UINT32 TotalInitiatedConnections;
	/// 成功的发起连接数
	UINT32 TotalSucceededInitiatedConnections;
	UINT32 TotalTimeoutInitiatedConnections;

	/// 收到的连接请求数
	UINT32 TotalRequestedConnections;
	/// 成功的请求连接数
	UINT32 TotalSucceededRequestedConnections;
	UINT32 TotalTimeoutRequestedConnections;

	UINT32 PendingPeerCount;

	CODE2_COUNTER DisconnectRecords[STATS_MAX_ERROR_CODES];
	CODE2_COUNTER DisconnectedRecords[STATS_MAX_ERROR_CODES];
	CODE4_COUNTER RefuseRecords[STATS_MAX_ERROR_CODES];

	CODE4_COUNTER RefusedRecords[STATS_MAX_ERROR_CODES];


	void Clear()
	{
		FILL_ZERO( *this );
		this->StructSize = sizeof( *this );

		this->Traffic.Clear();
	}

};


struct CONNECTOR_STATS : public DC_HEAD
{

	CONNECTOR_SUB_STATS UDP;
	CONNECTOR_SUB_STATS TCP;


	UINT32 TCPConnectSucceededTimes;
	UINT32 TCPConnectFailedTimes;

	UINT32 TCPConnectingPeerCount;
	UINT32 TCPHandshakingPeerCount;

	UINT32 UDPSwitchToUDP;
	UINT32 UDPSwitchToTCP;
	UINT32 TCPSwitchTo80;


	void Clear()
	{
		FILL_ZERO( *this );
		this->InitHead( DCT_CONNECTOR, sizeof( *this ) );

		this->UDP.Clear();
		this->TCP.Clear();
	}

	UINT32  GetTotalInitiatedConnections() const
	{
		return this->UDP.TotalInitiatedConnections + this->TCP.TotalInitiatedConnections;
	}
	UINT32  GetTotalSucceededInitiatedConnections() const
	{
		return this->UDP.TotalSucceededInitiatedConnections + this->TCP.TotalSucceededInitiatedConnections;
	}
	UINT32  GetTotalTimeoutInitiatedConnections() const
	{
		return this->UDP.TotalTimeoutInitiatedConnections + this->TCP.TotalTimeoutInitiatedConnections;
	}

	UINT32  GetTotalRequestedConnections() const
	{
		return this->UDP.TotalRequestedConnections + this->TCP.TotalRequestedConnections;
	}
	UINT32  GetTotalSucceededRequestedConnections() const
	{
		return this->UDP.TotalSucceededRequestedConnections + this->TCP.TotalSucceededRequestedConnections;
	}
	UINT32  GetTotalTimeoutRequestedConnections() const
	{
		return this->UDP.TotalTimeoutRequestedConnections + this->TCP.TotalTimeoutRequestedConnections;
	}

	UINT32 GetPendingPeerCount() const
	{
		return this->UDP.PendingPeerCount + this->TCP.PendingPeerCount;
	}


};

struct DEGREE_PAIR_COUNTER
{
	int In;
	int Out;

	void Clear()
	{
		this->In = 0;
		this->Out = 0;
	}

	DEGREE_PAIR ToSimple() const
	{
		DEGREE_PAIR result;
		result.In = static_cast<UINT8>( min( this->In, 255 ) );
		result.Out = static_cast<UINT8>( min( this->Out, 255 ) );
		return result;
	}

	int GetTotal() const { return this->In + this->Out; }

	void Inc(bool isInitFromRemote)
	{
		if (isInitFromRemote)
		{
			this->In++;
		}
		else
		{
			this->Out++;
		}
	}
	void Dec(bool isInitFromRemote)
	{
		if (isInitFromRemote)
		{
			LIVE_ASSERT(this->In > 0);
			this->In--;
		}
		else
		{
			LIVE_ASSERT(this->Out > 0);
			this->Out--;
		}
	}

};


inline DEGREE_PAIR_COUNTER operator-(const DEGREE_PAIR_COUNTER& x, const DEGREE_PAIR_COUNTER& y)
{
	DEGREE_PAIR_COUNTER result;
	result.In = x.In - y.In;
	result.Out = x.Out - y.Out;
	return result;
}

inline DEGREE_PAIR_COUNTER operator+(const DEGREE_PAIR_COUNTER& x, const DEGREE_PAIR_COUNTER& y)
{
	DEGREE_PAIR_COUNTER result;
	result.In = x.In + y.In;
	result.Out = x.Out + y.Out;
	return result;
}


struct DEGREE_COUNTER
{
	int Max;
	int Left;
	DEGREE_PAIR_COUNTER All;
	DEGREE_PAIR_COUNTER UDPT;
	DEGREE_PAIR_COUNTER VIP;

	void Clear()
	{
		this->All.Clear();
		this->UDPT.Clear();
		this->VIP.Clear();
		this->Left = 0;
		this->Max = 0;
	}

	DEGREE_INFO ToSimple() const
	{
		DEGREE_INFO result;
		result.All = this->All.ToSimple();
		result.UDPT = this->UDPT.ToSimple();
		if ( this->Left <= 127)
			result.Left = static_cast<INT8>( this->Left );
		else
			result.Left = 127;
		return result;
	}

	DEGREE_PAIR_COUNTER GetTCP() const
	{
		return this->All - this->UDPT;
	}

	void Update(int maxDegree)
	{
		this->Max = maxDegree;
		this->Left = maxDegree - this->All.GetTotal();
	}

	void Inc(bool isUDPT, bool isInitFromRemote, bool isVIP, int maxDegree)
	{
		this->All.Inc(isInitFromRemote);
		if (isUDPT)
			this->UDPT.Inc(isInitFromRemote);
		if (isVIP)
			this->VIP.Inc(isInitFromRemote);
		this->Update(maxDegree);
	}
	void Dec(bool isUDPT, bool isInitFromRemote, bool isVIP, int maxDegree)
	{
		this->All.Dec(isInitFromRemote);
		if (isUDPT)
			this->UDPT.Dec(isInitFromRemote);
		if (isVIP)
			this->VIP.Dec(isInitFromRemote);
		this->Update(maxDegree);
	}

};

struct PEER_MANAGER_STATS : public DC_HEAD
{


	//UINT64 TotalDownloadMediaBytes;
	//UINT64 TotalUploadMediaBytes;
	UINT32 ConnectionCount;
	UINT32 AverageQos;

	UINT16 AverageSkipPercent;

	UINT16 AverageLostPacketRate;


	TRAFFIC_COUNTER AnnounceTraffic;
	DEGREE_COUNTER Degrees;

	/// 历史总共的peer连接个数
	UINT TotalPeerCount;
	UINT DeletedPeerCount;

	TRAFFIC_COUNTER TotalTraffic;

	/// 协议流量信息
	TRAFFIC_COUNTER ProtocolTraffic;

	TRAFFIC_COUNTER ExternalTraffic;

//	CODE2_COUNTER ReceivedErrorCodes[10];
//	CODE4_COUNTER ReceivedKickReasons[10];

//	CODE2_COUNTER SentErrorCodes[10];
//	CODE4_COUNTER SentKickReasons[10];

	UINT64 TotalOldMaxSequenceID;

	UINT32 MaxConnectedPeerCount;

	UINT64 InitiatedNATConnections;
	UINT64 SucceededInitiatedNATConnections;

	UINT64 ReceivedNATConnections;
	UINT64 SucceededReceivedNATConnections;

	void Clear()
	{
		FILL_ZERO( *this );
		this->InitHead( DCT_PEERMANAGER, sizeof( *this ) );

		this->AnnounceTraffic.Clear();
		this->TotalTraffic.Clear();
		this->ProtocolTraffic.Clear();
		this->ExternalTraffic.Clear();
	}
};


struct STREAMBUFFER_STATS : public DC_HEAD
{
	enum { MAX_BUFFERRING_TIME_COUNT = 60 };

	/// 资源范围的最小值
	UINT32 MinIndex;

	/// 资源范围的最大值
	UINT32 MaxIndex;

	/// 开始下载的Piece编号
	UINT32 StartIndex;

	/// Skip指针 = MinIndex + BufferSize
	UINT32 SkipIndex;

	/// 严格按时间推进的缓冲区位置
	UINT HuntIndex; //?

	/// 换头参考位置
	UINT SeeIndex; //?

	/// 缓冲区的大小
	UINT32 BufferSize;

	/// 缓冲区内已经下载的个数
	UINT32 PieceCount;

	/// 缓冲区的时间长度，以毫秒计算
	UINT32 BufferTime;

	/// 缓冲区的时间长度(从min到skip的时间长度)，以毫秒计算
	UINT32 SkipBufferTime;

	/// 总的推过去的片数
	UINT32 TotalPushedPieceCount;

	/// 总的无用的片数(下载到时已经跳过的piece片数)
	UINT32 TotalUnusedPieceCount;

	/// 基准片的索引
	UINT BasePieceIndex;

	/// 基准片的时间戳
	UINT64 BaseTimeStamp;

	/// 基准片的接收时间
	UINT64 BaseReceiveTime;

	UINT64 MinTimeStamp;
	UINT64 MaxTimeStamp;
	UINT64 SkipTimeStamp;

	UINT FirstErrorPiece;
	UINT64 FirstErrorPieceTimeStamp;

	/// 片的大小
	UINT32 PieceSize;

	/// 片的大小
	UINT32 AveragePieceSize;

	/// 跳帧率
	UINT8 SkipPercent;

	/// 缓冲区重置的次数
	UINT ResetTimes;

	UINT JumpTimes;


	/// 总共跳过的piece片数
	UINT TotalSkippedPieceCount;

	/// 未完成的数据片
	UINT Reserved1;

	/// 数据校验失败的piece数
	UINT64 InvalidPieceCount;

	UINT32 BufferringUsedTime[MAX_BUFFERRING_TIME_COUNT];

	UINT32 DownloadedPieceCount;
	UINT32 Reserved2;

	UINT64 CurrentTimeStamp;

	void Clear()
	{
		FILL_ZERO( *this );
		this->InitHead( DCT_STREAMBUFFER, sizeof( *this ) );

		this->SkipPercent = 100;
	}
};



/// peer地址池的统计数据
struct IPPOOL_STATS : public DC_HEAD
{


	/// 总的探测次数
	UINT TotalDetectTimes;

	/// 总的成功探测次数
	UINT TotalDetectSucceededTimes;

	/// 总的连接次数
	UINT TotalConnectTimes;


	/// 从DetectingPool中提取peer地址的总次数
	UINT TotalPopDetectingTimes;

	/// 从DetectablePool中提取peer地址的总次数
	UINT TotalPopDetectableTimes;


	/// 从DetectedPool中提取peer地址的总次数
	UINT TotalPopDetectedTimes;

	/// 从UnconnectedPool中提取peer地址的总次数
	UINT TotalPopUnconnectedTimes;

	/// 从UnconnectablePool中提取peer地址的总次数
	UINT TotalPopUnconnectableTimes, TotalPopUnconnectableTimes2;

	/// 从UndetectablePool中提取peer地址的总次数
	UINT TotalPopUndetectableTimes;

	/// 从DisconnectPool中提取peer地址的总次数
	UINT TotalPopDisconnectTimes;




	/// 全部peer个数
	UINT TotalPoolSize;

	/// 候选peer个数
	UINT CandidatePoolSize;

	/// 探测刚完成等待连接的peer地址个数
	UINT DetectedPoolSize;

	UINT ConnectionPoolSize;


	/// 探测过的peer地址个数
	UINT DetectingPoolSize; //?

	/// 连接断开过的peer地址个数
	UINT DisconnectedPoolSize;

	/// 没有连接过的peer地址个数
	UINT UnconnectedPoolSize;

	/// 没有连接成功过的peer地址个数
	UINT UnconnectablePoolSize;


	/// 无法探测的个数（udp端口为0）
	UINT UnDetectableItems;


	UINT DetectIndexSize;
	UINT ConnectIndexSize;
	UINT ActiveIndexSize;
	UINT HelloIndexSize;

	WORD DetectCheckCount;
	WORD ConnectCheckCount;
	WORD DeleteCheckCount;
	WORD DetectingCount;



	/// 被删除的peer地址信息项的总个数
	UINT TotalDeletedCount;
	UINT TotalItemCount;
	//UINT TotalDeteledItemCount;

	UINT TotalIgnoredPeers;



	/// 清空
	void Clear()
	{
		FILL_ZERO( *this );
		this->InitHead( DCT_IPPOOL, sizeof( *this ) );

	}
};


/// peer地址池的统计数据
struct MEDIASERVER_STATS : public DC_HEAD
{
	UINT HeaderIndex;
	UINT PlayToIndex;
	UINT FirstPlayIndex;
	UINT64 FirstPlayPieceTimeStamp;
	UINT64 FirstPlayTime;

	/// 清空
	void Clear()
	{
		FILL_ZERO( *this );
		this->InitHead( DCT_MEDIASERVER, sizeof( *this ) );

	}
};

#endif