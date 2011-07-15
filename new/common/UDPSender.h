
#ifndef _LIVE_P2PCOMMON2_NEW_COMMON_UDPSENDER_H_
#define _LIVE_P2PCOMMON2_NEW_COMMON_UDPSENDER_H_

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

class PacketBase;
class UDPPacketBuilder;

class UDPConnectionlessPacketBuilder;
class UDPConnectionPacketBuilder;

class TCPConnectionlessPacketBuilder;
class TCPConnectionPacketBuilder;

class InetSocketAddress;


/// 负责udp的网络发送
class UDPSender : private boost::noncopyable
{
public:
	virtual ~UDPSender() { }

	virtual bool Send(const void* data, size_t size, const InetSocketAddress& sockAddr) = 0;
};


typedef boost::shared_ptr<UDPSender> UDPSenderPtr;

#endif
