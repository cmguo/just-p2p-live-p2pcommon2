
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_DEGREE_INFO_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_DEGREE_INFO_H_

#include <ppl/data/int.h>
#include <ppl/util/macro.h>
#include <iosfwd>


/// 出度入度数据
struct DEGREE_PAIR
{
	/// 连入的Peer数
	UINT8 In;

	/// 连出的Peer数
	UINT8 Out;

	enum { object_size = 2 };
};


/// 出度入度信息
struct DEGREE_INFO
{
	/// 本机剩余可连接个数=最大连接个数-当前个数
	INT8 Left;

	/// 总的
	DEGREE_PAIR All;

	/// udpt的
	DEGREE_PAIR UDPT;

	void Clear()
	{
		FILL_ZERO(*this);
	}

	enum { object_size = 5 };
};


#endif
