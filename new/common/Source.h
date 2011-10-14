
#ifndef _LIVE_P2PCOMMON2_NEW_COMMON_SOURCE_H_
#define _LIVE_P2PCOMMON2_NEW_COMMON_SOURCE_H_


#include "ClientAppModule.h"
#include "SourceList.h"
#include "StreamBuffer.h"
#include "MediaStorage.h"
#include "pos/TrackerRequester.h"
#include "PeerManager.h"
#include "UdpDetect.h"
#include "MediaPiece.h"
#include "Msgs.h"

#include "StreamIndicator.h"
#include "common/BaseInfo.h"

#include <synacast/protocol/MonoMediaPiece.h>




//#pragma comment(lib, PPL_LIB_FILE("../lib/p2pcore"))
//#pragma comment(lib, PPL_LIB_FILE("../lib/p2pcommon"))


struct DelayedPieceInfo
{
	MediaDataPiecePtr Piece;
	time_counter ReceiveTime;

	DelayedPieceInfo() { }

	explicit DelayedPieceInfo(MediaDataPiecePtr piece) : Piece(piece)
	{
	}
};

class StreamDelayer
{
public:
	StreamDelayer()
	{
	}

	void Put(MediaDataPiecePtr piece)
	{
		CheckDataPiece(piece);
		m_pieces.push_back(DelayedPieceInfo(piece));
	}
	MediaDataPiecePtr Get(UINT delayTime)
	{
		LIVE_ASSERT(delayTime > 0);
		if (!m_pieces.empty())
		{
			DelayedPieceInfo pieceInfo = m_pieces.front();
			if (pieceInfo.ReceiveTime.elapsed() > delayTime)
			{
				m_pieces.pop_front();
				return pieceInfo.Piece;
			}
		}
		return MediaDataPiecePtr();
	}

	PEER_MINMAX GetMinMax() const
	{
		PEER_MINMAX minmax;
		minmax.MinIndex = 0;
		minmax.MaxIndex = 0;
		if (!m_pieces.empty())
		{
			minmax.MinIndex = m_pieces.front().Piece->GetPieceIndex();
			minmax.MaxIndex = m_pieces.back().Piece->GetPieceIndex();
		}
		return minmax;
	}

	list<DelayedPieceInfo> m_pieces;

};




/// 用于mcc的AppModule
class SourceModule : public ServerLiveAppModule
{
public:
	explicit SourceModule(const LiveAppModuleCreateParam& param);
	~SourceModule();

	virtual bool IsSource() const { return true; }

	void SafeAddHeaderPiece(MonoMediaHeaderPiece* piece);

	void SafeAddDataPiece(MonoMediaDataPiece* piece);

	UINT GetDelayTime() const { return m_delayTime; }

protected:
	tstring m_logFilePath;
	StreamDelayer m_streamDelayer;
	/// 延迟时间
	UINT m_delayTime;

	virtual void DoCreateComponents();
	virtual TrackerRequester* DoCreateTracker()
	{
		return TrackerRequesterFactory::MCCCreate();
	}

	virtual void DoStart(const LiveAppModuleCreateParam& param);

	virtual void OnAppTimer();

	/// 检查piece的有效性
//	static bool CheckPiece(const PPMediaPacket* piece);
};



inline SourceModule::SourceModule(const LiveAppModuleCreateParam& param) 
	: ServerLiveAppModule(param, TEXT("OS_SYS_INFO")), m_delayTime(0)
{
//	CORE_LogOn(NETWRITER);
	VIEW_INFO("Create New SourceModule!  ResourceGUID=" << param.ChannelGUID);
	// source增加直接连接mds的功能
	LoadMDS(param.MDSs);
	m_PeerInformation->NetInfo->CoreInfo.PeerType = SOURCE_PEER;

	//m_lpSourceList->StartSourcePlay( "F:\\MediaTest\\1.asf", 1, NULL, 16*1024 );

	tstring filename = _T("mcc") + FormatGUID(GetChannelGUID()) + _T(".log");
	m_logFilePath = ppl::os::paths::combine( m_CreateParam.BaseDirectory, filename );

	ini_file ini;
	this->InitIni(ini);
	ini.set_section(_T("stream"));
	UINT delayTime = ini.get_int(_T("DelayTime"), 0); // 单位为秒
	if (delayTime > 0)
	{
		LIMIT_MIN_MAX(delayTime, 0, 10 * 60 * 60); // 最多允许延迟10小时
		m_delayTime = delayTime * 1000; // 转换为毫秒
	}
}

inline SourceModule::~SourceModule()
{
	//通过父类进行清理
	//m_Manager.reset();
	//m_streamBuffer.reset();
	//m_UdpDetect.reset();
	//m_tracker.reset();
	//m_LiveInfo = NULL;
	//m_MappingLiveInfo.Close();
}

inline void SourceModule::DoCreateComponents()
{
	LIVE_ASSERT(!m_tracker);
	LIVE_ASSERT(!m_streamBuffer);
	LIVE_ASSERT(!m_Manager);
	m_tracker.reset(DoCreateTracker());
	m_streamBuffer.reset(StreamBufferFactory::MCCCreate());
	m_Manager.reset(PeerManagerFactory::MCCCreate(this));
}

