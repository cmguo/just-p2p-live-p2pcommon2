
#ifndef _LIVE_P2PCOMMON2_NEW_COMMON_CLIENT_APPMODULE_H_
#define _LIVE_P2PCOMMON2_NEW_COMMON_CLIENT_APPMODULE_H_

/**
 * @file
 * @brief 包含视频直播的应用模块的实现类
 */


#include "common/AppModule.h"

#include "common/GloalTypes.h"
#include "pos/TrackerRequesterListener.h"
#include "util/flow.h"
#include "common/iptable.h"
#include "common/AppParam.h"
#include "framework/timer.h"

#include <synacast/protocol/DataCollecting.h>
#include <synacast/protocol/StatisticsInfo.h>
#include <synacast/protocol/DataIO.h>
#include <synacast/protocol/nat.h>

#include <ppl/net/socketfwd.h>
#include <ppl/util/random.h>
#include <ppl/data/guid.h>

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>


class SecureTrackerProxyPacketBuilder;
class SecureTrackerRequestPacketBuilder;
class UDPPacketBuilder;
class UDPSender;
typedef boost::shared_ptr<UDPSender> UDPSenderPtr;

class StunModule;

enum ExtraProxyEnum;
class PeerStatusInfo;
class PeerAuthInfo;
class PeerNetInfo;
class UDPSender;
class CUdpDetect;
class TrackerRequester;
class CStreamBuffer;
class CPeerManager;
class CIPPool;
class CMediaServer;
class MediaServerListener;
struct SECURE_TRACKER_PACKET_HEAD;


class TCPConnectionlessPacketBuilder;
class UDPConnectionlessPacketBuilder;

class SecureTrackerProxyPacketBuilder;

struct NEW_UDP_PACKET_HEAD;
struct OLD_PACKET_COMMON_HEAD;
typedef std::multimap<UINT, TRACKER_LOGIN_ADDRESS> SortedTrackerAddressCollection;


class LiveAppModuleImpl;
class BootModule;
class StunTypeResolver;
class LogClient;


class AppModuleStatistics : public APP_STATS
{
public:
	/// 总共的收包信息(下载)
	FlowMeasure TotalFlow;


	/// 各种sender对应的流量
	FlowMeasure OldUDPFlow; //? 跟TrackerFlow重复
	FlowMeasure TCPConnectionlessFlow;
	FlowMeasure TCPConnectionFlow;
	FlowMeasure UDPConnectionlessFlow;
	FlowMeasure UDPConnectionFlow;

	AppModuleStatistics()
	{
		this->Clear();
	}

	void SyncFlow()
	{
		this->TotalFlow.SyncTo( this->TotalTraffic );

		this->OldUDPFlow.SyncTo( this->OldUDPTraffic );
		this->TCPConnectionFlow.SyncTo( this->TCPConnectionTraffic );
		this->TCPConnectionlessFlow.SyncTo( this->TCPConnectionlessTraffic );
		this->UDPConnectionFlow.SyncTo( this->UDPConnectionTraffic );
		this->UDPConnectionlessFlow.SyncTo( this->UDPConnectionlessTraffic );
	}

	void UpdateFlow()
	{
		this->TotalFlow.Update();
		this->OldUDPFlow.Update();
		this->TCPConnectionFlow.Update();
		this->TCPConnectionlessFlow.Update();
		this->UDPConnectionFlow.Update();
		this->UDPConnectionlessFlow.Update();
	}
};



/// 视频直播的AppModule的实现类
class LiveAppModuleImpl : public AppModule, protected TrackerRequesterListener
{
	typedef boost::function<void (int)> ClientErrorCallback;
public:
	explicit LiveAppModuleImpl(const LiveAppModuleCreateParam& param, const TCHAR* tagname, bool tagMutate = false);
	~LiveAppModuleImpl();

	//virtual UINT CalcAddressDistance(UINT32 targetIP) const;

	void OnPlayerBuffering(bool isBufferOK);

	virtual boost::shared_ptr<PeerInformation> GetPeerInformation() { return m_PeerInformation; }

	virtual void RecordDownload(size_t size)
	{
		m_totalFlow.Download.Record((UINT)size);
	}

	virtual CUdpDetect& GetUDPDetect() { return *m_UdpDetect; }

	/// 获取当前最大允许的连接数
	size_t GetMaxConnectionCount() const;
	size_t GetMaxConnectionCount2() const;
	size_t GetMinConnectionCount() const; // Added by Tady, 082908: To Limit kick-connection.

	virtual GUID GetResourceGUID() const;
	GUID GetChannelGUID() const;

