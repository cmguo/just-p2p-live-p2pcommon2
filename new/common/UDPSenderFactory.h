
#ifndef _LIVE_P2PCOMMON2_NEW_COMMON_UDPSENDER_FACTORY_H_
#define _LIVE_P2PCOMMON2_NEW_COMMON_UDPSENDER_FACTORY_H_


#include "common/UDPSender.h"
#include <boost/function.hpp>

typedef boost::function<void (BYTE*, size_t, const InetSocketAddress&, UINT)> UDPSenderCallbackType;

class udp_socket;


class UDPSenderFactory
{
public:
#ifndef _PPL_USE_ASIO
	static UDPSender* CreateNormal(int udp);
#else
	static UDPSender* CreateNormal(boost::shared_ptr<udp_socket> udp);
#endif

	static UDPSender* CreateTCPProxy(UINT timeout, UDPSenderCallbackType callback);

	static UDPSender* CreateHTTPProxy(UINT timeout, UDPSenderCallbackType callback);
};

#endif
