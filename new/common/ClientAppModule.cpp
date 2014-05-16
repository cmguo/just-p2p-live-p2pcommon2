
#include "StdAfx.h"

#include "ClientAppModule.h"

#include "pos/TrackerRequester.h"
#include "pos/TrackerClient.h"
#include "UdpDetect.h"
#include "MediaServer.h"
#include "media/MediaServerListener.h"

#include "PeerManager.h"
#include "PeerManagerStatistics.h"

#include "StreamBuffer.h"
#include "StreamBufferStatistics.h"

#include "IpPool.h"
#include "IPPoolStatistics.h"

#include "util/flow.h"
#include "UDPSender.h"
#include "GloalTypes.h"
#include "PacketSender.h"

#include "util/StunModule.h"
#include "common/BootModule.h"
#include "common/VODProxy.h"
#include "common/BaseInfo.h"
#include "util/StunClient.h"
#include "common/LogClient.h"
#include "util/testlog.h"

#include <synacast/protocol/PacketBuilder.h>

#include <synacast/protocol/TrackerPacket.h>
#include <synacast/protocol/data/PeerAddressUtil.h>
#include <synacast/protocol/LogServer.h>

#include <ppl/util/ini_file.h>
#include <ppl/net/ip.h>
#include <ppl/data/numeric.h>
#include <ppl/data/stlutils.h>
#include <ppl/os/paths.h>

#include <boost/bind.hpp>





/// tcp发包的超时时间

BOOST_STATIC_ASSERT(1000 % APP_TIMER_TIMES_PER_SECOND == 0);
BOOST_STATIC_ASSERT(500 % APP_TIMER_TIMES_PER_SECOND == 0);


//#define _PPL_TEMP_DISABLE_TRACKER 1

#if defined(_PPL_TEMP_DISABLE_TRACKER)// && !_PPL_TEMP_DISABLE_TRACKER
#pragma message("****** temp disable tracker")
#endif



const TCHAR nat_ini_section[] = _T("nat");
const TCHAR nat_ini_key_type[] = _T("type");
const TCHAR nat_ini_key_time[] = _T("time");
const TCHAR nat_ini_key_ip[] = _T("ip");


class LiveStatisticsInfo;
CLiveInfo* CreateLiveInfo(LiveStatisticsInfo& statsInfo, ppl::os::memory_mapping& mmf, const TCHAR* tagname, UINT maxPeerCount, const LiveAppModuleCreateParam& param, UINT appVersion, bool tagMutate);



LiveAppModuleImpl::LiveAppModuleImpl(const LiveAppModuleCreateParam& param, const TCHAR* tagname, bool tagMutate) : 
                m_CreateParam( param ),
                m_NormalUDPSender(param.NormalUDPSender),		
                m_TCPProxyUDPSender(param.TCPProxyUDPSender), 
                m_HTTPProxyUDPSender(param.HTTPProxyUDPSender),
                m_AutoMaxAppPeerCount(20),
                m_AutoMinAppPeerCount(0), 
		m_BootModule( new BootModule(param.NormalUDPSender) )
{
	TEST_LOG_OPEN( paths::combine( param.ConfigDirectory, _T("live_test.log") ).c_str() );

	m_SysInfo = static_cast<const CSysInfo*>(param.SysInfo);
	m_PeerInformation.reset(new PeerInformation(param.ChannelGUID, param.SysInfo->PeerGUID, param.AppVersion, param.AppVersionNumber16, CreatePeerNetInfo(*m_SysInfo)));
	m_PeerInformation->NetInfo->CoreInfo.PeerType = param.PeerType;

	LIVE_ASSERT(m_SysInfo != NULL);
	m_PeerCountLimit = m_SysInfo->MaxAppPeerCount;
	LIMIT_MIN(m_PeerCountLimit, PPL_MIN_PEER_COUNT_LIMIT);
	// 传入的最大peer个数应该不超过一个最大限制PPL_MAX_PEER_COUNT_LIMIT
	LIVE_ASSERT(m_PeerCountLimit <= PPL_MAX_PEER_COUNT_LIMIT);
	LIMIT_MAX(m_PeerCountLimit, PPL_MAX_PEER_COUNT_LIMIT);


	m_iptable.Load(m_CreateParam.BaseDirectory);

	boost::shared_ptr<IDGenerator> trackerTransactionID(new IDGenerator());
	boost::shared_ptr<IDGenerator> peerTransactionID(new IDGenerator());
	m_UDPConnectionlessPacketBuilder.reset(new UDPConnectionlessPacketBuilder(peerTransactionID));
	m_TCPConnectionlessPacketBuilder.reset(new TCPConnectionlessPacketBuilder());

	m_OldTrackerPacketBuilder.reset(new UDPPacketBuilder(trackerTransactionID)); // 用于tracker报文
	m_SecureUDPPacketBuilder.reset( new SecureTrackerRequestPacketBuilder( trackerTransactionID ) );

	boost::shared_ptr<UDPConnectionPacketBuilder> udpConnectionPacketBuilder(new UDPConnectionPacketBuilder(peerTransactionID));
	boost::shared_ptr<TCPConnectionPacketBuilder> tcpPacketBuilder(new TCPConnectionPacketBuilder());
	m_SecureTrackerProxyPacketBuilder.reset( new SecureTrackerProxyPacketBuilder( trackerTransactionID ) );
	m_SecureTrackerProxyPacketBuilder->Init( m_PeerInformation->ChannelGUID, m_PeerInformation->PeerGUID );

	m_OldTrackerPacketBuilder->InitHeadInfo( m_PeerInformation->ChannelGUID, m_PeerInformation->PeerGUID );
	m_SecureUDPPacketBuilder->Init( m_PeerInformation->ChannelGUID, m_PeerInformation->PeerGUID, m_PeerInformation->NetInfo->CoreInfo, m_PeerInformation->AppVersion );
	m_UDPConnectionlessPacketBuilder->PeerInfo.Init( m_PeerInformation->ChannelGUID, m_PeerInformation->PeerGUID, 
		*m_PeerInformation->NetInfo, m_PeerInformation->AppVersion);
	m_TCPConnectionlessPacketBuilder->PeerInfo.Init( m_PeerInformation->ChannelGUID, m_PeerInformation->PeerGUID, 
		*m_PeerInformation->NetInfo, m_PeerInformation->AppVersion);

	m_UDPPacketSender.reset(new TrackerPacketSender(m_OldTrackerPacketBuilder, m_SecureUDPPacketBuilder, m_NormalUDPSender, m_TCPProxyUDPSender, m_HTTPProxyUDPSender, 
		m_PeerInformation, m_Statistics.OldUDPFlow.Upload, m_Statistics.TotalFlow.Upload));

	m_UDPConnectionlessPacketSender.reset( new UDPConnectionlessPacketSender( m_UDPConnectionlessPacketBuilder, m_NormalUDPSender, 
		m_PeerInformation, m_Statistics.UDPConnectionlessFlow.Upload, m_Statistics.TotalFlow.Upload ) );
	m_UDPConnectionPacketSender.reset( new UDPConnectionPacketSender( udpConnectionPacketBuilder, m_NormalUDPSender, m_Statistics.UDPConnectionFlow.Upload, m_Statistics.TotalFlow.Upload ) );

	m_TCPConnectionlessPacketSender.reset( new TCPConnectionlessPacketSender( m_TCPConnectionlessPacketBuilder, m_PeerInformation, m_Statistics.TCPConnectionlessFlow.Upload, m_Statistics.TotalFlow.Upload ) );
	m_TCPConnectionPacketSender.reset( new TCPConnectionPacketSender( tcpPacketBuilder, m_Statistics.TCPConnectionFlow.Upload, m_Statistics.TotalFlow.Upload ) );

	VIEW_ERROR("Create New AppModule! " << this << " ResourceGUID=" << param.ChannelGUID);
	m_LiveInfo = CreateLiveInfo(m_StatisticsInfo, m_MappingLiveInfo, tagname, m_PeerCountLimit, param, m_PeerInformation->AppVersion, tagMutate);

	LoadTracker(param.Trackers);

	InitPeerAuthInfo(*m_PeerInformation->AuthInfo, param);

	{
//#if defined(_PPL_PLATFORM_LINUX) || defined(_PPL_USE_ASIO)
//		ini_file ini;
//		this->InitIni(ini);
//		ini.set_section(_T("media"));
//		int netWriterMode = ini.get_int(_T("NetWriterMode"), 0);
//		m_mediaServerListener.reset(CreateMediaServerListener(netWriterMode));
//#elif defined(_PPL_PLATFORM_MSWIN)
		m_mediaServerListener.reset(CreateMediaServerListener(*m_LiveInfo));
//#endif
	}

/* ----------Removed by Tady, 04142010: Moved into Func. Start().
	if ( NORMAL_PEER == param.PeerType )
	{
		// 检查stun type
		MY_STUN_NAT_TYPE natType = this->LoadNATType();
		if ( STUN_TYPE_INVALID == natType )
		{
			// 读取nat类型失败，另起线程检测
			m_StunTypeResolver.reset( new StunTypeResolver );
			m_StunTypeResolver->Resolve(StunTypeResolver::CallbackType(boost::bind(&LiveAppModuleImpl::OnNATQuerySucceeded, this, _1)));
		}
		else
		{
			// 保存nat类型到NetInfo对象
			m_PeerInformation->NetInfo->SetNATType( natType );
			m_LiveInfo->LocalPeerInfo.NetInfo.CoreInfo.NATType = natType;
		}
	}
*/
	{
		std::vector<InetSocketAddress> stunServers;
		ini_file ini;
		this->InitConfig(ini);
		ini.set_section(_T("stun"));
		tstring serverListStr = ini.get_string(_T("servers"), _T(""));
		int day = ini.get_int(_T("time"), -1);
		SYSTEMTIME st;
		GetLocalTime(&st);
		if ( false == serverListStr.empty() && ( day > 0 ) && (abs(st.wDay - day) < 3) )
		{
			std::vector<tstring> serverList;
			strings::split(std::back_inserter(serverList), serverListStr, _T('|'));
			for ( size_t serverIndex = 0; serverIndex < serverList.size(); ++serverIndex )
			{
				pair<tstring, tstring> ipAndPort = strings::split_pair(serverList[serverIndex], _T(':'));
				if ( false == ipAndPort.first.empty() && false == ipAndPort.second.empty() )
				{
					UINT ip = ResolveHostName(ipAndPort.first);
					WORD port = static_cast<WORD>( _ttoi(ipAndPort.second.c_str()) );
					if ( ip != 0 && ip != INADDR_NONE && port != 0)
					{
						stunServers.push_back(InetSocketAddress(ip, port));
					}
				}
			}
		}
		if ( false == stunServers.empty() )
		{
			this->StartStunModule(stunServers);
		}
		else
		{
			// 向boot server查询
			m_BootModule->QueryNTSList(BootModule::NTSListCallbackType(boost::bind(&LiveAppModuleImpl::OnNTSListOK, this, _1)));
		}
	}

	m_LogClient.reset( new LogClient( m_CreateParam.BaseDirectory, m_CreateParam.ConfigDirectory, m_PeerInformation ) );

	m_timer.set_callback(boost::bind(&LiveAppModuleImpl::OnAppTimer, this));


//#ifdef _DEBUG
#ifdef _PPL_PLATFORM_MSWIN
#pragma message("!!!!!!从params.ini中加载TransferMethod设置")
#endif
	{
		ini_file ini;
		this->InitIni(ini);
		ini.set_section(_T("connector"));
		int method = ini.get_int(_T("TransferMethod"), 0);
		VIEW_DEBUG("Load TransferMethod " << method);
		if (method != TRANSFER_ALL && method != TRANSFER_UDP && method != TRANSFER_TCP && method != TRANSFER_NO_DETECT)
		{
			method = TRANSFER_ALL;
		}
		m_LiveInfo->TransferMethod = (BYTE)method;
	}
//#endif


	TEST_LOG_OUT("live start");
	TEST_LOG_OUT("channel id " << param.ChannelGUID);
	TEST_LOG_OUT("peer id " << param.SysInfo->PeerGUID);
	TEST_LOG_OUT("peer address " << m_PeerInformation->NetInfo->Address);
	TEST_LOG_FLUSH();
}

