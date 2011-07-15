#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_HTTP_RESPONSE_READER_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_HTTP_RESPONSE_READER_H_

#include "HttpReader.h"

#include "HttpResponse.h"
#include "HttpChunk.h"

namespace Synacast
{
	namespace Protocol
	{
		namespace Http
		{

			class HttpResponseReader : public HttpReader
			{
			public:
				HttpResponseReader(void);
				virtual ~HttpResponseReader(void);

			private:
				HttpResponse m_Response;

			public:
				/************************************************************************/
				/* 
				异常：
					FormatException
					IOException
				*/
				/************************************************************************/
				virtual void Read(SOCKET server);
				/************************************************************************/
				/* 
				异常：
					FormatException
					IOException
				*/
				/************************************************************************/
				virtual void ReadHeader(SOCKET server);
				/************************************************************************/
				/* 
				异常：
					IOException
				*/
				/************************************************************************/
				virtual void ReadContent(SOCKET server);
				/************************************************************************/
				/* 
				异常：
				IOException
				*/
				/************************************************************************/
				static const HttpChunk ReadChunk(SOCKET server);
				const HttpResponse & GetResponse(void) const;
			};

		}
	}
}
#endif