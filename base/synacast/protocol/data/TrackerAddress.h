
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_TRACKER_ADDRESS_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_TRACKER_ADDRESS_H_

#include <ppl/data/int.h>
#include <ppl/util/macro.h>
#include <synacast/protocol/data/PeerAddress.h>
#include <iosfwd>



/// TRACKER地址
struct TRACKER_ADDRESS
{
	/// ip地址
	UINT32 IP;

	/// 端口号
	UINT16 Port;

	/// 类型：TRACKER_TYPE_UDP or TRACKER_TYPE_TCP
	UINT8 Type;

	/// 状态码，用于显示调试数据信息
	UINT8 ReservedStatusCode;

	enum { object_size = 8 };

	/// 清空
	void Clear()
	{
		FILL_ZERO(*this);
	}

	/// 转换为64位整数
	UINT64 ToUINT64() const
	{
		DWORD highWord = MAKE_DWORD(this->Type, this->ReservedStatusCode);
		DWORD highDword = MAKE_DWORD(this->Port, highWord);
		return MAKE_ULONGLONG(this->IP, highDword);
	}
};

/// tracker登录地址
struct TRACKER_LOGIN_ADDRESS
{
	/// 服务器地址
	TRACKER_ADDRESS ServerAddress;

	/// 登录的peer地址
	PEER_ADDRESS LoginAddress;
};


/// TRACKER_ADDRESS的小于比较
inline bool operator<(const TRACKER_ADDRESS& x, const TRACKER_ADDRESS& y)
{
	//	return memcmp(&x, &y, sizeof(x)) < 0;
	return x.ToUINT64() < y.ToUINT64();
}

/// 不比较StatusCode
inline bool operator==(const TRACKER_ADDRESS& x, const TRACKER_ADDRESS& y)
{
	return x.IP == y.IP && x.Port == y.Port && x.Type == y.Type;
}

inline bool operator!=(const TRACKER_ADDRESS& x, const TRACKER_ADDRESS& y)
{
	return !(x == y);
}


/// 将TRACKER_ADDRESS输出到流
std::ostream& operator<<(std::ostream& os, const TRACKER_ADDRESS& trackerAddr);

/// 将TRACKER_LOGIN_ADDRESS输出到流
std::ostream& operator<<(std::ostream& os, const TRACKER_LOGIN_ADDRESS& trackerAddr);

#endif