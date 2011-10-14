
#include "StdAfx.h"

#include "GloalTypes.h"
#include "media/MediaServerListener.h"
#include "StreamBufferStatistics.h"
#include "IPPoolStatistics.h"
#include "util/flow.h"
#include "BaseInfo.h"
#include "AppParam.h"
#include "framework/log.h"

#include <synacast/protocol/StatisticsInfo.h>
#include <synacast/protocol/TrackerProtocol.h>
#include <synacast/protocol/PeerPacket.h>
#include <synacast/protocol/PacketHead.h>
#include <synacast/protocol/data/NetType.h>

#include <ppl/util/ini_file.h>
#include <ppl/os/process.h>
#include <ppl/data/guid.h>
#include <ppl/data/strings.h>

#include <boost/static_assert.hpp>



BOOST_STATIC_ASSERT( sizeof(bool) == 1 );
BOOST_STATIC_ASSERT( sizeof(INT8) == 1 );
BOOST_STATIC_ASSERT( sizeof(UINT8) == 1 );
BOOST_STATIC_ASSERT( sizeof(UINT16) == 2 );
BOOST_STATIC_ASSERT( sizeof(UINT32) == 4 );
BOOST_STATIC_ASSERT( sizeof(UINT64) == 8 );


BOOST_STATIC_ASSERT(sizeof(NET_TYPE) == 8);
BOOST_STATIC_ASSERT(sizeof(PEER_ADDRESS) == 8);
BOOST_STATIC_ASSERT(sizeof(PEER_MINMAX) == 8);
BOOST_STATIC_ASSERT(sizeof(CANDIDATE_PEER_INFO) == sizeof(PEER_ADDRESS) + sizeof(NET_TYPE));
BOOST_STATIC_ASSERT(sizeof(CANDIDATE_PEER_INFO) == sizeof(PEER_ADDRESS) + sizeof(PEER_CORE_INFO));
BOOST_STATIC_ASSERT(sizeof(PEER_STATUS) == 8);
BOOST_STATIC_ASSERT(INNER_CANDIDATE_PEER_INFO::object_size == 25 );


BOOST_STATIC_ASSERT(sizeof(PEER_CORE_INFO) == 8);


BOOST_STATIC_ASSERT(sizeof(PEER_ADDRESS) == 8);
BOOST_STATIC_ASSERT(sizeof(PEER_MINMAX) == 8);

BOOST_STATIC_ASSERT( sizeof(PEER_INFORMATION) == 152 );
BOOST_STATIC_ASSERT( sizeof(PEER_INFO_ITEM) == sizeof(PEER_INFORMATION) + 4 );


BOOST_STATIC_ASSERT(sizeof(LOCAL_STATISTICS) == 36);

BOOST_STATIC_ASSERT(sizeof(PEER_DETAILED_INFO) == sizeof(PEER_INFORMATION) + sizeof(LOCAL_STATISTICS) + 8);
BOOST_STATIC_ASSERT(sizeof(PEER_DETAILED_INFO) == 152 + 36 + 8);
BOOST_STATIC_ASSERT(sizeof(PEER_DETAILED_INFO) == 196);

BOOST_STATIC_ASSERT( PeerStatusEx::object_size == 10 );

BOOST_STATIC_ASSERT(sizeof(SYSINFO) == 1264);

BOOST_STATIC_ASSERT(sizeof(SYSINFO) == sizeof(CSysInfo));
BOOST_STATIC_ASSERT(sizeof(LIVE_INFO) == sizeof(CLiveInfo));


BOOST_STATIC_ASSERT(sizeof(IP_POOL_INFO) == 8);

//BOOST_STATIC_ASSERT(sizeof(LIVE_CHANNEL_INFO) == 264);
BOOST_STATIC_ASSERT(sizeof(LIVE_CHANNEL_INFO) == 268);
BOOST_STATIC_ASSERT(sizeof(CPeerInfoItem) == 156);


//BOOST_STATIC_ASSERT(sizeof(LIVE_INFO) == 256 * 156 + 1094);
BOOST_STATIC_ASSERT(sizeof(LIVE_INFO) == PPL_MIN_PEER_COUNT_LIMIT * 156 + 1096);
BOOST_STATIC_ASSERT(sizeof(LIVE_INFO) == 41032);

