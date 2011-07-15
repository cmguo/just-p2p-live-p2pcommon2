#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_IOEXCEPTION_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_IOEXCEPITON_H_

#include "SourceReaderLib.h"

#include "BaseException.h"

namespace Synacast
{
	namespace Exception
	{
		class SOURCEREADERLIB_API IOException :
			public BaseException
		{
		public:
			IOException(void);
			IOException( const string & message );
			IOException( const string & message, BaseException * innerException );
			virtual ~IOException(void);
		};

	}
}
#endif