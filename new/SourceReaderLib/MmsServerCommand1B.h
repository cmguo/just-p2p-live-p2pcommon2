#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_MMS_SERVER_COMMAND1B_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_MMS_SERVER_COMMAND1B_H_
#include "mmsservercommand.h"


namespace Synacast
{
	namespace Protocol
	{
		namespace Mms
		{

			class MmsServerCommand1B :
				public MmsServerCommand
			{
			public:
				MmsServerCommand1B( UINT32 sequence, double time );
				~MmsServerCommand1B(void);
			};

		}
	}
}
#endif