BOOST_STATIC_ASSERT(sizeof(DEGREE_PAIR) == 2);
BOOST_STATIC_ASSERT(sizeof(DEGREE_INFO) == 5);


BOOST_STATIC_ASSERT(sizeof(PARAM_UPDATE_TRACKERS) == 36);

//BOOST_STATIC_ASSERT(sizeof(OLD_PACKET_COMMON_HEAD) == 4);
BOOST_STATIC_ASSERT(sizeof(OLD_UDP_PACKET_HEAD) == 12);
BOOST_STATIC_ASSERT(OLD_UDP_PACKET_HEAD::object_size == 12);
BOOST_STATIC_ASSERT(sizeof(NEW_UDP_PACKET_HEAD) == 8);
//BOOST_STATIC_ASSERT(sizeof(UDP_PEER_PACKET_HEAD) == 44);

BOOST_STATIC_ASSERT(PACKET_PEER_INFO::object_size == 74);

BOOST_STATIC_ASSERT( SECURE_REQUEST_HEAD::object_size == 15 );

//BOOST_STATIC_ASSERT( sizeof( UDPT_HEAD_INFO ) == 16 );

// BOOST_STATIC_ASSERT(sizeof(noncopyable) == 1);
// BOOST_STATIC_ASSERT(sizeof(pool_object) == 1);
// BOOST_STATIC_ASSERT(sizeof(PacketBase) == 8);
// BOOST_STATIC_ASSERT(sizeof(DynamicPacket) == 24);

//BOOST_STATIC_ASSERT(sizeof(PP_HELLO) == 45);


BOOST_STATIC_ASSERT(SubPieceUnit::object_size == 5);


BOOST_STATIC_ASSERT( PeerExchangeItem::object_size == 9);

BOOST_STATIC_ASSERT( REDIRECT_PEER_INFO::object_size == 40 );


BOOST_STATIC_ASSERT( PPL_IS_EMBEDDED_SYSTEM == 0 );






void CLiveInfo::Init(const LiveAppModuleCreateParam& param, UINT appVersion)
{
	this->AppType = param.AppType;
	this->ResourceGUID = param.ChannelGUID;

	this->Username[63] = 0;
	this->Password[63] = 0;
	strncpy(this->Username, param.Username.c_str(), 63);
	strncpy(this->Password, param.Password.c_str(), 63);

	const SYSINFO& sysInfo = *param.SysInfo;
	this->LocalPeerInfo.LIVE_STATE = 1;
	this->LocalPeerInfo.NetInfo.Address.IP = sysInfo.LocalIPs[0];
	this->LocalPeerInfo.NetInfo.Address.TcpPort = sysInfo.TcpPort;
	this->LocalPeerInfo.NetInfo.Address.UdpPort = sysInfo.UdpPort;
	this->LocalPeerInfo.NetInfo.CoreInfo.PeerType = param.PeerType;
	this->LocalPeerInfo.PeerGUID = sysInfo.PeerGUID;
	this->LocalPeerInfo.DownloadingPieces[7] = appVersion;
}

void CLiveInfo::SyncUDPDetectorStatistics(const UDPDetectorStatistics& statistics)
{
}

void CLiveInfo::SyncIPPoolStatistics(const IPPoolStatistics& statistics)
{
	CLiveInfo& liveInfo = *this;
	IP_POOL_INFO& poolInfo = this->IPPoolInfo;
	poolInfo.CandidatePoolSize = min(statistics.CandidatePoolSize, static_cast<UINT32>(USHRT_MAX));
	poolInfo.DetectedPoolSize = min(statistics.DetectedPoolSize, static_cast<UINT32>(USHRT_MAX));
	poolInfo.TotalPoolSize = min(statistics.TotalPoolSize, static_cast<UINT32>(USHRT_MAX));
	LIVE_CHANNEL_INFO& channelInfo = liveInfo.ChannelInfo;
	channelInfo.DetectIndexSize = min(statistics.DetectIndexSize, static_cast<UINT32>(USHRT_MAX));
	channelInfo.ConnectIndexSize = min(statistics.ConnectIndexSize, static_cast<UINT32>(USHRT_MAX));
	channelInfo.ActiveIndexSize = min(statistics.ActiveIndexSize, static_cast<UINT32>(USHRT_MAX));
	channelInfo.HelloIndexSize = min(statistics.HelloIndexSize, static_cast<UINT32>(USHRT_MAX));
	channelInfo.ConnectionPoolSize = min(statistics.ConnectionPoolSize, static_cast<UINT32>(USHRT_MAX));
	channelInfo.UnconnectablePoolSize = min(statistics.UnconnectablePoolSize, static_cast<UINT32>(USHRT_MAX));
	channelInfo.DetectCheckCount = statistics.DetectCheckCount;
	channelInfo.ConnectCheckCount = statistics.ConnectCheckCount;
	channelInfo.DeleteCheckCount = statistics.DeleteCheckCount;
	channelInfo.IPPoolDetectingCount = statistics.DetectingCount;
}

