#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_RM_PACKET_CHUNK_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_RM_PACKET_CHUNK_H_

#include "PacketChunk.h"

#include "RmDataPacket.h"
using namespace Synacast::Format;

namespace Synacast
{
	namespace SourceReader
	{
		class SOURCEREADERLIB_API RmPacketChunk :
			public PacketChunk
		{
		public:
			RmPacketChunk(void);
			~RmPacketChunk(void);

		private:
			RmDataPacket		m_DataPacket;

		public:
			void SetRawData( const char * data, const size_t size );
			const DataBuffer & GetRawData();

			void SetStreamType( const RmStreamType type )	{ m_DataPacket.SetStreamType( type ); };
			UINT16	GetStreamID()	const					{ return m_DataPacket.GetStreamID(); };

			const RmDataPacket & GetDataPacket() const { return m_DataPacket; };
			bool IsEmpty() { return m_DataPacket.GetData().GetSize() == 0; };

			size_t GetPacketLength()	{ return m_DataPacket.GetLength(); };
			UINT32 GetTimeStamp()		{ return m_DataPacket.GetTimeStamp(); };
		};
	}
}

#endif