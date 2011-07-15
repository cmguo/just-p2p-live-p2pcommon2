
#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_ASFPACKET_CHUNK_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_ASFPACKET_CHUNK_H_
#include "PacketChunk.h"
#include "AsfDataPacket.h"
using namespace Synacast::Format;

namespace Synacast
{
	namespace SourceReader
	{
		class SOURCEREADERLIB_API AsfPacketChunk :
			public PacketChunk
		{
		public:
			AsfPacketChunk(void);
			~AsfPacketChunk(void);

		private:
			AsfDataPacket		m_DataPacket;

		public:
			void SetRawData( const char * data, const size_t size );
			const DataBuffer & GetRawData();

			const AsfDataPacket & GetAsfDataPacket() const { return m_DataPacket; };
			bool	IsEmpty() { return m_DataPacket.GetRawData().GetSize() == 0; };

			size_t GetPacketLength() { return m_DataPacket.GetRawData().GetSize(); };
			UINT32 GetTimeStamp() { return m_DataPacket.GetSendTime(); };
		};

	}
}
#endif