void CLiveInfo::SyncStreamBufferStatistics(const StreamBufferStatistics& statistics)
{
	CLiveInfo& liveInfo = *this;
	CDetailedPeerInfo& localInfo = this->LocalPeerInfo;
	LOCAL_STATISTICS& localStatistics = localInfo.Statistics;
	PEER_MINMAX& minmax = localInfo.StatusInfo.MinMax;
	localStatistics.MinIndex = statistics.MinIndex;
	localStatistics.MaxIndex = statistics.MaxIndex;
	localStatistics.SkipIndex = statistics.SkipIndex;
	localStatistics.StartIndex = statistics.StartIndex;
	localStatistics.TotalPushedCount = statistics.TotalPushedPieceCount;
	localStatistics.BufferSize = statistics.BufferSize;
	localStatistics.PieceCount = statistics.PieceCount;
	localStatistics.BufferTime = statistics.SkipBufferTime;

	minmax.MinIndex = statistics.MinIndex;
	minmax.MaxIndex = statistics.MaxIndex;

	localInfo.StatusInfo.Status.SkipPercent = statistics.SkipPercent;

	localInfo.DownloadingPieces[5] = statistics.TotalSkippedPieceCount;

	LIVE_CHANNEL_INFO& channelInfo = liveInfo.ChannelInfo;
	channelInfo.BaseIndex = statistics.BasePieceIndex;
	channelInfo.BaseTimeStamp = static_cast<UINT32>( statistics.BaseTimeStamp );
	channelInfo.MinTimeStamp = static_cast<UINT32>( statistics.MinTimeStamp );
	channelInfo.MaxTimeStamp = static_cast<UINT32>( statistics.MaxTimeStamp );
	channelInfo.SkipTimeStamp = static_cast<UINT32>( statistics.SkipTimeStamp );
	channelInfo.FirstErrorPiece = statistics.FirstErrorPiece;
	channelInfo.FirstErrorPieceTimeStamp = static_cast<UINT32>( statistics.FirstErrorPieceTimeStamp );
	VIEW_INFO("StreamBufferStatistics: " << make_tuple(statistics.BasePieceIndex, statistics.BaseTimeStamp, statistics.SkipIndex));
}

void CLiveInfo::SyncSourceMinMax(const PEER_MINMAX& sourceMinMax)
{
	this->ChannelInfo.SourceMinMax = sourceMinMax;
	this->ChannelInfo.GetSourceMinMaxTime = time_counter::get_system_count32();
}


CLiveInfo* CreateLiveInfo(LiveStatisticsInfo& statsInfo, ppl::os::memory_mapping& mmf, const TCHAR* tagname, UINT maxPeerCount, const LiveAppModuleCreateParam& param, UINT appVersion, bool tagMutate)
{
	VIEW_INFO("CreateLiveInfo " << param.ChannelGUID << " " << param.SysInfo->PeerGUID << " " << tagname);
	tstring mmfName = tagname + FormatGUID(param.ChannelGUID);
	if (tagMutate)
	{
		//mmfName += strings::format(_T("%d"), ::GetCurrentProcessId());
	}
	const size_t mmf_size = sizeof(LIVE_INFO);
	if (false == mmf.create(mmfName.c_str(), mmf_size) )
	{
		APP_ERROR("CreateLiveInfo: failed to create memory mapping " << mmfName);
		return NULL;
	}
	if (false == mmf.map_all())
	{
		APP_ERROR("CreateLiveInfo: failed to map memory mapping " << mmfName);
		return NULL;
	}
	memset(mmf.get_view(), 0, mmf_size);
	CLiveInfo* liveInfo = reinterpret_cast<CLiveInfo*>(mmf.get_view());
	liveInfo->Init(param, appVersion);

	mmfName += strings::format(_T("%dSI"), ppl::os::process::current_process_id());
	statsInfo.Create( mmfName.c_str() );
	return liveInfo;
}






