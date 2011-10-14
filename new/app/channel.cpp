
#include "StdAfx.h"

#include "app/channel.h"

#include "common/VersionNumber.h"
#include "common/UDPSender.h"
#include "common/UDPSenderFactory.h"
#include "common/Peer.h"

#include <synacast/live/url.h>
#include <synacast/live/startupinfo.h>

#include <ppl/net/asio/udp_socket.h>
#include <ppl/net/asio/message_tcp_acceptor.h>

#include <ppl/os/process.h>
#include <ppl/data/guid.h>
#include <ppl/net/asio/io_service_runner.h>

#include <ppl/net/ip.h>
#include <ppl/os/file_system.h>

#include <ppl/util/ini_file.h>

#include "framework/log.h"

const size_t PPL_MAX_UDP_RECEIVE_SIZE = 5 * 1024;

void ParseLiveTrackers(const vector<TrackerAddressInfo>& src, vector<TRACKERADDR>& dst)
{
	dst.clear();
	STL_FOR_EACH_CONST(vector<TrackerAddressInfo>, src, iter)
	{
		const TrackerAddressInfo& srcAddr = *iter;
		TRACKERADDR addr = { 0 };
		addr.IP = ResolveHostName(srcAddr.Host.c_str());
		addr.Port = _ttoi(srcAddr.Port.c_str());
		addr.Type = 0xFF;
		if (srcAddr.Type == TEXT("udpt"))
			addr.Type = PPL_TRACKER_TYPE_UDP;
		else if (srcAddr.Type == TEXT("tcpt"))
			addr.Type = PPL_TRACKER_TYPE_TCP;
		else if (srcAddr.Type == TEXT("httpt"))
			addr.Type = PPL_TRACKER_TYPE_HTTP;
		if (addr.IP == INADDR_NONE || addr.IP == INADDR_ANY || addr.Port == 0 || addr.Type == 0xFF)
			continue;
		dst.push_back(addr);
	}
}

void ParseLivePeers(const vector<PeerAddressInfo>& src, vector<PEERADDR>& dst)
{
	dst.clear();
	STL_FOR_EACH_CONST(vector<PeerAddressInfo>, src, iter)
	{
		const PeerAddressInfo& srcAddr = *iter;
		PEERADDR addr = { 0 };
		addr.IP = ResolveHostName(srcAddr.Host.c_str());
		addr.TCPPort = _ttoi(srcAddr.TCPPort.c_str());
		addr.UDPPort = _ttoi(srcAddr.UDPPort.c_str());
		if (addr.IP == INADDR_NONE || addr.IP == INADDR_ANY || addr.TCPPort == 0 || addr.UDPPort == 0)
			continue;
		dst.push_back(addr);
	}
}

void ParseLivePeers(const vector<PeerAddressInfo>& src, PeerAddressArray& dst)
{
	dst.clear();
	STL_FOR_EACH_CONST(vector<PeerAddressInfo>, src, iter)
	{
		const PeerAddressInfo& srcAddr = *iter;
		PEER_ADDRESS addr ;
		addr.IP = ResolveHostName(srcAddr.Host.c_str());
		addr.TcpPort = _ttoi(srcAddr.TCPPort.c_str());
		addr.UdpPort = _ttoi(srcAddr.UDPPort.c_str());
		if (addr.IP == INADDR_NONE || addr.IP == INADDR_ANY || addr.TcpPort == 0 || addr.UdpPort == 0)
			continue;
		dst.push_back(addr);
	}
}

LiveChannel::LiveChannel(int node_type) 
: m_tcp(new message_tcp_acceptor), m_udp(new udp_socket), m_callback(NULL), m_channel_handle(0), m_playing(false), m_started(false)
, m_node_type(node_type)
{
	m_timer.set_callback(boost::bind(&LiveChannel::OnAppTimer, this));
	m_udp->set_listener(this);
	m_tcp->set_listener(this);
}

LiveChannel::~LiveChannel()
{
	this->Stop();
}

