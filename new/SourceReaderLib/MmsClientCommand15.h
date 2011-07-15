#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_MMS_CLIENT_COMMAND15_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_MMS_CLIENT_COMMAND15_H_
#include "mmsclientcommand.h"

namespace Synacast
{
	namespace Protocol
	{
		namespace Mms
		{


			class MmsClientCommand15 :
				public MmsClientCommand
			{
			public:
				MmsClientCommand15(void);
				~MmsClientCommand15(void);

			private:
				INT32	m_ClientID;

			public:
				void ParseData();
				INT32	GetClientID() { return m_ClientID; };
			};

		}
	}
}
#endif