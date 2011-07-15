
#ifndef _LIVE_P2PCOMMON2_NEW_POS_TRACKER_REQUESTER_H_
#define _LIVE_P2PCOMMON2_NEW_POS_TRACKER_REQUESTER_H_

#include "TrackerClientListener.h"
#include <synacast/protocol/DataIO.h>
#include <boost/shared_ptr.hpp>


struct OLD_UDP_PACKET_HEAD;
struct NEW_UDP_PACKET_HEAD;
struct TRACKER_LOGIN_ADDRESS;
class TrackerRequesterListener;
class TrackerClient;
class PeerInformation;
class TrackerPacketSender;
class InetSocketAddress;
class SecureTrackerRequestPacketSender;
struct SECURE_TRACKER_PACKET_HEAD;


/// Tracker�ͻ��˹������Ľӿ��࣬��װһ��Tracker�ͻ��˵����Ӳ���
class TrackerRequester
{
public:
	TrackerRequester() { }
	virtual ~TrackerRequester() { }

	/// ����tracker
	virtual void Start( TrackerRequesterListener& listener, UINT count, const TRACKER_LOGIN_ADDRESS addrs[], 
		boost::shared_ptr<PeerInformation> peerInformation, boost::shared_ptr<TrackerPacketSender> packetSender ) = 0;

	virtual bool IsStarted() const = 0;

	/// ����tracker
	virtual void Restart() = 0;

	virtual void KeepAlive() = 0;

	/// TO-DO: ���·������б�
	virtual void Update(UINT count, const TRACKER_LOGIN_ADDRESS addrs[]) { }

	/// ���Դ����Ӧ����
	virtual bool TryHandleResponse( data_input_stream& is, const InetSocketAddress& sockAddr, BYTE proxyType ) = 0;
	virtual bool TryHandleSecureResponse( data_input_stream& is, NEW_UDP_PACKET_HEAD& head, const InetSocketAddress& sockAddr, BYTE proxyType, bool isPeerProxy ) = 0;

	/// ���������ϻ�ȡ�ɹ����ʵ�peer�б�
	virtual void ListPeers() = 0;

	/// ��ȡ�ͻ��˸���
	virtual size_t GetClientCount() const = 0;

	/// ����������ȡ�ͻ���
	virtual TrackerClientPtr GetClient(size_t index) = 0;

	/// ��ȡ��ǰ�Ŀͻ���
	virtual size_t GetCurrentClient() const { return 0; }

	/// ��ȡ��ǰ�Ƿ�����tcp tracker
	virtual bool IsTCPTrackerUsed() const { return false; }

	virtual void EnabledTCPTracker() { };

	virtual TrackerStatstics& GetStatistics() = 0;
};



/// TrackerRequester�๤��
class TrackerRequesterFactory
{
public:
	/// ��������peer�˵�tracker
	static TrackerRequester* PeerCreate();
	/// ��������mcc�˵�tracker
	static TrackerRequester* MCCCreate();
	/// ��������mds�˵�tracker
	static TrackerRequester* MDSCreate();
	/// ��������SimpleMDS�˵�tracker
	static TrackerRequester* SimpleMDSCreate();
	/// Added by Tady, 022311: For SSN
	static TrackerRequester* SparkMDSCreate();
};

#endif

