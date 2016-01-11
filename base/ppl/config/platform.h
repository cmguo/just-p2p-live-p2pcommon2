
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_CONFIG_PLATFORM_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_CONFIG_PLATFORM_H_


#if defined(linux) || defined(__linux) || defined(__linux__) || defined(__FreeBSD__)||defined(__ANDROID__)

#include <ppl/config/platform/linux.h>

#elif defined(_WIN32) || defined(__WIN32__) || defined(WIN32)

#include <ppl/config/platform/mswin.h>

#else

#error invalid platform

#endif
#endif