LiveAppModuleImpl::~LiveAppModuleImpl()
{
	APP_EVENT("destruct LiveAppModuleImpl begin " << this);

	m_LogClient->DoSaveLog(m_streamBuffer->GetStatistics(), *m_Manager, *m_ipPool);
	m_LogClient->SendLog();

	m_timer.stop();

	if (m_StunTypeResolver)
	{
		m_StunTypeResolver->Cancel();
		m_StunTypeResolver.reset();
	}

	// 注意创建/启动/删除的顺序，PeerManager依赖于StreamBuffer
	m_mediaServer.reset();
	APP_EVENT("LiveAppModuleImpl::~LiveAppModuleImpl delete mediaserver");
	m_tracker.reset();
	APP_EVENT("LiveAppModuleImpl::~LiveAppModuleImpl delete tracker");
	m_streamBuffer.reset();
	APP_EVENT("LiveAppModuleImpl::~LiveAppModuleImpl delete streambuffer");
	m_Manager.reset();
	APP_EVENT("LiveAppModuleImpl::~LiveAppModuleImpl delete peermanager");


	APP_EVENT("destruct LiveAppModuleImpl end " << this);
	TRACE(TEXT("destruct LiveAppModuleImpl end %p\n"), this);


	TEST_LOG_OUT_ONCE("live end.");
}


void LiveAppModuleImpl::OnSocketAccept(tcp_socket_ptr newClient, const InetSocketAddress& addr)
{
	m_Manager->OnSocketAccept(newClient, addr);
}


void LiveAppModuleImpl::InitIni(ini_file& ini)
{
	this->InitIniFile(ini, _T("params.ini"));
}

void LiveAppModuleImpl::InitIniFile(ini_file& ini, const TCHAR* filename)
{
	tstring path = ppl::os::paths::combine( m_CreateParam.BaseDirectory, filename );
	ini.set_filename(path);
}

void LiveAppModuleImpl::InitConfig( ini_file& ini )
{
	this->InitConfigFile( ini, _T("live.cfg") );
}

void LiveAppModuleImpl::InitConfigFile( ini_file& ini, const TCHAR* filename )
{
	tstring path = ppl::os::paths::combine( m_CreateParam.ConfigDirectory, filename );
	ini.set_filename(path);
}