	void OnNATQuerySucceeded(MY_STUN_NAT_TYPE natType);


	/// 启动模块
	void Start(const LiveAppModuleCreateParam& param);

	virtual void InitIni(ini_file& ini);
	virtual void InitIniFile(ini_file& ini, const TCHAR* filename);

	virtual void InitConfig(ini_file& ini);
	virtual void InitConfigFile(ini_file& ini, const TCHAR* filename);

	//virtual size_t EnumCandidatePeers(UINT32 remoteDetectedIP, CANDIDATE_PEER_INFO peers[], size_t count) const;

	/// 获取peer管理器
	virtual PeerManager& GetPeerManager() { return *m_Manager; }

	virtual CStreamBuffer& GetStreamBuffer() { return *m_streamBuffer; }
	virtual TrackerRequester& GetTracker() { return *m_tracker; }

	/// 获取peer池
	virtual CIPPool& GetIPPool() { return *m_ipPool; }

	virtual StunModule* GetStunModule() { return m_StunModule.get(); }




	//virtual bool SendUDPPacket(UINT ip, UINT16 port, const PacketBase& packet) { return this->SendPacket(ip, port, PROXY_UDP, packet); }

	virtual const PeerStatusInfo& GetStatusInfo() const;

	virtual const PeerNetInfo& GetNetInfo() const;

	virtual const PeerAuthInfo& GetAuthInfo() const;

	/// 更新mds和vip
	virtual void UpdateMDSVIP(const PeerAddressArray& mds, const PeerAddressCollection& vip);
	/// 加载配置
	virtual void LoadConfiguration();
	/// 更新tracker列表
	void UpdateTrackers(const std::vector<TRACKER_LOGIN_ADDRESS>& trackers);

	void OnSocketAccept(tcp_socket_ptr newClient, const InetSocketAddress& addr);

	virtual bool CheckPieceIndexValid(UINT pieceIndex) const;

	//virtual bool SendPacket(UINT ip, UINT16 port, BYTE proxyType, const PacketBase& packet);

	void StartStunModule(const std::vector<InetSocketAddress>& stunServers);

	virtual const LiveAppModuleCreateParam& GetAppCreateParam() const { return m_CreateParam; }
	
protected:
	/// 启动模块
	virtual void DoStart(const LiveAppModuleCreateParam& param);

	/// 创建各个功能组件
	virtual void DoCreateComponents() = 0;

	/// 创建Tracker管理对象
	virtual TrackerRequester* DoCreateTracker() = 0;


	virtual void OnAppTimer();

	/// 加载MDS列表到mds pool中
	void LoadMDS(const PeerAddressArray& mds);
	/// 加载tracker列表到live_info中
	void LoadTracker(const std::vector<TRACKER_LOGIN_ADDRESS>& trackers);
	void RandomlyAppendTracker(std::vector<TRACKER_LOGIN_ADDRESS>& trackers, RandomGenerator& rnd);
	bool SortTrackerAddress(SortedTrackerAddressCollection& sortedTrackers, const std::vector<TRACKER_LOGIN_ADDRESS>& trackers);
	void AddSortedTrackerAddress(const SortedTrackerAddressCollection& sortedTrackers);

	virtual void OnPeerListed(const TRACKER_ADDRESS& trackerAddr, UINT8 count, const CANDIDATE_PEER_INFO addrs[], UINT8 lanPeerCount, const INNER_CANDIDATE_PEER_INFO lanPeers[], UINT64 sourceTimeStamp);

	virtual void OnLoginComplete(const TRACKER_ADDRESS& addr, bool success);

	virtual void SaveDetectedAddress(UINT32 detectedIP, UINT16 detectedUDPPort, const TRACKER_ADDRESS& trackerAddr);

	virtual bool SaveSourceMinMax(const PEER_MINMAX& sourceMinMax);

	virtual bool SaveVIPMinMax(const PEER_MINMAX& vipMinMax);
	void DoSaveSourceMinMax(const PEER_MINMAX& sourceMinMax);

	void StartTracker();

	/// 尝试处理peer-proxy消息
	bool TryHandlePeerProxyMessage( data_input_stream& is, const NEW_UDP_PACKET_HEAD& head, const InetSocketAddress& addr, UINT8 proxyType );

	void OnNTSListOK(const std::vector<InetSocketAddress>& servers);

