#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_MMS_SERVER_COMMAND07_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_MMS_SERVER_COMMAND07_H_
#include "mmsservercommand.h"


namespace Synacast
{
	namespace Protocol
	{
		namespace Mms
		{
			class MmsServerCommand07 :
				public MmsServerCommand
			{
			public:
				MmsServerCommand07( UINT32 sequence, double time );
				~MmsServerCommand07(void);
				void BuildRawData(void);

			private:
				double	m_SeekTime;
				UINT32	m_MaxStreamTime;
				INT32	m_PacketIDType;
			};

		}
	}
}

#endif