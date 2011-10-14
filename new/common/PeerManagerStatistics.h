
#ifndef _LIVE_P2PCOMMON2_NEW_COMMON_PEER_MANAGER_STATISTICS_H_
#define _LIVE_P2PCOMMON2_NEW_COMMON_PEER_MANAGER_STATISTICS_H_

#include "util/flow.h"
#include <synacast/protocol/DataCollecting.h>
#include <boost/noncopyable.hpp>
#include <map>
#include <vector>


/*
class DegreeInfo : public DEGREE_COUNTER
{
public:
DegreeInfo()
{
this->Clear();
}

};*/



class UploaderStatistics : public UPLOADER_STATS, private boost::noncopyable
{
public:
	FlowMeasure Flow;
	RateMeasure MediaFlow;

	UploaderStatistics()
	{
		this->Clear();
	}

	void SyncFlow()
	{
		this->Flow.SyncTo( this->Traffic );
		this->MediaFlow.SyncTo( this->MediaTraffic );
	}
	void UpdateFlow()
	{
		this->Flow.Update();
		this->MediaFlow.Update();
	}
};


class DownloaderStatistics : public DOWNLOADER_STATS, private boost::noncopyable
{
public:
	FlowMeasure Flow;
	RateMeasure MediaFlow;
	RateMeasure RedundentFlow;
	const time_counter StartTime;

	RateMeasure SubPieceRequestRate;
	RateMeasure SubPieceResponseRate;

	DownloaderStatistics() : RedundentFlow(10), SubPieceRequestRate(10), SubPieceResponseRate(10)
	{
		this->Clear();
	}

	void SyncFlow()
	{
		this->Flow.SyncTo( this->Traffic );
		this->MediaFlow.SyncTo( this->MediaTraffic );
		this->RecentReceivedSubPieces = this->RedundentFlow.GetRecentPackets();
		this->RecentRedundentSubPieces = this->RedundentFlow.GetRecentBytes();
	}
	void UpdateFlow()
	{
		this->Flow.Update();
		this->MediaFlow.Update();
		this->RedundentFlow.Update();
		this->SubPieceRequestRate.Update();
		this->SubPieceResponseRate.Update();
	}

	double CalcSubPieceSucceededRate() const
	{
		if (this->SubPieceRequestRate.GetRate() == 0)
			return 0.0;
		return this->SubPieceResponseRate.GetRate() * 1.0 / this->SubPieceRequestRate.GetRate();
	}

};



class PeerConnectorSubStatisticsInfo
{
public:
	FlowMeasure Flow;

	std::map<UINT16, UINT> DisconnectErrors;
	std::map<UINT16, UINT> DisconnectedErrors;

	std::map<UINT32, UINT> RefuseErrors;
	std::map<UINT32, UINT> RefusedErrors;

	void SyncErrorCodes(CONNECTOR_SUB_STATS& stats) const
	{
		Sync16( this->DisconnectErrors, stats.DisconnectRecords );
		Sync16( this->DisconnectedErrors, stats.DisconnectedRecords );
		Sync32( this->RefuseErrors, stats.RefuseRecords );
		Sync32( this->RefusedErrors, stats.RefusedRecords );
	}

	static void Sync16( const std::map<UINT16, UINT>& src, CODE2_COUNTER dst[])
	{
		std::multimap<UINT, UINT16, std::greater<UINT> > temp;
		for ( std::map<UINT16, UINT>::const_iterator iter = src.begin(); iter != src.end(); ++iter )
		{
			temp.insert( std::make_pair( iter->second, iter->first ) );
		}

		size_t index = 0;
		for ( std::multimap<UINT, UINT16, std::greater<UINT> >::const_iterator iter = temp.begin(); iter != temp.end(); ++iter )
		{
			if ( index >= STATS_MAX_ERROR_CODES )
				break;
			dst[index].Code = iter->second;
			dst[index].Count = iter->first;
			++index;
		}
		while ( index < STATS_MAX_ERROR_CODES )
		{
			dst[index].Clear();
			++index;
		}
	}
	static void Sync32( const std::map<UINT32, UINT>& src, CODE4_COUNTER dst[])
	{
		std::multimap<UINT, UINT32, std::greater<UINT> > temp;
		for ( std::map<UINT32, UINT>::const_iterator iter = src.begin(); iter != src.end(); ++iter )
		{
			temp.insert( std::make_pair( iter->second, iter->first ) );
		}

		size_t index = 0;
		for ( std::multimap<UINT, UINT32, std::greater<UINT> >::const_iterator iter = temp.begin(); iter != temp.end(); ++iter )
		{
			if ( index >= STATS_MAX_ERROR_CODES )
				break;
			dst[index].Code = iter->second;
			dst[index].Count = iter->first;
			++index;
		}
		while ( index < STATS_MAX_ERROR_CODES )
		{
			dst[index].Clear();
			++index;
		}
	}
};


/// 状态统计信息
class PeerConnectorStatistics : public CONNECTOR_STATS, private boost::noncopyable
{
public:
	PeerConnectorSubStatisticsInfo UDPInfo;
	PeerConnectorSubStatisticsInfo TCPInfo;


	PeerConnectorStatistics()
	{
		this->Clear();
	}

	void SyncFlow()
	{
		this->UDPInfo.Flow.SyncTo( this->UDP.Traffic );
		this->TCPInfo.Flow.SyncTo( this->TCP.Traffic );
	}

	void UpdateFlow()
	{
		this->UDPInfo.Flow.Update();
		this->TCPInfo.Flow.Update();

		this->UDPInfo.SyncErrorCodes( this->UDP );
		this->TCPInfo.SyncErrorCodes( this->TCP );
	}

