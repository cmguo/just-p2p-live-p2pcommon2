#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_MMS_SERVER_COMMAND33_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_MMS_SERVER_COMMAND33_H_
#include "mmsservercommand.h"

#include <vector>
using namespace std;

namespace Synacast
{
	namespace Protocol
	{
		namespace Mms
		{
			class StreamSelect
			{
			public:
				INT16	m_Flags;
				INT16	m_ID;
				INT16	m_State;

				StreamSelect( INT16 id ) : m_Flags( (INT16)-1 ), m_ID( id ), m_State( 0 ) {};
				StreamSelect( INT16 id, INT16 state ) : m_Flags( (INT16)-1 ), m_ID( id ), m_State( state ) {};
			};

			class MmsServerCommand33 :
				public MmsServerCommand
			{
			public:
				MmsServerCommand33( UINT32 sequence, double time );
				~MmsServerCommand33(void);

			public:
				void BuildRawData(void);
				void AddStream( StreamSelect stream ) { m_SelectedStreams.push_back( stream ); };

			private:
				vector<StreamSelect> m_SelectedStreams;
			};

		}
	}
}
#endif