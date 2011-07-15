#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_SOURCE_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_SOURCE_H_
#include "SourceReaderLib.h"


#include <string>
using namespace std;

#include "SourceException.h"
using namespace Synacast::Exception;

#include "SourceTypes.h"
#include "Url.h"
using namespace Synacast::Common;

#include "log.h"
// #define SOURCE_LOG_TYPE			1000
// 
// #define SOURCE_LIB_INFO(message)		SOURCE_LIB_LOG_IMPL(SOURCE_LOG_TYPE, __INFO, message)
// #define SOURCE_LIB_EVENT(message)		SOURCE_LIB_LOG_IMPL(SOURCE_LOG_TYPE, __EVENT, message)
// #define SOURCE_LIB_WARN(message)		SOURCE_LIB_LOG_IMPL(SOURCE_LOG_TYPE, __WARN, message)
// #define SOURCE_LIB_ERROR(message)		SOURCE_LIB_LOG_IMPL(SOURCE_LOG_TYPE, __ERROR, message)
// #define SOURCE_LIB_DEBUG(message)		SOURCE_LIB_LOG_IMPL(SOURCE_LOG_TYPE, __DEBUG, message)

namespace Synacast 
{
	namespace StreamSource 
	{

		class SOURCEREADERLIB_API Source
		{
		public:
			virtual ~Source(void);

		protected:
//			string		m_Url;
			Url			m_Url;
			SourceType	m_Type;
			wstring		m_Name;
			FileFormat	m_Format;
			bool		m_Open;

		protected:
			Source( SourceType _type, const wstring & _name );
			Source( const string & _url, SourceType _type, const wstring & _name );

			/************************************************************************/
			/* 
			Exceptions:
				IllegalArgumentException, header == NULL || size < 4
			*/
			/************************************************************************/
			static FileFormat CheckFormat( const char * id, size_t size );
		public:
		// info
			virtual void		SetUrl( const string & u ) { SetUrl( Url( u ) ); };
			virtual void		SetUrl( const Url & u ) { m_Url = u; };
			virtual Url &		GetUrl() { return m_Url; };

			virtual SourceType	GetSourceType() { return m_Type; };
			virtual wstring &	GetName() { return m_Name; };

			virtual FileFormat	GetFileFormat() { return m_Format; };

		// operations
			/************************************************************************/
			/* 
			Exceptions:
				SourceException
			*/
			/************************************************************************/
			virtual void	Open()		= 0;
			virtual void	Close()		= 0;
			/************************************************************************/
			/* 
			返回值：
			返回读取的字节数，0表示已经读完。
			Exceptions:
				SourceException
			*/
			/************************************************************************/
			virtual	size_t	Read( char * buffer, size_t size )		= 0;
			/************************************************************************/
			/* 
			Exceptions:
				SourceException
			*/
			/************************************************************************/
			virtual INT64	Seek( INT64 offset, SeekOrigin origin )	= 0;

		// attributes
			virtual bool	CanRead()		= 0;
			virtual bool	IsSeekable()	= 0;

		// status
			virtual bool	IsOpen() { return m_Open; };
		};

	}
}

#endif