	UINT64 GetTotalDownloadBytes() const
	{
		return this->UDPInfo.Flow.Download.GetTotalBytes() + this->TCPInfo.Flow.Download.GetTotalBytes();
	}
	UINT64 GetTotalUploadBytes() const
	{
		return this->UDPInfo.Flow.Upload.GetTotalBytes() + this->TCPInfo.Flow.Upload.GetTotalBytes();
	}
	UINT64 GetTotalDownloadPackets() const
	{
		return this->UDPInfo.Flow.Download.GetTotalPackets() + this->TCPInfo.Flow.Download.GetTotalPackets();
	}
	UINT64 GetTotalUploadPackets() const
	{
		return this->UDPInfo.Flow.Upload.GetTotalPackets() + this->TCPInfo.Flow.Upload.GetTotalPackets();
	}
};



/// 当播放质量下降时的状态数据
struct QualityData
{
	INT32 PrepareTime;
	UINT32 ConnectedPeerCount;
	UINT32 DownloadSpeed;
	UINT32 UploadSpeed;
	/// 上次到这次的持续时间，单位：毫秒
	UINT32 Duration;
	/// 变化的时刻，单位：秒
	UINT32 TimeOfChange;

	QualityData()
	{
		FILL_ZERO(*this);
	}
};

struct QualityDataInfo : private boost::noncopyable
{
	std::vector<QualityData> BadInfo;
	std::vector<QualityData> GoodInfo;
};


class PeerManagerStatistics : public PEER_MANAGER_STATS
{
public:
	PeerManagerStatistics() : m_LongTimeFlow(30), GoodQuality(false)
	{
		this->Clear();
	}

	PeerConnectorStatistics ConnectorData;
	DownloaderStatistics DownloaderData;
	UploaderStatistics UploaderData;

	FlowMeasure AnnounceFlow;

	std::map<UINT16, UINT> ReceivedErrorCodes;
	std::map<UINT32, UINT> ReceivedKickReasons;

	std::map<UINT16, UINT> SentErrorCodes;
	std::map<UINT32, UINT> SentKickReasons;

	/// 流量信息
	FlowMeasure TotalFlow;

	/// 协议流量信息
	FlowMeasure ProtocolFlow;

	FlowMeasure ExternalFlow;

	FlowMeasure m_LongTimeFlow;

	QualityDataInfo InternalInfo;
	QualityDataInfo ExternalInfo;

	time_counter InternalLastTimeQualityChanged;
	time_counter ExternalLastTimeQualityChanged;
	bool GoodQuality;


	void SyncFlow()
	{
		this->ConnectorData.SyncFlow();
		this->DownloaderData.SyncFlow();
		this->UploaderData.SyncFlow();

		this->AnnounceFlow.SyncTo( this->AnnounceTraffic );
		this->TotalFlow.SyncTo( this->TotalTraffic );
		this->ProtocolFlow.SyncTo( this->ProtocolTraffic );
		this->ExternalFlow.SyncTo( this->ExternalTraffic );
	}

	void UpdateFlow()
	{
		this->m_LongTimeFlow.Update();

		this->ConnectorData.UpdateFlow();
		this->DownloaderData.UpdateFlow();
		this->UploaderData.UpdateFlow();

		this->ExternalFlow.Update();
		this->TotalFlow.Update();
		this->ProtocolFlow.Update();
		this->AnnounceFlow.Update();
		this->UploaderData.Flow.Update();
	}

	/// 记录缓冲质量下降
	void RecordQualityGoBad(bool isGoodQuality, int prepareTime, UINT32 connectedPeerCount, UINT32 downSpeed, UINT32 upSpeed, UINT32 timeOfChange, bool isInternal)
	{
		UINT duration = 0;
		if ( isInternal )
		{
			LIVE_ASSERT(this->GoodQuality != isGoodQuality);
			this->GoodQuality = isGoodQuality;
			duration = this->InternalLastTimeQualityChanged.elapsed32();
			this->InternalLastTimeQualityChanged.sync();
		}
		else
		{
			duration = this->ExternalLastTimeQualityChanged.elapsed32();
			this->ExternalLastTimeQualityChanged.sync();
		}

		QualityData data;
		data.PrepareTime = prepareTime;
		data.ConnectedPeerCount = connectedPeerCount;
		data.DownloadSpeed = downSpeed;
		data.UploadSpeed = upSpeed;
		data.TimeOfChange = timeOfChange;
		data.Duration = duration;
		QualityDataInfo& info = isInternal ? this->InternalInfo : this->ExternalInfo;
		std::vector<QualityData>& infoItems = isGoodQuality ? info.GoodInfo : info.BadInfo;
		if ( infoItems.size() > 500 )
			return;
		infoItems.push_back(data);
	}

	//UINT64 GetTotalDownloadBytes() const
	//{
	//	return this->DownloaderData.Flow.Download.GetTotalBytes() + this->UploaderData.Flow.Download.GetTotalBytes() + this->ConnectorData.GetTotalDownloadBytes();
	//}
	//UINT64 GetTotalUploadBytes() const
	//{
	//	return this->DownloaderData.Flow.Upload.GetTotalBytes() + this->UploaderData.Flow.Upload.GetTotalBytes() + this->ConnectorData.GetTotalUploadBytes();
	//}
	//UINT64 GetTotalDownloadPackets() const
	//{
	//	return this->DownloaderData.Flow.Download.GetTotalPackets() + this->UploaderData.Flow.Download.GetTotalPackets() + this->ConnectorData.GetTotalDownloadPackets();
	//}
	//UINT64 GetTotalUploadPackets() const
	//{
	//	return this->DownloaderData.Flow.Upload.GetTotalPackets() + this->UploaderData.Flow.Upload.GetTotalPackets() + this->ConnectorData.GetTotalUploadPackets();
	//}
};

#endif
