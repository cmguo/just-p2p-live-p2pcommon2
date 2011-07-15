
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_CONCURRENT_THREAD_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_CONCURRENT_THREAD_H_

#include <ppl/config.h>


#if defined(_PPL_PLATFORM_LINUX)

#include <ppl/boostlib/thread.h>

namespace ppl { namespace concurrent { 

using ppl::boostlib::thread;

} }

#elif defined(_PPL_PLATFORM_MSWIN)

#include <ppl/mswin/thread.h>

namespace ppl { namespace concurrent { 

using ppl::mswin::thread;

} }


#endif


#endif





