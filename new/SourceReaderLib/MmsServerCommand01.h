#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_MMS_SERVER_COMMAND01_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_MMS_SERVER_COMMAND01_H_
#include "mmsservercommand.h"


namespace Synacast
{
	namespace Protocol
	{
		namespace Mms
		{

			class MmsServerCommand01 :
				public MmsServerCommand
			{
			public:
				MmsServerCommand01( UINT32 sequence, double time );
				~MmsServerCommand01(void);

			private:
				wstring m_PlayerInfo;

			public:
				void SetPlayerInfo( const wstring & player ) { m_PlayerInfo = player; };
				const wstring & GetPlayerInfo() const { return m_PlayerInfo; };

				void BuildRawData();
			};

		}
	}
}
#endif