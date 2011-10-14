
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_NET_INET_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_NET_INET_H_


/**
 * @file
 * @brief tcp/ip�еĹ�����
 */


#include <ppl/config.h>
#include <ppl/data/tchar.h>
#include <ppl/util/macro.h>
#include <ppl/data/numeric.h>
#include <ppl/util/log.h>
#include <ppl/data/tstring.h>
#include <assert.h>
#include <stddef.h>

#if defined(_UNICODE) || defined(UNICODE)
#include <ppl/text/encoding.h>
#endif


#include <vector>

#include <iosfwd>

#if defined(_PPL_PLATFORM_MSWIN)

#ifndef _WINSOCKAPI_
#include <winsock2.h>
#endif

#pragma comment(lib, "ws2_32.lib")

#else

#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

inline int WSAGetLastError()
{
	return errno;
}

#endif




class IPAddress;
class SocketAddress;
class InetSocketAddress;




#if defined(_PPL_PLATFORM_MSWIN)

class WinsockLib
{
public:
	WinsockLib()
	{
		WSADATA wsaData = { 0 };
		m_errcode = ::WSAStartup(MAKEWORD(1, 2), &wsaData);
	}
	~WinsockLib()
	{
		::WSACleanup();
	}

private:
	int m_errcode;
};

class SocketError
{
public:
	static int LastError()
	{
		return ::WSAGetLastError();
	}
};

#else

class SocketError
{
public:
	static int LastError()
	{
		return errno;
	}
};


#endif



/// ��ip��ַ��ʽ��Ϊ�ַ���
tstring FormatIPAddress(u_long ip);

/// ��������
u_long ResolveHostName(const char* hostname);

/// ��socket��ַ��ʽ��Ϊ�ַ���
string FormatSocketAddress(const sockaddr_in& addr);


struct win32_in_addr {
        union {
                struct { u_char s_b1,s_b2,s_b3,s_b4; } S_un_b;
                struct { u_short s_w1,s_w2; } S_un_w;
                u_long S_addr;
        } S_un;
};


/// ��ip��ַ��ʽ��Ϊ�ַ���
inline int FormatIPAddress(u_long ip, char* buffer, size_t size)
{
	const win32_in_addr* addr = reinterpret_cast<const win32_in_addr*>(&ip);
	return snprintf(buffer, size, "%u.%u.%u.%u", addr->S_un.S_un_b.s_b1, addr->S_un.S_un_b.s_b2, addr->S_un.S_un_b.s_b3, addr->S_un.S_un_b.s_b4);
}

#if defined(_PPL_PLATFORM_MSWIN)

/// ��ip��ַ��ʽ��Ϊ�ַ���
inline int FormatIPAddress(u_long ip, wchar_t* buffer, size_t size)
{
	const win32_in_addr* addr = reinterpret_cast<const win32_in_addr*>(&ip);
	return _snwprintf(buffer, size, L"%u.%u.%u.%u", addr->S_un.S_un_b.s_b1, addr->S_un.S_un_b.s_b2, addr->S_un.S_un_b.s_b3, addr->S_un.S_un_b.s_b4);
}

#endif



/// ��hostent�ṹ���м���ip�б�
size_t LoadIPs(size_t maxCount, u_long ipArray[], const hostent& host);

/// ���ر���ip�б�
size_t LoadLocalIPs(size_t maxCount, u_long ipArray[]);

/// ���ر���ip�б�
void LoadLocalIPs(std::vector<u_long>& ipArray);



/// ��תip���ֽ���
u_long TransIP(u_long ip);

/// �ж�һ��IP�Ƿ�Ϊ������ַ��������ַ��
bool IsPrivateIP(u_long ip);

/// �ж�һ��IP�Ƿ�����Ǳ�����ַ��������ַ��
bool HasExternalIP(const std::vector<u_long>& ipArray);

/// �ж�һ��IP�Ƿ�����Ǳ�����ַ��������ַ��
u_long FindExternalIP(const std::vector<u_long>& ipArray);

/// Ѱ��ƥ���IP��ַ
u_long FindRightIP(const std::vector<u_long>& ipArray, u_long ip);

/// Ѱ��������ַ�����û���ҵ�������0
u_long FindRealExternalIP(const std::vector<u_long>& ipArray);


/// �ж�һ��IP�Ƿ�����Ǳ�����ַ��������ַ��
bool HasExternalIP(const std::vector<UINT32>& ipArray);

/// �ж�һ��IP�Ƿ�����Ǳ�����ַ��������ַ��
u_long FindExternalIP(const std::vector<UINT32>& ipArray);

/// Ѱ��ƥ���IP��ַ
u_long FindRightIP(const std::vector<UINT32>& ipArray, u_long ip);

