
#ifndef _LIVE_P2PCOMMON2_NEW_POS_TRACKER_CLIENT_IMPL_H_
#define _LIVE_P2PCOMMON2_NEW_POS_TRACKER_CLIENT_INPL_H_



/**
 * @file
 * @brief 包含tracker客户端的相关类
 */

#include "TrackerClient.h"
#include "framework/timer.h"
#include <synacast/protocol/data/TrackerAddress.h>
#include <ppl/util/time_counter.h>
#include <boost/shared_ptr.hpp>
#include <map>

class TrackerStatstics;
class PeerInformation;
class PeerNetInfo;
class PeerStatusInfo;
class PeerAuthInfo;
class TrackerPacketSender;
class PacketBase;
struct PEER_MINMAX;
struct SECURE_RESPONSE_HEAD;


class TransactionInfo
{
public:
	UINT32 TransactionID;
	UINT8 Action;
	time_counter RequestTime;

	TransactionInfo() : TransactionID( 0 ), Action( 0 ) { }
	explicit TransactionInfo(UINT32 transactionID, UINT8 action) : TransactionID( transactionID ), Action( action ) { }
};


typedef std::map<UINT32, TransactionInfo> TransactionInfoCollection;

/// tracker客户端，封装和一个tracker通讯的细节
class TrackerClientImpl : public TrackerClient
{
public:
	TrackerClientImpl();
	virtual ~TrackerClientImpl();

	virtual void Init( TrackerClientListener& listener, const TRACKER_LOGIN_ADDRESS& addr, boost::shared_ptr<PeerInformation> peerInformation, boost::shared_ptr<TrackerPacketSender> packetSender );

	/// 枚举，描述tracker客户端的状态
	enum TrackerClientState
	{
		tcs_offline	= 0, 
		tcs_online	= 1, 
	};

	enum TrackerSecurityStateEnum
	{
		tsse_undetermined = 0, 
		tsse_insecure = 1,
		tsse_secure = 2, 
	};

	/// 获取服务器的地址
	const TRACKER_ADDRESS& GetServerAddress() const
	{
		LIVE_ASSERT( 0 == m_address.ServerAddress.ReservedStatusCode );
		return m_address.ServerAddress;
	}
	UINT8 GetServerType() const { return GetServerAddress().Type; }

	UINT8 GetProxyType() const
	{
		return GetServerType();
	}


	/// 服务器是否是UDP方式的
	bool IsUDP() const;
	bool IsTCP() const;
	bool IsHTTP() const;

	virtual void SaveDHTTransactionID(UINT32 transactionID, UINT8 action);


	/// 是否已经登录到服务器
	bool IsOnline() const	{ return m_State != tcs_offline; }

	/// 启动客户端
	void Start();
	void Restart();
	/// 停止客户端
	void Stop();

	/// 到服务器做一次保活
	void KeepAlive();
	/// 到服务器上获取可供访问的peer列表
	void ListPeers() { }

	/// 登录服务器
	virtual void Login() = 0;
	/// 退出服务器
	virtual void Logout() = 0;

	virtual long GetStatusCode() const;

	virtual bool IsActive() const { return m_active; }

	virtual bool HandleSecureResponse( data_input_stream& is, const NEW_UDP_PACKET_HEAD& head, bool isPeerProxy );

protected:
	void CheckTrackerSecurity( const OLD_UDP_PACKET_HEAD& head );
	void SetTrackerSecurityOK( )
	{
		m_TrackerSecurity = tsse_secure;
	}

	bool CheckResponse( const OLD_UDP_PACKET_HEAD& head );
	bool CheckResponse( const NEW_UDP_PACKET_HEAD& head, bool isPeerProxy );

	bool DoCheckResponse( UINT8 action, UINT32 transactionID, bool isSecure, bool isPeerProxy );
	void CheckExpiredResponse();

	/// 保存保活间隔时间，interval为间隔时间，单位：秒
	void SaveKeepAliveInterval(UINT interval);
	void StartKeepAliveTimer();

	/// 请求超时
	void OnRequestTimeout();

	/// 检查回应报文是否为成功
	bool CheckSuccess(const OLD_UDP_PACKET_HEAD& head);

	/// 处理服务器的错误回应报文
	void HandleError(INT8 errcode);


	/// 发送一个请求报文，不需要检查超时
	bool SendRequest(const PacketBase& req, bool isSecure);

	/// 从服务器上退出
	void DoLeave(const string& user, const string& pwd);

	/// 重置客户端状态
	void ResetState();
	/// 切换到登录成功状态
	void ChangeToOnline();

	/// 操作成功
	void OnSuccess(const PEER_MINMAX& sourceMinMax);
	/// 操作失败
	bool OnFail();

	void OnSecureSuccess( const NEW_UDP_PACKET_HEAD& head, const SECURE_RESPONSE_HEAD& respHead );

	/// 启动超时定时器
	void StartTimeoutTimer( bool isLogin );

	/// 成功的回应报文处理函数
	typedef void (TrackerClientImpl::*FUNC_ON_RESPONSE_SUCCEEDED)(data_input_stream&);
	/// 失败的回应报文处理函数
	typedef void (TrackerClientImpl::*FUNC_ON_RESPONSE_FAILED)(INT8);

