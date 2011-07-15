#pragma warning( disable: 4786 )
#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_STRING_UTIL_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_STRING_UTIL_H_

#include <string>
using namespace std;

namespace Synacast
{
	namespace Common
	{
		class StringUtil
		{
		public:
			StringUtil(void);
			~StringUtil(void);
			static string Lower(const string & s );
			static string Upper( const string & s );
			static bool StartWith( const string & s, const string & start );
			static bool EndWith( const string &s, const string & end );
			static string EncodeUrl( const string & s );
			static bool IsUrlNormalCharacter( char c );

			static string Oem2Utf8( const string & s );
			static wstring Oem2Unicode( const string & s );
		};
	}
}
#endif