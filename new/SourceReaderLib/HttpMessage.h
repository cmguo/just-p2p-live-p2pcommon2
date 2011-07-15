#pragma warning(disable: 4786)
#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_HTTP_MESSAGE_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_HTTP_MESSAGE_H_

#include <map>
#include <string>
#include <vector>
using namespace std;

#include "DataBuffer.h"
using namespace Synacast::Common;

namespace Synacast
{
	namespace Protocol
	{
		namespace Http
		{
			class HttpMessage
			{
			public:
				HttpMessage(void);
				virtual ~HttpMessage(void);

			protected:
				multimap<string, string> m_Headers;

				DataBuffer m_Header;
				DataBuffer m_Content;

			public:
				/************************************************************************/
				/* 
				异常：
					FormatException
				*/
				/************************************************************************/
				virtual void ParseHeader();

			protected:
				virtual void ParseHeaders( const string & raw);
				virtual void ParseOneHeader( const string & line);
				virtual void ParseHeadLine( const string & line) = 0;

			public:
				/************************************************************************/
				/* 
				设置一个头，对于唯一的头，都应该使用这个方法，而不应该使用AddHeader，否则可能有多个同key的头
				*/
				/************************************************************************/
				virtual void SetHeader(const string & key, const string & value);
				virtual void SetHeader( const char * key, const char * value );
				/************************************************************************/
				/* 
				添加一个头，该方法允许头的key相同。对于唯一的key，应该使用SetHeader
				*/
				/************************************************************************/
				virtual void AddHeader( const string & key, const string & value);
				virtual void AddHeader( const char * key, const char * value );
				/************************************************************************/
				/* 
				获取所有key的头，如果没有，vector为空
				*/
				/************************************************************************/
				virtual const vector<string> GetHeaders( const string & key ) const;
				/************************************************************************/
				/* 
				获取头，对于有多个key的头，只返回其中一个
				如果要获取所有同名的头，使用GetHeaders
				异常：
					NotFoundException
				*/
				/************************************************************************/
				virtual const string &		GetHeader( const string & key) const;

				/************************************************************************/
				/* 
				设置内容。
				*/
				/************************************************************************/
				virtual void SetContent( const char * data, size_t size);
				/************************************************************************/
				/* 
				追加内容
				*/
				/************************************************************************/
				virtual void AppendContent( const DataBuffer & data );
				virtual void AppendContent( const char * data, size_t size);

				virtual const DataBuffer&	GetContent(void) const;

				/************************************************************************/
				/* 
				异常：
					FormatException
				*/
				/************************************************************************/
				virtual void SetRawHeader( const DataBuffer & header );
				/************************************************************************/
				/* 
				异常：
				FormatException
				*/
				/************************************************************************/
				virtual void SetRawHeader( const char * data, const size_t size );

				virtual const DataBuffer & GetRawHeader() const { return m_Header; };
			};

		}
	}
}
#endif