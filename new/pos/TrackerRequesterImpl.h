
#ifndef _LIVE_P2PCOMMON2_NEW_POS_TRACKER_REQUESTER_IMPL_H_
#define _LIVE_P2PCOMMON2_NEW_POS_TRACKER_REQUESTER_IMPL_H_

#include "TrackerRequester.h"
#include "TrackerClientListener.h"
#include "framework/timer.h"
#include "framework/memory.h"
#include <synacast/protocol/data/TrackerAddress.h>
#include <boost/shared_ptr.hpp>
#include <map>
#include <set>

struct TRACKER_ADDRESS;
class PeerInformation;
class TrackerClient;
typedef std::vector<TrackerClientPtr>	TrackerClientCollection;

typedef std::map<TRACKER_ADDRESS	, TrackerClientPtr> TrackerClientIndex;




/// Tracker客户端管理器的接口类，封装一组Tracker客户端的连接策略
class TrackerRequesterImpl : public TrackerRequester, protected TrackerClientListener, public pool_object
{
public:
	TrackerRequesterImpl();
	virtual ~TrackerRequesterImpl();

	virtual void ListPeers() { LIVE_ASSERT(false); }

	virtual bool IsStarted() const { return m_isStarted; }

	virtual void Start( TrackerRequesterListener& listener, UINT count, const TRACKER_LOGIN_ADDRESS addrs[], boost::shared_ptr<PeerInformation> peerInformation, boost::shared_ptr<TrackerPacketSender> packetSender );

	virtual bool TryHandleResponse( data_input_stream& is, const InetSocketAddress& sockAddr, BYTE proxyType );
	virtual bool TryHandleSecureResponse( data_input_stream& is, NEW_UDP_PACKET_HEAD& head, const InetSocketAddress& sockAddr, BYTE proxyType, bool isPeerProxy );

	virtual void HandleSuccess(TrackerClient& client) { }

	virtual bool HandleFail(TrackerClient& client) { return true; }

	virtual int GetMaxKeepAliveTimeoutTimes() const;

	virtual size_t GetClientCount() const;

	virtual TrackerClientPtr GetClient(size_t index);

	virtual TrackerStatstics& GetStatistics() { return m_Statistics; }

	virtual TrackerStatstics& GetTrackerStatistics() { return m_Statistics; };

	virtual void Restart() { }
	virtual void KeepAlive() { }


protected:
	/// 根据指定的tracker地址列表启动客户端
	virtual void DoStart(size_t trackerCount, const TRACKER_LOGIN_ADDRESS addrs[]);


	/// 根据指定的tracker地址创建一个tracker客户端
	virtual TrackerClient* CreateClient() = 0;

	/// 创建一组客户端
	void CreateClients(size_t count, const TRACKER_LOGIN_ADDRESS addrs[]);

	/// 删除所有的客户端
	void DeleteClients();

	/// 启动客户端
	virtual void DoStartClients();

	// 确定保持运行的timer事件函数
	virtual void OnKeepRunningTimer();

	virtual void OnTrackerClientLogin(TrackerClient* sender);

	virtual void OnTrackerClientLoginFailed(TrackerClient* sender, long errcode);

	virtual void OnTrackerClientKeepAlive(TrackerClient* sender);

	virtual void OnTrackerClientKeepAliveFailed(TrackerClient* sender, long errcode);

	virtual void OnTrackerClientList(TrackerClient* sender, size_t count, const CANDIDATE_PEER_INFO peers[], size_t lanPeerCount, const INNER_CANDIDATE_PEER_INFO lanPeers[], UINT64 sourceTimeStamp);

	virtual void OnTrackerClientListFailed(TrackerClient* sender, long errcode);


	virtual bool SaveSourceMinMax(const PEER_MINMAX& minmax);

	virtual void SaveDetectedAddress(UINT detectedIP, UINT16 detectedUDPPort, TrackerClient* sender);

protected:
	boost::shared_ptr<PeerInformation> m_PeerInformation;
	boost::shared_ptr<TrackerPacketSender> m_PacketSender;