/// Ѱ��������ַ�����û���ҵ�������0
u_long FindRealExternalIP(const std::vector<UINT32>& ipArray);


std::ostream& operator<<(std::ostream& os, const sockaddr_in& addr);

std::ostream& operator<<(std::ostream& os, const in_addr& addr);

std::ostream& operator<<(std::ostream& os, const IPAddress& addr);

std::ostream& operator<<(std::ostream& os, const SocketAddress& addr);


inline bool CheckBitsValid( UINT val, UINT bits )
{
	UINT bitsVal = val & bits;
	return bitsVal != 0 && bitsVal != bits;
}

/// 169.254.x.x(dhcpʧ�ܵ�)
inline bool CheckIPIsDHCPFailed( UINT ip )
{
	const UINT ip_mask = 0x0000FEA9;
	return (ip & ip_mask) == ip_mask;
}

inline bool CheckIPValid( UINT ip )
{
	return CheckBitsValid( ip, 0x000000FF );
}


inline bool CheckIPFullyValid( UINT ip )
{
	return CheckIPValid( ip ) && false == CheckIPIsDHCPFailed( ip );
}


struct IP_BYTES
{
	UINT8 Byte1;
	UINT8 Byte2;
	UINT8 Byte3;
	UINT8 Byte4;
};

/// IP��ַ��
class IPAddress : public in_addr
{
public:
	IPAddress(u_long ip = INADDR_ANY)
	{
		this->s_addr = ip;
	}
	explicit IPAddress( const char* ipstr )
	{
		Parse( ipstr );
	}

	string ToAnsiString() const
	{
		return ::inet_ntoa(*this);
	}

	tstring ToString() const
	{
		const size_t max_buffer_size = 64;
		TCHAR buf[max_buffer_size + 1] = { 0 };
		::FormatIPAddress( this->s_addr, buf, max_buffer_size );
		return tstring(buf);
	}
	u_long GetAddress() const
	{
		return this->s_addr;
	}

	operator u_long() const
	{
		return GetAddress();
	}

	void Parse( const char* ipstr )
	{
		this->s_addr = inet_addr(ipstr);
	}

	bool IsNone() const
	{
		return INADDR_NONE == this->s_addr;
	}
	bool IsAny() const
	{
		return INADDR_ANY == this->s_addr;
	}
	bool IsBroadcast() const
	{
		return INADDR_BROADCAST == this->s_addr;
	}

	bool IsInvalidClient() const
	{
		return IsNone() || IsAny() || IsBroadcast();
	}

};


/// socket��ַ�Ľӿ���
class SocketAddress
{
public:
	SocketAddress() { }
	virtual ~SocketAddress() { }

	/// ��ȡͨ�õ�sockaddrָ��(ֻ��)
	virtual const sockaddr* GetAddress() const = 0;

	/// ��ȡͨ�õ�sockaddrָ��(��д)
	virtual sockaddr* GetAddress() = 0;

	/// ��ȡsocket��ַ���ݵĳ���
	virtual int GetAddressLength() const = 0;

	/// ת��Ϊ�ַ���
	virtual string ToString() const = 0;
};




/// ipv4��ʹ�õ�Socket��ַ�� GetIP��GetPort���ص���host-byteorder�������ճ����е�ʹ��ϰ��
class InetSocketAddress : public SocketAddress
{
public:
	/// ���ƶ���
	InetSocketAddress(const sockaddr_in& addr) : m_addr(addr) { }

	/// ����ip/port�������ipΪ�����ֽ���portΪ�����ֽ���
	explicit InetSocketAddress(u_long ip = 0, u_short port = 0)
	{
		m_addr.sin_family = AF_INET;
		FILL_ZERO(m_addr.sin_zero);
		this->Set(ip, port);
	}

	virtual const sockaddr* GetAddress() const { return GetRawSocketAddress(); }
	virtual sockaddr* GetAddress() { return GetRawSocketAddress(); }
	virtual int GetAddressLength() const { return sizeof(sockaddr_in); }
	virtual string ToString() const;

	bool Parse( const string& s )
	{
		size_t pos = s.find( ':' );
		if ( pos == string::npos )
			return false;
		u_short port = 0;
		if ( false == numeric<u_short>::try_parse( s.substr( pos + 1 ), port ) )
			return false;
		this->SetPort( port );
		this->SetIP( s.substr( 0, pos ).c_str() );
		if ( INADDR_NONE == this->GetRawIP() )
			return false;
		return true;
	}


	/// ����ip��ַ�Ͷ˿ں�
	void Set(u_long ip, u_short port)
	{
		SetIP(ip);
		SetPort(port);
	}

	/// ��ȡIP��ַ�����ص��Ǳ����ֽ���
	u_long GetIPHostByteOrder() const
	{
		return ntohl(this->GetRawIP());
	}


