#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_HTTP_REQUEST_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_HTTP_REQUEST_H_

#include "HttpMessage.h"

namespace Synacast
{
	namespace Protocol
	{
		namespace Http
		{

			/************************************************************************/
			/* 
			HTTP请求，HTTP请求的格式如下：
			[method][space][path][space][protocol]\r\n
			[headers]
			\r\n
			[content]
			*/
			/************************************************************************/
			class HttpRequest : public HttpMessage
			{
			public:
				HttpRequest(void);
				HttpRequest( const string & method, const string & path, const string & protocol);
				virtual ~HttpRequest(void);

			protected:
				string m_Method;
				string m_Path;
				string m_Protocol;

				DataBuffer m_ByteBuffer;
			public:

				/************************************************************************/
				/* 
				将HTTP Request转换成字节数据。
				*/
				/************************************************************************/
				virtual const DataBuffer & ToDataBuffer();

			protected:
				/************************************************************************/
				/* 
				Exceptions:
					FormatException
				*/
				/************************************************************************/
				virtual void ParseHeadLine( const string & line);

			private: // not implemented
				virtual void FromData( const DataBuffer & data );
				virtual void FromData( const char * data, size_t size );
			public:
				virtual void			SetMethod( const string & method);
				virtual const string &	GetMethod(void) const;

				virtual void			SetPath( const string & path);
				virtual const string &	GetPath(void) const;

				virtual void			SetProtocol( const string & protocol);
				virtual const string &	GetProtocol(void) const;

			};

		}
	}
}
#endif