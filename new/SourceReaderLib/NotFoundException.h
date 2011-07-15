#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_NOTFOUND_EXCEPTION_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_NOTFOUND_EXCEPTION_H_

#include "BaseException.h"

namespace Synacast
{
	namespace Exception
	{
		class NotFoundException :
			public BaseException
		{
		public:
			NotFoundException(void);
			NotFoundException( const string & message );
			virtual ~NotFoundException(void);
		};

	}
}
#endif