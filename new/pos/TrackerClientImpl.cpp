
#include "StdAfx.h"

#include "TrackerClientImpl.h"
#include "TrackerClientListener.h"

#include "common/BaseInfo.h"
#include "common/PacketSender.h"
#include <synacast/protocol/TrackerPacket.h>
#include <ppl/data/stlutils.h>


//BOOST_STATIC_ASSERT(sizeof(EXTRA_PEER_NET_INFO) == sizeof(NET_TYPE));
//BOOST_STATIC_ASSERT(sizeof(LISTED_PEER_INFO) == sizeof(CANDIDATE_PEER_INFO));


/// 默认的保活间隔时间(单位：毫秒)
const UINT TRACKER_KEEP_ALIVE_INTERVAL = 60 * 1000;

/// tracker请求的超时时间(单位：毫秒)
const UINT TRACKER_REQUEST_TIMEOUT = 4 * 1000;
const UINT TRACKER_SERVER_LOGIN_MAX_REQUEST_TIMEOUT = 35 * 1000;




TrackerClient* TrackerClientFactory::PeerCreate()
{
	return new PeerTrackerClient();
}

TrackerClient* TrackerClientFactory::MCCCreate()
{
	return new SourceTrackerClient();
}

TrackerClient* TrackerClientFactory::SimpleMDSCreate()
{
	return new SimpleMDSTrackerClient();
}






TrackerClientImpl::TrackerClientImpl() 
	: m_listener( NULL )
	, m_Statistics( NULL )
        , m_State(tcs_offline)
        , m_active(false)
	, m_TrackerSecurity( tsse_undetermined)
	, m_RequestTimeout( TRACKER_REQUEST_TIMEOUT )
	, m_LoginRequestTimeout( TRACKER_REQUEST_TIMEOUT )
	, m_IsServer( false )
{
	m_address.LoginAddress.Clear();
	m_address.ServerAddress.Clear();

	//m_PeerInformation = m_listener->GetPeerInformation();
	//m_PacketSender = m_listener->GetPacketSender();
	m_lastError = 0;
	FailedTimes = 0;
	ListTimes = 0;
	ListCount = 60;
	m_KeepAliveInterval = TRACKER_KEEP_ALIVE_INTERVAL;
	m_KeepAliveTimeoutTimes = 0;

	m_KeepAliveTimer.set_callback(boost::bind(&TrackerClientImpl::KeepAlive, this));
	m_TimeoutTimer.set_callback(boost::bind(&TrackerClientImpl::OnRequestTimeout, this));
}

TrackerClientImpl::~TrackerClientImpl()
{
	assert(!IsOnline());
}


void TrackerClientImpl::Restart()
{
	m_active = false;
	m_KeepAliveTimer.stop();
	m_TimeoutTimer.stop();
	ResetState();

	m_active = true;
	Login();
}

void TrackerClientImpl::Start()
{
	Stop();
	m_active = true;
	Login();

}

void TrackerClientImpl::Stop()
{
	m_active = false;
	m_KeepAliveTimer.stop();
	m_TimeoutTimer.stop();
	if (IsOnline())
	{
		Logout();
		ResetState();
	}
}

void TrackerClientImpl::KeepAlive()
{
	TRACKER_DEBUG("KeepAlive " << m_address);
	assert(IsUDP());
	if (!IsUDP())
		return;
	assert( ( tsse_insecure == m_TrackerSecurity || tsse_secure == m_TrackerSecurity ) || false == this->IsOnline() );

	if (m_TimeoutTimer.is_started())
	{
		TRACKER_ERROR("An request has been queued." << m_address);
		return;
	}

	UINT16 qos = m_StatusInfo->UseFixedQOS ? m_StatusInfo->FixedQOS : m_StatusInfo->Status.Qos;
	if ( tsse_secure == m_TrackerSecurity )
	{
		PTSKeepAliveRequest req;
		req.Status = m_StatusInfo->GetStatusEx();
		req.MinMax = m_StatusInfo->MinMax;
		req.SetStunServerInfo( m_NetInfo->NeedNAT(), m_NetInfo->GetStunServerAddress(), m_NetInfo->GetStunDetectedAddress() );
		req.SourceTimeStamp = m_StatusInfo->CurrentTimeStamp;
			m_StatusInfo->Degrees, m_StatusInfo->Status.SkipPercent, m_StatusInfo->MinMax,
		SendRequest(req, true);
		m_Statistics->KeepAlive.RequestTimes++;
	}
	else
	{
		PEER_STATUS_INFO statusInfo = *m_StatusInfo;
		statusInfo.Status.Qos = qos;
		PTKeepAliveRequest req;
		req.StatusInfo = statusInfo;
		SendRequest(req, false);
		m_Statistics->KeepAlive.RequestTimes++;
	}
	StartTimeoutTimer( false );
}

