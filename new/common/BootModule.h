
#ifndef _LIVE_P2PCOMMON2_NEW_COMMON_BOOT_MODULE_H_
#define _LIVE_P2PCOMMON2_NEW_COMMON_BOOT_MODULE_H_

#include "common/UDPSender.h"
#include "framework/timer.h"
#include <synacast/protocol/vod.h>
#include <ppl/net/inet.h>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <vector>

class NameResolverThread;


class BootModule : private boost::noncopyable
{
public:
	typedef boost::function<void (const std::vector<InetSocketAddress>&)> NTSListCallbackType;

	explicit BootModule(UDPSenderPtr packetSender);
	~BootModule();

	void QueryNTSList(NTSListCallbackType callback);

	void OnServerNameResolved(u_long ip);

	bool HandlePacket( BYTE* data, size_t size, const InetSocketAddress& remoteAddr);

private:
	void DoQueryNTSList();
	void OnTimeout();

private:
	UDPSenderPtr m_PacketSender;
	NTSListCallbackType m_Callback;
	boost::shared_ptr<NameResolverThread> m_ResolverThread;
	u_long m_ServerIP;

	VODBootServerRequestPacketBuilder m_PacketBuilder;

	once_timer m_TimeoutTimer;

};

#endif