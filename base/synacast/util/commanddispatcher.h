
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_UTIL_COMMAND_DISPATCHER_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_UTIL_COMMAND_DISPATCHER_H_

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>


class CommandDispatcher : private boost::noncopyable
{
public:
	CommandDispatcher() { }
	virtual ~CommandDispatcher() { }

	virtual void PostCommand(boost::function<void ()> fun) = 0;
};




class command_dispatcher : private boost::noncopyable
{
public:
	command_dispatcher() { }
	virtual ~command_dispatcher() { }

	virtual void post_command( boost::function<void ()> fun ) = 0;
};


#endif