void InitPeerAuthInfo(PeerAuthInfo& authInfo, const LiveAppModuleCreateParam& param)
{
	authInfo.CookieType = param.CookieType;
	authInfo.CookieValue = param.CookieValue;
	authInfo.Username = param.Username;
	authInfo.Password = param.Password;
	APP_ERROR("Cookie = " << make_tuple(authInfo.CookieType, authInfo.CookieValue) << make_tuple(authInfo.Username, authInfo.Password));
}

void InitPeerNetInfo(PEER_NET_INFO& netInfo, const SYSINFO& sysInfo)
{
//	netInfo.NetType.Province = sysInfo.NetType.Area;
//	netInfo.NetType.ISP = sysInfo.NetType.ISP;
//	netInfo.NetType.Country = sysInfo.NetType.Region;
//	netInfo.NetType.City = sysInfo.NetType.Unit;
	netInfo.CoreInfo.Clear();
	netInfo.Address.IP = sysInfo.LocalIPs[0];
	netInfo.Address.TcpPort = sysInfo.TcpPort;
	netInfo.Address.UdpPort = sysInfo.UdpPort;
}


void SaveFlowInfo(FLOW_INFO& flowInfo, const RateMeasure& measure)
{
	flowInfo.SetSingleFlow(static_cast<UINT>(measure.GetTotalBytes()), measure.GetRate());
}

void SaveFlowInfo(FLOW_STATISTICS& flowInfo, const FlowMeasure& measure)
{
	SaveFlowInfo(flowInfo.Download, measure.Download);
	SaveFlowInfo(flowInfo.Upload, measure.Upload);
}

inline void SaveLocalFlowInfo(FLOW_INFO& flowInfo, const RateMeasure& measure)
{
	flowInfo.SetFlow(static_cast<UINT>(measure.GetTotalBytes()), measure.GetRate());
}

void SaveLocalFlowInfo(FLOW_STATISTICS& flowInfo, const FlowMeasure& measure)
{
	SaveLocalFlowInfo(flowInfo.Download, measure.Download);
	SaveLocalFlowInfo(flowInfo.Upload, measure.Upload);
}



//#if defined(_PPL_PLATFORM_LINUX) || defined(_PPL_USE_ASIO)
//
//class LiveMediaServerListener : public MediaServerListener
//{
//public:
//	explicit LiveMediaServerListener(int netWriterMode) : m_NetWriterMode(netWriterMode) { }
//	virtual void NotifyGUI(UINT msg, WPARAM wp, LPARAM lp)
//	{
//	}
//	virtual int GetGraphMode()
//	{
//		return 0;
//	}
//	virtual int GetNetWriterMode()
//	{
//		return m_NetWriterMode;
//	}
//
//private:
//	int m_NetWriterMode;
//};
//
//MediaServerListener* CreateMediaServerListener(int netWriterMode)
//{
//	return new LiveMediaServerListener(netWriterMode);
//}
//
//#elif defined(_PPL_PLATFORM_MSWIN)

class LiveMediaServerListener : public MediaServerListener
{
public:
	explicit LiveMediaServerListener(const CLiveInfo& liveInfo) : m_liveInfo(liveInfo) { }
	virtual void NotifyGUI(UINT msg, WPARAM wp, LPARAM lp)
	{
#if defined(_PPL_PLATFORM_MSWIN)
		HWND hwnd = m_liveInfo.NotifyWindow;
		if (hwnd == NULL)
			return;
		//if (::PostMessage(hwnd, msg, wp, lp))
			return;
		APP_ERROR("NotifyLiveGUI failed, message=" << msg << ", param=" << make_tuple(wp, lp) << " errcode=" << ::GetLastError());
#endif
	}
	virtual int GetGraphMode()
	{
		return m_liveInfo.GraphMode;
	}
	virtual int GetNetWriterMode()
	{
		return 0;
	}

private:
	const CLiveInfo& m_liveInfo;
};

MediaServerListener* CreateMediaServerListener(const CLiveInfo& liveInfo)
{
	return new LiveMediaServerListener(liveInfo);
}

//#else
//
//#error "invalid platform for LiveMediaServerListener"
//
//#endif