	/// 通用的回应报文处理函数
	bool DoHandleResponse( data_input_stream& is, const OLD_UDP_PACKET_HEAD& head, FUNC_ON_RESPONSE_SUCCEEDED fun1, FUNC_ON_RESPONSE_FAILED fun2 );

protected:
	bool OnSecureJoinSucceeded( data_input_stream& is );
	bool OnSecureListedPeersSucceeded( data_input_stream& is, const NEW_UDP_PACKET_HEAD& head );
	bool OnSecureErrorResponse( data_input_stream& is );

	virtual bool DoHandleSecureResponse( data_input_stream& is, const NEW_UDP_PACKET_HEAD& head, const SECURE_RESPONSE_HEAD& respHead ) = 0;

public:
	/// KeepAlive成功
	virtual void OnKeepAliveSucceeded( data_input_stream& is );
	/// KeepAlive失败
	virtual void OnKeepAliveFailed(INT8 errcode);

	/// Register成功
	virtual void OnRegisterSucceeded( data_input_stream& is ) { }
	/// Register失败
	virtual void OnRegisterFailed(INT8 errcode) { }



	/// Join成功
	virtual void OnJoinSucceeded( data_input_stream& is ) { }
	/// Join失败
	virtual void OnJoinFailed(INT8 errcode) { }

	/// List成功
	virtual void OnListSucceeded( data_input_stream& is ) { }
	/// List失败
	virtual void OnListFailed(INT8 errcode) { }

	/// needUpdateKeepAliveInterval表示是否需要更新保活间隔时间，source暂时不更新此时间
	bool OnSecureKeepAliveSucceeded( data_input_stream& is, bool needUpdateKeepAliveInterval );

protected:
	bool HandleKeepAliveResponse(const OLD_UDP_PACKET_HEAD& head, data_input_stream& is);

	virtual void HandleSecureErrorResponse( UINT8 errorAction, INT8 errcode );


protected:
	boost::shared_ptr<PeerInformation> m_PeerInformation;
	boost::shared_ptr<const PeerNetInfo> m_NetInfo;
	boost::shared_ptr<const PeerStatusInfo> m_StatusInfo;
	boost::shared_ptr<const PeerAuthInfo> m_AuthInfo;
	boost::shared_ptr<TrackerPacketSender> m_PacketSender;

	/// tracker客户端事件的侦听者
	TrackerClientListener* m_listener;

	TrackerStatstics* m_Statistics;

	/// 用于定时保活的定时器
	periodic_timer m_KeepAliveTimer;

	/// 用于检查超时的定时器
	once_timer m_TimeoutTimer;

	/// 服务器地址
	TRACKER_LOGIN_ADDRESS m_address;

	/// 保活间隔时间
	UINT m_KeepAliveInterval;

	/// 客户端的当前状态
	TrackerClientState m_State;


	/// 最近一次的错误
	long m_lastError;

	/// KeepAlive超时次数
	int	m_KeepAliveTimeoutTimes;

	/// 是否活动
	bool m_active;

	TransactionInfoCollection m_Transactions;

	TrackerSecurityStateEnum m_TrackerSecurity;

	UINT m_RequestTimeout;
	UINT m_LoginRequestTimeout;

	bool m_IsServer;
};





/// 用于peer端的Tracker客户端
class PeerTrackerClient : public TrackerClientImpl
{
public:
	PeerTrackerClient() : m_LastListTime(0) { }

	virtual void Login();
	virtual void Logout();
	
	virtual void ListPeers();

	virtual bool HandleResponse( data_input_stream& is, const OLD_UDP_PACKET_HEAD& head );
	virtual bool DoHandleSecureResponse( data_input_stream& is, const NEW_UDP_PACKET_HEAD& head, const SECURE_RESPONSE_HEAD& respHead );

protected:
	/// 将peer加入到tracker
	virtual void DoJoin();

	/// 处理服务器的List回应报文
	bool HandleListResponse(const OLD_UDP_PACKET_HEAD& head, data_input_stream& is);

	/// 处理服务器的Join回应报文
	bool HandleJoinResponse(const OLD_UDP_PACKET_HEAD& head, data_input_stream& is);


	/// Join成功
	void OnJoinSucceeded( data_input_stream& is );
	/// Join失败
	void OnJoinFailed(INT8 errcode);

	/// List成功
	void OnListSucceeded( data_input_stream& is );
	/// List失败
	void OnListFailed(INT8 errcode);


private:
	time_counter m_LastListTime;
};

/// 用于source端的tracker客户端
class SourceTrackerClient : public TrackerClientImpl
{
public:
	SourceTrackerClient();

	virtual void Login();
	virtual void Logout();

	virtual bool HandleResponse( data_input_stream& is, const OLD_UDP_PACKET_HEAD& head );
	virtual bool DoHandleSecureResponse( data_input_stream& is, const NEW_UDP_PACKET_HEAD& head, const SECURE_RESPONSE_HEAD& respHead );

protected:
	/// 将source注册到tracker
	void DoRegister();
	void DoSourceExit();

	bool HandleRegisterResponse(const OLD_UDP_PACKET_HEAD& head, data_input_stream& is);

	void OnRegisterSucceeded( data_input_stream& is );
	void OnRegisterFailed(INT8 errcode);

	bool OnSecureRegisterSucceeded( data_input_stream& is );
};

/// 用于SimpleMDS端的tracker客户端(操作类似于PeerTrackerClient，只是暂不使用TCP Tracker)
class SimpleMDSTrackerClient : public PeerTrackerClient
{
public:
	SimpleMDSTrackerClient();

	virtual void Login();

};

#endif
