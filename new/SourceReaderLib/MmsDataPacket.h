#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_MMS_DATA_PACKET_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_MMS_DATA_PACKET_H_

#include "DataBuffer.h"
using namespace Synacast::Common;

#include "DataPacket.h"
using namespace Synacast::Protocol;

namespace Synacast
{
	namespace Protocol
	{
		namespace Mms
		{
			/************************************************************************/
			/* 
			MmsDataPacket的头部由sequence, packet id, flags, length可以唯一确定，
			因此并不存储头部，只存储数据部分。
			*/
			/************************************************************************/
			class MmsDataPacket : public DataPacket
			{
			public:
				MmsDataPacket(void);
				virtual ~MmsDataPacket(void);

			protected:
				char			m_PacketID;
				char			m_Flags;
				UINT16			m_Length;

			public:
				char				GetPacketID()	const { return m_PacketID; };
				char				GetFlags()		const { return m_Flags; };
				UINT16				GetLength()		const { return m_Length; };
				UINT16				GetDataLength()	const { return m_Length - 8; };

				void				ParseHeader( const DataBuffer & data );
				void				ParseHeader( const char * data );
			};
		}
	}
}

#endif