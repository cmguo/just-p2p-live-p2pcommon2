#pragma warning(disable: 4251)
#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_URL_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_URL_H_
#include "SourceReaderLib.h"

#include <string>
using namespace std;

namespace Synacast
{
	namespace Common
	{

		/************************************************************************/
		/* 
		Url类，提供Url的封装解析操作，支持互联网URL协议的解析，文件URL的解析(file://, file:///)
		URL通用格式如下：
		[request]?[param]
		[request] = [schema]://[host]:[port]/[path]
		*/
		/************************************************************************/
		class SOURCEREADERLIB_API Url
		{
		public:
			Url();
			/************************************************************************/
			/* 
			创建一个URL对象，解析字符串u
			*/
			/************************************************************************/
			Url( const string & u );
			Url( const char * u );
			virtual ~Url(void);

		public:
			/************************************************************************/
			/* 
			解析u
			*/
			/************************************************************************/
			void ParseUrl( const string & u );

			void			SetPort( unsigned short _port );
			unsigned short	GetPort() const;

			void			SetPath( const string & path )		{ m_Path = path; };
			const string &	GetPath() const;

			void			SetHost( const string & host )		{ m_Host = host; };
			const string &	GetHost() const;

			void			SetScheme( const string & scheme )	{ m_Scheme = scheme; };
			const string &	GetScheme() const;

			void			SetParam( const string & param )	{ m_Param = param; };
			const string &	GetParam() const;

			const string &	GetRequest() const;

			const string &	GetOriginalUrl() const;

		private:
			void ParseAsFile( const string & u );

		private:
			string m_OriginalUrl;

			string m_Scheme;
			string m_Host;
			unsigned short m_Port;
			string m_Path;
			string m_Param;
			string m_Request;

			string m_result;

		public:
			const string& ToString( void );
		};

	}
}
#endif