bool LiveAppModuleImpl::DispatchUdpPacket(BYTE* data, int size, const InetSocketAddress& remoteSockAddr, UINT proxyType, ExtraProxyEnum extraProxy)
{
	m_Statistics.TotalFlow.Download.Record( size );
	const size_t totalDataSize = size;
	APP_EVENT( "DispatchUdpPacket " << make_buffer_pair(data, size) << " from " << remoteSockAddr );//<< " " << HexEncoding::Encode(string((const char*)data, size)) );
	//BYTE* rawData = data;
	//size_t rawSize = size;
	this->RecordDownload(size);
	SimpleSocketAddress sockAddr(remoteSockAddr);

	if ((size_t) size >= sizeof( OLD_UDP_PACKET_HEAD ) + sizeof( GUID ) )
	{
#ifndef _PPL_TEMP_DISABLE_TRACKER
		PacketInputStream is( data, size );
		// 可能是老报文，尝试由tracker模块处理
		if (m_tracker->TryHandleResponse( is, remoteSockAddr, (BYTE)proxyType ))
		{
			m_trackerFlow.Download.Record(size);
			return true;
		}
#endif
	}
	// 解混淆
	int len = PacketObfuscator::UnObfuscate( reinterpret_cast<BYTE*>( data ), size );
	if ( len <= 0 )
	{
		// 解混淆失败，可能是vod的报文
		if ( VODProxy::HandlePacket( data, size, remoteSockAddr, m_BootModule.get(), m_StunModule.get() ) )
		{
			return true;
		}
		m_Statistics.TotalInvalidPackets++;
		if ( size >= 3 )
		{
			VIEW_ERROR( "LiveAppModuleImpl::DispatchUdpPacket unshuffle failed 1 " << make_tuple( size, len ) << sockAddr << " action=" << strings::format( "0x%02X%02X 0x%02X", data[0], data[1], data[2] ) );
		}
		else
		{
			VIEW_ERROR( "LiveAppModuleImpl::DispatchUdpPacket unshuffle failed 1 " << make_tuple( size, len ) << sockAddr );
		}
		//LIVE_ASSERT( false );
		return false;
	}
	if ( size < len )
	{
		m_Statistics.TotalInvalidPackets++;
		VIEW_ERROR( "LiveAppModuleImpl::DispatchUdpPacket unshuffle failed 2 " << make_tuple( size, len ) << sockAddr );
		return false;
	}
	// 截去混淆部分的头，取出真正的报文部分
	data += len;
	size -= len;

	PacketInputStream is( data, size );
	NEW_UDP_PACKET_HEAD head;
	is >> head;
	if ( !is )
	{
		m_Statistics.TotalInvalidPackets++;
		VIEW_ERROR( "LiveAppModuleImpl::DispatchUdpPacket invalid 2 " << size << " " << sockAddr );
		return false;
	}
	if ( head.ProtocolVersion < SYNACAST_VERSION_3 )
	{
		m_Statistics.TotalInvalidPackets++;
		// 检查版本号与MAGIC
		APP_ERROR("Invalid protocol version " << head.ProtocolVersion << " " << sockAddr);
		return false;
	}
	UDPT_INFO("LiveAppModuleImpl::DispatchUdp "<<sockAddr << " " << (int)head.Action);

	// session 处理完
	if ( head.Action & PPL_P2P_CONNECTION_ACTION_FLAG )
	{
		UDP_SESSION_INFO sessionInfo;
		is >> sessionInfo;
		if ( !is )
		{
			m_Statistics.TotalInvalidPackets++;
			APP_ERROR("Invalid session size " << size << " " << sockAddr);
			return false;
		}
		bool res = m_Manager->HandleUDPSessionPacket(is, head, sessionInfo, sockAddr);
		if ( false == res )
		{
			m_Statistics.TotalInvalidPackets++;
		}
		else
		{
			m_Statistics.UDPConnectionFlow.Download.Record(totalDataSize);
		}
		return res;
	}


	if ( TryHandlePeerProxyMessage( is, head, remoteSockAddr, static_cast<UINT8>( proxyType ) ) )
		return true;

	if ( m_tracker->TryHandleSecureResponse( is, head, remoteSockAddr, (BYTE)proxyType, EXTRA_PROXY_PEER == extraProxy ) )
	{
		//Tracer::Trace("peer proxy secure message %s\n", strings::format_object(remoteSockAddr).c_str());
		return true;
	}


	PACKET_PEER_INFO packetPeerInfo;
	is >> packetPeerInfo;
	if ( !is )
	{
		m_Statistics.TotalInvalidPackets++;
		APP_ERROR("invalid peer-info " << sockAddr << " size=" << is.total_size());
		return false;
	}

	// 检查guid，如果不是同一频道，跳过
	if ( packetPeerInfo.ChannelGUID != m_PeerInformation->ChannelGUID )
	{
		m_Statistics.TotalInvalidPackets++;
		APP_ERROR("invalid channel guid " << packetPeerInfo.ChannelGUID << " " << m_PeerInformation->ChannelGUID);
		return false;
	}

	APP_DEBUG("DispatchUdp " << make_tuple(static_cast<int>(head.Action), (int)head.ReservedActionType, head.ProtocolVersion, head.TransactionID));

	m_PeerInformation->NetInfo->SavePeerDetectedAddress( packetPeerInfo.DetectedRemoteAddress, sockAddr, false, true );

	if ( m_UdpDetect->HandleUDPPacket( is, head, packetPeerInfo, sockAddr ) )
	{
		m_detectFlow.Download.Record(size);
		return true;
	}
	LIVE_ASSERT(proxyType == PROXY_UDP);
	bool success = m_Manager->HandleUDPPacket( is, head, packetPeerInfo, sockAddr, extraProxy );
	if ( false == success )
	{
		m_Statistics.TotalInvalidPackets++;
	}
	m_otherUDPFlow.Download.Record(size);
	return success;
}

void LiveAppModuleImpl::LoadConfiguration()
{
	m_Manager->LoadConfiguration();
}

void LiveAppModuleImpl::UpdateTrackers(const std::vector<TRACKER_LOGIN_ADDRESS>& trackers)
{
	APP_DEBUG("LiveAppModuleImpl::UpdateTrackers " << trackers.size());
	LoadTracker(trackers);
	m_tracker.reset(DoCreateTracker());
	this->StartTracker();
}

void LiveAppModuleImpl::StartTracker()
{
	m_tracker->Start(*this, (UINT)(m_trackerAddressList.size()), m_trackerAddressList.empty() ? NULL : &m_trackerAddressList[0], m_PeerInformation, m_UDPPacketSender);
}

void LiveAppModuleImpl::UpdateMDSVIP(const PeerAddressArray& mds, const PeerAddressCollection& vip)
{
}

// GetMDS为虚函数，所以LoadMDS不能在基类的构造函数中直接调用
void LiveAppModuleImpl::LoadMDS(const PeerAddressArray& mds)
{
	// m_mdsPeers = mds;

	m_mdsPeers.clear();
	PeerAddressArray::const_reverse_iterator mdsPeer = mds.rbegin();
	for ( ; mdsPeer != mds.rend(); ++mdsPeer )
	{
		if ( mdsPeer->IP == 0 )	// 0 之后就是MDS开始，一直到结束
		{
			break;
		}
		m_mdsPeers.insert( *mdsPeer );
	}

	APP_EVENT("LoadMDS: " << m_mdsPeers.size());
	STL_FOR_EACH_CONST(PeerAddressCollection, m_mdsPeers, iter)
	{
		APP_EVENT("AddMDS " << *iter);
	}
}

void LiveAppModuleImpl::RandomlyAppendTracker(std::vector<TRACKER_LOGIN_ADDRESS>& trackers, Random& rnd)
{
	while (!trackers.empty())
	{
		// 从tracker地址列表中随机的选取一个，加入到实际要用的tracker地址集合中，然后从trackers中删除
		int pos = rnd.Next() % (int)trackers.size();
		LIVE_ASSERT(pos >= 0 && (UINT)pos < trackers.size());
		APP_EVENT("LoadTracker " << pos << ": " << trackers[pos]);
		m_trackerAddressList.push_back(trackers[pos]);
		trackers.erase(trackers.begin() + pos);
	}
}

