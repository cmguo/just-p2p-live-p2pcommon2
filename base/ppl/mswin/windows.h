#ifndef _LIVE_P2PCOMMON2_BASE_PPL_MSWIN_WINDOWS_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_MAWIN_WINDOWS_H_

#include <ppl/config.h>

#if !defined(_PPL_PLATFORM_MSWIN)
#error "platform must be mswin "
#endif



#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#endif

// Windows Header Files:
#include <windows.h>


#endif
