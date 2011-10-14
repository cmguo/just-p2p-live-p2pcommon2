
#ifndef _LIVE_P2PCOMMON2_NEW_POS_TRACKER_CLIENT_IMPL_H_
#define _LIVE_P2PCOMMON2_NEW_POS_TRACKER_CLIENT_INPL_H_



/**
 * @file
 * @brief ����tracker�ͻ��˵������
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

/// tracker�ͻ��ˣ���װ��һ��trackerͨѶ��ϸ��
class TrackerClientImpl : public TrackerClient
{
public:
	TrackerClientImpl();
	virtual ~TrackerClientImpl();

	virtual void Init( TrackerClientListener& listener, const TRACKER_LOGIN_ADDRESS& addr, boost::shared_ptr<PeerInformation> peerInformation, boost::shared_ptr<TrackerPacketSender> packetSender );

	/// ö�٣�����tracker�ͻ��˵�״̬
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

	/// ��ȡ�������ĵ�ַ
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


	/// �������Ƿ���UDP��ʽ��
	bool IsUDP() const;
	bool IsTCP() const;
	bool IsHTTP() const;

	virtual void SaveDHTTransactionID(UINT32 transactionID, UINT8 action);


	/// �Ƿ��Ѿ���¼��������
	bool IsOnline() const	{ return m_State != tcs_offline; }

	/// �����ͻ���
	void Start();
	void Restart();
	/// ֹͣ�ͻ���
	void Stop();

	/// ����������һ�α���
	void KeepAlive();
	/// ���������ϻ�ȡ�ɹ����ʵ�peer�б�
	void ListPeers() { }

	/// ��¼������
	virtual void Login() = 0;
	/// �˳�������
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

	/// ���汣����ʱ�䣬intervalΪ���ʱ�䣬��λ����
	void SaveKeepAliveInterval(UINT interval);
	void StartKeepAliveTimer();

	/// ����ʱ
	void OnRequestTimeout();

	/// ����Ӧ�����Ƿ�Ϊ�ɹ�
	bool CheckSuccess(const OLD_UDP_PACKET_HEAD& head);

	/// ����������Ĵ����Ӧ����
	void HandleError(INT8 errcode);


	/// ����һ�������ģ�����Ҫ��鳬ʱ
	bool SendRequest(const PacketBase& req, bool isSecure);

	/// �ӷ��������˳�
	void DoLeave(const string& user, const string& pwd);

	/// ���ÿͻ���״̬
	void ResetState();
	/// �л�����¼�ɹ�״̬
	void ChangeToOnline();

	/// �����ɹ�
	void OnSuccess(const PEER_MINMAX& sourceMinMax);
	/// ����ʧ��
	bool OnFail();

	void OnSecureSuccess( const NEW_UDP_PACKET_HEAD& head, const SECURE_RESPONSE_HEAD& respHead );

	/// ������ʱ��ʱ��
	void StartTimeoutTimer( bool isLogin );

	/// �ɹ��Ļ�Ӧ���Ĵ�����
	typedef void (TrackerClientImpl::*FUNC_ON_RESPONSE_SUCCEEDED)(data_input_stream&);
	/// ʧ�ܵĻ�Ӧ���Ĵ�����
	typedef void (TrackerClientImpl::*FUNC_ON_RESPONSE_FAILED)(INT8);

	/// ͨ�õĻ�Ӧ���Ĵ�����
	bool DoHandleResponse( data_input_stream& is, const OLD_UDP_PACKET_HEAD& head, FUNC_ON_RESPONSE_SUCCEEDED fun1, FUNC_ON_RESPONSE_FAILED fun2 );

protected:
	bool OnSecureJoinSucceeded( data_input_stream& is );
	bool OnSecureListedPeersSucceeded( data_input_stream& is, const NEW_UDP_PACKET_HEAD& head );
	bool OnSecureErrorResponse( data_input_stream& is );

	virtual bool DoHandleSecureResponse( data_input_stream& is, const NEW_UDP_PACKET_HEAD& head, const SECURE_RESPONSE_HEAD& respHead ) = 0;

public:
	/// KeepAlive�ɹ�
	virtual void OnKeepAliveSucceeded( data_input_stream& is );
	/// KeepAliveʧ��
	virtual void OnKeepAliveFailed(INT8 errcode);

	/// Register�ɹ�
	virtual void OnRegisterSucceeded( data_input_stream& is ) { }
	/// Registerʧ��
	virtual void OnRegisterFailed(INT8 errcode) { }



	/// Join�ɹ�
	virtual void OnJoinSucceeded( data_input_stream& is ) { }
	/// Joinʧ��
	virtual void OnJoinFailed(INT8 errcode) { }

	/// List�ɹ�
	virtual void OnListSucceeded( data_input_stream& is ) { }
	/// Listʧ��
	virtual void OnListFailed(INT8 errcode) { }

	/// needUpdateKeepAliveInterval��ʾ�Ƿ���Ҫ���±�����ʱ�䣬source��ʱ�����´�ʱ��
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

	/// tracker�ͻ����¼���������
	TrackerClientListener* m_listener;

	TrackerStatstics* m_Statistics;

	/// ���ڶ�ʱ����Ķ�ʱ��
	periodic_timer m_KeepAliveTimer;

	/// ���ڼ�鳬ʱ�Ķ�ʱ��
	once_timer m_TimeoutTimer;

	/// ��������ַ
	TRACKER_LOGIN_ADDRESS m_address;

	/// ������ʱ��
	UINT m_KeepAliveInterval;

	/// �ͻ��˵ĵ�ǰ״̬
	TrackerClientState m_State;


	/// ���һ�εĴ���
	long m_lastError;

	/// KeepAlive��ʱ����
	int	m_KeepAliveTimeoutTimes;

	/// �Ƿ�
	bool m_active;

	TransactionInfoCollection m_Transactions;

	TrackerSecurityStateEnum m_TrackerSecurity;

	UINT m_RequestTimeout;
	UINT m_LoginRequestTimeout;

	bool m_IsServer;
};





/// ����peer�˵�Tracker�ͻ���
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
	/// ��peer���뵽tracker
	virtual void DoJoin();

	/// �����������List��Ӧ����
	bool HandleListResponse(const OLD_UDP_PACKET_HEAD& head, data_input_stream& is);

	/// �����������Join��Ӧ����
	bool HandleJoinResponse(const OLD_UDP_PACKET_HEAD& head, data_input_stream& is);


	/// Join�ɹ�
	void OnJoinSucceeded( data_input_stream& is );
	/// Joinʧ��
	void OnJoinFailed(INT8 errcode);

	/// List�ɹ�
	void OnListSucceeded( data_input_stream& is );
	/// Listʧ��
	void OnListFailed(INT8 errcode);


private:
	time_counter m_LastListTime;
};

/// ����source�˵�tracker�ͻ���
class SourceTrackerClient : public TrackerClientImpl
{
public:
	SourceTrackerClient();

	virtual void Login();
	virtual void Logout();

	virtual bool HandleResponse( data_input_stream& is, const OLD_UDP_PACKET_HEAD& head );
	virtual bool DoHandleSecureResponse( data_input_stream& is, const NEW_UDP_PACKET_HEAD& head, const SECURE_RESPONSE_HEAD& respHead );

protected:
	/// ��sourceע�ᵽtracker
	void DoRegister();
	void DoSourceExit();

	bool HandleRegisterResponse(const OLD_UDP_PACKET_HEAD& head, data_input_stream& is);

	void OnRegisterSucceeded( data_input_stream& is );
	void OnRegisterFailed(INT8 errcode);

	bool OnSecureRegisterSucceeded( data_input_stream& is );
};

/// ����SimpleMDS�˵�tracker�ͻ���(����������PeerTrackerClient��ֻ���ݲ�ʹ��TCP Tracker)
class SimpleMDSTrackerClient : public PeerTrackerClient
{
public:
	SimpleMDSTrackerClient();

	virtual void Login();

};

#endif
