#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_RM_INDEX_RECORD_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_RM_INDEX_RECORD_H_
#include "SourceReaderLib.h"

namespace Synacast
{
	namespace Format
	{

		class SOURCEREADERLIB_API RmIndexRecord
		{
		public:
			RmIndexRecord(void);
			~RmIndexRecord(void);

		private:
			UINT16		m_Version;		// object_version;

			UINT32		m_TimeStamp;	//	timestamp;
			UINT32		m_Offset;		//  offset;
			UINT32		m_PacketCount;	//  packet_count_for_this_packet;

		public:
			UINT16			GetVersion()		const { return m_Version; };

			UINT32			GetTimeStamp()		const { return m_TimeStamp; };
			UINT32			GetOffset()			const { return m_Offset; };
			UINT32			GetPacketCount()	const { return m_PacketCount; };
		};
	}
}

#endif