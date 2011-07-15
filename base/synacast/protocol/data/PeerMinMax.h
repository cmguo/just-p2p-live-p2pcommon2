
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_PEER_MIN_MAX_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_PEER_MIN_MAX_H_

#include <ppl/data/int.h>
#include <iosfwd>


/// peer����Դ��Χ
struct PEER_MINMAX
{
	/// ��ʼλ��
	UINT32 MinIndex;

	/// ����λ��
	UINT32 MaxIndex;

	enum { object_size = 8 };

	/// ���
	void Clear()
	{
		MinIndex = 0;
		MaxIndex = 0;
	}

	/// �Ƿ�Ϊ��
	bool IsEmpty() const
	{
		return (MinIndex == 0) && (MaxIndex == 0);
	}

	/// ��ȡ��Դ����
	UINT32 GetLength() const
	{
		return MaxIndex - MinIndex;
	}
};



bool operator==(const PEER_MINMAX& val1, const PEER_MINMAX& val2);

/// ��PEER_MINMAX�������
std::ostream& operator<<(std::ostream& os, const PEER_MINMAX& minmax);


#endif