	/// ��ȡԭʼ��IP��ַ��Ϊnetwork-byteorder
	u_long GetRawIP() const { return m_addr.sin_addr.s_addr; }

	/// ��ȡԭʼ�Ķ˿ںţ�Ϊnetwork-byteorder
	u_short GetRawPort() const { return m_addr.sin_port; }

	/// ��ȡIP��ַ�����ص���network-byteorder�������ճ����к�һ���ʹ��ϰ��
	u_long GetIP() const { return GetRawIP(); }

	/// ��ȡ�˿ںţ����ص���host-byteorder�������ճ����к�һ���ʹ��ϰ��
	u_short GetPort() const { return ntohs(GetRawPort()); }

	/// ���ö˿ںţ�portΪ�����ֽ���Ķ˿�ֵ
	void SetPort(u_short port) { m_addr.sin_port = htons(port); }

	/// ����IP��ipΪ�����ֽ���
	void SetIP(u_long ip)
	{
		LIVE_ASSERT(ip != INADDR_NONE);
		m_addr.sin_addr.s_addr = ip;
	}

	/// ����IP������Ϊip�ַ���
	void SetIP(const char* ip)
	{
		LIVE_ASSERT(ip != NULL);
		m_addr.sin_addr.s_addr = inet_addr(ip);
	}

	/// ��ȡԭʼ��ַ����
	const sockaddr_in& GetRawAddress() const { return m_addr; }

	/// ��ȡsockaddr����
	const sockaddr* GetRawSocketAddress() const { return reinterpret_cast<const sockaddr*>(&m_addr); }

	/// ��ȡsockaddr����
	sockaddr* GetRawSocketAddress() { return reinterpret_cast<sockaddr*>(&m_addr); }

private:
	/// ԭʼ�ĵ�ַ����
	sockaddr_in m_addr;
};

/*
inline bool operator==(const InetSocketAddressBase& x, const InetSocketAddressBase& y)
{
	return x.GetRawIP() == y.GetRawIP() && x.GetRawPort() == y.GetRawPort();
}
*/


inline bool operator==(const InetSocketAddress& x, const InetSocketAddress& y)
{
	return x.GetRawIP() == y.GetRawIP() && x.GetRawPort() == y.GetRawPort();
}

inline bool operator!=(const InetSocketAddress& x, const InetSocketAddress& y)
{
	return !(x == y);
}







inline u_long TransIP(u_long ip)
{
	win32_in_addr a;
	a.S_un.S_addr=ip;
	return   (( (u_long)a.S_un.S_un_b.s_b1 ) << 24) +
			 (( (u_long)a.S_un.S_un_b.s_b2 ) << 16) +
			 (( (u_long)a.S_un.S_un_b.s_b3 ) << 8)  +
			 ( (u_long)a.S_un.S_un_b.s_b4 ) ;
}

inline bool IsPrivateIP(u_long ip)
{
	win32_in_addr a;
	a.S_un.S_addr=ip;

	u_char b1 = a.S_un.S_un_b.s_b1;
	u_char b2 = a.S_un.S_un_b.s_b2;

	//10.x.x.x
	if (b1==10) return true;

	//172.16.x.x -- 172.31.x.x
	if ( (b1==172) && (16<=b2) && (b2<=31)  ) return true;

	//192.168.x.x
	if ( (b1==192) && (b2==168) ) return true;

	//169.254.x.x
	if ( (b1==169) && (b2==254) ) return true;

	return false;
}


inline bool HasExternalIP(const std::vector<u_long>& ipArray)
{
	for (size_t i = 0; i < ipArray.size(); ++i)
	{
		u_long ip = ipArray[i];
		if (!IsPrivateIP(ip))
			return true;
	}
	return false;
}

inline u_long FindRealExternalIP(const std::vector<u_long>& ipArray)
{
	for (size_t i = 0; i < ipArray.size(); ++i)
	{
		u_long ip = ipArray[i];
		if (!IsPrivateIP(ip))
			return ip;
	}
	return 0;
}

inline u_long FindExternalIP(const std::vector<u_long>& ipArray)
{
	for (size_t i = 0; i < ipArray.size(); ++i)
	{
		u_long ip = ipArray[i];
		if (!IsPrivateIP(ip))
			return ip;
	}
	if (ipArray.empty())
		return 0;
	return ipArray[0];
}

inline u_long FindRightIP(const std::vector<u_long>& ipArray, u_long ip)
{
	if ( ipArray.empty() )
	{
		LIVE_ASSERT( false );
		return 0;
	}
	u_long i;
	u_long min_value,min_index,value;

	min_value= TransIP(ip) ^ TransIP(ipArray[0]);
	min_index= 0;

	for (i=1; i<ipArray.size(); i++)
	{
		value= TransIP(ip) ^ TransIP(ipArray[i]);
		if (value < min_value)
		{
			min_value= value;
			min_index= i;
		}
	}

	return ipArray[min_index];
}



