#ifndef _LIVE_P2PCOMMON2_NEW_COMMON_STDBASE_H_
#define _LIVE_P2PCOMMON2_NEW_COMMON_STDBASE_H_


#include <ppl/config.h>
#include <ppl/config/vc8warnings.h>

#if defined(_PPL_PLATFORM_MSWIN)

#include <ppl/mswin/windows.h>

#elif defined(_PPL_PLATFORM_LINUX)

#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <unistd.h>

#endif

#if defined(_PPL_PLATFORM_MSWIN)

// 先包含allocator.h，避免包含memoryleak.h时可能导致的出错
#include <ppl/data/pool.h>
#include <ppl/diag/memoryleak.h>

#endif


#include <boost/version.hpp>

//#if BOOST_VERSION != 103600
//#error invalid boost library version, 1.36 is required
//#endif


//#if defined(BOOST_SP_USE_QUICK_ALLOCATOR)
//#pragma message("------use boost.smart_ptr with BOOST_SP_USE_QUICK_ALLOCATOR")
//#elif defined(BOOST_SP_USE_STD_ALLOCATOR)
//#pragma message("------use boost.smart_ptr with BOOST_SP_USE_STD_ALLOCATOR")
//#else
//#pragma message("------use boost.smart_ptr")
//#endif






//#include <ppl/data/guid.h>
//#include <ppl/data/guid_less.h>

//#include <ppl/util/time_counter.h>
namespace ppl { namespace util {
class time_counter;
} }
using ppl::util::time_counter;

//#include <ppl/net/inet.h>

#include <synacast/mfcmodule.h>

#define CORE_MALLOC(size) malloc(size)
#define CORE_FREE(p) free(p)


//#include <ppl/util/stlpool.h>


/*

// precompiled header files
#include <ppl/util/macro.h>
#include <ppl/net/inet.h>
#include <ppl/data/guid.h>
#include <ppl/data/guid_less.h>
#include <ppl/util/time_counter.h>
#include <ppl/data/int.h>

#include <boost/shared_ptr.hpp>
*/


#include "framework/log.h"

#include "framework/timer.h"

#endif