void TrackerClientImpl::HandleError(INT8 errcode)
{
	m_lastError = errcode;
	switch (errcode)
	{
	case PT_ERROR_NO_CHANNEL:
		TRACKER_DEBUG("HandleErrorNoChannel " << m_address);
		break;
	case PT_ERROR_NO_PEER:
		TRACKER_DEBUG("HandleErrorNoPeer " << m_address);
		break;
	case PT_ERROR_PEER_EXIST:
		TRACKER_DEBUG("HandleErrorPeerExist " << m_address);
		break;
	case PT_ERROR_AUTH_FAILED:
		TRACKER_DEBUG("HandleErrorAuthFailed " << m_address);
		break;
	case PT_ERROR_ZONE_DENIED:
		TRACKER_DEBUG("HandleErrorZoneDenied " << m_address);
		break;
	default:
		TRACKER_DEBUG("Unrecognized error code " << int(errcode) << " at " << m_address << ", ignore.");
	}
}


void TrackerClientImpl::SaveKeepAliveInterval(UINT interval)
{
	assert( interval < 1000 );
	TRACKER_DEBUG("Save KeepAlive interval " << interval);
	// 将interval限制在10秒到10分钟之内
	LIMIT_MIN_MAX(interval, 10, 600);
	m_KeepAliveInterval = interval * 1000;
}

void TrackerClientImpl::OnRequestTimeout()
{
	TRACKER_DEBUG("Send request timeout, retry login " << m_address);
	if (IsUDP() && IsOnline())
	{
		// 保活失败
		// 如果是udp tracker保活失败，则再次进行保活
		++m_KeepAliveTimeoutTimes;
		if (m_KeepAliveTimeoutTimes < m_listener->GetMaxKeepAliveTimeoutTimes())
		{
			KeepAlive();
			return;
		}
	}
	// 其它情况，则转到下一个tracker，或再次登录
	if (!OnFail())
		return;
	Login();
}


void TrackerClientImpl::DoLeave(const string& user, const string& pwd)
{
	TRACKER_DEBUG("DoLeave " << m_address);
	if (!IsUDP())
		return;
	assert( tsse_secure == m_TrackerSecurity || tsse_insecure == m_TrackerSecurity );
	if ( tsse_secure == m_TrackerSecurity )
	{
		PTSLeaveRequest req;
		this->SendRequest( req, true );
		m_Statistics->LeaveTimes++;
	}
	else
	{
		PTLeaveRequest req;
		req.Username = user;
		req.Password = pwd;
		this->SendRequest(req, false);
		m_Statistics->LeaveTimes++;
	}
}

void TrackerClientImpl::ResetState()
{
	TRACKER_DEBUG("ppnet offline. " << m_address);
	m_KeepAliveTimer.stop();
	m_KeepAliveTimeoutTimes = 0;

	m_State = tcs_offline;
}
void TrackerClientImpl::StartKeepAliveTimer()
{
	m_KeepAliveTimer.stop();
	m_KeepAliveTimer.start(m_KeepAliveInterval);
}

void TrackerClientImpl::ChangeToOnline()
{
	m_State = tcs_online;

	// 重新启动保活定时器
	m_KeepAliveTimeoutTimes = 0;
	this->StartKeepAliveTimer();
	KeepAlive();
	TRACKER_DEBUG("Start KeepAlive Timer at interval " << m_KeepAliveInterval << " at " << m_address);
}

void TrackerClientImpl::OnSuccess(const PEER_MINMAX& sourceMinMax)
{
	m_TimeoutTimer.stop();
	m_listener->SaveSourceMinMax(sourceMinMax);
	m_listener->HandleSuccess(*this);
	m_KeepAliveTimeoutTimes = 0;
}

bool TrackerClientImpl::OnFail()
{
	ResetState();
	return m_listener->HandleFail(*this);
}

bool TrackerClientImpl::SendRequest(const PacketBase& req, bool isSecure)
{
	CheckExpiredResponse();
	const TRACKER_ADDRESS& addr = m_address.ServerAddress;
	size_t size = m_PacketSender->Send(req, InetSocketAddress(addr.IP, addr.Port), GetProxyType(), isSecure);
	if ( size <= 0 )
	{
		return false;
	}
	m_Statistics->Flow.Upload.Record( size );
	// 发送请求成功，记录Transaction信息
	TransactionInfo info( m_PacketSender->GetTransactionID(), req.GetAction() );
	assert( false == containers::contains( m_Transactions, info.TransactionID ) );
	m_Transactions[info.TransactionID] = info;
	return true;
}

void TrackerClientImpl::StartTimeoutTimer( bool isLogin )
{
	UINT timeout = isLogin ? m_LoginRequestTimeout : m_RequestTimeout;
	m_TimeoutTimer.start(timeout);
	if ( isLogin && m_IsServer )
	{
		// 如果是服务器端使用的，如source和sn等，则可以加大relogin的时间间隔
		// 屏蔽此功能 2009-03-03 chenmp
		//m_LoginRequestTimeout *= 2;
		LIMIT_MAX( m_LoginRequestTimeout, TRACKER_SERVER_LOGIN_MAX_REQUEST_TIMEOUT );
	}
}


