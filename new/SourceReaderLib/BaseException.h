#ifdef _PPL_PLATFORM_MSWIN
#pragma warning(disable: 4275)
#pragma warning(disable: 4251)
#endif
#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_BASEEXCEPTION_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_BASEEXCEPTION_H_


#include "SourceReaderLib.h"

#include <string>
#include <exception>
using namespace std;

namespace Synacast
{
	namespace Exception
	{

		class SOURCEREADERLIB_API BaseException :
			public std::exception
		{
		public:
			BaseException(void);
			BaseException( const string & message );
			BaseException( const char * message );
			BaseException( const string & message, const BaseException * innerException );
			virtual ~BaseException(void) throw();

		protected:
			string					m_Message;
			const BaseException *	m_InnerException;

		public:
			virtual void SetMessage( const string & message ) { m_Message = message; };
			virtual void SetMessage( const char * message );

			virtual void SetInnerException( const BaseException * exception ) { m_InnerException = exception; };

			virtual const string & GetMessage() const { return m_Message; };
			virtual const char * GetMessageCStr() const;
			virtual const BaseException * GetInnerException() const { return m_InnerException; };
		};

	}
}

#endif
