
#ifndef _LIVE_P2PCOMMON2_NEW_COMMON_IPINFO_H_
#define _LIVE_P2PCOMMON2_NEW_COMMON_IPINFO_H_

#include "framework/memory.h"

#include <synacast/protocol/data/PeerAddress.h>
#include <synacast/protocol/data/CandidatePeerInfo.h>
#include <boost/noncopyable.hpp>


/// ippool中Peer地址的端口连接情况
enum IPPOOL_CONNECT_FLAGS
{
	/// 未指定的
	IPPOOL_CONNECT_NONE = 0x0000, 

	/// 正常端口可以连接上
	IPPOOL_CONNECT_NORMAL = 0x0001, 

	/// 可以连接上80端口
	IPPOOL_CONNECT_80 = 0x0002, 

	/// 可以使用udpt方式连接
	IPPOOL_CONNECT_UDPT = 0x0003, 

	/// 屏蔽
	IPPOOL_CONNECT_BLOCKED = 0x0004, 
};


enum CandidatePeerTypeEnum
{
	CANDIDATE_FROM_TRACKER = 1, 
	CANDIDATE_FROM_UDP_ECHO = 2, 
	CANDIDATE_FROM_TCP_ECHO = 3, 
	CANDIDATE_FROM_UDP_HELLO = 4, 
	CANDIDATE_FROM_REDIRECT = 5, 
};




class PeerItem : public pool_object
{
public:
	/// 保存了Peer基本网络信息：地址和网络类型
	CANDIDATE_PEER_INFO Info;

	/// 本机到对方的RTT(ms)
	UINT RTT;

	PEER_ADDRESS StunServerAddress;

	/// 连接的标志
	WORD ConnectFlags;

	/// 是否能探测
	bool CanDetect;

	/// 连接次数
	WORD ConnectTimes;

	void Init()
	{
		FILL_ZERO(*this);
		this->RTT = UINT_MAX;
		this->ConnectFlags = IPPOOL_CONNECT_NONE;
		this->CanDetect = false;
	}
};


/// peer地址信息项
class CIPInfo : public PeerItem
{
public:
	/// 开始记录的时间
	UINT StartTime;

	/// 最后一次探测的时间
	UINT LastDetectTime;

	/// 最后一次连接时间
	UINT LastConnectTime;

	/// 最后一次连接失败的时间
	UINT LastConnectFailedTime;

	/// 最后一次连接断开时间
	UINT LastDisconnectTime;

	/// 上次的连接持续时间
//	UINT LastConnectionTime;

	/// 探测次数
	WORD DetectTimes;

	/// 探测成功次数
	WORD DetectSucceededTimes;

	/// 连接成功次数
	WORD ConnectSucceededTimes;

	/// 连接失败次数
	WORD ConnectFailedTimes;
	WORD UDPConnectFailedTimes;
	WORD TCPConnectFailedTimes;

	/// 断开连接的次数
	WORD DisconnectTimes;

	/// 状态
//	enum IPPoolState state;



	explicit CIPInfo(const PEER_ADDRESS& addr)
	{
		Clear();
		this->Info.Address = addr;
	}
	CIPInfo(const CANDIDATE_PEER_INFO& info)
	{
		Clear();
		this->Info = info;
	}

	/// 获取peer地址
	const PEER_ADDRESS& GetAddress() const { return this->Info.Address; }

	/// 获取探测失败次数
	UINT GetDetectFailedTimes() const
	{
		if (DetectTimes >= DetectSucceededTimes)
			return DetectTimes - DetectSucceededTimes;
//		LIVE_ASSERT(false);
		return 0;
	}

	/// 获取连接失败次数
	UINT GetConnectFailedTimes() const
	{
		if (ConnectTimes >= ConnectSucceededTimes)
			return ConnectTimes - ConnectSucceededTimes;
	//	LIVE_ASSERT(false);
		return 0;
	}

	/// 是否没有探测成功过
	bool IsUndetectable() const
	{
		return DetectTimes > 0 && DetectSucceededTimes == 0;
	}

	/// 是否没有连接成功过
	bool IsUnconnectable() const
	{
		return ConnectTimes > 0 && ConnectSucceededTimes == 0;
	}

	/// 转换为字符串
	string ToString() const
	{
		char str[1025] = { 0 };
#if defined(_PPL_PLATFORM_MSWIN)
        _snprintf(str, 1024, "%s %u %u %u %u %u %u %hu %hu %hu %hu %hu %hu %hu", 
            this->GetAddress().ToString().c_str(), 
            this->RTT, 
            this->LastDetectTime, 
            this->LastConnectTime, 
            this->LastConnectFailedTime, 
            this->LastConnectFailedTime, 
            this->LastDisconnectTime, 
            this->DetectTimes, 
            this->DetectSucceededTimes, 
            this->ConnectTimes, 
            this->ConnectSucceededTimes, 
            this->ConnectFailedTimes, 
            this->DisconnectTimes, 
            this->ConnectFlags);
        return str;
#else
        snprintf(str, 1024, "%s %u %u %u %u %u %u %hu %hu %hu %hu %hu %hu %hu", 
            this->GetAddress().ToString().c_str(), 
            this->RTT, 
            this->LastDetectTime, 
            this->LastConnectTime, 
            this->LastConnectFailedTime, 
            this->LastConnectFailedTime, 
            this->LastDisconnectTime, 
            this->DetectTimes, 
            this->DetectSucceededTimes, 
            this->ConnectTimes, 
            this->ConnectSucceededTimes, 
            this->ConnectFailedTimes, 
            this->DisconnectTimes, 
            this->ConnectFlags);
#endif
		
		return str;
	}

private:
	///清除信息
	void Clear()
	{
		memset(this, 0, sizeof(CIPInfo));
		ConnectFlags = IPPOOL_CONNECT_NONE;
		this->CanDetect = false;
	}
};

#endif