class LivePeerNetInfo : public PeerNetInfo
{
public:
	explicit LivePeerNetInfo(const SYSINFO& sysInfo) : m_sysInfo(sysInfo)
	{
		this->Address.TcpPort = sysInfo.TcpPort;
		this->Address.UdpPort = sysInfo.UdpPort;
		this->Load(m_sysInfo.LocalIPs, m_sysInfo.LocalIPCount);
	}

	virtual PORT_PAIR GetOuterPorts() const
	{
		PORT_PAIR ports;
		if (this->IsUPNPEnabled())
		{
			ports.TCPPort = m_sysInfo.ExtenalTcpPort;
			ports.UDPPort = m_sysInfo.ExtenalUdpPort;
			this->m_HasUPNPBeenEnabledForTracker = true;
		}
		else
		{
			ports = this->Address.GetPorts();
		}
		return ports;
	}

	virtual bool IsUPNPEnabled() const
	{
		// upnp映射有一个成功就启用
		return m_sysInfo.ExtenalTcpPort > 0 || m_sysInfo.ExtenalUdpPort > 0;
	}

	virtual void CheckPeerNetType()
	{
		if ( 0 == this->m_DetectedIP )
		{
			this->CoreInfo.PeerNetType = PNT_INVALID;
		}
		else if ( this->IsExternalIP() )
		{
			this->CoreInfo.PeerNetType = PNT_OUTER;
		}
		else if ( this->IsUPNPEnabled() || this->IsFullConeNAT() || this->IsPublicNAT() )
		{
			this->CoreInfo.PeerNetType = PNT_UPNP;
		}
		else
		{
			this->CoreInfo.PeerNetType = PNT_INNER;
		}
	}

	virtual PEER_ADDRESS GetUPNPAddress() const
	{
		LIVE_ASSERT( this->IsUPNPEnabled() );
		PEER_ADDRESS upnpAddress;
		upnpAddress.IP = this->m_OuterAddress.IP;
		upnpAddress.TcpPort = this->m_sysInfo.ExtenalTcpPort;
		upnpAddress.UdpPort = this->m_sysInfo.ExtenalUdpPort;
		LIVE_ASSERT( upnpAddress.IsFullyValid() );
		return upnpAddress;
	}



private:
	const SYSINFO& m_sysInfo;
};

PeerNetInfo* CreatePeerNetInfo(const SYSINFO& sysInfo)
{
	return new LivePeerNetInfo(sysInfo);
}




#ifdef NEED_LOG

#include <ppl/os/paths.h>
#include <ppl/util/test_case.h>
#include <ppl/net/asio/log_service.h>

class LogLoader
{
	typedef std::map<tstring, int> PredefiendCollection;
public:
	LogLoader()
	{
		m_predefined[_T("log.core")] = 0;

		m_predefined[_T("util")] = PPL_LOG_TYPE_UTIL;
		m_predefined[_T("app")] = PPL_LOG_TYPE_APP;
		m_predefined[_T("testcase")] = PPL_LOG_TYPE_TESTCASE;
		m_predefined[_T("udpt")] = PPL_LOG_TYPE_UDPT;
		m_predefined[_T("view")] = PPL_LOG_TYPE_VIEW;

		m_predefined[_T("upload")] = PPL_LOG_TYPE_UPLOAD;

		m_predefined[_T("core")] = CORE;
		m_predefined[_T("network")] = NETWORK;
		m_predefined[_T("framework")] = FRAMEWORK;
		m_predefined[_T("peer")] = PEER;
		m_predefined[_T("source")] = SOURCE;
		m_predefined[_T("tracker")] = TRACKER;
		m_predefined[_T("ippool")] = IPPOOL;
		m_predefined[_T("udpdetect")] = UDPDETECT;
		m_predefined[_T("peerconnection")] = CONNECTION;
		m_predefined[_T("peermanager")] = PEERMANAGER;
		m_predefined[_T("streambuffer")] = PPL_LOG_TYPE_STREAMBUFFER;
		m_predefined[_T("mediaserver")] = MEDIASERVER;
		m_predefined[_T("netwriter")] = NETWRITER;

		m_predefined[_T("reader")] = READER;
		m_predefined[_T("wmfsdkreader")] = WMFSDKREADER;
		m_predefined[_T("asffilereader")] = ASFFILEREADER;
		m_predefined[_T("rmfffilereader")] = RMFFFILEREADER;
		m_predefined[_T("asfhttpreader")] = ASFHTTPREADER;
		m_predefined[_T("asfmmshreader")] = ASFMMSHREADER;
		m_predefined[_T("asfmmstreader")] = ASFMMSTREADER;
		m_predefined[_T("asfrtspreader")] = ASFRTSPREADER;
		m_predefined[_T("rmffhttpreader")] = RMFFHTTPREADER;
		m_predefined[_T("rmffrtspreader")] = RMFFRTSPREADER;
		STL_FOR_EACH_CONST(PredefiendCollection, m_predefined, iter)
		{
			const tstring& s = iter->first;
			LIVE_ASSERT(!strings::is_upper(s));
		}
	}

