#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_MMS_SERVER_COMMAND15_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_MMS_SERVER_COMMAND15_H_
#include "mmsservercommand.h"


namespace Synacast
{
	namespace Protocol
	{
		namespace Mms
		{
			class MmsServerCommand15 :
				public MmsServerCommand
			{
			public:
				MmsServerCommand15( UINT32 sequence, double time );
				~MmsServerCommand15(void);
				void BuildRawData(void);
			};

		}
	}
}

#endif