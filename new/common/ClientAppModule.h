
#ifndef _LIVE_P2PCOMMON2_NEW_COMMON_CLIENT_APPMODULE_H_
#define _LIVE_P2PCOMMON2_NEW_COMMON_CLIENT_APPMODULE_H_

/**
 * @file
 * @brief ������Ƶֱ����Ӧ��ģ���ʵ����
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
	/// �ܹ����հ���Ϣ(����)
	FlowMeasure TotalFlow;


	/// ����sender��Ӧ������
	FlowMeasure OldUDPFlow; //? ��TrackerFlow�ظ�
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



/// ��Ƶֱ����AppModule��ʵ����
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

	/// ��ȡ��ǰ��������������
	size_t GetMaxConnectionCount() const;
	size_t GetMaxConnectionCount2() const;
	size_t GetMinConnectionCount() const; // Added by Tady, 082908: To Limit kick-connection.

	virtual GUID GetResourceGUID() const;
	GUID GetChannelGUID() const;

	void OnNATQuerySucceeded(MY_STUN_NAT_TYPE natType);


	/// ����ģ��
	void Start(const LiveAppModuleCreateParam& param);

	virtual void InitIni(ini_file& ini);
	virtual void InitIniFile(ini_file& ini, const TCHAR* filename);

	virtual void InitConfig(ini_file& ini);
	virtual void InitConfigFile(ini_file& ini, const TCHAR* filename);

	//virtual size_t EnumCandidatePeers(UINT32 remoteDetectedIP, CANDIDATE_PEER_INFO peers[], size_t count) const;

	/// ��ȡpeer������
	virtual PeerManager& GetPeerManager() { return *m_Manager; }

	virtual CStreamBuffer& GetStreamBuffer() { return *m_streamBuffer; }
	virtual TrackerRequester& GetTracker() { return *m_tracker; }

	/// ��ȡpeer��
	virtual CIPPool& GetIPPool() { return *m_ipPool; }

	virtual StunModule* GetStunModule() { return m_StunModule.get(); }




	//virtual bool SendUDPPacket(UINT ip, UINT16 port, const PacketBase& packet) { return this->SendPacket(ip, port, PROXY_UDP, packet); }

	virtual const PeerStatusInfo& GetStatusInfo() const;

	virtual const PeerNetInfo& GetNetInfo() const;

	virtual const PeerAuthInfo& GetAuthInfo() const;

	/// ����mds��vip
	virtual void UpdateMDSVIP(const PeerAddressArray& mds, const PeerAddressCollection& vip);
	/// ��������
	virtual void LoadConfiguration();
	/// ����tracker�б�
	void UpdateTrackers(const std::vector<TRACKER_LOGIN_ADDRESS>& trackers);

	void OnSocketAccept(tcp_socket_ptr newClient, const InetSocketAddress& addr);

	virtual bool CheckPieceIndexValid(UINT pieceIndex) const;

	//virtual bool SendPacket(UINT ip, UINT16 port, BYTE proxyType, const PacketBase& packet);

	void StartStunModule(const std::vector<InetSocketAddress>& stunServers);

	virtual const LiveAppModuleCreateParam& GetAppCreateParam() const { return m_CreateParam; }
	
protected:
	/// ����ģ��
	virtual void DoStart(const LiveAppModuleCreateParam& param);

	/// ���������������
	virtual void DoCreateComponents() = 0;

	/// ����Tracker�������
	virtual TrackerRequester* DoCreateTracker() = 0;


	virtual void OnAppTimer();

	/// ����MDS�б�mds pool��
	void LoadMDS(const PeerAddressArray& mds);
	/// ����tracker�б�live_info��
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

	/// ���Դ���peer-proxy��Ϣ
	bool TryHandlePeerProxyMessage( data_input_stream& is, const NEW_UDP_PACKET_HEAD& head, const InetSocketAddress& addr, UINT8 proxyType );

	void OnNTSListOK(const std::vector<InetSocketAddress>& servers);

	void OnStunLogin();
	void OnStunTransmit(BYTE* data, size_t bytes, const InetSocketAddress& remoetAddr);

public:
	/// �ɷ�udp��Ϣ
	bool DispatchUdpPacket(BYTE* data, int size, const InetSocketAddress& addr, UINT proxyType, ExtraProxyEnum extraProxy);
	/// Added by Tady, 082509
	void SetClientErrorCallback(ClientErrorCallback cbHandler) { m_cbClientError = cbHandler; }

private:
	/// ����TrackerRequesterģ���ͳ�����ݺ�״̬��Ϣ�������ڴ�
	void UpdateTrackerRequesterStatistics(UINT seconds);


	MY_STUN_NAT_TYPE LoadNATType();

protected:
	const LiveAppModuleCreateParam m_CreateParam;

	/// dllģ�����
//	Module m_dllModule;

	/// LIVE_INFO�Ĺ����ڴ�
	ppl::os::memory_mapping m_MappingLiveInfo;
	LiveStatisticsInfo m_StatisticsInfo;



	/// tracker��ַ�б�
	std::vector<TRACKER_LOGIN_ADDRESS> m_trackerAddressList;

	UDPSenderPtr m_NormalUDPSender;
	UDPSenderPtr m_TCPProxyUDPSender;
	UDPSenderPtr m_HTTPProxyUDPSender;

	boost::scoped_ptr<MediaServerListener> m_mediaServerListener;

	/// peer��ַ��(�൱��·�ɱ�)
	boost::scoped_ptr<CIPPool> m_ipPool;

	/// �����ݻ�����
	boost::scoped_ptr<CStreamBuffer> m_streamBuffer;

	/// Tracker�ͻ��˹���ģ��
	boost::scoped_ptr<TrackerRequester> m_tracker;

	/// �鷺̽��ģ��
	boost::scoped_ptr<CUdpDetect> m_UdpDetect;

	/// ý�������
	boost::scoped_ptr<CMediaServer> m_mediaServer;

	/// Peer����ģ��
	boost::scoped_ptr<PeerManager> m_Manager;

	FlowMeasure m_trackerFlow;
	FlowMeasure m_detectFlow;
	FlowMeasure m_otherUDPFlow;
	FlowMeasure m_totalFlow;

	/// ����ģ�����timer
	periodic_timer m_timer;

	/// �Զ����ڵ�������Ӹ���
	size_t m_AutoMaxAppPeerCount;
	size_t m_AutoMinAppPeerCount; // Added by Tady, 082908: To limit kick.
	/// ip��
	IPTable m_iptable;

	boost::shared_ptr<UDPPacketBuilder> m_OldTrackerPacketBuilder;
	boost::shared_ptr<TCPConnectionlessPacketBuilder> m_TCPConnectionlessPacketBuilder;
	boost::shared_ptr<UDPConnectionlessPacketBuilder> m_UDPConnectionlessPacketBuilder;
	boost::shared_ptr<SecureTrackerProxyPacketBuilder> m_SecureTrackerProxyPacketBuilder;
	boost::shared_ptr<SecureTrackerRequestPacketBuilder> m_SecureUDPPacketBuilder;

	AppModuleStatistics m_Statistics;

	boost::shared_ptr<BootModule> m_BootModule;


private:
	/// peer��������(�������ƣ���Ҫ��live_info�е���������)
	size_t m_PeerCountLimit;

	/// ��������ֱ��ģ�����������ӣ�����ɾ��������
	OldRandomSeed m_RandomSeed;

	boost::shared_ptr<StunModule> m_StunModule;
	boost::shared_ptr<StunTypeResolver> m_StunTypeResolver;

	boost::scoped_ptr<LogClient> m_LogClient;

	time_counter m_LogDataSaveTime;

	ClientErrorCallback m_cbClientError;
};




/// ����˵�AppModule��ʵ��mcc/mdsģ��Ļ�������
class ServerLiveAppModule : public LiveAppModuleImpl
{
public:
	explicit ServerLiveAppModule(const LiveAppModuleCreateParam& param, const TCHAR* tagname, bool tagMutate = false);
	~ServerLiveAppModule();

protected:
	virtual void UpdateMDSVIP(const PeerAddressArray& mds, const PeerAddressCollection& vip);

	virtual void DoStart(const LiveAppModuleCreateParam& param);
};



/// �ͻ��˵�AppModule��ʵ��peer/SimpleMDSģ��Ļ�������
class ClientLiveAppModule : public LiveAppModuleImpl
{
public:
	explicit ClientLiveAppModule(const LiveAppModuleCreateParam& param, const TCHAR* mmfPrefix = TEXT("OS_SYS_META"), bool tagMutate = false);
	~ClientLiveAppModule();

protected:
	virtual void UpdateMDSVIP(const PeerAddressArray& mds, const PeerAddressCollection& vip);
};
#endif
