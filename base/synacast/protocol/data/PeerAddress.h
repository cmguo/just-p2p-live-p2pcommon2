
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_PEER_ADDRESS_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_PEER_ADDRESS_H_

#include <ppl/data/int.h>
#include <ppl/util/macro.h>
#include <synacast/protocol/data/SimpleSocketAddress.h>
#include <iosfwd>
#include <string>


/// һ�Զ˿�(TcpPort��UdpPort)
struct PORT_PAIR
{
	/// tcp�˿ں�(�����ֽ���)
	UINT16 TCPPort;

	/// udp�˿ں�(�����ֽ���)
	UINT16 UDPPort;

	enum { object_size = 4 };

};

/// peer��ַ
struct PEER_ADDRESS
{
	/// ip��ַ(�����ֽ���)
	UINT32 IP;

	/// tcp�˿ں�(�����ֽ���)
	UINT16 TcpPort;

	/// udp�˿ں�(�����ֽ���)
	UINT16 UdpPort;

	enum { object_size = 8 };

	PEER_ADDRESS()
		: IP(0)
		, TcpPort(0)
		, UdpPort(0)
	{
	}

	/// ���
	void Clear()
	{
		this->IP = 0;
		this->TcpPort = 0;
		this->UdpPort = 0;
	}

	/// �Ƿ�Ϊ��
	//bool IsEmpty() const
	//{
	//	return IP == 0 || TcpPort == 0 || UdpPort == 0;
	//}

	/// �Ƿ���Ч--�˿�������һ����Ϊ��
	bool IsIPValid() const
	{
		return this->IP != 0;
	}

	/// �Ƿ���Ч--�˿�������һ����Ϊ��
	bool IsValid() const
	{
		return this->IP != 0 && ( this->TcpPort != 0 || this->UdpPort != 0 );
	}

	/// �Ƿ���Ч--ȫ������Ϊ��
	bool IsFullyValid() const
	{
		return this->IP != 0 && this->TcpPort != 0 && this->UdpPort != 0;
	}

	/// �Ƿ���Ч--ȫ������Ϊ��
	bool IsUDPValid() const
	{
		return this->IP != 0 && this->UdpPort != 0;
	}

	/// �Ƿ���Ч--ȫ������Ϊ��
	bool IsTCPValid() const
	{
		return this->IP != 0 && this->TcpPort != 0;
	}

	/// ת��Ϊ64λ����
	UINT64 ToInteger() const
	{
		ULARGE_INTEGER val;
		// tcpport�ڸ�λ,udpport�ڵ�λ
		val.LowPart = MAKE_DWORD(this->UdpPort, this->TcpPort);
		val.HighPart = this->IP;
		return val.QuadPart;
	}

	/// ת��Ϊ�ַ���
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



/// PEER_ADDRESS��С�ڱȽ�
inline bool operator<(const PEER_ADDRESS& addr1, const PEER_ADDRESS& addr2)
{
	return addr1.ToInteger() < addr2.ToInteger();
}
/// PEER_ADDRESS�ĵ��ڱȽ�
inline bool operator==(const PEER_ADDRESS& addr1, const PEER_ADDRESS& addr2)
{
	return addr1.ToInteger() == addr2.ToInteger();
}
/// PEER_ADDRESS�Ĳ����ڱȽ�
inline bool operator!=(const PEER_ADDRESS& addr1, const PEER_ADDRESS& addr2)
{
	return !(addr1 == addr2);
}


/// ��PORT_PAIR�������
std::ostream& operator<<(std::ostream& os, const PORT_PAIR& ports);

/// ��PEER_ADDRESS�������
std::ostream& operator<<(std::ostream& os, const PEER_ADDRESS& addr);


#endif