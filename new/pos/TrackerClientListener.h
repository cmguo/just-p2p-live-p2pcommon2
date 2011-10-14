
#ifndef _LIVE_P2PCOMMON2_NEW_POS_TRACKER_CLIENT_LISTENER_H_
#define _LIVE_P2PCOMMON2_NEW_POS_TRACKER_CLIENT_LISTENER_H_

#include <synacast/protocol/DataCollecting.h>
#include "util/flow.h"
#include <synacast/protocol/DataIO.h>
#include <boost/shared_ptr.hpp>


/**
 * @file
 * @brief ����tracker�ͻ��˵������
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
	/// ��������������
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




/// tracker�ͻ��˵��¼�
class TrackerClientListener
{
public:
	virtual ~TrackerClientListener() { }

	/// ��¼�ɹ�
	virtual void OnTrackerClientLogin(TrackerClient* sender) = 0;

	/// ��¼ʧ��
	virtual void OnTrackerClientLoginFailed(TrackerClient* sender, long errcode) { LIVE_ASSERT(false); }

	/// ����ɹ�
	virtual void OnTrackerClientKeepAlive(TrackerClient* sender) { LIVE_ASSERT(false); }

	/// ����ʧ��
	virtual void OnTrackerClientKeepAliveFailed(TrackerClient* sender, long errcode) { LIVE_ASSERT(false); }

	/// list�ɹ�
	virtual void OnTrackerClientList(TrackerClient* sender, size_t count, const CANDIDATE_PEER_INFO peers[], 
		size_t lanPeerCount, const INNER_CANDIDATE_PEER_INFO lanPeers[], UINT64 sourceTimeStamp) = 0;

	/// listʧ��
	virtual void OnTrackerClientListFailed(TrackerClient* sender, long errcode) { LIVE_ASSERT(false); }



	/// ��������ɹ�
	virtual void HandleSuccess(TrackerClient& client) { LIVE_ASSERT(false); }

	/// �������ʧ��
	virtual bool HandleFail(TrackerClient& client) { LIVE_ASSERT(false); return true; }


	/// ����source��minmax
	virtual bool SaveSourceMinMax(const PEER_MINMAX& minmax) { LIVE_ASSERT(false); return false; }

	/// ���汻̽�⵽���ⲿ��ַ
	virtual void SaveDetectedAddress(UINT detectedIP, UINT16 detectedUDPPort, TrackerClient* sender) = 0;

	/// ��ȡ���ʱ�����ٴα����������
	virtual int GetMaxKeepAliveTimeoutTimes() { return 5; }

	virtual TrackerStatstics& GetTrackerStatistics() = 0;
};


#endif