bool LiveChannel::Init(const tstring& urlstr, int tcpPort, int udpPort, const tstring& configDir, const string& baseDir)
{
	m_playing = false;
	m_urlstr = urlstr;
	//LIVE_ASSERT( ppl::os::file_system::directory_exists(configDir.c_str()) );
	LIVE_ASSERT( ppl::os::file_system::directory_exists(baseDir.c_str()) );

	LiveAppModuleCreateParam& param = m_param;

	synacast_url url;
	if (false == url.parse(urlstr))
	{
		TRACEOUT("pplive:invalid synacast url: ");
		TRACEOUT(urlstr.c_str());
		TRACEOUT("\n");
		LIVE_ASSERT(false);
		return false;
	}
	LiveChannelStartupInfo startupInfo;
	if ( false == startupInfo.ChannelGUID.Parse(url.channel_guid()) )
	{
		TRACEOUT("pplive:invalid channel guid %s \n", url.channel_guid().c_str());
		return false;
	}
	startupInfo.AppType = url.app_type();
	startupInfo.TestVersion = url.test_version_number();

	ParseLiveTrackers(url.trackers(), startupInfo.Trackers);
	ParseLivePeers(url.mdss(), startupInfo.MDSs);

	if ( startupInfo.Trackers.empty() )
	{
		TRACEOUT("pplive:invalid urlstr: no trackers ");
		TRACEOUT(urlstr.c_str());
		TRACEOUT("\n");
		LIVE_ASSERT(false);
		return false;
	}

	RandomGenerator rnd;

	{
		int count = 0;
		for (;;)
		{
			if ( count >= 1000 )
			{
				LIVE_ASSERT( false );
				if ( false == m_udp->open(0) )
				{
					TRACEOUT("PPlive: Open udp socket Failed! ");
					LIVE_ASSERT( false );
					return false;
				}
				break;
			}
			if ( m_udp->open(udpPort) )
				break;
			// Added by Tady, 022411
			if (m_node_type == 2) // Spark-SN 
				return false;
			++udpPort;
			++count;
		}
		udpPort = m_udp->local_address().GetPort();
	}

	{
		int count = 0;
		for (;;)
		{
			if ( count >= 1000 )
			{
				LIVE_ASSERT( false );
				if ( false == m_tcp->open(0) )
				{
					TRACEOUT("PPlive: Open tcp socket Failed! ");
					LIVE_ASSERT( false );
					return false;
				}
				break;
			}
			if ( m_tcp->open(tcpPort) )
				break;
			// Added by Tady, 022411
			if (m_node_type == 2) // Spark-SN
				return false;
			++tcpPort;
			++count;
		}
		tcpPort = m_tcp->local_address().GetPort();
	}

 
	tstring fmname = strings::format(_T("%dOS_SYS_INFO%s"), ppl::os::process::current_process_id(), startupInfo.ChannelGUID.ToString().c_str());
	if (false == m_mapping.create(fmname.c_str(), sizeof(SYSINFO)))
	{
		APP_ERROR("failed to create mmf for sys_info " << fmname);
		TRACEOUT("failed to create mmf for sys_info %s", fmname.c_str());
		return false;
	}
	if (false == m_mapping.map_all())
	{
		APP_ERROR("failed to map mmf for sys_info " << fmname);
		TRACEOUT("failed to map mmf for sys_info %s", fmname.c_str());
		return false;
	}
	SYSINFO* lpSysInfo = static_cast<SYSINFO*>( m_mapping.get_view() );
	FILL_ZERO(*lpSysInfo);
	Guid::GenerateGUID(lpSysInfo->PeerGUID);
	lpSysInfo->LocalIPCount = IPArrayLoader::LoadLocalIPs(lpSysInfo->LocalIPs, 30);
	APP_ERROR("LocalIPArray " << lpSysInfo->LocalIPs[0]);
	lpSysInfo->MaxAppPeerCount = 36;
	lpSysInfo->MaxConnectPendingCount = 10;
	lpSysInfo->TcpPort = tcpPort;
	lpSysInfo->UdpPort = udpPort;
	lpSysInfo->AppCount = 1;
	lpSysInfo->AppItems[0].AppType = PPL_P2P_LIVE2;
	lpSysInfo->AppItems[0].ResourceGuid = startupInfo.ChannelGUID;

	param.AppType = startupInfo.AppType;

	param.AppVersion = P2P_MODULE_VERSION_NUMBER;
	param.AppVersionNumber16 = P2P_MODULE_VERSION_NUMBER_UINT16;
	param.ChannelGUID = startupInfo.ChannelGUID;
	param.SysInfo = lpSysInfo;

	param.ConfigDirectory = configDir;
	param.BaseDirectory = baseDir;

	param.Trackers.clear();
	AppendTrackerAddresses(param.Trackers, &startupInfo.Trackers[0], startupInfo.Trackers.size(), 16);

	param.MDSs.resize(startupInfo.MDSs.size());
	for (UINT i = 0; i < startupInfo.MDSs.size(); ++i)
	{
		param.MDSs[i].IP = startupInfo.MDSs[i].IP;
		param.MDSs[i].TcpPort = startupInfo.MDSs[i].TCPPort;
		param.MDSs[i].UdpPort = startupInfo.MDSs[i].UDPPort;
	}

	APP_EVENT("LiveAppModuleWrapper mds:vip " << make_tuple(param.MDSs.size(), param.VIPs.size()));

	param.PeerType = NORMAL_PEER;

	// 禁用智能连接控制(主要针对mid等情况)
	//param.BootConfig.IntelligentConnectionControlDisabled = true;

	// Added by Tady, 011811: Spark.
	ParseLivePeers(url.sparks(), param.Sparks);
	param.SparkTranstype = url.spark_trans_type();
	param.SparkLen = url.spark_len();

	return true;
}


