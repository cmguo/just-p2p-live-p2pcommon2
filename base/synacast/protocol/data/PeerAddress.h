
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_PEER_ADDRESS_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_PEER_ADDRESS_H_

#include <ppl/data/int.h>
#include <ppl/util/macro.h>
#include <synacast/protocol/data/SimpleSocketAddress.h>
#include <iosfwd>
#include <string>


/// 一对端口(TcpPort和UdpPort)
struct PORT_PAIR
{
	/// tcp端口号(本机字节序)
	UINT16 TCPPort;

	/// udp端口号(本机字节序)
	UINT16 UDPPort;

	enum { object_size = 4 };

};

/// peer地址
struct PEER_ADDRESS
{
	/// ip地址(网络字节序)
	UINT32 IP;

	/// tcp端口号(本机字节序)
	UINT16 TcpPort;

	/// udp端口号(本机字节序)
	UINT16 UdpPort;

	enum { object_size = 8 };

	PEER_ADDRESS()
		: IP(0)
		, TcpPort(0)
		, UdpPort(0)
	{
	}

	/// 清空
	void Clear()
	{
		this->IP = 0;
		this->TcpPort = 0;
		this->UdpPort = 0;
	}

	/// 是否为空
	//bool IsEmpty() const
	//{
	//	return IP == 0 || TcpPort == 0 || UdpPort == 0;
	//}

	/// 是否有效--端口至少有一个不为空
	bool IsIPValid() const
	{
		return this->IP != 0;
	}

	/// 是否有效--端口至少有一个不为空
	bool IsValid() const
	{
		return this->IP != 0 && ( this->TcpPort != 0 || this->UdpPort != 0 );
	}

	/// 是否有效--全部都不为空
	bool IsFullyValid() const
	{
		return this->IP != 0 && this->TcpPort != 0 && this->UdpPort != 0;
	}

	/// 是否有效--全部都不为空
	bool IsUDPValid() const
	{
		return this->IP != 0 && this->UdpPort != 0;
	}

	/// 是否有效--全部都不为空
	bool IsTCPValid() const
	{
		return this->IP != 0 && this->TcpPort != 0;
	}

	/// 转换为64位整数
	UINT64 ToInteger() const
	{
		ULARGE_INTEGER val;
		// tcpport在高位,udpport在低位
		val.LowPart = MAKE_DWORD(this->UdpPort, this->TcpPort);
		val.HighPart = this->IP;
		return val.QuadPart;
	}

	/// 转换为字符串
	std::string ToString() const
	{
		char str[64] = { 0 };
		in_addr addr;
		addr.s_addr = this->IP;
#if defined(_PPL_PLATFORM_MSWIN)
        _snprintf(str, 63, "(%s:%hu:%hu)", ::inet_ntoa(addr), this->TcpPort, this->UdpPort);
#elif defined(_PPL_PLATFORM_LINUX)
        snprintf(str, 63, "(%s:%hu:%hu)", ::inet_ntoa(addr), this->TcpPort, this->UdpPort);
#endif
		return str;
	}

	SimpleSocketAddress ToTCPAddress() const
	{
		return SimpleSocketAddress(this->IP, this->TcpPort);
	}
	SimpleSocketAddress ToUDPAddress() const
	{
		return SimpleSocketAddress(this->IP, this->UdpPort);
	}

	bool IsAddressValid() const
	{
		return this->IsValid() || IsPrivateIP( this->IP) || inet_addr("127.0.0.1") == this->IP;
	}

	PORT_PAIR GetPorts() const
	{
		PORT_PAIR ports;
		ports.TCPPort = this->TcpPort;
		ports.UDPPort = this->UdpPort;
		return ports;
	}
};



/// PEER_ADDRESS的小于比较
inline bool operator<(const PEER_ADDRESS& addr1, const PEER_ADDRESS& addr2)
{
	return addr1.ToInteger() < addr2.ToInteger();
}
/// PEER_ADDRESS的等于比较
inline bool operator==(const PEER_ADDRESS& addr1, const PEER_ADDRESS& addr2)
{
	return addr1.ToInteger() == addr2.ToInteger();
}
/// PEER_ADDRESS的不等于比较
inline bool operator!=(const PEER_ADDRESS& addr1, const PEER_ADDRESS& addr2)
{
	return !(addr1 == addr2);
}


/// 将PORT_PAIR输出到流
std::ostream& operator<<(std::ostream& os, const PORT_PAIR& ports);

/// 将PEER_ADDRESS输出到流
std::ostream& operator<<(std::ostream& os, const PEER_ADDRESS& addr);


#endif