#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_RDT_RTSP_DATA_PACKET_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_RDT_RTSP_DATA_PACKET_H_
#include "RtspTypes.h"
#include "DataPacket.h"
using namespace Synacast::Protocol;

namespace Synacast
{
	namespace Protocol
	{
		namespace Rtsp
		{

			class RdtRtspDataPacket :
				public DataPacket
			{
			public:
				RdtRtspDataPacket(void);
				virtual ~RdtRtspDataPacket(void);

			private:
				char		m_RdtFlag;
				bool		m_LengthIncluded;
				bool		m_NeedReliable;
				UINT16		m_StreamID;
				bool		m_IsReliable;

				PacketType	m_PacketType;
				UINT16		m_Sequence;				// rolling at 0xFEFF

				UINT16		m_RdtPacketLength;

				bool		m_Back2Back;
				bool		m_SlowData;
				UINT16		m_AsmRule;

				UINT32		m_TimeStamp;

				UINT16		m_TotalReliable;

				bool		m_PacketSent;		// stream end packet only
				bool		m_ExtFlag;			// stream end packet only

			public:
				PacketType	GetPacketType() const { return m_PacketType; };
				UINT16		GetSequence() const { return m_Sequence; };
				void		SetRawData( const char * data, size_t size );

				UINT16		GetStreamID()	const { return m_StreamID; };
				UINT32		GetTimeStamp()	const { return m_TimeStamp; };

				UINT16		GetAsmRule() const { return m_AsmRule; };

			private:
				void		ParseStreamEnd( const char * data, size_t size );
				void		ParseData( const char * data, size_t size );
				void		ParseLatency( const char * data, size_t size );
				void		ParseRttResponse( const char * data, size_t size );
			};

		}
	}
}
#endif