void LiveChannel::Start()
{
	LIVE_ASSERT(false == m_started);
	if (false == m_started)
	{
		m_started = true;
//		io_service_provider::default_service().post(boost::bind(&LiveChannel::DoStart, this->shared_from_this()));
		DoStart();
	}
}

void LiveChannel::Stop()
{
	if (m_started)
	{
		m_started = false;
		boost::shared_ptr<LiveChannel> channel;
		try
		{
			channel = this->shared_from_this();
		}
		catch (std::exception&)
		{
			channel.reset();
		}
		if (channel)
		{
			io_service_provider::default_service().post(boost::bind(&LiveChannel::DoStop, channel));
		}
	}
}

void LiveChannel::DoStart()
{
	m_normalUDPSender.reset( UDPSenderFactory::CreateNormal(m_udp));
	UDPSenderCallbackType callback(boost::bind(&LiveChannel::OnUDPSenderResponse, this, _1, _2, _3, _4));
	m_tcpProxyUDPSender.reset( UDPSenderFactory::CreateTCPProxy(30 * 1000, callback) );
	m_httpProxyUDPSender.reset( UDPSenderFactory::CreateHTTPProxy(30 * 1000, callback) );

	m_param.NormalUDPSender = m_normalUDPSender;
	m_param.TCPProxyUDPSender = m_tcpProxyUDPSender;
	m_param.HTTPProxyUDPSender = m_httpProxyUDPSender;

	// Added by Tady, 022411
	switch (m_node_type)
	{
	case 1:
		m_AppModule.reset(new SuperNodeModule(m_param));
		break;
	case 2:
		m_AppModule.reset(new SparkSNModule(m_param));
		break;
	case 0:
		m_AppModule.reset(new PeerModule(m_param));
	default:
		break;
	}
	m_AppModule->SetClientErrorCallback(boost::bind(&LiveChannel::HanlePlayerClientError, this, _1));
	m_AppModule->Start(m_param);

 	for ( int receiveCount = 0; receiveCount < 100; ++receiveCount )
	{
		m_udp->receive(PPL_MAX_UDP_RECEIVE_SIZE);
	}

	m_timer.start(1000);
}

void LiveChannel::DoStop()
{
	m_AppModule.reset();
	m_udp->close();
	m_tcp->close();
	m_playing = false;
}

void LiveChannel::OnUDPSenderResponse( BYTE* data, size_t size, const InetSocketAddress& sockAddr, UINT proxyType )
{
	if (m_AppModule)
	{
		m_AppModule->DispatchUdpPacket(data, size, sockAddr, proxyType, EXTRA_PROXY_NONE);
	}
}

