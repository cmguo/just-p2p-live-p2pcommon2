#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_MMS_SERVER_COMMAND02_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_MMS_SERVER_COMMAND02_H_
#include "mmsservercommand.h"


namespace Synacast
{
	namespace Protocol
	{
		namespace Mms
		{

			class MmsServerCommand02 :
				public MmsServerCommand
			{
			public:
				MmsServerCommand02( UINT32 sequence, double time );
				~MmsServerCommand02(void);

			private:
				wstring m_Transport;

			public:
				void SetTransportInfo( const wstring & transport ) { m_Transport = transport; };
				const wstring & GetTransportInfo() const { return m_Transport; };

				void BuildRawData();
			};

		}
	}
}
#endif