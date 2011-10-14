
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_NET_TYPE_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_NET_TYPE_H_

#include <ppl/data/int.h>
#include <iosfwd>


/// 网络类型
struct NET_TYPE
{
	/// 国家或地区
	UINT16 Country;

	/// 互联网服务提供商(telecom/cnc/edu...)
	UINT16 ISP;

	/// 省
	UINT16 Province;

	/// 市/县
	UINT16 City;

	/// 清空
	void Clear()
	{
		this->Country = 0;
		this->ISP = 0;
		this->Province = 0;
		this->City = 0;
	}

	/// 转化为整数
	UINT32 ToInteger() const
	{
		LIVE_ASSERT(Province < 50);
		LIVE_ASSERT(City < 200);
		LIVE_ASSERT(City < 500);
		return ISP * 100 * 1000 + Province * 1000 + City;
	}
};



std::ostream& operator<<(std::ostream& os, const NET_TYPE& netType);



#endif