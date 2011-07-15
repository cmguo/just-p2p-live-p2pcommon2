#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_SOURCE_CREATE_CONFIG_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_SOURCE_CREATE_CONFIG_H_
#include "SourceReaderLib.h"

#include <vector>
using namespace std;

#include "SourceTypes.h"

namespace Synacast
{
	namespace StreamSource
	{

		class SOURCEREADERLIB_API SourceCreateConfig
		{
		public:
			SourceCreateConfig(void);
			virtual ~SourceCreateConfig(void);

		public:
			virtual const vector<SourceType>  & GetTryList( const string & url ) const = 0;
		};

	}
}
#endif