bool TrackerClientImpl::CheckSuccess(const OLD_UDP_PACKET_HEAD& head)
{
	INT8 errcode = head.ActionType;
	if (errcode == 0)
		return true;
	if (errcode > 0)
	{
		TRACKER_DEBUG("Invalid response action type " << head.ActionType << " from " << GetServerAddress());
		return false;
	}
	return false;
}

bool TrackerClientImpl::DoHandleResponse( data_input_stream& is, const OLD_UDP_PACKET_HEAD& head, FUNC_ON_RESPONSE_SUCCEEDED fun1, FUNC_ON_RESPONSE_FAILED fun2 )
{
	if ( false == CheckResponse( head ) )
		return false;
	if (CheckSuccess(head))
	{
		PEER_MINMAX sourceMinMax;
		if ( is >> sourceMinMax )
		{
			OnSuccess( sourceMinMax );
			(this->*fun1)(is);
			TRACKER_DEBUG("HandleResponse source minmax " << sourceMinMax << " from " << GetServerAddress());
		}
		else
		{
			assert(false);
		}
		return true;
	}
	(this->*fun2)(head.ActionType);
	return false;
}

bool TrackerClientImpl::HandleKeepAliveResponse(const OLD_UDP_PACKET_HEAD& head, data_input_stream& is)
{
	TRACKER_DEBUG("TrackerClientImpl::HandleKeepAliveResponse " << GetServerAddress());
	if (!IsUDP())
		return false;
	// 即使没有登录成功或者已经掉线，还是处理KeepAlive的回应，甚至可以考虑转到online状态
	return DoHandleResponse( is, head, &TrackerClientImpl::OnKeepAliveSucceeded, &TrackerClientImpl::OnKeepAliveFailed );
}

void TrackerClientImpl::OnKeepAliveSucceeded( data_input_stream& is )
{
	TRACKER_DEBUG("OnKeepAliveSucceeded " << m_address);
	m_TimeoutTimer.stop();
	m_Statistics->KeepAlive.SucceededTimes++;
	m_KeepAliveTimeoutTimes = 0;
	PTKeepAliveResponse resp;
	if ( resp.Read( is ) )
	{
		m_listener->SaveDetectedAddress(resp.DetectedIP, resp.DetectedUDPPort, this);
	}
}

void TrackerClientImpl::OnKeepAliveFailed(INT8 errcode)
{
	TRACKER_ERROR("TrackerClientImpl::OnKeepAliveFailed " << static_cast<int>(errcode));
	HandleError( errcode );
	m_Statistics->KeepAlive.FailedTimes++;
	if ( PT_ERROR_NO_CHANNEL == errcode )
	{
		// 通知本次超时处理时跳到下一个tracker进行登录
		//m_KeepAliveTimeoutTimes = m_listener->GetMaxKeepAliveTimeoutTimes();
	}
	// 停止超时定时器，立即登录
//	m_TimeoutTimer.Stop();
//	ResetState();
//	Login();
}

long TrackerClientImpl::GetStatusCode() const
{
	return IsOnline() ? 1 : m_lastError;
}

bool TrackerClientImpl::DoCheckResponse( UINT8 action, UINT32 transactionID, bool isSecure, bool isPeerProxy )
{
	CheckExpiredResponse();

	TransactionInfoCollection::iterator iter = m_Transactions.find( transactionID );
	if ( iter == m_Transactions.end() )
	{
		TRACKER_ERROR( "TrackerClientImpl::CheckResponse no info " << make_tuple( action, transactionID ) << " from " << GetServerAddress() );
		return false;
	}
	TransactionInfo info = iter->second;
	m_Transactions.erase( iter );
	if ( isSecure && ( action != info.Action + 1 ) && ( action != PTS_ACTION_ERROR_RESPONSE ) )
	{
		TRACKER_ERROR( "TrackerClientImpl::CheckResponse invalid action " << make_tuple( action, transactionID, info.Action ) << " from " << GetServerAddress() );
		assert( false );
		return false;
	}
	if ( false == isSecure && ( action != info.Action ) )
	{
		TRACKER_ERROR( "TrackerClientImpl::CheckResponse invalid action " << make_tuple( action, transactionID, info.Action ) << " from " << GetServerAddress() );
		assert( false );
		return false;
	}
	if ( transactionID != info.TransactionID )
	{
		TRACKER_ERROR( "TrackerClientImpl::CheckResponse invalid transaction id " << make_tuple( action, transactionID, info.TransactionID ) << " from " << GetServerAddress() );
		assert( false );
		return false;
	}
	UINT maxTransactionTimeout = 20 * 1000;
	if ( isPeerProxy )
		maxTransactionTimeout *= 2;
	if ( info.RequestTime.elapsed() > maxTransactionTimeout )
	{
		TRACKER_ERROR( "TrackerClientImpl::CheckResponse expired " << make_tuple( action, transactionID, info.RequestTime.elapsed() ) << " from " << GetServerAddress() );
		return false;
	}
	return true;
}

