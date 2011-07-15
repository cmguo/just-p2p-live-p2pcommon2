#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_HTTP_RESPONSE_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_HTTP_RESPONSE_H_

#include "HttpMessage.h"

namespace Synacast
{
	namespace Protocol
	{
		namespace Http
		{

			/************************************************************************/
			/* 
			HTTP ��Ӧ��ͨ�ø�ʽ���£�
			[protocol][space][status][space][reason]\r\n
			[headers]
			\r\n
			[content]
			*/
			/************************************************************************/
			class HttpResponse : public HttpMessage
			{
			public:
				HttpResponse(void);
				HttpResponse( const string & protocol, const unsigned int status, const string & reason );
				~HttpResponse(void);

			private:
				string m_Protocol;
				unsigned int m_Status;
				string m_Reason;

				DataBuffer m_RawData;

			public:
				const string &	GetProtocol()	const { return m_Protocol; };
				unsigned int	GetStatus()		const { return m_Status; };
				const string &	GetReason()		const { return m_Reason; };
				virtual bool	IsOK()			const { return m_Status >= 200 && m_Status < 300;}

				// build
				virtual const DataBuffer & ToDataBuffer();

			public:
				/************************************************************************/
				/* 
				�쳣��
					FormatException
				*/
				/************************************************************************/
				virtual void SetRawData( const DataBuffer & data );
				/************************************************************************/
				/* 
				�쳣��
				FormatException
				*/
				/************************************************************************/
				virtual void SetRawData( const char * data, const size_t size );
				/************************************************************************/
				/* 
				�쳣��
				FormatException
				*/
				/************************************************************************/
				virtual void AppendRawData( const DataBuffer & data );
				/************************************************************************/
				/* 
				�쳣��
				FormatException
				*/
				/************************************************************************/
				virtual void AppendRawData( const char * data, const size_t size );
//				virtual const DataBuffer & GetRawResponse() const;


			protected:
				/************************************************************************/
				/* 
				����HTTP Response
				�쳣��
					FormatException
				*/
				/************************************************************************/
				void Parse();
				/************************************************************************/
				/* 
				Exceptions:
					FormatException
				*/
				/************************************************************************/
				virtual void ParseHeadLine( const string & line);
			};
		}
	}
}
#endif