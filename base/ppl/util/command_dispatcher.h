#ifndef _LIVE_P2PCOMMON2_BASE_PPL_UTIL_COMMAND_DISPATCHER_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_UTIL_COMMAND_DISPATCHER_H_

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>



class command_dispatcher : private boost::noncopyable
{
public:
	command_dispatcher() { }
	virtual ~command_dispatcher() { }

	virtual void post_command( boost::function0<void> fun ) = 0;
};


#endif