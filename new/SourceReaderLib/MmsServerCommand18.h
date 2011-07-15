#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_MMS_SERVER_COMMAND18_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_MMS_SERVER_COMMAND18_H_
#include "mmsservercommand.h"


namespace Synacast
{
	namespace Protocol
	{
		namespace Mms
		{

			class MmsServerCommand18 :
				public MmsServerCommand
			{
			public:
				MmsServerCommand18( UINT32 sequence, double time );
				~MmsServerCommand18(void);
			};

		}
	}
}
#endif