inline bool HasExternalIP(const std::vector<UINT32>& ipArray)
{
	return HasExternalIP(reinterpret_cast<const std::vector<u_long>&>(ipArray));
}

inline u_long FindExternalIP(const std::vector<UINT32>& ipArray)
{
	return FindExternalIP(reinterpret_cast<const std::vector<u_long>&>(ipArray));
}

inline u_long FindRightIP(const std::vector<UINT32>& ipArray, u_long ip)
{
	return FindRightIP(reinterpret_cast<const std::vector<u_long>&>(ipArray), ip);
}

/// Ѱ��������ַ�����û���ҵ�������0
inline u_long FindRealExternalIP(const std::vector<UINT32>& ipArray)
{
	return FindRealExternalIP(reinterpret_cast<const std::vector<u_long>&>(ipArray));
}



inline u_long ParseIPAddress( const char* ipstr )
{
	u_long ip = inet_addr( ipstr );
	return ip;
}

inline tstring FormatIPAddress(u_long ip)
{
	IPAddress ipaddr(ip);
	return ipaddr.ToString();
}

inline string AnsiFormatIPAddress(u_long ip)
{
	IPAddress ipaddr(ip);
	return ipaddr.ToAnsiString();
}

inline u_long ResolveHostName(const char* hostname)
{
	u_long ip = ::inet_addr(hostname);
	if (ip != INADDR_NONE)
		return ip;
	struct hostent* host = ::gethostbyname(hostname);
	if (host == NULL)
	{
		UTIL_ERROR("ResolveHostName: gethostbyname(" << hostname << ") failed, ErrorCode=" << SocketError::LastError());
		return INADDR_NONE;
	}
	const in_addr* addr = reinterpret_cast<const in_addr*>(host->h_addr_list[0]);
	return addr->s_addr;
}

inline u_long ResolveHostName(const string& hostname)
{
	return ResolveHostName(hostname.c_str());
}


#if defined(_UNICODE) || defined(UNICODE)

inline u_long ResolveHostName(const wstring& hostname)
{
	string ahostname = encoding::w2a(hostname);
	return ResolveHostName(ahostname.c_str());
}

#endif

inline string FormatSocketAddress(const sockaddr_in& addr)
{
	const int buffer_size = 255;
	char str[buffer_size + 1] = { 0 };
	snprintf(str, buffer_size, "%s:%hu", ::inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
	return str;
}



inline std::ostream& operator<<(std::ostream& os, const sockaddr_in& addr)
{
	os << FormatSocketAddress(addr);
	return os;
}

inline std::ostream& operator<<(std::ostream& os, const in_addr& addr)
{
	os << ::inet_ntoa(addr);
	return os;
}



inline std::ostream& operator<<(std::ostream& os, const IPAddress& addr)
{
	const in_addr& rawAddr = addr;
	os << rawAddr;
	return os;
}

inline std::ostream& operator<<(std::ostream& os, const SocketAddress& addr)
{
	os << addr.ToString();
	return os;
}



inline string InetSocketAddress::ToString() const
{
	return FormatSocketAddress(GetRawAddress());
}


#ifdef _RUN_TEST

#include <ppl/util/test_case.h>

class InetTestCase : public ppl::util::test_case
{
	virtual void DoRun()
	{
		u_long ip = inet_addr("192.168.0.1");
		u_long ip1 = inet_addr("192.168.0.113");
		u_long ip2 = inet_addr("202.168.0.1");
		std::vector<u_long> ipArray;
		ipArray.push_back(inet_addr("192.168.0.113"));
		ipArray.push_back(inet_addr("202.168.0.1"));

		LIVE_ASSERT(FindRightIP(ipArray, ip) == ip1);

		ip = inet_addr("58.19.11.33");

		ip = inet_addr("192.168.0.3");
		LIVE_ASSERT( IsPrivateIP( ip ) );
		ip = inet_addr("169.254.0.3");
		LIVE_ASSERT( IsPrivateIP( ip ) );
		ip = inet_addr("10.0.0.3");
		LIVE_ASSERT( IsPrivateIP( ip ) );
		ip = inet_addr("172.16.0.3");
		LIVE_ASSERT( IsPrivateIP( ip ) );
		ip = inet_addr("172.31.0.3");
		LIVE_ASSERT( IsPrivateIP( ip ) );
	}
};

CPPUNIT_TEST_SUITE_REGISTRATION(InetTestCase);

#endif

#endif