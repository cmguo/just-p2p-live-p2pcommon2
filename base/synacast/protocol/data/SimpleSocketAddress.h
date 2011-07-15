
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_SIMPLE_SOCKET_ADDRESS_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_SIMPLE_SOCKET_ADDRESS_H_

#include <ppl/data/int.h>
#include <ppl/util/macro.h>
#include <ppl/net/inet.h>
#include <iosfwd>


/// peer socket ��ַ(���������ֽ����IP�ͱ����ֽ���Ķ˿�)
class SimpleSocketAddress
{
public:
	/// ip��ַ(�����ֽ���)
	UINT32 IP;

	/// �˿ں�(�����ֽ���)
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


	/// ���
	void Clear()
	{
		IP = 0;
		Port = 0;
	}

	/// �Ƿ�Ϊ��
	bool IsEmpty() const
	{
		return IP == 0 || Port == 0;
	}

	/// ת��Ϊ64λ����
	UINT64 ToInteger() const
	{
		ULARGE_INTEGER val;
		val.LowPart = MAKE_DWORD(0, Port);
		val.HighPart = IP;
		return val.QuadPart;
	}

	/// ת��Ϊ�ַ���
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



/// PEER_ADDRESS��С�ڱȽ�
inline bool operator<(const SimpleSocketAddress& addr1, const SimpleSocketAddress& addr2)
{
	return addr1.ToInteger() < addr2.ToInteger();
}
/// PEER_ADDRESS�ĵ��ڱȽ�
inline bool operator==(const SimpleSocketAddress& addr1, const SimpleSocketAddress& addr2)
{
	return addr1.ToInteger() == addr2.ToInteger();
}
/// PEER_ADDRESS�Ĳ����ڱȽ�
inline bool operator!=(const SimpleSocketAddress& addr1, const SimpleSocketAddress& addr2)
{
	return !(addr1 == addr2);
}

/// ��PEER_SOCKET_ADDRESS�������
std::ostream& operator<<(std::ostream& os, const SimpleSocketAddress& addr);


#endif