void TrackerClientImpl::CheckExpiredResponse()
{
	assert( m_Transactions.size() < 100 );
	for ( TransactionInfoCollection::iterator iter = m_Transactions.begin(); iter != m_Transactions.end();  )
	{
		const TransactionInfo& info = iter->second;
		if ( info.RequestTime.elapsed() > 30 * 1000 )
		{
			assert( iter->first == iter->second.TransactionID );
			//Tracer::Trace("erase transaction id %lu %s\n", iter->second.TransactionID, strings::format_object(m_address).c_str());
			m_Transactions.erase( iter++ );
		}
		else
		{
			++iter;
		}
	}
}

void TrackerClientImpl::Init( TrackerClientListener& listener, const TRACKER_LOGIN_ADDRESS& addr, boost::shared_ptr<PeerInformation> peerInformation, boost::shared_ptr<TrackerPacketSender> packetSender )
{
	m_listener = &listener;
	m_Statistics = &listener.GetTrackerStatistics();
	m_address = addr;
	m_address.ServerAddress.ReservedStatusCode = 0;
	m_PeerInformation = peerInformation;

	m_PacketSender = packetSender;
	m_NetInfo = m_PeerInformation->NetInfo;
	m_StatusInfo = m_PeerInformation->StatusInfo;
	m_AuthInfo = m_PeerInformation->AuthInfo;
}

void TrackerClientImpl::CheckTrackerSecurity( const OLD_UDP_PACKET_HEAD& head )
{
	if ( tsse_undetermined == m_TrackerSecurity )
	{
		if ( head.ProtocolVersion >= SYNACAST_VERSION_3 )
		{
			m_TrackerSecurity = tsse_secure;
		}
		else
		{
			m_TrackerSecurity = tsse_insecure;
		}
	}
}

bool TrackerClientImpl::OnSecureKeepAliveSucceeded( data_input_stream& is, bool needUpdateKeepAliveInterval )
{
	PTSKeepAliveResponse resp;
	if ( false == resp.Read( is ) )
		return false;
	m_Statistics->KeepAlive.SucceededTimes++;
	m_TimeoutTimer.stop();
	m_KeepAliveTimeoutTimes = 0;
//	UINT16 interval = READ_MEMORY( data, UINT16 );
	//if ( needUpdateKeepAliveInterval )
	//{
	//	UINT32 oldInterval = m_KeepAliveInterval;
	//	SaveKeepAliveInterval( interval );
	//	if ( oldInterval != m_KeepAliveInterval )
	//	{
	//		// 如果定时器间隔变化，重新启动定时器
	//		StartKeepAliveTimer();
	//	}
	//}
	return true;
}

void TrackerClientImpl::HandleSecureErrorResponse( UINT8 errorAction, INT8 errcode )
{
	TRACKER_ERROR("secure error resposne " << make_tuple( (int)errorAction, (int)errcode ) << " " << m_address);
	m_lastError = errcode;
	if ( PTS_ACTION_KEEP_ALIVE_REQUEST == errorAction && -2 == errcode)
	{
		// keep alive 失败，可以立即重新
		m_KeepAliveTimeoutTimes = m_listener->GetMaxKeepAliveTimeoutTimes();
	}
}

bool TrackerClientImpl::CheckResponse( const OLD_UDP_PACKET_HEAD& head )
{
	return this->DoCheckResponse( head.Action, head.TransactionID, false, false );
}

bool TrackerClientImpl::CheckResponse( const NEW_UDP_PACKET_HEAD& head, bool isPeerProxy )
{
	return this->DoCheckResponse( head.Action, head.TransactionID, true, isPeerProxy );
}

void TrackerClientImpl::SaveDHTTransactionID( UINT32 transactionID, UINT8 action )
{
	TransactionInfo info( transactionID, action );
	assert( false == containers::contains( m_Transactions, info.TransactionID ) );
	m_Transactions[transactionID] = info;
}

bool TrackerClientImpl::OnSecureJoinSucceeded( data_input_stream& is )
{
	PTSJoinResponse resp;
	if ( false == resp.Read( is ) )
		return false;

	TRACKER_EVENT("Secure Join Succeeded " << m_address);
	m_Statistics->Join.SucceededTimes++;
	SetTrackerSecurityOK();
	if (m_IsServer)
	{
		m_LoginRequestTimeout = TRACKER_REQUEST_TIMEOUT;
	}
	if (!IsOnline())
	{
		m_TimeoutTimer.stop();
		SaveKeepAliveInterval( resp.KeepAliveInterval );
		ChangeToOnline();
	}
	m_listener->OnTrackerClientLogin(this);
	return true;
}