	/// 时间侦听器
	TrackerRequesterListener* m_listener;

	/// tracker客户端集合
	TrackerClientCollection m_clients;
	TrackerClientIndex m_clientIndex;

	// 保持运行的timer
	periodic_timer m_KeepRunningTimer;

	/// 定期操作的间隔时间
	UINT m_KeepRunningInterval;

	TrackerStatstics m_Statistics;

	bool m_isStarted;
};





/// 用于peer端的tracker客户端管理器
class PeerTrackerRequester : public TrackerRequesterImpl
{
public:
	PeerTrackerRequester();
	~PeerTrackerRequester();

	virtual void Restart();
	virtual void KeepAlive();

	virtual void ListPeers();

	virtual bool HandleFail(TrackerClient& client);
	virtual void HandleSuccess(TrackerClient& client);

	/// 获取当前的客户端
	TrackerClientPtr GetCurrentClient()
	{
//		LIVE_ASSERT(!m_clients.empty() && m_CurrentClient < m_clients.size());
		if (m_clients.empty() || m_CurrentClient >= m_clients.size())
		{
			TrackerClientPtr nullClient(CreateClient()) ;
			return nullClient;
		}

		return m_clients[m_CurrentClient];
	}
	
	virtual void EnabledTCPTracker()
	{
		TRACKER_EVENT("EnabledTCPTracker " << m_UseTCP);
		m_UseTCP = true;
	}
	virtual void OnTrackerClientList(TrackerClient* sender, size_t count, const CANDIDATE_PEER_INFO peers[], size_t lanPeerCount, const INNER_CANDIDATE_PEER_INFO lanPeers[], UINT64 sourceTimeStamp);
	//virtual void OnTrackerClientListFailed(TrackerClient* sender, long errcode);
	
protected:
	virtual TrackerClient* CreateClient();
	virtual void DoStartClients();
	virtual void OnKeepRunningTimer();

	virtual void DoStart(size_t trackerCount, const TRACKER_LOGIN_ADDRESS addrs[]);

	virtual size_t GetCurrentClientIndex() const { return m_CurrentClient; }
	virtual bool IsTCPTrackerUsed() const { return m_UseTCP; }

	virtual void OnTrackerClientLogin(TrackerClient* sender);

private:
	/// 当前活动的客户端的索引
	BYTE	m_CurrentClient;
	/// 是否启用TCP Tracker
	bool	m_UseTCP;
};


/// 用于source端的tracker客户端管理器
class SourceTrackerRequester : public TrackerRequesterImpl
{
public:
	SourceTrackerRequester()
	{
	}

protected:
	virtual TrackerClient* CreateClient();
};




/// 用于SimpleMDS端的tracker客户端管理器(策略类似于SourceTracker，另外需要提供List操作)
class SimpleMDSTrackerRequester : public SourceTrackerRequester
{
public:
	SimpleMDSTrackerRequester()
	{
		/// mas的KeepRunning间隔时间可以长一些
		m_KeepRunningInterval *= 10;
	}

	virtual void ListPeers();

protected:
	virtual TrackerClient* CreateClient();
	virtual void OnKeepRunningTimer();

	virtual void DoStart(size_t trackerCount, const TRACKER_LOGIN_ADDRESS addrs[]);

};


class MDSTrackerRequester : public SimpleMDSTrackerRequester
{
public:
	MDSTrackerRequester()
	{
	}

	/// mds不需要做List操作
	virtual void ListPeers() { }

};

/// Added by Tady, 022311: Spark-SN, no JOIN, just LIST.
class SparkMDSTrackerRequester : public SimpleMDSTrackerRequester
{
public:
	SparkMDSTrackerRequester()
	{
	}

protected:
	virtual void DoStart(size_t trackerCount, const TRACKER_LOGIN_ADDRESS addrs[]);
};


#endif