
#ifndef _LIVE_P2PCOMMON2_NEW_COMMON_VODPROXY_H_
#define _LIVE_P2PCOMMON2_NEW_COMMON_VODPROXY_H_


class InetSocketAddress;
class StunModule;
class BootModule;


class VODProxy
{
public:
	static bool HandlePacket(BYTE* data, size_t size, const InetSocketAddress& remoteAddr, BootModule* bootModule, StunModule* stunModule);
};

#endif