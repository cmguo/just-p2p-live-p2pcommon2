#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_FILE_SOURECE_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_FILE_SOURECE_H_

#include <string>
using namespace std;

#include "Source.h"

namespace Synacast
{
	namespace StreamSource
	{
		class SOURCEREADERLIB_API FileSource :
			public Source
		{
		public:
			FileSource(void);
			FileSource( const string & u);
			virtual ~FileSource(void);

		private:
			HANDLE		m_FileHandle;
			UINT64		m_CurrentPointer;

		public:
			/************************************************************************/
			/* 
			Exceptions:
				SourceException
			*/
			/************************************************************************/
			virtual void	Open(void);
			virtual void	Close(void);
			/************************************************************************/
			/* 
			Exceptions:
				SourceException
			*/
			/************************************************************************/
			virtual size_t	Read(char * buffer, size_t size);
			/************************************************************************/
			/* 
			Exceptions:
				SourceException
			*/
			/************************************************************************/
			virtual INT64	Seek(INT64 offset, SeekOrigin origin);
			virtual bool	CanRead(void);
			virtual bool	IsSeekable(void);
			/************************************************************************/
			/* 
			Exceptions:
				SourceException
			*/
			/************************************************************************/
			void CheckFormat(void);
		};

	}
}
#endif