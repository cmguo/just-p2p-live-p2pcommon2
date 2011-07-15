
#ifndef _LIVE_P2PCOMMON2_NEW_POS_TRACKER_CLIENT_H_
#define _LIVE_P2PCOMMON2_NEW_POS_TRACKER_CLIENT_H_

#include <synacast/protocol/DataIO.h>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

/**
 * @file
 * @brief 包含tracker客户端的相关类
 */


struct OLD_UDP_PACKET_HEAD;
struct NEW_UDP_PACKET_HEAD;
struct TRACKER_ADDRESS;
struct TRACKER_LOGIN_ADDRESS;
class TrackerClientListener;
class RateMeasure;
class PeerInformation;
class TrackerPacketSender;
class SecureTrackerRequestPacketSender;
struct SECURE_TRACKER_PACKET_HEAD;



/// tracker客户端，封装和一个tracker通讯的细节
class TrackerClient : private boost::noncopyable
{
public:
	TrackerClient() { }
	virtual ~TrackerClient() { }

	virtual void Init( TrackerClientListener& listener, const TRACKER_LOGIN_ADDRESS& addr, boost::shared_ptr<PeerInformation> peerInformation, boost::shared_ptr<TrackerPacketSender> packetSender ) = 0;

	/// 获取服务器的地址
	virtual const TRACKER_ADDRESS& GetServerAddress() const = 0;

	/// 服务器是否是UDP方式的
	virtual bool IsUDP() const = 0;

	/// 是否已经登录到服务器
	virtual bool IsOnline() const = 0;

	/// 是否活动
	virtual bool IsActive() const = 0;

	/// 启动客户端
	virtual void Start() = 0;

	/// 停止客户端
	virtual void Stop() = 0;
	virtual void Restart() = 0;

	/// 到服务器做一次保活
	virtual void KeepAlive() = 0;

	/// 到服务器上获取可供访问的peer列表
	virtual void ListPeers() = 0;

	/// 登录服务器
	virtual void Login() = 0;
	/// 退出服务器
	virtual void Logout() = 0;


	/// 处理回应报文，返回true表示处理了
	virtual bool HandleResponse( data_input_stream& is, const OLD_UDP_PACKET_HEAD& head ) = 0;
	/// 处理安全回应报文，返回true表示处理了
	virtual bool HandleSecureResponse( data_input_stream& is, const NEW_UDP_PACKET_HEAD& head, bool isPeerProxy ) = 0;

	/// 获取状态代码(1/0/错误码)
	virtual long GetStatusCode() const = 0;

	virtual void SaveDHTTransactionID(UINT32 transactionID, UINT8 action) = 0;

public:
	/// 失败次数
	UINT16		FailedTimes;
	/// 做List操作的次数
	UINT8		ListTimes;
	/// List的个数
	UINT8		ListCount;
};



/// TrackerClient类工厂
class TrackerClientFactory
{
public:
	static TrackerClient* PeerCreate();

	static TrackerClient* MCCCreate();

	static TrackerClient* SimpleMDSCreate();

};


#endif