void LiveAppModuleImpl::LoadTracker(const std::vector<TRACKER_LOGIN_ADDRESS>& trackers)
{
	LIVE_ASSERT(trackers.size() > 0);
	LIVE_ASSERT(trackers.size() <= TRACKER_CLIENT_COUNT_LIMIT);
	std::set<TRACKER_ADDRESS> trackerSet; // 用于检查重复的tracker服务器地址

#if 0
#pragma message("********** add temp trackers")
	TRACKER_ADDRESS tempServer;
	tempServer.IP = ::inet_addr("61.155.8.27");
	tempServer.Port = 8000;
	tempServer.Type = PROXY_UDP;
	tempServer.StatusCode = 0;
	trackerSet.insert(tempServer);
//	tempServer.IP = ::inet_addr("60.209.127.8");
//	trackerSet.insert(tempServer);
#endif

#if 0
	{
		TRACKER_ADDRESS tempServer;
		tempServer.IP = ::inet_addr("61.155.8.27");
		tempServer.Port = 80;
		tempServer.Type = PROXY_HTTP;
		tempServer.StatusCode = 0;
		trackerSet.insert(tempServer);
		//	tempServer.IP = ::inet_addr("60.209.127.8");
		//	trackerSet.insert(tempServer);
	}
#endif

	std::vector<TRACKER_LOGIN_ADDRESS> udpTrackers, tcpTrackers;
	bool hasHTTPTracker = false;
	for (size_t index = 0; index < trackers.size(); ++index)
	{
		const TRACKER_LOGIN_ADDRESS& tracker = trackers[index];
		const TRACKER_ADDRESS& srcAddr = tracker.ServerAddress;
		LIVE_ASSERT(srcAddr.IP != INADDR_NONE && srcAddr.IP != INADDR_ANY && srcAddr.Port != 0);
		if (containers::contains(trackerSet, srcAddr))
		{
			APP_ERROR("LoadTracker: reduplicative tracker server address " << tracker);
			LIVE_ASSERT(!"LoadTracker: reduplicative tracker server address ");
			continue;
		}
		trackerSet.insert(srcAddr);
		if (srcAddr.Type == PPL_TRACKER_TYPE_UDP)
		{
			udpTrackers.push_back(tracker);
		}
		else if (srcAddr.Type == PPL_TRACKER_TYPE_TCP || srcAddr.Type == PPL_TRACKER_TYPE_HTTP)
		{
			if ( PPL_TRACKER_TYPE_HTTP == srcAddr.Type )
			{
				hasHTTPTracker = true;
			}
			tcpTrackers.push_back(tracker);
		}
		else
		{
			APP_ERROR("LoadTracker invalid tracker type " << tracker);
			LIVE_ASSERT(!"LoadTracker invalid tracker type");
		}
	}

#if 0
	if ( m_PeerInformation->IsNormalPeer() )
	{
#pragma message("********** add temp http trackers")
		TRACKER_ADDRESS tempServer;
		tempServer.IP = ::inet_addr("61.155.8.27");
		tempServer.Port = 80;
		tempServer.Type = PROXY_HTTP;
		tempServer.StatusCode = 0;
		TRACKER_LOGIN_ADDRESS tempTracker;
		FILL_ZERO( tempTracker );
		tempTracker.ServerAddress = tempServer;
		if ( trackerSet.find( tempServer ) == trackerSet.end() )
		{
			tcpTrackers.push_back( tempTracker );
		}
	}
#endif

	m_trackerAddressList.clear();
	m_trackerAddressList.reserve(trackers.size());

#if 0
#pragma message("------ ****** only temp tracker")
	TRACKER_LOGIN_ADDRESS tempAddr;
	FILL_ZERO(tempAddr);
	tempAddr.ServerAddress.IP = ResolveHostName("61.155.8.27");
	tempAddr.ServerAddress.Port = 8000;
	tempAddr.ServerAddress.Type = PPL_TRACKER_TYPE_UDP;
	m_trackerAddressList.push_back(tempAddr);
	return;
#endif

	SortedTrackerAddressCollection sortedUdpTrackers, sortedTcpTrackers;
	if (SortTrackerAddress(sortedUdpTrackers, udpTrackers) && SortTrackerAddress(sortedTcpTrackers, tcpTrackers))
	{
		// 根据ip的地域差值来排序成功
		AddSortedTrackerAddress(sortedUdpTrackers);
		AddSortedTrackerAddress(sortedTcpTrackers);
	}
	else
	{
		// 根据ip的地域差值来排序成功，随机排序
		RandomGenerator rnd;
		RandomlyAppendTracker(udpTrackers, rnd);
		RandomlyAppendTracker(tcpTrackers, rnd);
	}
	//if ( false == hasHTTPTracker && m_PeerInformation->NetInfo->CoreInfo.PeerType == NORMAL_PEER )
	//{
	//	TRACKER_LOGIN_ADDRESS addr;
	//	FILL_ZERO( addr );
	//	addr.ServerAddress.IP = ::inet_addr("60.209.127.8");
	//	addr.ServerAddress.Port = 8080;
	//	addr.ServerAddress.Type = PPL_TRACKER_TYPE_HTTP;
	//	m_trackerAddressList.push_back( addr );
	//	addr.ServerAddress.IP = ::inet_addr("61.155.8.27");
	//	addr.ServerAddress.Port = 8080;
	//	addr.ServerAddress.Type = PPL_TRACKER_TYPE_HTTP;
	//	m_trackerAddressList.push_back( addr );
	//}

	APP_DEBUG("LiveAppModuleImpl::LoadTracker " << make_tuple(trackers.size(), m_trackerAddressList.size()));
	m_LiveInfo->TotalTrackerCount = (UINT16)m_trackerAddressList.size();
}

bool LiveAppModuleImpl::SortTrackerAddress(SortedTrackerAddressCollection& sortedTrackers, const std::vector<TRACKER_LOGIN_ADDRESS>& trackers)
{
	UINT localIP = (m_PeerInformation->NetInfo->GetDetectedIP() != 0) ? m_PeerInformation->NetInfo->GetDetectedIP() : m_PeerInformation->NetInfo->Address.IP;
	if (!m_iptable.IsValid())
		return false;
	if (IsPrivateIP(localIP))
		return false;
	NET_TYPE localNetType = m_iptable.LocateIP(localIP);
	if (localNetType.ToInteger() == 0)
		return false;
	sortedTrackers.clear();
	typedef std::vector<TRACKER_LOGIN_ADDRESS> TrackerLoginAddressArray;
	STL_FOR_EACH_CONST(TrackerLoginAddressArray, trackers, iter)
	{
		const TRACKER_LOGIN_ADDRESS& addr = *iter;
		NET_TYPE targetNetType = m_iptable.LocateIP(addr.ServerAddress.IP);
		UINT distance = CalcNetTypeDistance(localNetType, targetNetType);
		sortedTrackers.insert(make_pair(distance, addr));
	}
	return true;
}

void LiveAppModuleImpl::AddSortedTrackerAddress(const SortedTrackerAddressCollection& sortedTrackers)
{
	STL_FOR_EACH_CONST(SortedTrackerAddressCollection, sortedTrackers, iter)
	{
		m_trackerAddressList.push_back(iter->second);
	}
}

void LiveAppModuleImpl::Start(const LiveAppModuleCreateParam& param)
{
	m_LiveInfo->LocalPeerInfo.Flow.Reset();

	LIVE_ASSERT(m_ipPool);


	DoCreateComponents();

	// Added by Tady, 04142010: From line 151.
	if ( NORMAL_PEER == param.PeerType )
	{
		// 检查stun type
		MY_STUN_NAT_TYPE natType = this->LoadNATType();
		if ( STUN_TYPE_INVALID == natType )
		{
			// 读取nat类型失败，另起线程检测
			m_StunTypeResolver.reset( new StunTypeResolver );
			m_StunTypeResolver->Resolve(StunTypeResolver::CallbackType(boost::bind(&LiveAppModuleImpl::OnNATQuerySucceeded, this, _1)));
		}
		else
		{
			// 保存nat类型到NetInfo对象
			m_PeerInformation->NetInfo->SetNATType( natType );
			m_LiveInfo->LocalPeerInfo.NetInfo.CoreInfo.NATType = natType;
		}
	}

	m_UdpDetect.reset(new CUdpDetect(*m_ipPool, *m_Manager, m_PeerInformation, m_UDPConnectionlessPacketSender, *m_LiveInfo));
	LIVE_ASSERT(m_UdpDetect);

	LIVE_ASSERT(m_streamBuffer);
	m_streamBuffer->SetPeerInformation( m_PeerInformation );
	m_streamBuffer->GenerateKey(m_PeerInformation->ChannelGUID);

	m_mediaServer.reset(new CMediaServer(*m_mediaServerListener, *m_streamBuffer, 8888));
	m_mediaServer->SetClientErrorCallback(m_cbClientError);
	m_streamBuffer->SetMediaServer(m_mediaServer.get());
	LIVE_ASSERT(m_tracker);
	LIVE_ASSERT(m_Manager);
	LIVE_ASSERT(m_mediaServer);

	ini_file ini;
	this->InitIni(ini);
	{
		IPPoolConfig config;
		ini.set_section(_T("ippool"));
		config.Load(ini);
		m_ipPool->SetConfig(config);
	}

	{
		StreamBufferConfig config;
		ini.set_section(TEXT("stream"));
		config.Load(ini);
		m_streamBuffer->SetConfig(config);
	}

#ifdef _DEBUG
	{

		ini_file ini;
		this->InitIniFile(ini, _T("paramsd.ini"));
		ini.set_section(_T("connector"));
		tstring mds = ini.get_string(_T("mds"), _T(""));
		PeerAddressUtil::ParseAddressList( m_mdsPeers, mds );
	}
#endif

	DoStart(param);
	m_mediaServer->Start( SM_FORWORD, 5*1000, 15*1000 );
	LIVE_ASSERT(500 % APP_TIMER_TIMES_PER_SECOND == 0);
	m_timer.start(APP_TIMER_INTERVAL);
}

void LiveAppModuleImpl::DoStart(const LiveAppModuleCreateParam& param)
{
	this->StartTracker();
	m_UdpDetect->Start(true);
	m_streamBuffer->Start();
	m_Manager->Start();
}

void LiveAppModuleImpl::OnLoginComplete(const TRACKER_ADDRESS& addr, bool success)
{
	if (success)
	{
		m_LiveInfo->LocalPeerInfo.LIVE_STATE = 2;
	}
}

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

