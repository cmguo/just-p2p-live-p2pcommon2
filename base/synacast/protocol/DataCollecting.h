
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
	/// �ܹ�����
	UINT64 Total;
	/// ��ǰ����
	UINT32 Rate;
	/// ƽ������
	UINT32 AverageRate;
	/// ��ʷ�������
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

	/// �����Ĵ���
	UINT64 SentCount;

	/// �յ����������
	UINT64 ReceivedCount;

	/// �յ�����Ч���ĵĸ���
	UINT64 InvalidCount;

	/// ����ʧ�ܵĴ���
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

/// ��ʼ20���ͳ������
class START_STATS
{
public:
	// �����Գ���100
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
	/// ����
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

	/// ��������������
	TRAFFIC_COUNTER Traffic;

	UINT32 SwitchClientTimes;

	TRACKER_REQUEST_COUNTER Register;
	TRACKER_REQUEST_COUNTER Join;
	TRACKER_REQUEST_COUNTER KeepAlive;
	TRACKER_REQUEST_COUNTER List;
	TRACKER_REQUEST_COUNTER TCPList;

	UINT32 LeaveTimes;


	/// �ܹ�List����peer����
	UINT32 TotalListedPeers;
	UINT32 TCPTotalListedPeers;

	/// ��һ��List����peer����
	UINT16 LastListedPeers;
	UINT16 TCPLastListedPeers;

	/// �ܹ�List����peer����
	UINT32 TotalListedLANPeers;

	/// ��һ��List����peer����
	UINT16 LastListedLANPeers;

	UINT32 TotalInvalidResponse;
	UINT32 TotalInvalid2Response;

	UINT32 TotalIgnoredResponse;


	/// join/keepalive/listʧ�ܵĴ���
	/// �л�tracker�Ĵ���
	/// tcp tracker��list������list����peer����

	void Clear()
	{
		FILL_ZERO( *this );
		this->InitHead( DCT_TRACKER, sizeof( *this ) );

		this->Traffic.Clear();
	}
};




struct APP_STATS : public  DC_HEAD
{

	/// PacketSender��Ϣ
	UINT32 StartTime;
	UINT32 CurrentTime;
	SYSTEMTIME StartDateTime;

	/// �ܹ����հ���Ϣ(����)
	TRAFFIC_COUNTER TotalTraffic;


	/// ����sender��Ӧ������
	TRAFFIC_COUNTER OldUDPTraffic; //? ��TrackerFlow�ظ�
	TRAFFIC_COUNTER TCPConnectionlessTraffic;
	TRAFFIC_COUNTER TCPConnectionTraffic;
	TRAFFIC_COUNTER UDPConnectionlessTraffic;
	TRAFFIC_COUNTER UDPConnectionTraffic;

	UINT64 TotalIgnoredPackets;
	UINT64 TotalInvalidPackets;

	/// unshuffle����-1�ı���
	UINT64 TotalTooShortPackets;

	/// ��0xe903��ͷ�Ĳ�ʶ����ϱ���
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

	/// �յ���������
	UINT64 TotalSubPieceRequestsReceived;
	/// ʵ���ϴ���subpiece��
	UINT64 TotalSubPiecesUploaded;
	/// û���ҵ���subpiece��
	UINT64 TotalMissingSubPieces;
	/// �����ϴ����ٵ���û���ϴ���subpiece��
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
	/// �ظ���
	UINT64 TotalDuplicateSubPieces;
	/// ���ڵģ�����С��MinIndex
	UINT64 TotalExpiredSubPieces;

	UINT FirstStartIndex;
	UINT TotalRequestCount;
	UINT PeerTunnelCount;

	DOWNLOADER_SUB_STATS UDP;
	DOWNLOADER_SUB_STATS TCP;

	UINT RecentReceivedSubPieces;
	UINT RecentRedundentSubPieces;

	/// ��ʼ��λ�����ʱ��
	UINT StartLocateTime;

	/// ��ʼԤ�������������ʱ��
	UINT StartAssignTime;

	/// ��ʼʵ�������ʱ��
	UINT StartRequestTime;

	/// �յ���һ��subpiece��ʱ��
	UINT FirstSubPieceReceivedTime;

	/// δ��ɵ�����Ƭ
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

	/// ���������������
	UINT32 TotalInitiatedConnections;
	/// �ɹ��ķ���������
	UINT32 TotalSucceededInitiatedConnections;
	UINT32 TotalTimeoutInitiatedConnections;

	/// �յ�������������
	UINT32 TotalRequestedConnections;
	/// �ɹ�������������
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

	/// ��ʷ�ܹ���peer���Ӹ���
	UINT TotalPeerCount;
	UINT DeletedPeerCount;

	TRAFFIC_COUNTER TotalTraffic;

	/// Э��������Ϣ
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

	/// ��Դ��Χ����Сֵ
	UINT32 MinIndex;

	/// ��Դ��Χ�����ֵ
	UINT32 MaxIndex;

	/// ��ʼ���ص�Piece���
	UINT32 StartIndex;