bool TrackerClientImpl::OnSecureListedPeersSucceeded( data_input_stream& is, const NEW_UDP_PACKET_HEAD& head )
{
	PTSListPeersResponse resp;
	if ( false == resp.Read( is ) )
		return false;

	m_Statistics->List.SucceededTimes++;
	m_Statistics->TotalListedPeers += resp.WANPeers.size();
	m_Statistics->LastListedPeers = static_cast<UINT16>( resp.WANPeers.size() );
	// lan peers
	m_Statistics->TotalListedLANPeers += resp.LANPeers.size();
	m_Statistics->LastListedLANPeers = static_cast<UINT16>( resp.LANPeers.size() );

	TRACKER_EVENT("secure list ok " << make_tuple( resp.WANPeers.size(), resp.LANPeers.size(), resp.SourceTimeStamp ) << " from " << m_address);
	// 取vector的缓冲区地址时，vector不能为空
	m_listener->OnTrackerClientList( 
		this, 
		resp.WANPeers.size(), resp.WANPeers.empty() ? NULL : &resp.WANPeers[0], 
		resp.LANPeers.size(), resp.LANPeers.empty() ? NULL : &resp.LANPeers[0], 
		resp.SourceTimeStamp );
	return true;
}

bool TrackerClientImpl::OnSecureErrorResponse( data_input_stream& is )
{
	PTSErrorResponse resp;
	if ( false == resp.Read( is ) )
		return false;

	HandleSecureErrorResponse(resp.ErrorAction, resp.ErrorIndex);
	return true;
}

bool TrackerClientImpl::HandleSecureResponse( data_input_stream& is, const NEW_UDP_PACKET_HEAD& head, bool isPeerProxy )
{
	if ( false == CheckResponse( head, isPeerProxy ) )
	{
		m_Statistics->TotalInvalidResponse++;
		return false;
	}

	assert ( false == isPeerProxy || head.Action == PTS_ACTION_LIST_PEERS_RESPONSE );

	SECURE_RESPONSE_HEAD respHead;
	is >> respHead;
	if ( !is )
	{
		m_Statistics->TotalInvalid2Response++;
		return true;
	}
	assert( head.ProtocolVersion >= SYNACAST_VERSION_3 );

	return DoHandleSecureResponse( is, head, respHead );
}

void TrackerClientImpl::OnSecureSuccess( const NEW_UDP_PACKET_HEAD& head, const SECURE_RESPONSE_HEAD& respHead )
{
	SetTrackerSecurityOK();
	m_listener->SaveDetectedAddress(respHead.DetectedAddress.IP, respHead.DetectedAddress.UdpPort, this);
	if ( head.Action != PTS_ACTION_ERROR_RESPONSE )
	{
		TRACKER_DEBUG("HandleSecureResponse source minmax " << respHead.SourceMinMax << " from " << GetServerAddress());
		OnSuccess( respHead.SourceMinMax );
	}
}

bool TrackerClientImpl::IsUDP() const
{
	return GetProxyType() == PPL_TRACKER_TYPE_UDP;
}

bool TrackerClientImpl::IsTCP() const
{
	return GetProxyType() == PPL_TRACKER_TYPE_TCP;
}

bool TrackerClientImpl::IsHTTP() const
{
	return GetProxyType() == PPL_TRACKER_TYPE_HTTP;
}

//////////////////////////////////////////////////////////////////////////
// PeerTrackerClient成员函数
//////////////////////////////////////////////////////////////////////////

void PeerTrackerClient::Login()
{
	if (m_TimeoutTimer.is_started())
	{
		TRACKER_ERROR("login: An request has been queued." << m_address);
		return;
	}

	if (IsUDP())
	{
		DoJoin();
	}
	else
	{
		// 如果是TCP Tracker，则做一次list操作
		ListPeers();
		TRACKER_DEBUG("Login TCP Tracker. DoList " << m_address);
	}
	// 开始计算超时，超时后切换到下一个tracker
	StartTimeoutTimer( true );
}

void PeerTrackerClient::Logout()
{
	DoLeave("", "");
}

void PeerTrackerClient::DoJoin()
{
	assert(IsUDP());

	if ( tsse_secure == m_TrackerSecurity || tsse_undetermined == m_TrackerSecurity )
	{
		TRACKER_DEBUG("DoJoin Secure" << m_address);
		//vector<PEER_ADDRESS> addrs(m_NetInfo->IPArray.size(), outerAddress);
		//for ( size_t index = 0; index < addrs.size(); ++index )
		//{
		//	addrs[index].IP = m_NetInfo->IPArray[index];
		//}
		PTSJoinRequest req;
		req.LocalAddresses = m_NetInfo->GetLocalAddresses();
		req.TimeFromStart = m_PeerInformation->GetStartedSeconds();
		req.CookieValue = m_AuthInfo->CookieValue;

		SendRequest(req, true);
		m_Statistics->Join.RequestTimes++;
	}
	if ( tsse_insecure == m_TrackerSecurity || tsse_undetermined == m_TrackerSecurity )
	{
		PORT_PAIR outerPorts = m_NetInfo->GetOuterPorts();
		TRACKER_DEBUG("DoJoin " << m_address << outerPorts);
		PTJoinRequest req;
		req.CoreInfo = m_NetInfo->CoreInfo;
		req.PeerVersion = m_PeerInformation->AppVersion;
		req.TCPPort = outerPorts.TCPPort;
		req.UDPPort = outerPorts.UDPPort;
		req.RealIPs = m_NetInfo->GetIPArray();
		req.CookieValue = m_AuthInfo->CookieValue;

		SendRequest(req, false);
		m_Statistics->Join.RequestTimes++;
	}
}