void LiveAppModuleImpl::OnAppTimer()
{
	UINT times = m_timer.get_times();
	m_LiveInfo->ChannelInfo.InternalAliveRecord = times;

	if (times % APP_TIMER_TIMES_PER_SECOND == 0)
	{
		m_PeerInformation->NetInfo->CheckPeerNetType();
		m_LiveInfo->LocalPeerInfo.NetInfo.CoreInfo.PeerNetType = m_PeerInformation->NetInfo->CoreInfo.PeerNetType;

		// 更新几个报文发送器的流量计算
		m_Statistics.UpdateFlow();

		//m_UDPPacketSender->GetFlow().Update();
		//m_UDPConnectionlessPacketSender->GetFlow().Update();
		//m_UDPConnectionPacketSender->GetFlow().Update();
		//m_TCPConnectionlessPacketSender->GetFlow().Update();
		//m_TCPConnectionPacketSender->GetFlow().Update();

		m_trackerFlow.Update();
		m_detectFlow.Update();
		m_otherUDPFlow.Update();
		m_totalFlow.Update();
		m_LiveInfo->ChannelInfo.TrackerDownloadBytes = m_trackerFlow.Download.GetTotalBytes();
		m_LiveInfo->ChannelInfo.DetectDownloadBytes = m_detectFlow.Download.GetTotalBytes();
		m_LiveInfo->ChannelInfo.OtherUDPDownloadBytes = m_otherUDPFlow.Download.GetTotalBytes();
		m_LiveInfo->ChannelInfo.TotalDownloadBytes = m_totalFlow.Download.GetTotalBytes();
		m_LiveInfo->ChannelInfo.TotalUploadBytes = 
			m_UDPPacketSender->GetFlow().GetTotalBytes() 
			+ m_UDPConnectionlessPacketSender->GetFlow().GetTotalBytes() 
			+ m_UDPConnectionPacketSender->GetFlow().GetTotalBytes() 
			+ m_TCPConnectionlessPacketSender->GetFlow().GetTotalBytes()
			+ m_TCPConnectionPacketSender->GetFlow().GetTotalBytes();

		m_ipPool->OnTimer(times / APP_TIMER_TIMES_PER_SECOND);

		// 将总的流量信息记入共享内存
		UINT64 totalDownload = m_Manager->GetFlow().Download.GetTotalBytes();
		UINT64 totalUpload = m_Manager->GetFlow().Upload.GetTotalBytes();
		CDetailedPeerInfo& localInfo = m_LiveInfo->LocalPeerInfo;
		localInfo.DownloadingPieces[0] = HI_DWORD(totalDownload);
		localInfo.DownloadingPieces[1] = LO_DWORD(totalDownload);
		localInfo.DownloadingPieces[2] = HI_DWORD(totalUpload);
		localInfo.DownloadingPieces[3] = LO_DWORD(totalUpload);
		VIEW_INFO("Speed " << make_tuple(m_Manager->GetFlow().Download.GetAverageRate(), m_Manager->GetFlow().Upload.GetAverageRate()) 
			<< make_tuple(localInfo.Flow.GetAverageDownloadSpeed(), localInfo.Flow.GetAverageUploadSpeed()));
	}
	const IPPoolStatistics* ipPoolStatistics = m_ipPool->GetStatistics();
	if (ipPoolStatistics != NULL)
	{
		m_LiveInfo->SyncIPPoolStatistics(*ipPoolStatistics);
	}
	LIVE_CHANNEL_INFO& channelInfo = m_LiveInfo->ChannelInfo;
	channelInfo.OnceHelloCount = static_cast<UINT16>(min(m_UdpDetect->GetStatistics().OnceHelloCount, USHRT_MAX));
	channelInfo.RealOnceHelloCount = static_cast<UINT16>(min(m_UdpDetect->GetStatistics().RealOnceHelloCount, USHRT_MAX));
	channelInfo.RealOnceDetectCount = static_cast<UINT16>(min(m_UdpDetect->GetStatistics().RealOnceDetectCount, USHRT_MAX));
	if (times % APP_TIMER_TIMES_PER_HALF_SECOND == 0)
	{
		m_streamBuffer->OnAppTimer(times / APP_TIMER_TIMES_PER_HALF_SECOND, m_Manager->GetFlow().Download.GetRate());
		const StreamBufferStatistics& streamStats = m_streamBuffer->GetStatistics();
		m_LiveInfo->SyncStreamBufferStatistics(streamStats);
		m_LiveInfo->PlayPort = m_mediaServer->GetPort();
		m_LiveInfo->MediaType = (WORD)m_mediaServer->GetPlayerType();
		m_LiveInfo->LocalPeerInfo.Statistics.PlayToIndex = m_mediaServer->GetPlaytoIndex();
		m_PeerInformation->StatusInfo->Update(m_LiveInfo->LocalPeerInfo.StatusInfo);
		m_PeerInformation->StatusInfo->BufferTime = streamStats.BufferTime;
		m_PeerInformation->StatusInfo->CurrentTimeStamp = streamStats.CurrentTimeStamp;

		UpdateTrackerRequesterStatistics(times / APP_TIMER_TIMES_PER_SECOND);

		// 检查upnp是否启用，如果刚启用，则需要重启tracker客户端
		if ( m_PeerInformation->NetInfo->NeedReLoginForUPNP() )
		{
			m_tracker->Restart();
		}
	}

	m_Manager->OnAppTimer(times);
	m_UdpDetect->OnAppTimer(times);

	// 更新PeerManager的统计信息
	CDetailedPeerInfo& localInfo = m_LiveInfo->LocalPeerInfo;
	{
		const PeerConnectorStatistics& statistics = m_Manager->GetStatistics().ConnectorData;
		UINT lastInitiateConnections = LOWORD(localInfo.DownloadingPieces[6]);
		UINT lastSucceededConnections = HIWORD(localInfo.DownloadingPieces[6]);
		LIVE_ASSERT(lastInitiateConnections >= 0);
		LIVE_ASSERT(lastSucceededConnections >= 0);
		LIVE_ASSERT(lastInitiateConnections < USHRT_MAX);
		//LIVE_ASSERT(lastInitiateConnections >= lastSucceededConnections);
		//LIVE_ASSERT(statistics.TotalInitiateConnections >= lastInitiateConnections);
		//LIVE_ASSERT(statistics.TotalInitiateConnections >= statistics.TotalSucceededConnections);
		//localInfo.DownloadingPieces[6] = MAKE_DWORD(statistics.TotalInitiateConnections, statistics.TotalSucceededConnections);
		localInfo.DownloadingPieces[6] = MAKE_DWORD(statistics.GetTotalInitiatedConnections(), statistics.GetTotalSucceededInitiatedConnections());
		m_LiveInfo->IPPoolInfo.PendingConnectionCount = (WORD)statistics.GetPendingPeerCount();
		m_LiveInfo->ChannelInfo.HandshakingPeerCount = (WORD)statistics.TCPHandshakingPeerCount;
	}
	{
		const PeerManagerStatistics& statistics = m_Manager->GetStatistics();
		LIVE_CHANNEL_INFO& channelInfo = m_LiveInfo->ChannelInfo;
		localInfo.Flow.AverageQos = (UINT16)statistics.AverageQos;
		localInfo.Flow.AverageSkipPercent = (UINT8)statistics.AverageSkipPercent;
		m_LiveInfo->RemotePeerCount = (WORD)statistics.ConnectionCount;
		channelInfo.UDPTDegree = statistics.Degrees.UDPT.ToSimple();
		channelInfo.TCPDegree = statistics.Degrees.GetTCP().ToSimple();
		channelInfo.TotalDownloadMediaBytes = statistics.DownloaderData.MediaFlow.GetTotalBytes();
		channelInfo.TotalUploadMediaBytes = statistics.UploaderData.MediaFlow.GetTotalBytes();
		m_PeerInformation->StatusInfo->Status.DegreeLeft = (INT8)statistics.Degrees.Left;
		m_PeerInformation->StatusInfo->Status.InDegree = (UINT8)statistics.Degrees.All.In;
		m_PeerInformation->StatusInfo->Status.OutDegree = (UINT8)statistics.Degrees.All.Out;
		m_PeerInformation->StatusInfo->Degrees = statistics.Degrees.ToSimple();
		localInfo.StatusInfo.Status = m_PeerInformation->StatusInfo->Status;

		channelInfo.TotalSubPieceRequestsReceived = static_cast<UINT>( statistics.UploaderData.TotalSubPieceRequestsReceived );
		channelInfo.TotalSubPiecesUploaded = static_cast<UINT>( statistics.UploaderData.TotalSubPiecesUploaded );

		SaveLocalFlowInfo(localInfo.Flow, m_Manager->GetFlow());
	}
	//m_LiveInfo->LocalPeerInfo.StatusInfo = *m_PeerInformation->StatusInfo;

	if (times % APP_TIMER_TIMES_PER_SECOND == 0)
	{
		m_Statistics.SyncFlow();
		m_tracker->GetStatistics().UpdateFlow();
		m_tracker->GetStatistics().SyncFlow();

		// 整秒
		UINT seconds = times / APP_TIMER_TIMES_PER_SECOND;
		if ( seconds < START_STATS::MAX_RECORDS )
		{
			m_Statistics.StartData.Records[seconds].ConnectionCount = m_Manager->GetPeerCount();
			m_Statistics.StartData.Records[seconds].DownloadSpeed = m_Statistics.TotalFlow.Download.GetRate();
		}

		*m_StatisticsInfo.AppStats = m_Statistics;
		*m_StatisticsInfo.DetectorStats = m_UdpDetect->GetStatistics();
		*m_StatisticsInfo.TrackerStats = m_tracker->GetStatistics();
		const PeerManagerStatistics& peerManagerStats = m_Manager->GetStatistics();
		*m_StatisticsInfo.PeerManagerStats = peerManagerStats;
		*m_StatisticsInfo.ConnectorStats = peerManagerStats.ConnectorData;
		*m_StatisticsInfo.DownloaderStats = peerManagerStats.DownloaderData;
		*m_StatisticsInfo.UploaderStats = peerManagerStats.UploaderData;
		*m_StatisticsInfo.StreamBufferStats = m_streamBuffer->GetStatistics();
		*m_StatisticsInfo.MediaServerStats = m_mediaServer->GetStatistics();

		if ( m_ipPool->GetStatistics() )
		{
			*m_StatisticsInfo.IPPoolStats = *m_ipPool->GetStatistics();
		}


		m_LogClient->SaveLog(m_streamBuffer->GetStatistics(), *m_Manager, *m_ipPool);
	}

	if ( times % APP_TIMER_TIMES_PER_SECOND == 0 )
	{
		const StreamBufferStatistics& streamStat = m_streamBuffer->GetStatistics();
                (void)streamStat;
		TEST_LOG_OUT_ONCE( 
			"stream stat: buffer time = " << streamStat.BufferTime 
			<< ", buffer size=" << streamStat.BufferSize 
			<< ", play position=" << m_mediaServer->GetStatistics().PlayToIndex 
			<< ", minmax=" << m_PeerInformation->StatusInfo->MinMax );

#ifdef ENABLE_PRINT_STREAM_STAT
        std::cout << 
            "stream stat: buffer time = " << streamStat.BufferTime 
            << ", buffer size=" << streamStat.BufferSize 
            << ", play position=" << m_mediaServer->GetStatistics().PlayToIndex 
            << ", minmax=" << m_PeerInformation->StatusInfo->MinMax
            << ", upbw=" << m_LiveInfo->LocalPeerInfo.Flow.GetRecentUploadSpeed()
            << ", peercnt=" << m_LiveInfo->IPPoolInfo.TotalPoolSize
            << std::endl;
#endif
	}
}

