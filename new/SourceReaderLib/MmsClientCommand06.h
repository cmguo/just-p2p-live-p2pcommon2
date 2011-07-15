#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_MMS_CLIENT_COMMAND06_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_MMS_CLIENT_COMMAND06_H_
#include "mmsclientcommand.h"

namespace Synacast
{
	namespace Protocol
	{
		namespace Mms
		{


			class MmsClientCommand06 :
				public MmsClientCommand
			{
			public:
				MmsClientCommand06(void);
				~MmsClientCommand06(void);

			private:
				INT32	m_ResultFlag;
				INT32	m_BroadcastFlag;
				double	m_TimePoint;
				INT32	m_MediaDuration;
				UINT32	m_PacketSize;
				UINT32	m_PacketCount;
				UINT32	m_MaxBitrate;
				UINT32	m_HeaderSize;

			public:
				void ParseData();

				INT32	GetResultFlag()		const	{ return m_ResultFlag; };
				INT32	GetBroadcastFlag()	const	{ return m_BroadcastFlag; };
				double	GetTimePoint()		const	{ return m_TimePoint; };
				INT32	GetMediaDuration()	const	{ return m_MediaDuration; };
				UINT32	GetPacketSize()		const	{ return m_PacketSize; };
//				UINT32	GetPacketCount()	const	{ return m_PacketCount; };
				UINT32	GetMaxBitrate()		const	{ return m_MaxBitrate; };
				UINT32	GetHeaderSize()		const	{ return m_HeaderSize; };

				bool	IsAccepted()		const	{ return m_ResultFlag > 0; };
			};

		}
	}
}
#endif