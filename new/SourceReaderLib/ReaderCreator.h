#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_READER_CREATOR_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_READER_CREATOR_H_
#include "SourceReaderLib.h"
#include "Reader.h"
#include "Source.h"
using Synacast::StreamSource::Source;

namespace Synacast
{
	namespace SourceReader
	{
		// Creator º¯ÊýÔ­ÐÎ
		typedef PVOID (_stdcall *LPFN_CREATOR)( void *);
		
		class SOURCEREADERLIB_API ReaderCreator
		{
		public:
			ReaderCreator(void);
			~ReaderCreator(void);

		public:
			Reader * CreateReader( Source & source, ReaderType type );
			Reader * CreateReader( Source & source );
			Reader * CreateReader( LPCTSTR library, LPCSTR creatorName, void * param );
		};
	}
}

#endif