
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




/// Tracker�ͻ��˹������Ľӿ��࣬��װһ��Tracker�ͻ��˵����Ӳ���
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
	/// ����ָ����tracker��ַ�б������ͻ���
	virtual void DoStart(size_t trackerCount, const TRACKER_LOGIN_ADDRESS addrs[]);


	/// ����ָ����tracker��ַ����һ��tracker�ͻ���
	virtual TrackerClient* CreateClient() = 0;

	/// ����һ��ͻ���
	void CreateClients(size_t count, const TRACKER_LOGIN_ADDRESS addrs[]);

	/// ɾ�����еĿͻ���
	void DeleteClients();

	/// �����ͻ���
	virtual void DoStartClients();

	// ȷ���������е�timer�¼�����
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

	/// ʱ��������
	TrackerRequesterListener* m_listener;

	/// tracker�ͻ��˼���
	TrackerClientCollection m_clients;
	TrackerClientIndex m_clientIndex;

	// �������е�timer
	periodic_timer m_KeepRunningTimer;

	/// ���ڲ����ļ��ʱ��
	UINT m_KeepRunningInterval;

	TrackerStatstics m_Statistics;

	bool m_isStarted;
};





/// ����peer�˵�tracker�ͻ��˹�����
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

	/// ��ȡ��ǰ�Ŀͻ���
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
	/// ��ǰ��Ŀͻ��˵�����
	BYTE	m_CurrentClient;
	/// �Ƿ�����TCP Tracker
	bool	m_UseTCP;
};


/// ����source�˵�tracker�ͻ��˹�����
class SourceTrackerRequester : public TrackerRequesterImpl
{
public:
	SourceTrackerRequester()
	{
	}

protected:
	virtual TrackerClient* CreateClient();
};




/// ����SimpleMDS�˵�tracker�ͻ��˹�����(����������SourceTracker��������Ҫ�ṩList����)
class SimpleMDSTrackerRequester : public SourceTrackerRequester
{
public:
	SimpleMDSTrackerRequester()
	{
		/// mas��KeepRunning���ʱ����Գ�һЩ
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

	/// mds����Ҫ��List����
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