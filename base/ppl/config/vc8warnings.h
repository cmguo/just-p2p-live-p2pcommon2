#ifndef _LIVE_P2PCOMMON2_BASE_PPL_CONFIG_VC8WARNINGS_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_CONFIG_VC8WARNINGS_H_

#define _SECURE_ATL 1

#ifndef _CRT_NONSTDC_NO_WARNINGS
#define _CRT_NONSTDC_NO_WARNINGS
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS 1
#endif

#ifndef _SCL_SECURE_NO_DEPRECATE
#define _SCL_SECURE_NO_DEPRECATE
#endif

#ifdef _PPL_PLATFORM_MSWIN
#pragma warning(disable: 4786)
#pragma warning(disable: 4127)
#endif

#ifndef _CRT_RAND_S
#define _CRT_RAND_S
#endif

#ifdef _PPL_PLATFORM_MSWIN
#pragma warning(disable: 4100)
#pragma warning(disable: 4189)
#pragma warning(disable: 4996)
#pragma warning(disable: 4127)


#pragma warning(disable: 4819)
#endif

#endif
