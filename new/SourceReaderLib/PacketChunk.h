#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_PACKET_CHUNK_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_PACKET_CHUNK_H_
#include "DataBuffer.h"
using namespace Synacast::Common;

namespace Synacast
{
	namespace SourceReader
	{

		class SOURCEREADERLIB_API PacketChunk
		{
		public:
			PacketChunk(void);
			virtual ~PacketChunk(void);

		public:
			virtual void SetRawData( const char * data, const size_t size ) = 0;
			virtual const DataBuffer & GetRawData() = 0;

			virtual size_t GetPacketLength()	= 0;
			virtual UINT32 GetTimeStamp()		= 0;
		};

	}
}

#endif