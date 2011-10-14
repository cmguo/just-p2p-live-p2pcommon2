
#ifndef _LIVE_P2PCOMMON2_NEW_POS_TRACKER_CLIENT_LISTENER_H_
#define _LIVE_P2PCOMMON2_NEW_POS_TRACKER_CLIENT_LISTENER_H_

#include <synacast/protocol/DataCollecting.h>
#include "util/flow.h"
#include <synacast/protocol/DataIO.h>
#include <boost/shared_ptr.hpp>


/**
 * @file
 * @brief 包含tracker客户端的相关类
 */


struct TRACKER_ADDRESS;
struct CANDIDATE_PEER_INFO;
struct PEER_MINMAX;
struct INNER_CANDIDATE_PEER_INFO;

class PacketBase;
class TrackerClient;
class TrackerStatstics;

typedef boost::shared_ptr<TrackerClient> TrackerClientPtr;





class TrackerStatstics : public TRACKER_STATS
{
public:
	/// 报文流量和数量
	FlowMeasure Flow;

	TrackerStatstics()
	{
		this->Clear();
	}

	void SyncFlow()
	{
		this->Flow.SyncTo( this->Traffic );
	}
	void UpdateFlow()
	{
		this->Flow.Update();
	}


};




/// tracker客户端的事件
class TrackerClientListener
{
public:
	virtual ~TrackerClientListener() { }

	/// 登录成功
	virtual void OnTrackerClientLogin(TrackerClient* sender) = 0;

	/// 登录失败
	virtual void OnTrackerClientLoginFailed(TrackerClient* sender, long errcode) { LIVE_ASSERT(false); }

	/// 保活成功
	virtual void OnTrackerClientKeepAlive(TrackerClient* sender) { LIVE_ASSERT(false); }

	/// 保活失败
	virtual void OnTrackerClientKeepAliveFailed(TrackerClient* sender, long errcode) { LIVE_ASSERT(false); }

	/// list成功
	virtual void OnTrackerClientList(TrackerClient* sender, size_t count, const CANDIDATE_PEER_INFO peers[], 
		size_t lanPeerCount, const INNER_CANDIDATE_PEER_INFO lanPeers[], UINT64 sourceTimeStamp) = 0;

	/// list失败
	virtual void OnTrackerClientListFailed(TrackerClient* sender, long errcode) { LIVE_ASSERT(false); }



	/// 处理操作成功
	virtual void HandleSuccess(TrackerClient& client) { LIVE_ASSERT(false); }

	/// 处理操作失败
	virtual bool HandleFail(TrackerClient& client) { LIVE_ASSERT(false); return true; }


	/// 保存source的minmax
	virtual bool SaveSourceMinMax(const PEER_MINMAX& minmax) { LIVE_ASSERT(false); return false; }

	/// 保存被探测到的外部地址
	virtual void SaveDetectedAddress(UINT detectedIP, UINT16 detectedUDPPort, TrackerClient* sender) = 0;

	/// 获取保活超时后尝试再次保活的最大次数
	virtual int GetMaxKeepAliveTimeoutTimes() { return 5; }

	virtual TrackerStatstics& GetTrackerStatistics() = 0;
};


#endif