void LiveAppModuleImpl::UpdateTrackerRequesterStatistics(UINT seconds)
{
	int count = (int)m_tracker->GetClientCount();
	LIMIT_MAX(count, MAX_TRACKER_CLIENT_COUNT);
	for (int i = 0; i < count; ++i)
	{
		TrackerClientPtr client = m_tracker->GetClient(i);
		if ( !client )
			break;
		const TRACKER_ADDRESS& addr = client->GetServerAddress();
		TRACKER_ADDRESS& status = m_LiveInfo->Trackers[i].ServerAddress;
		status.IP = addr.IP;
		status.Port = addr.Port;
		status.Type = addr.Type;
		status.ReservedStatusCode = (UINT8)client->GetStatusCode();
	}
	m_LiveInfo->TrackerCount = (BYTE)count;
	m_LiveInfo->CurrentTracker = (BYTE)m_tracker->GetCurrentClient();
	m_LiveInfo->UseTCPTracker = m_tracker->IsTCPTrackerUsed();

	// 15秒以内不更新TrackerErrorCode
	if (seconds <= 15 || m_ipPool->GetSize() > 0)
	{
		m_LiveInfo->TrackerErrorCode = 0;
	}
	else
	{
		BYTE errcode = TRACKER_ERROR_SUCCESS;
		std::map<long, long> statusCodes;
		for (size_t i = 0; i < m_tracker->GetClientCount(); ++i)
		{
			TrackerClientPtr client = m_tracker->GetClient(i);
			LIVE_ASSERT(client);
			if ( client )
			{
				long statusCode = client->GetStatusCode();
				statusCodes[statusCode]++;
			}
		}
		if (statusCodes[1] > 0)
		{
			// 如果有一个成功，则表示登录成功
			errcode = TRACKER_ERROR_SUCCESS;
		}
		else if (statusCodes[PT_ERROR_ZONE_DENIED] + statusCodes[PT_ERROR_LANGUAGE_DENIED] > 0)
		{
			// 如果有一个为地域限制
			errcode = TRACKER_ERROR_ZONE_DENIED;
		}
		else if (statusCodes[PT_ERROR_NO_CHANNEL] > 0)
		{
			// 如果有一个为频道不存在
			errcode = TRACKER_ERROR_NO_CHANNEL;
		}
		else if (statusCodes[PT_ERROR_OBSOLETE_PEER_VERSION] > 0)
		{
			// 如果有一个为过期版本
			errcode = TRACKER_ERROR_OBSOLETE_PEER_VERSION;
		}
		m_LiveInfo->TrackerErrorCode = errcode;
		if (errcode != TRACKER_ERROR_SUCCESS)
		{
			APP_ERROR("Tracker Login Failed " << static_cast<int>(errcode));
		}
	}

}


void LiveAppModuleImpl::OnPeerListed(const TRACKER_ADDRESS& trackerAddr, UINT8 count, const CANDIDATE_PEER_INFO addrs[], UINT8 lanPeerCount, const INNER_CANDIDATE_PEER_INFO lanPeers[], UINT64 sourceTimeStamp)
{
	TEST_LOG_OUT((int)count << " wan peers from tracker " << trackerAddr);
	for ( UINT8 addrIndex = 0; addrIndex < count; ++addrIndex )
	{
		const CANDIDATE_PEER_INFO& info = addrs[addrIndex];
		TEST_LOG_OUT("wan peer " << (int)addrIndex << ": " << info.Address 
			<< " with (nat,net,peer)=" 
			<< make_tuple(info.CoreInfo.NATType, info.CoreInfo.PeerNetType, info.CoreInfo.PeerType));
		TRACKER_DEBUG("OnListSucceeded: Peer " << addrIndex << ": " << info.Address << " type=" << make_tuple(info.CoreInfo.PeerType, info.CoreInfo.PeerNetType, info.CoreInfo.NATType));
		if ( NORMAL_PEER != info.CoreInfo.PeerType )
		{
			TRACKER_DEBUG("NonNormalPeer " << info.Address << " " << info.CoreInfo.PeerType);
		}
	}
	m_ipPool->AddCandidate(count, addrs, CANDIDATE_FROM_TRACKER);
	TEST_LOG_OUT((int)lanPeerCount << " lan peers from tracker " << trackerAddr);
	for ( UINT8 index = 0; index < lanPeerCount; ++index )
	{
		const INNER_CANDIDATE_PEER_INFO& peer = lanPeers[index];
		if ( peer.CoreInfo.NATType != 0 )
		{
			TRACKER_DEBUG("OnPeerListed " << index << " " << peer.DetectedAddress << " with nts " << peer.StunServerAddress);
			m_ipPool->AddInnerCandidate( peer, CANDIDATE_FROM_TRACKER );
		}
		TEST_LOG_OUT("lan peer " << (int)index << ": " << peer.DetectedAddress 
			<< " with (nat,net,peer)=" 
			<< make_tuple(peer.CoreInfo.NATType, peer.CoreInfo.PeerNetType, peer.CoreInfo.PeerType) 
			<< " nts=" << peer.StunServerAddress);
		//if ( lanPeers[index].PublicHostCount > 0 )
		//{
		//	UINT8 publicHostCount = min( 3, lanPeers[index].PublicHostCount );
		//	for ( UINT8 publicHostIndex = 0; publicHostIndex < publicHostCount; ++publicHostIndex )
		//	{
		//		CANDIDATE_PEER_INFO peer;
		//		//peer.Address = lanPeers[index].PublicHosts[publicHostIndex];
		//		//FILL_ZERO( peer.CoreInfo );
		//		//m_ipPool->AddCandidate( peer, CANDIDATE_FROM_TRACKER );
		//	}
		//}
	}
	if ( sourceTimeStamp != 0 )
	{
		m_PeerInformation->StatusInfo->GetSourceResource()->SaveTimeStamp( sourceTimeStamp );
	}
	for ( UINT8 lanPeerIndex = 0; lanPeerIndex < lanPeerCount; ++lanPeerIndex )
	{
	//	const INNER_CANDIDATE_PEER_INFO& info = lanPeers[lanPeerIndex];
	}
	TEST_LOG_FLUSH();
}

