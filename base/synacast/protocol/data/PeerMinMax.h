
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_PEER_MIN_MAX_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_PEER_MIN_MAX_H_

#include <ppl/data/int.h>
#include <iosfwd>


/// peer的资源范围
struct PEER_MINMAX
{
	/// 起始位置
	UINT32 MinIndex;

	/// 结束位置
	UINT32 MaxIndex;

	enum { object_size = 8 };

	/// 清空
	void Clear()
	{
		MinIndex = 0;
		MaxIndex = 0;
	}

	/// 是否为空
	bool IsEmpty() const
	{
		return (MinIndex == 0) && (MaxIndex == 0);
	}

	/// 获取资源长度
	UINT32 GetLength() const
	{
		return MaxIndex - MinIndex;
	}
};



bool operator==(const PEER_MINMAX& val1, const PEER_MINMAX& val2);

/// 将PEER_MINMAX输出到流
std::ostream& operator<<(std::ostream& os, const PEER_MINMAX& minmax);


#endif