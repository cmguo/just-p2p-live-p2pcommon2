#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_ILLEGAL_ARGUMENT_EXCEPTION_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_ILLEGAL_ARGUMENT_EXCEPTION_H_

#include "BaseException.h"

namespace Synacast
{
	namespace Exception
	{

		class IllegalArgumentException :
			public BaseException
		{
		public:
			IllegalArgumentException(void);
			IllegalArgumentException( const string & message );
			IllegalArgumentException( const char * message );
			virtual ~IllegalArgumentException(void);
		};

	}
}
#endif