	void Load(const tstring& baseDir)
	{
		tstring iniPath = ppl::os::paths::combine( baseDir, _T("paramsd.ini") );
		m_ini.set_filename(iniPath);
		m_ini.set_section(_T("log"));
		int logLevel = m_ini.get_int(_T("log.level"), -1);
		if (logLevel != -1)
		{
//			CORE_SetLogLevel(logLevel); 
			log_service::instance().set_log_level(logLevel);
		}
		tstring logOn = m_ini.get_string(_T("log.on"), _T(""));
		this->SetLogString(logOn, true);
		tstring logOff = m_ini.get_string(_T("log.off"), _T(""));
		this->SetLogString(logOn, true);
		STL_FOR_EACH_CONST(PredefiendCollection, m_predefined, iter)
		{
			int isOnOrOff = m_ini.get_int(iter->first.c_str(), -1);
			if (isOnOrOff == 1)
			{
				this->SetPredefined(iter->first, true);
			}
			else if (isOnOrOff == 0)
			{
				this->SetPredefined(iter->first, false);
			}
		}
	}

	void SetLogString(const tstring& logstr, bool isOnOrOff)
	{
		std::list<tstring> logs;
		strings::split(std::back_inserter(logs), logstr, _T(','));
		STL_FOR_EACH_CONST(std::list<tstring>, logs, iter)
		{
			tstring item = strings::trim(*iter);
			this->SetLog(item, isOnOrOff);
		}
	}
	void SetLog(const tstring& logstr, bool isOnOrOff)
	{
		if (logstr.empty())
			return;
		int val = _ttoi(logstr.c_str());
		if (val > 0)
		{
			this->SetLog(val, isOnOrOff);
		}
		else
		{
			this->SetPredefined(logstr, isOnOrOff);
		}
	}
	void SetPredefined(const tstring& logstr, bool isOnOrOff)
	{
		tstring s = strings::lower(logstr);
		PredefiendCollection::const_iterator iter = m_predefined.find(s);
		if (iter != m_predefined.end())
		{
			this->SetLog(iter->second, isOnOrOff);
		}
	}
	void SetLog(int logType, bool isOnOrOff)
	{
		if (isOnOrOff)
		{
			log_service::instance().log_on(logType);
		}
		else
		{
			log_service::instance().log_off(logType);
		}
	}

private:
	ini_file m_ini;
	PredefiendCollection m_predefined;
};

void LoadLogSetting(const tstring& baseDir)
{
//	CORE_LogOn( PPL_LOG_TYPE_APP );
//	CORE_LogOn( PPL_LOG_TYPE_UPLOAD );
//	CORE_LogOff(CORE);
//	CORE_LogOff( NETWORK );
//	CORE_LogOff( FRAMEWORK );

//	CORE_LogOff( CONNECTION );
//	CORE_LogOff( PEERMANAGER );
//	CORE_LogOff( TRACKER );
//	CORE_LogOff( SOURCE );
//	CORE_LogOff( PEER );
//	CORE_LogOff( IPPOOL );
//	CORE_LogOff( MEDIASERVER );
//	CORE_LogOff( NETWRITER );

//	CORE_LogOff( READER );
//	CORE_LogOff( WMFSDKREADER );

//	CORE_LogOn( PPL_LOG_TYPE_VIEW );
	//CORE_SetLogLevel( __INFO );
	//CORE_SetLogLevel( __DEBUG );

	log_service::instance().set_log_level(__INFO);


	LogLoader loader;
	loader.Load(baseDir);
}

#else

void LoadLogSetting(const tstring& )
{
}

#endif
