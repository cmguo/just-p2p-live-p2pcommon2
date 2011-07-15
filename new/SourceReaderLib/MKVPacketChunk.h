#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_MKV_PACKET_CHUNK_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_MKV_PACKET_CHUNK_H_
#include "PacketChunk.h"
#include "DataBuffer.h"

namespace Synacast
{
	namespace SourceReader
	{
		class SOURCEREADERLIB_API MKVPacketChunk :
			public PacketChunk
		{
		public:
			MKVPacketChunk(void);
			~MKVPacketChunk(void);

		private:
			DataBuffer	m_RawData;
			UINT32 m_TimeStamp;
		public:
			void SetRawData( const char * data, const size_t size );
			const DataBuffer & GetRawData();

			bool IsEmpty() { return FALSE; };

			size_t GetPacketLength();

			void SetTimeStamp(UINT32 TimeStamp);
			UINT32 GetTimeStamp();
		};

	}
}
#endif