void LiveAppModuleImpl::DoSaveSourceMinMax(const PEER_MINMAX& sourceMinMax)
{
	m_PeerInformation->StatusInfo->GetSourceResource()->SaveMinMax( sourceMinMax );
	m_LiveInfo->SyncSourceMinMax(sourceMinMax);
	APP_DEBUG("Save Source MinMax " << sourceMinMax);
}

bool LiveAppModuleImpl::SaveVIPMinMax(const PEER_MINMAX& vipMinMax)
{
	//if (vipMinMax.MaxIndex > m_PeerInformation->StatusInfo->SourceMinMax.MaxIndex && vipMinMax.MinIndex > m_PeerInformation->StatusInfo->SourceMinMax.MinIndex)
	if (m_PeerInformation->StatusInfo->GetSourceResource()->IsEmpty() && !vipMinMax.IsEmpty())
	{
		this->DoSaveSourceMinMax(vipMinMax);
		return true;
	}
	return false;
}

bool LiveAppModuleImpl::SaveSourceMinMax(const PEER_MINMAX& sourceMinMax)
{
	if (sourceMinMax.MinIndex == 0 && sourceMinMax.MaxIndex == 0)
	{
		APP_ERROR("Invalid source minmax " << sourceMinMax << ", source minmax=" << *m_PeerInformation->StatusInfo->GetSourceResource());
		return false;
	}
	this->DoSaveSourceMinMax(sourceMinMax);
	return true;
}

void LiveAppModuleImpl::SaveDetectedAddress(UINT32 detectedIP, UINT16 detectedUDPPort, const TRACKER_ADDRESS& trackerAddr)
{
	m_ipPool->SetExternalIP(detectedIP);
	m_LiveInfo->LocalPeerInfo.DetectedIP = detectedIP;

	m_PeerInformation->NetInfo->SaveDetectedAddress(detectedIP, detectedUDPPort, SimpleSocketAddress( trackerAddr.IP, trackerAddr.Port ));
	if (m_PeerInformation->NetInfo->NeedReLoginForUPNPOuterIP())
	{
		// 检测到没有使用过OuterIP进行join，重新join
		m_tracker->Restart();
	}

}

//bool LiveAppModuleImpl::SendPacket(UINT ip, UINT16 port, BYTE proxyType, const PacketBase& body)
//{
//	return m_UDPPacketSender->Send(body, ip, port, proxyType) > 0;
//}

size_t LiveAppModuleImpl::GetMaxConnectionCount() const
{
//	return 30;
	size_t maxCount = m_SysInfo->MaxAppPeerCount;
	if (m_LiveInfo->IntelligentConnectionControlDisabled == false && maxCount < m_AutoMaxAppPeerCount)
	{
		// 在用户设置的值提升若干，供测试网验证，以改善连接效果
//		maxCount += m_MaxAppPeerCountIncrement;
		maxCount = m_AutoMaxAppPeerCount;
		//LIMIT_MAX(maxCount, m_AutoMaxAppPeerCount);
	}
	// 临时修改，用于在测试验证提高最大连接数后的效果
	//LIMIT_MIN(maxCount, 50);
	// m_SysInfo->MaxAppPeerCount可能变化，需要限制不超过m_PeerCountLimit
	return min(maxCount, m_PeerCountLimit);
}

size_t LiveAppModuleImpl::GetMaxConnectionCount2() const
{
	size_t maxCount = m_AutoMaxAppPeerCount;
	if (m_LiveInfo->IntelligentConnectionControlDisabled == true)
		LIMIT_MAX(maxCount, m_SysInfo->MaxAppPeerCount);
	return min(maxCount, m_PeerCountLimit);
}

size_t LiveAppModuleImpl::GetMinConnectionCount() const
{
	size_t minCount = m_AutoMinAppPeerCount;
	if (m_LiveInfo->IntelligentConnectionControlDisabled == true)
		LIMIT_MAX(minCount, m_SysInfo->MaxAppPeerCount);
	return min(minCount, m_PeerCountLimit);
}

bool LiveAppModuleImpl::CheckPieceIndexValid(UINT pieceIndex) const
{
	//return true;  这句话是用来关闭 Source Min Max 参考的
	const PEER_MINMAX& sourceMinmax = m_PeerInformation->StatusInfo->GetSourceResource()->GetMinMax();
	if (sourceMinmax.IsEmpty() || sourceMinmax.GetLength() == 0)
		return true;
	return pieceIndex < (sourceMinmax.MaxIndex + 2*60*60*10) && (pieceIndex + 2000) > sourceMinmax.MinIndex;
}

bool LiveAppModuleImpl::TryHandlePeerProxyMessage( data_input_stream& is, const NEW_UDP_PACKET_HEAD& head, const InetSocketAddress& addr, UINT8 proxyType )
{
	if ( PTS_ACTION_PROXY_MESSAGE != head.Action || PROXY_UDP != proxyType )
		return false; // 返回false表示不是peer-proxy报文

	GUID channelGUID, peerGUID;
	PTSProxyMessagePacket proxyMessage;
	is >> channelGUID >> peerGUID >> proxyMessage;
	if ( !is )
	{
		m_Statistics.TotalInvalidPackets++;
		APP_ERROR("Invalid udp buffer 3(no channel guid or peer guid or invalid peer-proxy message) " << addr);
		return true; // 返回true，表示peer-proxy报文已经被处理
	}

	if ( proxyMessage.IsResponse )
	{
		// response
		if ( proxyMessage.ViaAddress == proxyMessage.ToAddress )
		{
			// 最终的回应
			//Tracer::Trace("HandlePeerProxyMessage stage 3 %s for %s\n", strings::format_object(addr).c_str(), strings::format_object(proxyMsgHeader.FromAddress).c_str());
			if ( proxyMessage.MessageData.size() > 0 )
			{
				this->DispatchUdpPacket( 
					&proxyMessage.MessageData[0], 
					proxyMessage.MessageData.size(), 
					proxyMessage.FromAddress.ToUDPAddress().ToInetSocketAddress(), 
					proxyType, 
					EXTRA_PROXY_PEER );
			}
		}
		else
		{
			// 需要转发的回应
			if ( proxyMessage.TTL > 0 )
			{
				proxyMessage.ViaAddress = proxyMessage.ToAddress;
				proxyMessage.TTL--;

				m_SecureTrackerProxyPacketBuilder->Build( proxyMessage, head.TransactionID );
				m_NormalUDPSender->Send( 
					m_SecureTrackerProxyPacketBuilder->GetData(), 
					m_SecureTrackerProxyPacketBuilder->GetSize(), 
					proxyMessage.DetectedAddress.ToUDPAddress().ToInetSocketAddress() );
			}
		}
	}
	else
	{
		// request，需要转发
		if ( proxyMessage.TTL > 0 )
		{
			proxyMessage.TTL--;
			proxyMessage.DetectedAddress.IP = addr.GetIP();
			proxyMessage.DetectedAddress.UdpPort = addr.GetPort();
			proxyMessage.DetectedAddress.TcpPort = 0;
			proxyMessage.ViaAddress = proxyMessage.ToAddress;

			m_SecureTrackerProxyPacketBuilder->Build( proxyMessage, head.TransactionID );
			m_NormalUDPSender->Send( m_SecureTrackerProxyPacketBuilder->GetData(), 
				m_SecureTrackerProxyPacketBuilder->GetSize(), 
				proxyMessage.ToAddress.ToUDPAddress().ToInetSocketAddress() );
		}
	}
	return true;
}

