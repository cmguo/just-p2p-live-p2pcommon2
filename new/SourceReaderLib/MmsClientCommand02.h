#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_MMS_CLIENT_COMMAND02_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_MMS_CLIENT_COMMAND02_H_
#include "mmsclientcommand.h"

namespace Synacast
{
	namespace Protocol
	{
		namespace Mms
		{

			class MmsClientCommand02 :
				public MmsClientCommand
			{
			public:
				MmsClientCommand02(void);
				~MmsClientCommand02(void);

			private:
				wstring	m_Info;

			public:
				void ParseData();
				const wstring & GetInfo() const	{ return m_Info; };
			};

		}
	}
}

#endif