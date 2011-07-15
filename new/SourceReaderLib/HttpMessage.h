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
				�쳣��
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
				����һ��ͷ������Ψһ��ͷ����Ӧ��ʹ���������������Ӧ��ʹ��AddHeader����������ж��ͬkey��ͷ
				*/
				/************************************************************************/
				virtual void SetHeader(const string & key, const string & value);
				virtual void SetHeader( const char * key, const char * value );
				/************************************************************************/
				/* 
				���һ��ͷ���÷�������ͷ��key��ͬ������Ψһ��key��Ӧ��ʹ��SetHeader
				*/
				/************************************************************************/
				virtual void AddHeader( const string & key, const string & value);
				virtual void AddHeader( const char * key, const char * value );
				/************************************************************************/
				/* 
				��ȡ����key��ͷ�����û�У�vectorΪ��
				*/
				/************************************************************************/
				virtual const vector<string> GetHeaders( const string & key ) const;
				/************************************************************************/
				/* 
				��ȡͷ�������ж��key��ͷ��ֻ��������һ��
				���Ҫ��ȡ����ͬ����ͷ��ʹ��GetHeaders
				�쳣��
					NotFoundException
				*/
				/************************************************************************/
				virtual const string &		GetHeader( const string & key) const;

				/************************************************************************/
				/* 
				�������ݡ�
				*/
				/************************************************************************/
				virtual void SetContent( const char * data, size_t size);
				/************************************************************************/
				/* 
				׷������
				*/
				/************************************************************************/
				virtual void AppendContent( const DataBuffer & data );
				virtual void AppendContent( const char * data, size_t size);

				virtual const DataBuffer&	GetContent(void) const;

				/************************************************************************/
				/* 
				�쳣��
					FormatException
				*/
				/************************************************************************/
				virtual void SetRawHeader( const DataBuffer & header );
				/************************************************************************/
				/* 
				�쳣��
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