void PeerTrackerClient::OnListSucceeded( data_input_stream& is )
{
	TRACKER_DEBUG("OnListSucceeded " << m_address);
	PTListPeersResponse resp;
	if (!resp.Read(is))
		return;

	m_Statistics->List.SucceededTimes++;
	m_Statistics->TotalListedPeers += resp.Peers.size();
	m_Statistics->LastListedPeers = resp.Peers.size();
	if ( false == IsUDP() )
	{
		m_Statistics->TCPList.SucceededTimes++;
		m_Statistics->TCPTotalListedPeers += resp.Peers.size();
		m_Statistics->TCPLastListedPeers = resp.Peers.size();
	}
	if (resp.Peers.size() > 0 && resp.Peers[0].Address.IP == 0)
	{
		assert(false);
	}
	//	m_listener->GetIPPool().AddCandidate(resp->Count, resp->Peers);
	if ( resp.Peers.size() > 0 )
	{
		// 取vector的缓冲区地址时，vector不能为空
		m_listener->OnTrackerClientList(this, resp.Peers.size(), &resp.Peers[0], 0, NULL, 0);
	}
	this->ListTimes++;
	this->FailedTimes = 0;
}

void PeerTrackerClient::OnListFailed(INT8 errcode)
{
	TRACKER_ERROR("TrackerClient::OnListFailed " << static_cast<int>(errcode));
	m_Statistics->List.FailedTimes++;
	if ( false == IsUDP() )
	{
		m_Statistics->TCPList.FailedTimes++;
	}
	HandleError(errcode);
}

void PeerTrackerClient::ListPeers()
{
	// 对于非udp的tracker，ListPeers操作需要保持一定的时间间隔
	if (!IsUDP() && m_LastListTime.elapsed() < 30 * 1000)
	{
		TRACKER_ERROR("PeerTrackerClient::ListPeers canceled " << GetServerAddress());
		return;
	}

	if ( ( IsUDP() && ( tsse_secure == m_TrackerSecurity || tsse_undetermined == m_TrackerSecurity ) ) || IsHTTP() )
	{
		// 安全协议
		// 对于udp pos，在tsse_secure和tsse_undetermined情况下发送安全报文
		// 对于http pos，发送安全报文
		PTSListPeersRequest req;
		req.RequestPeerCount = this->ListCount;
		req.RequestPeerTimes = this->ListTimes;
		req.LocalAddresses = m_NetInfo->GetLocalAddresses();
		SendRequest(req, true);
		m_Statistics->List.RequestTimes++;
		if ( false == IsUDP() )
		{
			m_Statistics->TCPList.RequestTimes++;
		}
	}

#if 1
	else if ( ( IsUDP() && ( tsse_insecure == m_TrackerSecurity/* || tsse_undetermined == m_TrackerSecurity */) ) || IsTCP() )
	{
		// 老的非安全协议
		// 对于udp pos，在tsse_insecure和tsse_undetermined情况下发送老报文
		// 对于tcp pos，发送老报文
		PTListPeersRequest req;
		req.StatusInfo = *m_StatusInfo;
		req.RequestTimes = this->ListTimes;
		req.RequestCount = this->ListCount;
		req.PeerVersion = m_PeerInformation->AppVersion;
		SendRequest(req, false);
		m_Statistics->List.RequestTimes++;
		if ( false == IsUDP() )
		{
			m_Statistics->TCPList.RequestTimes++;
		}
	}
#endif

	m_LastListTime.sync();;
	TRACKER_DEBUG("PeerTrackerClient::DoList " << GetServerAddress());
}

bool PeerTrackerClient::HandleJoinResponse(const OLD_UDP_PACKET_HEAD& head, data_input_stream& is)
{
	TRACKER_DEBUG("TrackerClientImpl::HandleJoinResponse " << GetServerAddress());
	if (!IsUDP())
		return false;
	return DoHandleResponse( is, head, &TrackerClientImpl::OnJoinSucceeded, &TrackerClientImpl::OnJoinFailed );
}

bool PeerTrackerClient::HandleListResponse(const OLD_UDP_PACKET_HEAD& head, data_input_stream& is)
{
	TRACKER_DEBUG("TrackerClientImpl::HandleListResponse " << GetServerAddress());
	return DoHandleResponse( is, head, &TrackerClientImpl::OnListSucceeded, &TrackerClientImpl::OnListFailed );
}

void PeerTrackerClient::OnJoinSucceeded( data_input_stream& is )
{
	TRACKER_DEBUG("OnJoinSucceeded" << m_address);
	PTJoinResponse resp;
	if (!resp.Read(is))
		return;
	m_Statistics->Join.SucceededTimes++;
	if (m_IsServer)
	{
		m_LoginRequestTimeout = TRACKER_REQUEST_TIMEOUT;
	}
	if (!IsOnline())
	{
		m_TimeoutTimer.stop();
		SaveKeepAliveInterval(resp.KeepAliveInterval);
		ChangeToOnline();
	}
	m_listener->OnTrackerClientLogin(this);
	m_listener->SaveDetectedAddress(resp.DetectedIP, resp.DetectedUDPPort, this);
}
void PeerTrackerClient::OnJoinFailed(INT8 errcode)
{
	TRACKER_ERROR("TrackerClient::OnJoinFailed " << static_cast<int>(errcode));
	// 等超时后重新连接
	m_Statistics->Join.FailedTimes++;
	HandleError(errcode);
}