inline void SourceModule::DoStart(const LiveAppModuleCreateParam& param)
{
	// mcc不需要洪泛探测，也不需要从别的节点处下载数据
	m_Manager->LoadVIP(param.VIPs);
	m_UdpDetect->Start(false);
	m_streamBuffer->Start();
	m_Manager->Start();

	if (m_delayTime == 0)
	{
		this->StartTracker();
	}
}


inline void SourceModule::OnAppTimer()
{
	ServerLiveAppModule::OnAppTimer();

	if (m_delayTime > 0)
	{
		for (;;)
		{
			MediaDataPiecePtr piece = m_streamDelayer.Get(m_delayTime);
			if (!piece)
				break;
			GetInfo().ChannelInfo.UndelayedSourceMinMax = m_streamDelayer.GetMinMax();
			m_streamBuffer->AddDataPiece(piece);
			if (!m_tracker->IsStarted())
			{
				this->StartTracker();
			}
		}
	}

/*
	const int APP_TIMER_TIMES_PER_MINUTE = APP_TIMER_TIMES_PER_SECOND * 60;
	LIVE_ASSERT(APP_TIMER_TIMES_PER_MINUTE > 10);
	if (times % APP_TIMER_TIMES_PER_MINUTE != 10)
		return;
	File fout;
	if (!fout.OpenAppend(m_logFilePath.c_str()))
	{
	//	LIVE_ASSERT(false);
		return;
	}
	const CDetailedPeerInfo& localInfo = m_LiveInfo->LocalPeerInfo;
	const PEER_MINMAX& minmax = localInfo.StatusInfo.MinMax;
	const PEER_STATUS& status = localInfo.StatusInfo.Status;
	UINT8 averagePeerSkipPercent = localInfo.Flow.AverageSkipPercent;
	SYSTEMTIME currentTime;
	::GetLocalTime(&currentTime);
	fout.WriteF("%04u-%02u-%02u %02u:%02u:%02u", 
		currentTime.wYear, currentTime.wMonth, currentTime.wDay, currentTime.wHour, currentTime.wMinute, currentTime.wSecond);
	int peerCount = m_Manager->GetPeerCount();
	int speed = m_Manager->GetFlow().Upload.GetRate();
	fout.WriteF(" (%lu,%lu:%d) (%d,%d,%d,%d) %d\r\n", 
		minmax.MinIndex, minmax.MaxIndex, minmax.GetLength(), 
		peerCount, speed, (peerCount == 0) ? 0 : speed / peerCount, 
		localInfo.Flow.AverageQos, 
		averagePeerSkipPercent);
	fout.Flush();*/
}

inline void SourceModule::SafeAddHeaderPiece(MonoMediaHeaderPiece* piece)
{
	if (piece)
	{
		MonoMediaHeaderPiecePtr piecePtr(piece);
		SOURCENEW_DEBUG("获得头部成功 PieceIndex=" << piece->GetPieceIndex() << " Length=" << piece->GetPieceLength() );

		LIVE_ASSERT(!::IsBadReadPtr(piece, sizeof(MonoMediaHeaderPiece)));
		LIVE_ASSERT(!m_streamBuffer->GetStorage().HasHeader(piece->GetPieceIndex()));
		MediaHeaderPiecePtr headerPiece = MediaHeaderPiece::FromMonoPiece( piecePtr, m_streamBuffer->GetSigner() );
		if ( ! headerPiece )
		{
			SOURCENEW_DEBUG("SafeAddHeaderPiece invalid header piece " << make_tuple(piece->GetPieceIndex(), piece->GetPieceLength()));
			return;
		}
		m_streamBuffer->AddHeaderPiece( headerPiece );
	}
}

inline void SourceModule::SafeAddDataPiece(MonoMediaDataPiece* piece)
{
	if (piece)
	{
		MonoMediaDataPiecePtr piecePtr(piece);
		SOURCENEW_DEBUG("获得数据片成功 PieceIndex=" << piece->GetPieceIndex() << " Length=" << piece->GetPieceLength()
			<< " HeaderIndex=" << piece->GetHeaderPiece() << " Timestamp=" << piece->GetTimeStamp() );

		LIVE_ASSERT(!::IsBadReadPtr(piece, sizeof(MonoMediaDataPiece)));
		LIVE_ASSERT(!m_streamBuffer->GetStorage().HasDataPiece(piece->GetPieceIndex()));
		MediaDataPiecePtr dataPiece = MediaDataPiece::FromMonoPiece( piecePtr, m_streamBuffer->GetSigner() );
		if ( ! dataPiece )
		{
			SOURCENEW_DEBUG("SafeAddHeaderPiece invalid header piece " << make_tuple(piece->GetPieceIndex(), piece->GetPieceLength()));
			return;
		}
		if (m_delayTime == 0)
		{
			m_streamBuffer->AddDataPiece(dataPiece);
		}
		else
		{
			m_streamDelayer.Put(dataPiece);
			GetInfo().ChannelInfo.UndelayedSourceMinMax = m_streamDelayer.GetMinMax();
		}
	}
}

#endif
