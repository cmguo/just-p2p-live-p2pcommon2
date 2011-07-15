
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_STATISTICS_INFO_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_STATISTICS_INFO_H_


#include "DataCollecting.h"
#include <ppl/os/memory_mapping.h>
#include <vector>


struct LIVE_STATISTICS_HEADER
{
	UINT32 HeaderSize;
	UINT32 IndexSize;
	UINT32 IndexCount;
	UINT32 TotalObjectSize;
};

struct LIVE_STATISTICS_INDEX
{
	UINT32 ObjectOffset;
	UINT32 ObjectSize;
};


class LiveStatisticsInfo
{
private:
	ppl::os::memory_mapping MemoryMappingData;
	std::vector<BYTE> BufferData;
public:
	LIVE_STATISTICS_HEADER* Header;
	LIVE_STATISTICS_INDEX* Indexes;

	APP_STATS* AppStats;
	IPPOOL_STATS* IPPoolStats;
	DETECTOR_STATS* DetectorStats;
	TRACKER_STATS* TrackerStats;
	STREAMBUFFER_STATS* StreamBufferStats;
	PEER_MANAGER_STATS* PeerManagerStats;
	CONNECTOR_STATS* ConnectorStats;
	DOWNLOADER_STATS* DownloaderStats;
	UPLOADER_STATS* UploaderStats;
	MEDIASERVER_STATS* MediaServerStats;


	LiveStatisticsInfo()
	{
		this->Header = NULL;
		this->Indexes = NULL;

		this->AppStats = NULL;
		this->IPPoolStats = NULL;
		this->DetectorStats = NULL;
		this->TrackerStats = NULL;
		this->StreamBufferStats = NULL;
		this->PeerManagerStats = NULL;
		this->ConnectorStats = NULL;
		this->DownloaderStats = NULL;
		this->UploaderStats = NULL;
		this->MediaServerStats = NULL;
	}

	void Create( LPCTSTR name )
	{
		UINT32 headerSize = sizeof( LIVE_STATISTICS_HEADER );
		UINT32 indexSize = sizeof( LIVE_STATISTICS_INDEX );
		UINT32 indexCount = 10;
		UINT32 totalObjectSize = 
			sizeof( APP_STATS ) + 
			sizeof( IPPOOL_STATS ) + 
			sizeof( DETECTOR_STATS ) + 
			sizeof( TRACKER_STATS ) + 
			sizeof( STREAMBUFFER_STATS ) + 
			sizeof( PEER_MANAGER_STATS ) + 
			sizeof( CONNECTOR_STATS ) + 
			sizeof( DOWNLOADER_STATS ) + 
			sizeof( UPLOADER_STATS ) + 
			sizeof( MEDIASERVER_STATS );
		BYTE* buf = NULL;
		UINT32 headerAndIndexSize = headerSize + indexSize * indexCount;
		UINT32 totalSize = headerAndIndexSize + totalObjectSize;
		if ( this->MemoryMappingData.create( name, totalSize ) && this->MemoryMappingData.map_all() )
		{
			buf = static_cast<BYTE*>( this->MemoryMappingData.get_view() );
		}
		else
		{
			this->BufferData.resize( totalSize );
			buf = &this->BufferData[0];
		}

		this->Header = reinterpret_cast<LIVE_STATISTICS_HEADER*>( buf );
		this->Indexes = reinterpret_cast<LIVE_STATISTICS_INDEX*>( this->Header + 1 );

		this->Header->HeaderSize = headerSize;
		this->Header->IndexCount = indexCount;
		this->Header->IndexSize = indexSize;
		this->Header->TotalObjectSize = totalObjectSize;

		buf += headerAndIndexSize;
		UINT32 objectOffset = 0;
		UINT32 objectIndex = 0;

#ifdef PPL_SI_INIT_OBJECT
#error PPL_SI_INIT_OBJECT has been defined
#endif

#define PPL_SI_INIT_OBJECT( obj, objType, oi, oo, startBuf ) \
	do { \
		this->Indexes[oi].ObjectOffset = oo; \
		this->Indexes[oi].ObjectSize = sizeof( objType ); \
		this->obj = reinterpret_cast<objType*>( startBuf + oo ); \
		this->obj->Clear(); \
		oi++; \
		oo += sizeof( objType ); \
	} while ( false )

		PPL_SI_INIT_OBJECT( AppStats, APP_STATS, objectIndex, objectOffset, buf );
		PPL_SI_INIT_OBJECT( IPPoolStats, IPPOOL_STATS, objectIndex, objectOffset, buf );
		PPL_SI_INIT_OBJECT( DetectorStats, DETECTOR_STATS, objectIndex, objectOffset, buf );
		PPL_SI_INIT_OBJECT( TrackerStats, TRACKER_STATS, objectIndex, objectOffset, buf );
		PPL_SI_INIT_OBJECT( StreamBufferStats, STREAMBUFFER_STATS, objectIndex, objectOffset, buf );
		PPL_SI_INIT_OBJECT( PeerManagerStats, PEER_MANAGER_STATS, objectIndex, objectOffset, buf );
		PPL_SI_INIT_OBJECT( ConnectorStats, CONNECTOR_STATS, objectIndex, objectOffset, buf );
		PPL_SI_INIT_OBJECT( DownloaderStats, DOWNLOADER_STATS, objectIndex, objectOffset, buf );
		PPL_SI_INIT_OBJECT( UploaderStats, UPLOADER_STATS, objectIndex, objectOffset, buf );
		PPL_SI_INIT_OBJECT( MediaServerStats, MEDIASERVER_STATS, objectIndex, objectOffset, buf );

		assert( 10 == objectIndex );
		assert( totalSize == objectOffset + headerAndIndexSize );

#undef PPL_SI_INIT_OBJECT

	}

};


#endif