
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_DEGREE_INFO_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_DEGREE_INFO_H_

#include <ppl/data/int.h>
#include <ppl/util/macro.h>
#include <iosfwd>


/// �����������
struct DEGREE_PAIR
{
	/// �����Peer��
	UINT8 In;

	/// ������Peer��
	UINT8 Out;

	enum { object_size = 2 };
};


/// ���������Ϣ
struct DEGREE_INFO
{
	/// ����ʣ������Ӹ���=������Ӹ���-��ǰ����
	INT8 Left;

	/// �ܵ�
	DEGREE_PAIR All;

	/// udpt��
	DEGREE_PAIR UDPT;

	void Clear()
	{
		FILL_ZERO(*this);
	}

	enum { object_size = 5 };
};


#endif
