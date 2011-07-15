#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_ASFDATA_PACKET_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_ASFDATA_PACKET_H_

#include "FormatTypes.h"
#include "DataBuffer.h"
using namespace Synacast::Common;

namespace Synacast
{
	namespace Format
	{

		class SOURCEREADERLIB_API AsfDataPacket
		{
		public:
			AsfDataPacket(void);
			~AsfDataPacket(void);

		public:
			UINT32		m_PacketLength;
			UINT32		m_Sequence;
			UINT32		m_PadLength;
			UINT32		m_SendTime;
			UINT16		m_Duration;

			bool		m_IsMultiPayloads;

			DataBuffer	m_RawData;

		public:
			void		SetRawData( const char * data, const size_t size );
			const DataBuffer & GetRawData() const { return m_RawData; };

			UINT32 GetPacketLength()const { return m_PacketLength; };
			UINT32 GetSequence()	const { return m_Sequence; };
			UINT32 GetPadLength()	const { return m_PadLength; };
			UINT32 GetSendTime()	const { return m_SendTime; };
			UINT16 GetDuration()	const { return m_Duration; };

		private:
			void		Parse();
			UINT32		GetLengthField( const char * data, size_t & offset, const AsfLengthType type );
		};

	}
}
#endif