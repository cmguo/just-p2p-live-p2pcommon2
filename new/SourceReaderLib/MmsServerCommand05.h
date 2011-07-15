#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_MMS_SERVER_COMMAND05_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_MMS_SERVER_COMMAND05_H_
#include "mmsservercommand.h"


namespace Synacast
{
	namespace Protocol
	{
		namespace Mms
		{

			class MmsServerCommand05 :
				public MmsServerCommand
			{
			public:
				MmsServerCommand05( UINT32 sequence, double time );
				~MmsServerCommand05(void);

			private:
				wstring		m_Filename;

			public:
				void SetRequestFile( const wstring & filename ) { m_Filename = filename; };
				void SetRequestFile( const string & filename );
				const wstring & GetRequestFile() const { return m_Filename; };
				void BuildRawData();
			};

		}
	}
}
#endif