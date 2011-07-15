
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_NET_ASIO_COMMAND_DISPATCHER_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_NET_ASIO_COMMAND_DISPATCHER_H_

#include <ppl/util/command_dispatcher.h>
#include <ppl/net/asio/io_service_provider.h>


class io_service_command_dispatcher : public command_dispatcher
{
public:
	io_service_command_dispatcher()
	{

	}

	virtual void post_command(boost::function0<void> fun)
	{
		io_service_provider::get_default()->post(fun);
	}
};
#endif