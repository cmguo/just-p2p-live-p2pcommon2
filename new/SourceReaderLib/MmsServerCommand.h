#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_MMS_SERVER_COMMAND_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_MMS_SERVER_COMMAND_H_

#include "MmsCommand.h"

namespace Synacast
{
	namespace Protocol
	{
		namespace Mms
		{
			class MmsServerCommand :
				public MmsCommand
			{
			public:
				MmsServerCommand( UINT16 command, UINT32 sequence, double timeStamp, INT32 prefix1, INT32 prefix2 );
				virtual ~MmsServerCommand(void);

			public:
				virtual void Build();
			protected:
				virtual void BuildHeader();
				virtual void BuildRawData();
				virtual void BuildData();
				virtual void PadData();

			};

		}
	}
}

#endif