
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_TEXT_CONV_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_TEXT_CONV_H_

#include <ppl/config.h>

#include <ppl/data/tchar.h>


#if defined(_PPL_PLATFORM_MSWIN )

#include <ppl/mswin/atl/conv.h>

#elif defined(_PPL_PLATFORM_LINUX)

#define CW2A(x) ( x )
#define CT2A(x) ( x )

#endif

#endif