bool PeerTrackerClient::HandleResponse( data_input_stream& is, const OLD_UDP_PACKET_HEAD& head )
{
	switch (head.Action)
	{
	case PT_ACTION_JOIN:
		CheckTrackerSecurity( head );
		HandleJoinResponse(head, is);
		break;
	case PT_ACTION_KEEPALIVE:
		CheckTrackerSecurity( head );
		HandleKeepAliveResponse(head, is);
		break;
	case PT_ACTION_LIST:
		CheckTrackerSecurity( head );
		HandleListResponse(head, is);
		break;


	default:
		return false;
	}
	return true;
}

bool PeerTrackerClient::DoHandleSecureResponse( data_input_stream& is, const NEW_UDP_PACKET_HEAD& head, const SECURE_RESPONSE_HEAD& respHead )
{
	bool success = false;
	switch (head.Action)
	{
	case PTS_ACTION_JOIN_RESPONSE:
		success = OnSecureJoinSucceeded( is );
		break;
	case PTS_ACTION_KEEP_ALIVE_RESPONSE:
		success = OnSecureKeepAliveSucceeded( is, false );
		break;
	case PTS_ACTION_LIST_PEERS_RESPONSE:
		success = OnSecureListedPeersSucceeded( is, head );
		break;
	case PTS_ACTION_ERROR_RESPONSE:
		success = OnSecureErrorResponse( is );
		break;
	default:
		return false; // 此报文没有被处理，返回false
	}
	if ( success )
	{
		OnSecureSuccess( head, respHead );
	}
	else
	{
		m_Statistics->TotalInvalid2Response++;
	}
	return true;
}



//////////////////////////////////////////////////////////////////////////
// SourceTrackerClient成员函数
//////////////////////////////////////////////////////////////////////////

SourceTrackerClient::SourceTrackerClient()
{
	m_IsServer = true;
}

void SourceTrackerClient::DoRegister()
{
/*	if (!IsUDP())
		return;
	TRACKER_DEBUG("DoRegister " << m_address);
	PEER_ADDRESS loginAddress = m_address.LoginAddress;
	assert( loginAddress.IsValid() );
	if ( false == loginAddress.IsValid())
	{
		loginAddress = m_NetInfo->Address;
	}
	vector<u_long> ipArray;
	ipArray.resize(1);
	ipArray[0] = loginAddress.IP;
	PTRegisterRequest req(
		m_NetInfo->CoreInfo, 
		m_PeerInformation->AppVersion, 
		loginAddress.TcpPort, 
		loginAddress.UdpPort, 
		ipArray, 
		m_AuthInfo->Username, 
		m_AuthInfo->Password, 
		16, 
		m_AuthInfo->CookieType);
	SendRequest(req, false);
	m_Statistics->Register.RequestTimes++;*/

	assert(IsUDP());

	if (!IsUDP())
		return;

	PEER_ADDRESS loginAddress = m_address.LoginAddress;
	assert( loginAddress.IsValid() );
	//if ( false == loginAddress.IsValid())
	//{
	//	loginAddress = m_NetInfo->Address;
	//}

	if ( tsse_secure == m_TrackerSecurity || tsse_undetermined == m_TrackerSecurity )
	{
		TRACKER_DEBUG("DoJoin Secure" << m_address);
		std::vector<PEER_ADDRESS> addrs;
		addrs.push_back(loginAddress);
		PTSRegisterRequest req; 
		req.LocalAddresses = (loginAddress.IP != 0) ? addrs : m_NetInfo->GetLocalAddresses();
		req.Username = m_AuthInfo->Username;
		req.Password = m_AuthInfo->Password;
		req.CurrentTimeStamp = m_StatusInfo->CurrentTimeStamp;
		req.MinMax = m_StatusInfo->MinMax;
		req.TimeFromStart = m_PeerInformation->GetStartedSeconds();
		SendRequest(req, true);
		m_Statistics->Register.RequestTimes++;
	}
	if ( tsse_insecure == m_TrackerSecurity || tsse_undetermined == m_TrackerSecurity )
	{

		TRACKER_DEBUG("DoRegister " << m_address);
		std::vector<UINT32> ipArray;
		ipArray.resize(1);
		ipArray[0] = loginAddress.IP;
		PTRegisterRequest req;
		req.CoreInfo = m_NetInfo->CoreInfo;
		req.PeerVersion = m_PeerInformation->AppVersion;
		req.TCPPort = loginAddress.TcpPort;
		req.UDPPort = loginAddress.UdpPort;
		req.RealIPs = (loginAddress.IP != 0) ? ipArray : m_NetInfo->GetIPArray();
		req.Username = m_AuthInfo->Username;
		req.Password = m_AuthInfo->Password;
		req.PieceSize = 16;
		req.CookieType = m_AuthInfo->CookieType;
		SendRequest(req, false);
		m_Statistics->Register.RequestTimes++;
	}
}