	/// Skipָ�� = MinIndex + BufferSize
	UINT32 SkipIndex;

	/// �ϸ�ʱ���ƽ��Ļ�����λ��
	UINT HuntIndex; //?

	/// ��ͷ�ο�λ��
	UINT SeeIndex; //?

	/// �������Ĵ�С
	UINT32 BufferSize;

	/// ���������Ѿ����صĸ���
	UINT32 PieceCount;

	/// ��������ʱ�䳤�ȣ��Ժ������
	UINT32 BufferTime;

	/// ��������ʱ�䳤��(��min��skip��ʱ�䳤��)���Ժ������
	UINT32 SkipBufferTime;

	/// �ܵ��ƹ�ȥ��Ƭ��
	UINT32 TotalPushedPieceCount;

	/// �ܵ����õ�Ƭ��(���ص�ʱ�Ѿ�������pieceƬ��)
	UINT32 TotalUnusedPieceCount;

	/// ��׼Ƭ������
	UINT BasePieceIndex;

	/// ��׼Ƭ��ʱ���
	UINT64 BaseTimeStamp;

	/// ��׼Ƭ�Ľ���ʱ��
	UINT64 BaseReceiveTime;

	UINT64 MinTimeStamp;
	UINT64 MaxTimeStamp;
	UINT64 SkipTimeStamp;

	UINT FirstErrorPiece;
	UINT64 FirstErrorPieceTimeStamp;

	/// Ƭ�Ĵ�С
	UINT32 PieceSize;

	/// Ƭ�Ĵ�С
	UINT32 AveragePieceSize;

	/// ��֡��
	UINT8 SkipPercent;

	/// ���������õĴ���
	UINT ResetTimes;

	UINT JumpTimes;


	/// �ܹ�������pieceƬ��
	UINT TotalSkippedPieceCount;

	/// δ��ɵ�����Ƭ
	UINT Reserved1;

	/// ����У��ʧ�ܵ�piece��
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



/// peer��ַ�ص�ͳ������
struct IPPOOL_STATS : public DC_HEAD
{


	/// �ܵ�̽�����
	UINT TotalDetectTimes;

	/// �ܵĳɹ�̽�����
	UINT TotalDetectSucceededTimes;

	/// �ܵ����Ӵ���
	UINT TotalConnectTimes;


	/// ��DetectingPool����ȡpeer��ַ���ܴ���
	UINT TotalPopDetectingTimes;

	/// ��DetectablePool����ȡpeer��ַ���ܴ���
	UINT TotalPopDetectableTimes;


	/// ��DetectedPool����ȡpeer��ַ���ܴ���
	UINT TotalPopDetectedTimes;

	/// ��UnconnectedPool����ȡpeer��ַ���ܴ���
	UINT TotalPopUnconnectedTimes;

	/// ��UnconnectablePool����ȡpeer��ַ���ܴ���
	UINT TotalPopUnconnectableTimes, TotalPopUnconnectableTimes2;

	/// ��UndetectablePool����ȡpeer��ַ���ܴ���
	UINT TotalPopUndetectableTimes;

	/// ��DisconnectPool����ȡpeer��ַ���ܴ���
	UINT TotalPopDisconnectTimes;




	/// ȫ��peer����
	UINT TotalPoolSize;

	/// ��ѡpeer����
	UINT CandidatePoolSize;

	/// ̽�����ɵȴ����ӵ�peer��ַ����
	UINT DetectedPoolSize;

	UINT ConnectionPoolSize;


	/// ̽�����peer��ַ����
	UINT DetectingPoolSize; //?

	/// ���ӶϿ�����peer��ַ����
	UINT DisconnectedPoolSize;

	/// û�����ӹ���peer��ַ����
	UINT UnconnectedPoolSize;

	/// û�����ӳɹ�����peer��ַ����
	UINT UnconnectablePoolSize;


	/// �޷�̽��ĸ�����udp�˿�Ϊ0��
	UINT UnDetectableItems;


	UINT DetectIndexSize;
	UINT ConnectIndexSize;
	UINT ActiveIndexSize;
	UINT HelloIndexSize;

	WORD DetectCheckCount;
	WORD ConnectCheckCount;
	WORD DeleteCheckCount;
	WORD DetectingCount;



	/// ��ɾ����peer��ַ��Ϣ����ܸ���
	UINT TotalDeletedCount;
	UINT TotalItemCount;
	//UINT TotalDeteledItemCount;

	UINT TotalIgnoredPeers;



	/// ���
	void Clear()
	{
		FILL_ZERO( *this );
		this->InitHead( DCT_IPPOOL, sizeof( *this ) );

	}
};


/// peer��ַ�ص�ͳ������
struct MEDIASERVER_STATS : public DC_HEAD
{
	UINT HeaderIndex;
	UINT PlayToIndex;
	UINT FirstPlayIndex;
	UINT64 FirstPlayPieceTimeStamp;
	UINT64 FirstPlayTime;

	/// ���
	void Clear()
	{
		FILL_ZERO( *this );
		this->InitHead( DCT_MEDIASERVER, sizeof( *this ) );

	}
};

#endif