
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_COLLECTING_PACKET_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_COLLECTING_PACKET_H_

#include "DataCollecting.h"
#include "FixedPacket2.h"


class DCRequestPacket : public FixedPacketImpl<DC_HEAD, PPT_DATA_COLLECTING_REQUEST>
{
public:
	explicit DCRequestPacket( DataCollectingType dataType )
	{
		m_body.DataType = static_cast<UINT16>( dataType );
	}
};


class DCCrashLogPacket : public VariantPacketImpl<DC_CRASH_LOG, PPT_DATA_COLLECTING_RESPONSE, unsigned char>
{
public:
	explicit DCCrashLogPacket( const byte_buffer& crashLog ) : m_CrashLog( crashLog )
	{
		m_body.DataType = DCT_CRASHLOG;
		m_body.StructSize = crashLog.size() + sizeof( UINT32 );
	}

	virtual size_t get_object_size() const
	{
		return this->GetFixedSize() + m_CrashLog.size();
	}

	virtual void write_object(memory_output_stream& os) const
	{
		this->WriteFixedBody(os);
		os.write_buffer(m_CrashLog);
	}

private:
	const byte_buffer& m_CrashLog;
};

template < typename DCPacketBodyT, DataCollectingType _DATA_TYPE >
class DCFixedPacketImpl : public FixedPacketImpl<DCPacketBodyT, PPT_DATA_COLLECTING_RESPONSE>
{
public:
	explicit DCFixedPacketImpl( const DCPacketBodyT& body )
	{
		m_body = body;
		m_body.DataType = _DATA_TYPE;
	}
};


//class DGAppStatsPacket : public Fix

typedef DCFixedPacketImpl<APP_STATS, DCT_APP> DCAppStatsPacket;

typedef DCFixedPacketImpl<DETECTOR_STATS, DCT_DETECTOR> DCDetectorStatsPacket;

typedef DCFixedPacketImpl<TRACKER_STATS, DCT_TRACKER> DCTrackerStatsPacket;

typedef DCFixedPacketImpl<PEER_MANAGER_STATS, DCT_PEERMANAGER> DCPeerManagerStatsPacket;

typedef DCFixedPacketImpl<STREAMBUFFER_STATS, DCT_STREAMBUFFER> DCStreamBufferStatsPacket;

typedef DCFixedPacketImpl<IPPOOL_STATS, DCT_IPPOOL> DCIPPoolStatsPacket;

typedef DCFixedPacketImpl<CONNECTOR_STATS, DCT_CONNECTOR> DCConnectorStatsPacket;

typedef DCFixedPacketImpl<DOWNLOADER_STATS, DCT_DOWNLOADER> DCDownloaderStatsPacket;

typedef DCFixedPacketImpl<UPLOADER_STATS, DCT_UPLOADER> DCUploaderStatsPacket;





#endif
