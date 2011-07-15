#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_HTTP_SOURCE_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_HTTP_SOURCE_H_

#include <string>
using namespace std;

#include "NetworkSource.h"


#include "HttpRequest.h"
#include "HttpResponse.h"
using namespace Synacast::Protocol::Http;

/*
virtual void	Open()		= 0;
virtual void	Close()	= 0;
virtual	int		Read( byte * buffer, size_t size )		= 0;
virtual int64	Seek( int64 offset, SeekOrigin origin )	= 0;

// attributes
virtual bool	CanRead()	= 0;
virtual bool	IsSeekable() = 0;
*/

namespace Synacast
{
	namespace StreamSource
	{

		class SOURCEREADERLIB_API HttpSource :
			public NetworkSource
		{
		public:
			HttpSource(void);
			HttpSource( const string & url );
			virtual ~HttpSource(void);

		protected:
			HttpRequest		m_Request;
			HttpResponse	m_Response;

			size_t			m_ContentLength;
			size_t			m_Read;

			string			m_TransferEncoding;	// not used now

		public:
			virtual void	SetUrl( const Url & u );
			virtual void	Open();
			virtual void	Close();
			/************************************************************************/
			/* 
			没有处理Chunk传输的情况
			也没有很好的处理没有Content-Length的情况

			Exceptions:
				SourceException
			*/
			/************************************************************************/
			virtual size_t	Read( char * buffer, size_t size );
			/************************************************************************/
			/* 
			Exceptions:
				SourceException
			*/
			/************************************************************************/
			virtual INT64	Seek( INT64 offset, SeekOrigin origin );

			virtual bool	CanRead();
			virtual bool	IsSeekable();
		protected:
			virtual void	SendRequest(void);
			/************************************************************************/
			/* 
			异常：
				FormatException
				SourceException
			*/
			/************************************************************************/
			virtual void	ReadResponse(void);
		public:
			virtual unsigned short GetDefaultPort(void);

		private:
			/************************************************************************/
			/* 
			异常：
				FormatException
				SourceException
			*/
			/************************************************************************/
			void CheckFormat(void);

			/************************************************************************/
			/* 
			Exceptions:
				SourceException
			*/
			/************************************************************************/
			void OpenSource(void);
		};

	}
}
#endif