void LiveAppModuleImpl::OnNTSListOK( const std::vector<InetSocketAddress>& servers )
{
	if ( false == servers.empty() )
	{
		// 保存到配置文件
		ini_file ini;
		this->InitConfig(ini);
		ini.set_section(_T("stun"));
		tstring strs;
		for ( size_t index = 0; index < servers.size(); ++index )
		{
			tstring str = IPAddress(servers[index].GetIP()).ToString() + _T(":") + strings::format(_T("%d"), servers[index].GetPort());
			strs += str;
			strs += _T("|");
		}
		SYSTEMTIME st;
		::GetLocalTime(&st);
		ini.set_int(_T("time"), st.wDay);
		ini.set_string(_T("servers"), strs);
	}
	this->StartStunModule( servers );
}

void LiveAppModuleImpl::StartStunModule(const std::vector<InetSocketAddress>& stunServers)
{
	m_StunModule.reset( new StunModule( stunServers, m_NormalUDPSender, m_PeerInformation, m_CreateParam.AppVersionNumber16 ) );
	m_StunModule->SetCallbacks(
		StunModule::LoginCallbackType( boost::bind( &LiveAppModuleImpl::OnStunLogin, this ) ), 
		StunModule::TransmitCallbackType( boost::bind( &LiveAppModuleImpl::OnStunTransmit, this, _1, _2, _3 ) )
		);
	m_StunModule->Start();
}

void LiveAppModuleImpl::OnStunLogin()
{
	InetSocketAddress rawServerAddr = m_StunModule->GetServerAddress();
	PEER_ADDRESS serverAddr;
	serverAddr.IP = rawServerAddr.GetIP();
	serverAddr.UdpPort = rawServerAddr.GetPort();
	serverAddr.TcpPort = 0;
	PEER_ADDRESS detectedAddr = m_StunModule->GetDetectedAddress();
	LIVE_ASSERT( serverAddr.IsUDPValid() && detectedAddr.IsUDPValid() );
	m_PeerInformation->NetInfo->SetStunAddress( serverAddr, detectedAddr );
	m_tracker->KeepAlive();
	VIEW_INFO("nat login ok " << detectedAddr << " on " << serverAddr);
}

void LiveAppModuleImpl::OnStunTransmit( BYTE* data, size_t bytes, const InetSocketAddress& remoetAddr )
{
	LIVE_ASSERT( data != NULL && bytes > 0 );
	this->DispatchUdpPacket(data, bytes, remoetAddr, PROXY_UDP, EXTRA_PROXY_NTS);
}

void LiveAppModuleImpl::OnNATQuerySucceeded( MY_STUN_NAT_TYPE natType )
{
	m_PeerInformation->NetInfo->SetNATType( natType );
	m_LiveInfo->LocalPeerInfo.NetInfo.CoreInfo.NATType = natType;

	if (m_tracker)
	{
		m_tracker->KeepAlive();
	}

	//  保存到配置文件

	ini_file ini;
	this->InitConfig(ini);
	ini.set_section(nat_ini_section);

	////保存获取到的NAT信息
	ini.set_int(nat_ini_key_type, natType);

	SYSTEMTIME st;
	GetLocalTime(&st);
	ini.set_int(nat_ini_key_time, st.wDay);

	////保存获取的本地IP        
	ini.set_uint(nat_ini_key_ip, IPArrayLoader::GetFirstLocalIP());
}

MY_STUN_NAT_TYPE LiveAppModuleImpl::LoadNATType()
{
	ini_file ini;
	this->InitConfig(ini);
	ini.set_section(nat_ini_section);

	//初始化MeidaEngine的配置文件路径
	int LastTime = -1;
	UINT32 LastLocalIP = static_cast<UINT32>( -1 );

	int natType = ini.get_int(nat_ini_key_type, STUN_TYPE_ERROR);
	// if ( snt_result < TYPE_FULLCONENAT || snt_result > TYPE_PUBLIC)
	if ( natType < STUN_TYPE_PUBLIC || natType > STUN_TYPE_SYMNAT)
	{
		return STUN_TYPE_INVALID;
	}

	LastLocalIP = ini.get_int(nat_ini_key_ip, -1);
	if ((UINT32) -1 == LastLocalIP )
		return STUN_TYPE_INVALID;
	if ( LastLocalIP != (size_t)IPArrayLoader::GetFirstLocalIP() )
	{
		return STUN_TYPE_INVALID;
	}

	LastTime = ini.get_int(nat_ini_key_time, -1);
	if ( -1 == LastTime )
		return STUN_TYPE_INVALID;
	SYSTEMTIME st;
	GetLocalTime(&st);
	//如果日期相差三天，则判断已经过期
	if ( abs(st.wDay - LastTime) >= 3)
	{
		return STUN_TYPE_INVALID;
	}

	return static_cast<MY_STUN_NAT_TYPE>( natType );
}

void LiveAppModuleImpl::OnPlayerBuffering( bool isBufferOK )
{
	if ( m_PeerInformation->IsNormalPeer() )
	{
		m_Manager->OnPlayerBufferring(isBufferOK);
	}
}

GUID LiveAppModuleImpl::GetResourceGUID() const
{
	return m_PeerInformation->ChannelGUID;
}

GUID LiveAppModuleImpl::GetChannelGUID() const
{
	return m_PeerInformation->ChannelGUID;
}

const PeerStatusInfo& LiveAppModuleImpl::GetStatusInfo() const
{
	return *m_PeerInformation->StatusInfo;
}

const PeerNetInfo& LiveAppModuleImpl::GetNetInfo() const
{
	return *m_PeerInformation->NetInfo;
}

const PeerAuthInfo& LiveAppModuleImpl::GetAuthInfo() const
{
	return *m_PeerInformation->AuthInfo;
}
/*
UINT LiveAppModuleImpl::CalcAddressDistance(UINT32 targetIP) const
{
	return 0;
}*/





ServerLiveAppModule::ServerLiveAppModule(const LiveAppModuleCreateParam& param, const TCHAR* tagname, bool tagMutate) 
	: LiveAppModuleImpl(param, tagname, tagMutate)
{
	m_ipPool.reset(IPPoolFactory::CreateTrivial());
}

ServerLiveAppModule::~ServerLiveAppModule()
{
}

void ServerLiveAppModule::UpdateMDSVIP(const PeerAddressArray& mds, const PeerAddressCollection& vip)
{
	// mpc/mds模块在处理UpdateMDSVIP时将提供的地址列表更新到mds pool中
	LoadMDS(mds);
	m_Manager->LoadInputSources( mds );
	m_Manager->LoadVIP(vip);
}

void ServerLiveAppModule::DoStart(const LiveAppModuleCreateParam& param)
{
	// mcc不需要洪泛探测，但需要从别的节点处下载数据
	m_Manager->LoadInputSources( param.MDSs );
	m_Manager->LoadVIP(param.VIPs);
	this->StartTracker();
	m_UdpDetect->Start(false);
	m_streamBuffer->Start();
	m_Manager->Start();
}






ClientLiveAppModule::ClientLiveAppModule(const LiveAppModuleCreateParam& param, const TCHAR* mmfPrefix, bool tagMutate) 
	: LiveAppModuleImpl(param, mmfPrefix, tagMutate)
{
	//	APP_INFO("Create New PeerModule!  ResourceGUID="<<FormatGUID(param->ResourceGuid));
	m_ipPool.reset(IPPoolFactory::PeerCreateNetTyped(m_PeerInformation->NetInfo, m_CreateParam.BaseDirectory, m_mdsPeers, m_iptable));
	//m_ipPool.reset(IPPoolFactory::PeerCreate(m_PeerInformation->NetInfo->Address, m_dllModule.GetHandle()));
	// 需要先创建IPPool，才能正确加载mds1
	LoadMDS(param.MDSs);
}

ClientLiveAppModule::~ClientLiveAppModule()
{
}

void ClientLiveAppModule::UpdateMDSVIP(const PeerAddressArray& mds, const PeerAddressCollection& vip)
{
	// peer模块在处理UpdateMDSVIP时的mds列表实际是局域网peer列表
	//m_lanPeers = mds;
	m_lanPeers.clear();
	STL_FOR_EACH_CONST(PeerAddressArray, mds, iter)
	{
		m_lanPeers.insert(*iter);
	}
	// PeerModule没有vip列表
}