	void OnStunLogin();
	void OnStunTransmit(BYTE* data, size_t bytes, const InetSocketAddress& remoetAddr);

public:
	/// 派发udp消息
	bool DispatchUdpPacket(BYTE* data, int size, const InetSocketAddress& addr, UINT proxyType, ExtraProxyEnum extraProxy);
	/// Added by Tady, 082509
	void SetClientErrorCallback(ClientErrorCallback cbHandler) { m_cbClientError = cbHandler; }

private:
	/// 更新TrackerRequester模块的统计数据和状态信息到共享内存
	void UpdateTrackerRequesterStatistics(UINT seconds);


	MY_STUN_NAT_TYPE LoadNATType();

protected:
	const LiveAppModuleCreateParam m_CreateParam;

	/// dll模块对象
//	Module m_dllModule;

	/// LIVE_INFO的共享内存
	ppl::os::memory_mapping m_MappingLiveInfo;
	LiveStatisticsInfo m_StatisticsInfo;



	/// tracker地址列表
	std::vector<TRACKER_LOGIN_ADDRESS> m_trackerAddressList;

	UDPSenderPtr m_NormalUDPSender;
	UDPSenderPtr m_TCPProxyUDPSender;
	UDPSenderPtr m_HTTPProxyUDPSender;

	boost::scoped_ptr<MediaServerListener> m_mediaServerListener;

	/// peer地址池(相当于路由表)
	boost::scoped_ptr<CIPPool> m_ipPool;

	/// 流数据缓冲区
	boost::scoped_ptr<CStreamBuffer> m_streamBuffer;

	/// Tracker客户端管理模块
	boost::scoped_ptr<TrackerRequester> m_tracker;

	/// 洪泛探测模块
	boost::scoped_ptr<CUdpDetect> m_UdpDetect;

	/// 媒体服务器
	boost::scoped_ptr<CMediaServer> m_mediaServer;

	/// Peer管理模块
	boost::scoped_ptr<PeerManager> m_Manager;

	FlowMeasure m_trackerFlow;
	FlowMeasure m_detectFlow;
	FlowMeasure m_otherUDPFlow;
	FlowMeasure m_totalFlow;

	/// 整个模块的主timer
	periodic_timer m_timer;

	/// 自动调节的最大连接个数
	size_t m_AutoMaxAppPeerCount;
	size_t m_AutoMinAppPeerCount; // Added by Tady, 082908: To limit kick.
	/// ip表
	IPTable m_iptable;

	boost::shared_ptr<UDPPacketBuilder> m_OldTrackerPacketBuilder;
	boost::shared_ptr<TCPConnectionlessPacketBuilder> m_TCPConnectionlessPacketBuilder;
	boost::shared_ptr<UDPConnectionlessPacketBuilder> m_UDPConnectionlessPacketBuilder;
	boost::shared_ptr<SecureTrackerProxyPacketBuilder> m_SecureTrackerProxyPacketBuilder;
	boost::shared_ptr<SecureTrackerRequestPacketBuilder> m_SecureUDPPacketBuilder;

	AppModuleStatistics m_Statistics;

	boost::shared_ptr<BootModule> m_BootModule;


private:
	/// peer个数限制(物理限制，主要是live_info中的容量限制)
	size_t m_PeerCountLimit;

	/// 用于设置直播模块的随机数种子，不能删除！！！
	OldRandomSeed m_RandomSeed;

	boost::shared_ptr<StunModule> m_StunModule;
	boost::shared_ptr<StunTypeResolver> m_StunTypeResolver;

	boost::scoped_ptr<LogClient> m_LogClient;

	time_counter m_LogDataSaveTime;

	ClientErrorCallback m_cbClientError;
};




/// 服务端的AppModule，实现mcc/mds模块的基本功能
class ServerLiveAppModule : public LiveAppModuleImpl
{
public:
	explicit ServerLiveAppModule(const LiveAppModuleCreateParam& param, const TCHAR* tagname, bool tagMutate = false);
	~ServerLiveAppModule();

protected:
	virtual void UpdateMDSVIP(const PeerAddressArray& mds, const PeerAddressCollection& vip);

	virtual void DoStart(const LiveAppModuleCreateParam& param);
};



/// 客户端的AppModule，实现peer/SimpleMDS模块的基本功能
class ClientLiveAppModule : public LiveAppModuleImpl
{
public:
	explicit ClientLiveAppModule(const LiveAppModuleCreateParam& param, const TCHAR* mmfPrefix = TEXT("OS_SYS_META"), bool tagMutate = false);
	~ClientLiveAppModule();

protected:
	virtual void UpdateMDSVIP(const PeerAddressArray& mds, const PeerAddressCollection& vip);
};
#endif
