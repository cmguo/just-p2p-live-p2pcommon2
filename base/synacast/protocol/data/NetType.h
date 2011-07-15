
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_NET_TYPE_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_NET_TYPE_H_

#include <ppl/data/int.h>
#include <iosfwd>


/// ��������
struct NET_TYPE
{
	/// ���һ����
	UINT16 Country;

	/// �����������ṩ��(telecom/cnc/edu...)
	UINT16 ISP;

	/// ʡ
	UINT16 Province;

	/// ��/��
	UINT16 City;

	/// ���
	void Clear()
	{
		this->Country = 0;
		this->ISP = 0;
		this->Province = 0;
		this->City = 0;
	}

	/// ת��Ϊ����
	UINT32 ToInteger() const
	{
		assert(Province < 50);
		assert(City < 200);
		assert(City < 500);
		return ISP * 100 * 1000 + Province * 1000 + City;
	}
};



std::ostream& operator<<(std::ostream& os, const NET_TYPE& netType);



#endif