void SourceTrackerClient::Login()
{
	DoRegister();
	StartTimeoutTimer( true );
}

void SourceTrackerClient::Logout()
{
	DoSourceExit();
}

bool SourceTrackerClient::HandleRegisterResponse(const OLD_UDP_PACKET_HEAD& head, data_input_stream& is)
{
	TRACKER_DEBUG("TrackerClientImpl::HandleRegisterResponse " << GetServerAddress());
	if (!IsUDP())
		return false;
	return DoHandleResponse( is, head, &TrackerClientImpl::OnRegisterSucceeded, &TrackerClientImpl::OnRegisterFailed );
}

void SourceTrackerClient::OnRegisterSucceeded( data_input_stream& is )
{
	TRACKER_DEBUG("OnRegisterSucceeded " << m_address);
	PTRegisterResponse resp;
	if (!resp.Read(is))
		return;
	m_Statistics->Register.SucceededTimes++;
	m_TimeoutTimer.stop();
	m_LoginRequestTimeout = TRACKER_REQUEST_TIMEOUT;
	SaveKeepAliveInterval(resp.KeepAliveInterval);
	ChangeToOnline();
	m_listener->OnTrackerClientLogin(this);
	m_listener->SaveDetectedAddress(resp.DetectedIP, 0, this);
}

void SourceTrackerClient::OnRegisterFailed(INT8 errcode)
{
	TRACKER_ERROR("TrackerClient::OnRegisterFailed " << static_cast<int>(errcode));
	m_Statistics->Register.FailedTimes++;
	// 等超时后重新连接
	HandleError(errcode);
}

bool SourceTrackerClient::HandleResponse( data_input_stream& is, const OLD_UDP_PACKET_HEAD& head )
{
	switch (head.Action)
	{
	case PT_ACTION_REGISTER:
		CheckTrackerSecurity( head );
		HandleRegisterResponse(head, is);
		break;
	case PT_ACTION_KEEPALIVE:
		CheckTrackerSecurity( head );
		HandleKeepAliveResponse(head, is);
		break;

	default:
		return false;
	}
	return true;
}

bool SourceTrackerClient::DoHandleSecureResponse( data_input_stream& is, const NEW_UDP_PACKET_HEAD& head, const SECURE_RESPONSE_HEAD& respHead )
{
	bool success = false;
	switch (head.Action)
	{
	case PTS_ACTION_REGISTER_RESPONSE:
		success = OnSecureRegisterSucceeded( is );
		break;
	case PTS_ACTION_KEEP_ALIVE_RESPONSE:
		success = OnSecureKeepAliveSucceeded( is, false );
		break;
	case PTS_ACTION_ERROR_RESPONSE:
		success = OnSecureErrorResponse( is );
		break;
	default:
		return false; // 此报文没有被处理，返回false
	}
	if ( success )
	{
		OnSecureSuccess( head, respHead );
	}
	else
	{
		m_Statistics->TotalInvalid2Response++;
	}
	return true;
}

void SourceTrackerClient::DoSourceExit()
{
	TRACKER_DEBUG("DoSourceExit " << m_address);
	assert(IsUDP());
	assert(IsOnline());
	if (!IsUDP())
		return;
	assert( tsse_insecure == m_TrackerSecurity || tsse_secure == m_TrackerSecurity );

	if ( tsse_secure == m_TrackerSecurity )
	{
		PTSSourceExitRequest req;
		req.Username = m_AuthInfo->Username;
		req.Password = m_AuthInfo->Password;
		SendRequest(req, true);
		m_Statistics->LeaveTimes++;
	}
	else
	{
		PTLeaveRequest req;
		req.Username = m_AuthInfo->Username;
		req.Password = m_AuthInfo->Password;
		SendRequest(req, false);
		m_Statistics->LeaveTimes++;
	}
}

bool SourceTrackerClient::OnSecureRegisterSucceeded( data_input_stream& is )
{
	PTSRegisterResponse resp;
	if ( false == resp.Read( is ) )
		return false;

	TRACKER_EVENT("Secure Register Succeeded " << m_address);
	m_Statistics->Register.SucceededTimes++;
	SetTrackerSecurityOK();
	m_LoginRequestTimeout = TRACKER_REQUEST_TIMEOUT;
	if (!IsOnline())
	{
		m_TimeoutTimer.stop();
		SaveKeepAliveInterval( resp.KeepAliveInterval );
		ChangeToOnline();
	}
	m_listener->OnTrackerClientLogin(this);
	return true;
}


SimpleMDSTrackerClient::SimpleMDSTrackerClient()
{
	m_IsServer = true;
	SetTrackerSecurityOK();
}

void SimpleMDSTrackerClient::Login()
{
	if ( IsUDP() )
	{
		PeerTrackerClient::Login();
	}
}


