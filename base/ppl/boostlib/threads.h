
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_BOOSTLIB_THREADS_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_BOOSTLIB_THREADS_H_

#include <boost/thread.hpp>

#include <ppl/config.h>

#if defined(_PPL_PLATFORM_LINUX)
#include <signal.h>
#endif

namespace ppl { namespace boostlib {


/// boost线程操作的工具类
class threads
{
public:
	static bool kill(boost::thread& t, int exitCode)
	{
#if defined(_PPL_PLATFORM_MSWIN)
		return FALSE != ::TerminateThread(t.native_handle(), exitCode);
#elif defined(_PPL_PLATFORM_LINUX)
		return 0 == ::pthread_kill(t.native_handle(), 0);
#else
#error no thread killer
#endif

	}
};


} }

#endif
