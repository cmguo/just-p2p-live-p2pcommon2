#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_RTP_RTSP_DATA_PACKET_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_RTP_RTSP_DATA_PACKET_H_

#include "DataBuffer.h"
#include "DataPacket.h"
using namespace Synacast::Protocol;

using namespace Synacast::Common;

namespace Synacast
{
	namespace Protocol
	{
		namespace Rtsp
		{

			class RtpRtspDataPacket : public DataPacket
			{
			public:
				RtpRtspDataPacket(void);
				virtual ~RtpRtspDataPacket(void);

				virtual UINT16 GetSequence() const { return (UINT16)m_Sequence; };

			private:
				DataBuffer	m_Header;

				UINT32		m_TimeStamp;
				INT32		m_SSRC;			// Synchronization Source Identifier
				INT32		m_CSRC;			// Contributing Source Identifier

			public:
				void		SetRawData( const char * data, const size_t size );
				/************************************************************************/
				/* 
					Header should be 16 bytes
				Exceptions:
					IllegalArgumentException, header == NULL || size < 16
				*/
				/************************************************************************/
				void SetHeader( const char * header, const size_t size );

			private:
				void		ParseHeader();
			};

		}
	}
}
#endif