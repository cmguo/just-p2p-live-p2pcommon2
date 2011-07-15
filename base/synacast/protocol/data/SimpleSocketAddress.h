
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_SIMPLE_SOCKET_ADDRESS_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_SIMPLE_SOCKET_ADDRESS_H_

#include <ppl/data/int.h>
#include <ppl/util/macro.h>
#include <ppl/net/inet.h>
#include <iosfwd>


/// peer socket 地址(包含网络字节序的IP和本机字节序的端口)
class SimpleSocketAddress
{
public:
	/// ip地址(网络字节序)
	UINT32 IP;

	/// 端口号(本机字节序)
	UINT16 Port;


	SimpleSocketAddress()
	{
		this->Clear();
	}
	SimpleSocketAddress(UINT32 ip, UINT16 port)
	{
		this->IP = ip;
		this->Port = port;
	}
	explicit SimpleSocketAddress( const InetSocketAddress& sockAddr )
	{
		this->IP = sockAddr.GetRawIP();
		this->Port = sockAddr.GetPort();
	}


	/// 清空
	void Clear()
	{
		IP = 0;
		Port = 0;
	}

	/// 是否为空
	bool IsEmpty() const
	{
		return IP == 0 || Port == 0;
	}

	/// 转换为64位整数
	UINT64 ToInteger() const
	{
		ULARGE_INTEGER val;
		val.LowPart = MAKE_DWORD(0, Port);
		val.HighPart = IP;
		return val.QuadPart;
	}

	/// 转换为字符串
	string ToString() const
	{
		char str[64] = { 0 };
		in_addr addr;
		addr.s_addr = this->IP;
        #if defined(_PPL_PLATFORM_MSWIN)
		    _snprintf(str, 63, "(%s:%hu)", ::inet_ntoa(addr), this->Port);
        #elif defined(_PPL_PLATFORM_LINUX)
            snprintf(str, 63, "(%s:%hu)", ::inet_ntoa(addr), this->Port);
        #endif
		return str;
	}

	InetSocketAddress ToInetSocketAddress() const
	{
		return InetSocketAddress( this->IP, this->Port );
	}
	UINT32 GetIP() const { return this->IP; }
	UINT16 GetPort() const { return this->Port; }

};



/// PEER_ADDRESS的小于比较
inline bool operator<(const SimpleSocketAddress& addr1, const SimpleSocketAddress& addr2)
{
	return addr1.ToInteger() < addr2.ToInteger();
}
/// PEER_ADDRESS的等于比较
inline bool operator==(const SimpleSocketAddress& addr1, const SimpleSocketAddress& addr2)
{
	return addr1.ToInteger() == addr2.ToInteger();
}
/// PEER_ADDRESS的不等于比较
inline bool operator!=(const SimpleSocketAddress& addr1, const SimpleSocketAddress& addr2)
{
	return !(addr1 == addr2);
}

/// 将PEER_SOCKET_ADDRESS输出到流
std::ostream& operator<<(std::ostream& os, const SimpleSocketAddress& addr);


#endif