void LiveChannel::on_socket_receive(udp_socket* sender, const InetSocketAddress& sockAddr, BYTE* data, size_t size)
{
	if (m_AppModule)
	{
		this->ReceiveUDP();
		m_AppModule->DispatchUdpPacket(data, size, sockAddr, PROXY_UDP, EXTRA_PROXY_NONE);
	}
}
void LiveChannel::on_socket_receive_failed(udp_socket* sender, int errcode)
{
	this->ReceiveUDP();
}
void LiveChannel::on_socket_accept(tcp_acceptor* sender, tcp_socket* client, const InetSocketAddress& addr)
{
	if (m_AppModule)
	{
		m_AppModule->OnSocketAccept(tcp_socket_ptr(client), addr);
	}
}
void LiveChannel::on_socket_accept_failed(tcp_acceptor* sender, int errcode)
{
	VIEW_ERROR("Controller::OnSocketAcceptFailed " << errcode);
}

void LiveChannel::OnAppTimer()
{
//  if ( m_timer.GetTimes() % 1 == 0 )
//  {
	if (false == m_playing && m_AppModule)
	{
		const CLiveInfo& liveInfo = m_AppModule->GetInfo();
		UINT32 bufferTime = liveInfo.LocalPeerInfo.Statistics.BufferTime;
		//UINT32 downloadSpeed = liveInfo.LocalPeerInfo.Flow.Download.Speed;
		//UINT32 peerCount = liveInfo.RemotePeerCount;
		//int skipPercent = liveInfo.LocalPeerInfo.StatusInfo.Status.SkipPercent;
		// 缓冲时间大于5秒开始播放
		if (bufferTime > 0 && liveInfo.MediaType != 0)
		{
			if (m_callback != NULL)
			{
				m_callback(m_channel_handle, UM_LIVEMSG_PLAY, 0, 0);
				m_playing = true;
			}
		}
	}
      //printf("status: buffer=%u speed=%u peers=%u bufferpercent=%d\n", bufferTime, downloadSpeed, peerCount, (100 - skipPercent));
//  }
}

void LiveChannel::ReceiveUDP()
{
	while ( false == m_udp->receive(PPL_MAX_UDP_RECEIVE_SIZE) ) { }
}

std::ostream& operator<<(std::ostream& os, const TRACKERADDR& trackerAddr);

void LiveChannel::AppendTrackerAddresses(vector<TRACKER_LOGIN_ADDRESS>& trackers, const TRACKERADDR* srcAddresses, UINT count, UINT maxCount)
{
	LIVE_ASSERT(count <= maxCount);
	LIMIT_MAX(count, maxCount);
	for (UINT i = 0; i < count; ++i)
	{
		const TRACKERADDR& srcAddr = srcAddresses[i];
		APP_EVENT("AppendTrackerAddress " << i << " " << srcAddr);
		if (srcAddr.IP != INADDR_NONE && srcAddr.IP != INADDR_ANY && srcAddr.Port != 0)
		{
			TRACKER_LOGIN_ADDRESS addr;
			addr.ServerAddress.IP = srcAddr.IP;
			addr.ServerAddress.Port = srcAddr.Port;
			addr.ServerAddress.Type = srcAddr.Type;
			addr.ServerAddress.ReservedStatusCode = 0;
			addr.LoginAddress.IP = srcAddr.PeerIP;
			addr.LoginAddress.TcpPort = srcAddr.PeerTcpPort;
			addr.LoginAddress.UdpPort = srcAddr.PeerUdpPort;
			trackers.push_back(addr);
		}
		else
		{
			APP_ERROR("AppendTrackerAddress invalid address " << srcAddr);
			LIVE_ASSERT(!"invalid tracker address or port");
		}
	}
}

CLiveInfo* LiveChannel::GetInfo()
{
    if (!m_AppModule)
        return NULL;
    return &(m_AppModule->GetInfo());
}

SYSINFO* LiveChannel::GetSysInfo()
{
	return static_cast<SYSINFO*>(m_mapping.get_view());
}

void LiveChannel::SetPlayerStatus( int pstatus )
{
	if (m_AppModule)
	{
		//m_AppModule->
	}
}

void LiveChannel::SetPlayerCallback( FUNC_CallBack callback, unsigned int channelHandle )
{
//	LIVE_ASSERT(callback != NULL);
//	LIVE_ASSERT(channelHandle != 0);
	m_callback = callback;
	m_channel_handle = channelHandle;
}

void LiveChannel::HanlePlayerClientError( int errcode )
{
	if (m_callback != NULL)
	{
		m_callback(m_channel_handle, UM_LIVEMSG_PLAYEROFF, errcode, 0);
	}
}
