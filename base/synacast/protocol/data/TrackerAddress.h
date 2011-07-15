
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_TRACKER_ADDRESS_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_TRACKER_ADDRESS_H_

#include <ppl/data/int.h>
#include <ppl/util/macro.h>
#include <synacast/protocol/data/PeerAddress.h>
#include <iosfwd>



/// TRACKER��ַ
struct TRACKER_ADDRESS
{
	/// ip��ַ
	UINT32 IP;

	/// �˿ں�
	UINT16 Port;

	/// ���ͣ�TRACKER_TYPE_UDP or TRACKER_TYPE_TCP
	UINT8 Type;

	/// ״̬�룬������ʾ����������Ϣ
	UINT8 ReservedStatusCode;

	enum { object_size = 8 };

	/// ���
	void Clear()
	{
		FILL_ZERO(*this);
	}

	/// ת��Ϊ64λ����
	UINT64 ToUINT64() const
	{
		DWORD highWord = MAKE_DWORD(this->Type, this->ReservedStatusCode);
		DWORD highDword = MAKE_DWORD(this->Port, highWord);
		return MAKE_ULONGLONG(this->IP, highDword);
	}
};

/// tracker��¼��ַ
struct TRACKER_LOGIN_ADDRESS
{
	/// ��������ַ
	TRACKER_ADDRESS ServerAddress;

	/// ��¼��peer��ַ
	PEER_ADDRESS LoginAddress;
};


/// TRACKER_ADDRESS��С�ڱȽ�
inline bool operator<(const TRACKER_ADDRESS& x, const TRACKER_ADDRESS& y)
{
	//	return memcmp(&x, &y, sizeof(x)) < 0;
	return x.ToUINT64() < y.ToUINT64();
}

/// ���Ƚ�StatusCode
inline bool operator==(const TRACKER_ADDRESS& x, const TRACKER_ADDRESS& y)
{
	return x.IP == y.IP && x.Port == y.Port && x.Type == y.Type;
}

inline bool operator!=(const TRACKER_ADDRESS& x, const TRACKER_ADDRESS& y)
{
	return !(x == y);
}


/// ��TRACKER_ADDRESS�������
std::ostream& operator<<(std::ostream& os, const TRACKER_ADDRESS& trackerAddr);

/// ��TRACKER_LOGIN_ADDRESS�������
std::ostream& operator<<(std::ostream& os, const TRACKER_LOGIN_ADDRESS& trackerAddr);

#endif