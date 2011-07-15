
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_PEER_STATUS_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_PEER_STATUS_H_

#include <synacast/protocol/data/DegreeInfo.h>
#include <ppl/data/int.h>
#include <ppl/util/macro.h>


/// peer状态信息
struct PEER_STATUS
{
	/// 剩余上传带宽(暂时没有使用)
	UINT16 UploadBWLeft;

	/// 服务质量(上传速度)
	UINT16 Qos;

	/// 剩余度
	INT8 DegreeLeft;

	/// 出度
	UINT8 OutDegree;

	/// 入度
	UINT8 InDegree;

	/// 跳帧率
	UINT8 SkipPercent;

	enum { object_size = 8 };

	int GetTotalDegree() const
	{
		return this->DegreeLeft + this->OutDegree + this->InDegree;
	}
};





/// peer状态信息，包括status/minmax
struct PEER_STATUS_INFO
{
	/// 连接和下载的状态信息
	PEER_STATUS Status;

	/// 资源范围
	PEER_MINMAX MinMax;

	/// 清空
	void Clear()
	{
		this->MinMax.Clear();
		FILL_ZERO(this->Status);
		this->Status.SkipPercent = 100;
	}

	enum { object_size = PEER_STATUS::object_size + PEER_MINMAX::object_size };

};




/*
struct EXTRA_PEER_NET_INFO
{
UINT32 DetectedIP;
UINT16 DetectedUDPPort;
INT8 DegreeLeft;
UINT8 InDegree;
};*/





/// 扩充版的peer状态信息
class PeerStatusEx
{
public:
	/// 剩余上传带宽(暂时没有使用)
	UINT16 UploadBWLeft;

	/// 服务质量(上传速度)
	UINT16 Qos;

	/// 出入度信息
	DEGREE_INFO Degrees;

	/// 跳帧率
	UINT8 SkipPercent;

	enum { object_size = sizeof(UINT16) * 2 + sizeof(DEGREE_INFO) + sizeof(UINT8) };

	PeerStatusEx()
	{
		UploadBWLeft = 0;
		Qos = 0;
		Degrees.Clear();
		// 初始的SkipPercent设为100，对应的缓冲率为0
		SkipPercent = 100;
	}

	PeerStatusEx( const PEER_STATUS& status, const DEGREE_INFO& degreeInfo )
	{
		UploadBWLeft = status.UploadBWLeft;
		Qos = status.Qos;
		Degrees = degreeInfo;
		SkipPercent = status.SkipPercent;
	}
};



#endif
