#ifndef _LIVE_P2PCOMMON2_BASE_PPL_OS_PROCESS_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_OS_PROCESS_H_

#include <ppl/config.h>

#include <boost/noncopyable.hpp>


#if defined(_PPL_PLATFORM_MSWIN)

#include <ppl/mswin/process.h>

namespace ppl { namespace os {

using ppl::mswin::process;
using ppl::mswin::process_object;

} }


#elif defined(_PPL_PLATFORM_LINUX)

namespace ppl { namespace os {

class process : private boost::noncopyable
{
public:
	static pid_t current_process_id()
	{
		return ::getpid();
	}
};

} }

#else

#error invalid platform for ppl.os.process

#endif

#endif