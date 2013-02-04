
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_DATA_TSTRING_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_DATA_TSTRING_H_

#include <ppl/config.h>


#include <string>

#include <ppl/data/tchar.h>

using std::string;
#if (defined _PPL_PLATFORM_MSWIN || defined _GLIBCXX_USE_WCHAR_T)
using std::wstring;
#endif



#ifdef UNICODE
typedef wstring tstring;
#else
typedef string tstring;
#endif



#endif
