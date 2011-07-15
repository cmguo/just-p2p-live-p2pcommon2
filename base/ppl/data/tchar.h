
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_DATA_TCHAR_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_DATA_TCHAR_H_

#include <ppl/config.h>

#include <wchar.h>
#include <stdio.h>


#if defined(_PPL_PLATFORM_MSWIN)

#include <tchar.h>
#ifndef vsnprintf
#define vsnprintf _vsnprintf
#endif


#else

#if defined(UNICODE) || defined(_UNICODE)

#define __T(x)      L ## x
typedef wchar_t TCHAR;

#else

#define __T(x)      x

#endif

typedef char TCHAR;

#ifndef _T
#define _T(x)       __T(x)
#endif

#define _TEXT(x)    __T(x)
#define TEXT(x) _T(x)

#endif


typedef const char* LPCSTR;
typedef char* LPSTR;

typedef const TCHAR* LPCTSTR;
typedef TCHAR* LPTSTR;


#endif
