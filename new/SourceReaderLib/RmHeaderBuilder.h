#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_RM_HEADER_BUILDER_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_RM_HEADER_BUILDER_H_

#include "RmObject.h"
#include "RmContentDescriptionHeader.h"
#include "RmDataChunkHeader.h"
#include "RmFileHeader.h"
#include "RmMediaPropertiesHeader.h"
#include "RmPropertiesHeader.h"
using namespace Synacast::Format;

#include "SdpAttribute.h"
#include "SdpDescription.h"

#include <vector>
using namespace std;

namespace Synacast
{
	namespace Format
	{
		class RmHeaderBuilder
		{
		public:
			RmHeaderBuilder(void);
			~RmHeaderBuilder(void);

		public:
			static vector<RmObject*>		BuildHeaders( const HttpResponse & response );

			static vector<SdpDescription>	ResponseToDescriptions( const HttpResponse & response );
			static void						ParseDescription( const vector<SdpDescription> &, vector<SdpDescription> &, vector<vector<SdpDescription> *> & );

			static RmContentDescriptionHeader *		BuildContentFromSession( const vector<SdpDescription> & session );
			static vector<RmMediaPropertiesHeader*> BuildMediaHeaders( const vector<vector<SdpDescription> *> & mediaDescriptions );
		};
	}
}

#endif