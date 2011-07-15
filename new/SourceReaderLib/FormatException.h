#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_FORMAT_EXCEPTION_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_FORMAT_EXCEPTION_H_

#include "BaseException.h"
using namespace Synacast::Exception;

namespace Synacast
{
	namespace Format
	{

		class FormatException :
			public BaseException
		{
		public:
			FormatException(void);
			FormatException( const string & message );
			virtual ~FormatException(void) throw();
		};

	}
}
#endif