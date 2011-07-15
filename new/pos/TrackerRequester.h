
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


/// Tracker客户端管理器的接口类，封装一组Tracker客户端的连接策略
class TrackerRequester
{
public:
	TrackerRequester() { }
	virtual ~TrackerRequester() { }

	/// 启动tracker
	virtual void Start( TrackerRequesterListener& listener, UINT count, const TRACKER_LOGIN_ADDRESS addrs[], 
		boost::shared_ptr<PeerInformation> peerInformation, boost::shared_ptr<TrackerPacketSender> packetSender ) = 0;

	virtual bool IsStarted() const = 0;

	/// 重启tracker
	virtual void Restart() = 0;

	virtual void KeepAlive() = 0;

	/// TO-DO: 更新服务器列表
	virtual void Update(UINT count, const TRACKER_LOGIN_ADDRESS addrs[]) { }

	/// 尝试处理回应报文
	virtual bool TryHandleResponse( data_input_stream& is, const InetSocketAddress& sockAddr, BYTE proxyType ) = 0;
	virtual bool TryHandleSecureResponse( data_input_stream& is, NEW_UDP_PACKET_HEAD& head, const InetSocketAddress& sockAddr, BYTE proxyType, bool isPeerProxy ) = 0;

	/// 到服务器上获取可供访问的peer列表
	virtual void ListPeers() = 0;

	/// 获取客户端个数
	virtual size_t GetClientCount() const = 0;

	/// 根据索引获取客户端
	virtual TrackerClientPtr GetClient(size_t index) = 0;

	/// 获取当前的客户端
	virtual size_t GetCurrentClient() const { return 0; }

	/// 获取当前是否启用tcp tracker
	virtual bool IsTCPTrackerUsed() const { return false; }

	virtual void EnabledTCPTracker() { };

	virtual TrackerStatstics& GetStatistics() = 0;
};



/// TrackerRequester类工厂
class TrackerRequesterFactory
{
public:
	/// 创建用于peer端的tracker
	static TrackerRequester* PeerCreate();
	/// 创建用于mcc端的tracker
	static TrackerRequester* MCCCreate();
	/// 创建用于mds端的tracker
	static TrackerRequester* MDSCreate();
	/// 创建用于SimpleMDS端的tracker
	static TrackerRequester* SimpleMDSCreate();
	/// Added by Tady, 022311: For SSN
	static TrackerRequester* SparkMDSCreate();
};

#endif

