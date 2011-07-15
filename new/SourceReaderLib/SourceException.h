#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_SOURCE_EXCEPTION_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_SOURCE_EXCEPTION_H_

#include "IOException.h"
using namespace Synacast::Exception;

namespace Synacast
{
	namespace StreamSource
	{

		class SOURCEREADERLIB_API SourceException :
			public BaseException
		{
		public:
			SourceException(void);
			SourceException( const string & message );
			SourceException( const char * message );
			SourceException( const string & message, const BaseException * innerException );
			virtual ~SourceException(void);
		};

	}

}
#endif