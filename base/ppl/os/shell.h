
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_OS_SHELL_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_OS_SHELL_H_

#include <ppl/config.h>

#if defined(_PPL_PLATFORM_MSWIN)

#include <ppl/mswin/shell.h>

#elif defined(_PPL_PLATFORM_LINUX)

#include <ppl/posix/shell.h>

#else

#error "invalid platform"

#endif


#endif
