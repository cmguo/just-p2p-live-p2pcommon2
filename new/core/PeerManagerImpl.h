
#ifndef _LIVE_P2PCOMMON2_NEW_CORE_PEER_MANAGER_IMPL_H_
#define _LIVE_P2PCOMMON2_NEW_CORE_PEER_MANAGER_IMPL_H_



#include "common/PeerManager.h"
#include "common/PeerManagerStatistics.h"
//#include <synacast/protocol/UDPPacketInfo.h>

#include "framework/memory.h"

#include <ppl/data/guid_less.h>

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <map>
#include <set>

class ini_file;
class HuffmanCoding;
class CPeerInfoItem;
class Downloader;
class Uploader;
class PeerConnector;
class CStreamBuffer;
class Storage;
class CIPPool;
class PeerNetInfo;
class PeerStatusInfo;

class TCPPeerConnectionInfo;
class UDPPeerConnectionInfo;
class PPHuffmanAnnounce;
struct REDIRECT_PEER_INFO;
class PeerInformation;

class PeerConnectParam;
struct REDIRECT_PEER_INFO;


struct FLOW_INFO;


/// ����첽��tcp���ӺͶ�д������ǽ��ͬ����ʽ��������⣬��Ϊʹ��udp��ʽ����p2p����
class TCPSyncProblemChecker : private boost::noncopyable
{
public:
	explicit TCPSyncProblemChecker(CLiveInfo& liveInfo, int peerType) : m_liveInfo(liveInfo), m_peerType(peerType)
	{
	}
	~TCPSyncProblemChecker();
	bool Check();
private:
	CLiveInfo& m_liveInfo;
	time_counter m_connectTime;
	int m_peerType;
};



class HistoricalRateMeasure
{
public:
        UINT MaximumSpeed;	
        RateMeasure Rate;

	HistoricalRateMeasure() : MaximumSpeed(0), Rate(10) // Added by Tady, 100708: The peak value is Not True.
	{
	}

	bool Record(size_t size);

	void Reset()
	{
		this->MaximumSpeed = 0;
	}
	void Update()
	{
		this->Rate.Update();
	}
};

class PeerConnectionConfig
{
public:
	UINT MaybeFailedPieceCount;
	/// ��λ����
	UINT MaxIdleTime;

	/// �ٷֱ�
	UINT PrelocateDensePosition;

	/// �ٷֱ�
	UINT SourceMinMaxReferRangePercent;
	UINT SourceMinMaxReferGap;

	PeerConnectionConfig();

	void Load(ini_file& ini);
};


class UploaderConfig
{
public:
	/// ����ϴ��ٶȣ������ϴ�����(��λ��KB/s�������ļ���ΪKb/s)
	/// ����ϴ��ٶ�Ϊ0����ʾ�ϴ��ٶ���Ч�������ϴ�SubPiece�ϴ���������
	/// Ĭ��ֵΪ0�����������ٶ�����
	UINT MaxUploadSpeed;

	/// ��ʼ��ÿ��sub-piece����ϴ�����
	/// �������ϴ��ٶ�Ϊ0���Ͱ��մ�������
	/// Ĭ��ֵ1000����1000���ϴ�
	UINT InitialMaxUploadTimesPerSubPiece;

	UploaderConfig();

	void Load(ini_file& ini);
};

class PeerTunnelConfig
{
public:
	UINT UsedTime;
	UINT UdpWindowSizeMin;
	UINT TcpWindowSizeMin;
	UINT UdpWindowSizeMax;
	UINT TcpWindowSizeMax;
	UINT TcpWindowSizeCal;
	UINT UdpIntTimeOutMin;
	UINT UdpIntTimeOutMax;
	UINT UdpUseArraySize;
	UINT ExTimeOutLevel;
	UINT PieceTCPPeerTunnelUsedTime;

	PeerTunnelConfig();	
	
	void Load(ini_file& ini);

};


/// ������ص�������
class PeerConnectorConfig
{
public:
	/// �ӳٷ������ӵ�ʱ�䣬��λ�ǵ�λʱ��Ĵ���
	UINT DelayTimes;

	/// ���ʱ�䣬��ʾ���ٸ���λʱ��(1/4��)
	UINT Interval;

	/// ÿ����෢������Ӹ���
	int MaxConnectionInitiateEveryTime;

	/// �̵����ӳ�ʱʱ�䣬��λ����
	UINT ShortConnectionTimeout;

	/// �еȵ����ӳ�ʱʱ�䣬��λ����
	UINT NormalConnectionTimeout;

	/// �������ӳ�ʱʱ�䣬��λ����
	UINT LongConnectionTimeout;

	/// vip�����ӳ�ʱʱ�䣬��λ����
	UINT VIPConnectionTimeout;

	/// ���ֳ�ʱ����λ������
	UINT HandshakeTimeout;

	/// udpt���ֳ�ʱ����λ������
	UINT UDPHandshakeTimeout;

	/// �տ�ʼʱ�෢��һЩ����
	UINT StartConnectIncrement;

	/// �Ƿ�ܾ�udpt��������
	bool IsRefuseUDPConnectionRequest;

	/// �Ƿ�ֻ����mds
	bool IfOnlyAcceptMDS;
	bool DisableDirectConnection;

	PeerConnectorConfig();

	void Load(ini_file& ini);
};


/// p2p-research�õ�������
class P2PResearchConfig
{
public:
	/// ����������ֹ���ӵĽڵ�
	PeerAddressCollection DeniedPeers;
	/// ����������ֹ���ӵĽڵ�ip
	std::set<UINT32> DeniedIPs;

	/// ���������������ӵĽڵ㣬���������ߵ�
	//PeerAddressCollection AllowedPeers;

	/// �������ӵĽڵ�
	PeerAddressCollection PreferredPeers;

	/// �趨���ϴ��ٶ�
	UINT16 FixedUploadSpeed;
	bool UseFixedUploadSpeed;
	/// p2p-research�õ����ӿ����ļ��ļ��ؼ��ʱ�䣨��λ���룩
	UINT ConnectionControllInterval;

	P2PResearchConfig() { Clear(); }
	void Clear();
	void Load( ini_file& ini );
};


/// ����PeerManager���ֵ�������Ϣ
class PeerManagerConfig
{
public:
	/// ���Ƶ���Դ�Ƚ��ܼ���λ�ã���λ���ٷֱȣ�0-100
	UINT DenseResourcePosition;

	/// ��������ٶ�
	UINT MaxDownloadSpeed;

	/// ����ʱ�Ĳο�peer����
	UINT StartupReferPeerCount;
	/// ����ʱ�Ĳο�ʱ��
	UINT StartupReferTime;

	/// ��λ��ʼ����λ��ʱ���max�ľ���
	UINT LocateFromMaxTimeSpan;

	UINT RequestStoppedTimeout;
	UINT PrelocateInterval;


	UINT AnnounceInterval;

	/// �����������ı�������λ���ٷֱȣ�0-100
	UINT ReservedDegree;
	UINT ReservedDegreeMin;
	UINT ReservedDegreeMax;

	UINT MinPeerCountForGoodStatus;
	UINT LANMinPeerCountForGoodStatus;
	/// ��λ����
	UINT InitialPeerProtectionTime;

	UINT MinListInterval;
	UINT MaxListInterval;

	UINT MaxOnceKickPeerCount;

	UINT WANMinLocalPeerCount;
	UINT WANMinLocalPeerPercent;
	UINT LANMinLocalPeerCount;

	UINT ConnectingPeerPercent;

	bool NoConnectOut;


	PeerConnectionConfig ConnectionConfig;
	UploaderConfig Uploader;
	PeerTunnelConfig TunnelConfig;

	/// ������ص�����
	PeerConnectorConfig ConnectorConfig;
	P2PResearchConfig ResearchConfig;

	PeerManagerConfig();

	void Load(ini_file& ini);

};


class AppModule;
class PeerConnection;
typedef std::set<PeerConnection*> PeerConnectionCollection;
typedef std::map<GUID, PeerConnection*> PeerConnectionIndex;
typedef std::map<UINT32, PeerConnection*> PeerSessionKeyIndex;

typedef std::multimap<PEER_ADDRESS, PeerConnection*> PeerConnectionAddressIndex;
typedef std::multimap<UINT32, PeerConnection*> PeerConnectionIPIndex;

class PeerConnectionInfo;
class PeerHandshakeInfo;
class tcp_socket;



/// ��С�����Ŀ������Ӹ���
const int MIN_RESERVED_DEGREE = 10;



/// peer��������ʵ����
class CPeerManager : public PeerManager, public pool_object
{
public:
	explicit CPeerManager(AppModule* lpPeerModule);
	virtual ~CPeerManager();

	virtual void OnPlayerBufferring( bool isBufferOK ) { assert(false); }

	virtual bool IsInputSourceIP( UINT ip ) const { return false; };

	//bool CheckIfNeedMultipleConnection(const PeerConnectionInfo& connInfo, const PeerHandshakeInfo& handshakeInfo) const;

	virtual size_t FillPeerAddresses(std::vector<PeerExchangeItem>& peers, size_t maxCount, UINT targetIP);

	virtual bool IsVIP(UINT ip) const;

	virtual AppModule& GetAppModule() { return m_PeerModule; }

	virtual void LoadVIP(const PeerAddressCollection& vips);

	/// ��ȡ������ص�����
	const PeerConnectorConfig& GetConnectorConfig() const { return m_config.ConnectorConfig; }

	/// ����ʱ��
	virtual void OnAppTimer(UINT times);
	
	virtual void DoAnnounce(PPHuffmanAnnounce* announcePacket);
	virtual void DoStatistics();

	virtual int GetMaxLocalPeerCount() const;

	virtual FlowMeasure& GetFlow() { return m_flow; }

	virtual UINT GetMaxExternalDownloadSpeed() const { return m_externalDownloadRate.MaximumSpeed; }
	virtual UINT GetMaxExternalUploadSpeed() const { return m_externalUploadRate.MaximumSpeed; }

	virtual UINT GetAvrgDownloadSpeed() const{ return /*m_statistics.DownloaderData.Flow.Download.GetRate();*/m_LongTimeDownloadFlow.GetRate(); }
	virtual UINT GetAvrgUploadSpeed() const { return /*m_statistics.UploaderData.Flow.Upload.GetRate();*/ m_LongTimeUploadFlow.GetRate(); }


	Downloader& GetDownloader() { return *m_downloader; }
	Uploader& GetUploader() { return *m_uploader; }

	virtual const PeerManagerStatistics& GetStatistics() const { return m_statistics; }

	PeerManagerStatistics& GetStatisticsData() { return m_statistics; }

	void NotifyDeletePeer(PeerConnection* conn, long errcode);

	void RecordUploadFlow(size_t size, UINT8 action, PeerConnection* pc);
	void RecordDownloadFlow(size_t size, UINT8 action, PeerConnection* pc);

	//void RecordDownloadMedia(size_t size)
	//{
	//	m_statistics.TotalDownloadMediaBytes += size;
	//}
	//void RecordUploadMedia(size_t size)
	//{
	//	m_statistics.TotalUploadMediaBytes += size;
	//}
	
	int GetStableConnectionCount() const;

	virtual void LoadInputSources( const PeerAddressArray &addresses ) {}

protected:
	static void RecordFlow(size_t size, UINT8 action, const PeerConnection* pc, RateMeasure& flow, RateMeasure& protocolFlow, HistoricalRateMeasure& externalFlow, FLOW_INFO& flowInfo);

public:
	HuffmanCoding& GetHuffmanCoding() { return *m_HuffmanCoding; }
	DEGREE_INFO GetDegreeInfo() const { return m_statistics.Degrees.ToSimple(); }
	int GetDegreeLeft() const;

	size_t GetBestPeersForRedirect(std::vector<REDIRECT_PEER_INFO>& peers, size_t maxCount, UINT targetIP) const;


	/// ��������������ʷ��������ٶ�����
	void ResetMaxExternalSpeed()
	{
		m_externalDownloadRate.Reset();
		// �ϴ��ٶȵ���ʷ��¼����
		//m_externalUploadRate.Reset();
	}

	/// ����һ��peer
	PeerConnection* AddUDPTPeer(boost::shared_ptr<UDPPeerConnectionInfo> connInfo, const PeerHandshakeInfo& handshakeInfo);


	/// ����һ��peer
	bool AddPeer(boost::shared_ptr<TCPPeerConnectionInfo> connInfo, const PeerHandshakeInfo& handshakeInfo);

	/// ��ȡ���Ӽ���
	const PeerConnectionCollection& GetConnections() const { return m_Connections; }


	/// ȷ���Ƿ���Ҫ���������ط�������
	virtual bool NeedData() const { return true; }

	/// �ж�һ�������Ƿ���Ҫ��������Դλͼ���������յ���announce�����н�ѹ��BitMap
	virtual bool NeedResource(const PeerConnection* pc) const;

	/// source������������ֻ���л����Ĳ���
	virtual bool DoStart();

	/// ֹͣ
	bool Stop();

	/// ���peer�Ƿ��Ѿ�����
	virtual bool IsPeerConnected(const GUID& peerGuid, bool isVIP, bool isInitFromRemote) const;

	/// ����IP�Ͷ˿���Ϣ������ͬ��Peer
	bool CheckConnected( UINT Ip, unsigned short TcpPort, bool isVip ) const;

	/// ����������Ϣ
	virtual void LoadConfiguration();

	/// ��ȡ������Ϣ
	const PeerManagerConfig& GetConfig() const { return m_config; }

	/// �ж�һ�������Ƿ���������������ݣ�������MDS��ֻ��vip���Ӳſ�����������
	virtual bool CanDownload(const PeerConnection* conn) const { return true; } 

	/// ���PeerConnection�ĸ���
	int GetPeerCount() const;
	int GetLocalPeerCount() const;
	int GetRemotePeerCount() const;



	virtual bool HandleUDPPacket( data_input_stream& is, const NEW_UDP_PACKET_HEAD& head, const PACKET_PEER_INFO& packetPeerInfo, const SimpleSocketAddress& sockAddr, ExtraProxyEnum extraProxy );
	virtual bool HandleUDPSessionPacket(data_input_stream& is, const NEW_UDP_PACKET_HEAD& head, const UDP_SESSION_INFO& sessionInfo, const SimpleSocketAddress& sockAddr);

	/// �Ƿ���Խ���ĳ��ip������������
	virtual bool CanAcceptConnection( UINT ip ) const;

	/// ����Ƿ���Խ�����������
	virtual UINT32 CheckAcceptHandshake(bool isVIP, UINT32 externalIP, const PeerHandshakeInfo& handshakeInfo) const
	{
		// Ĭ�Ͽ��Խ�����������
		return 0;
	}

	/// ��ȡ��ʼ�����
	double GetInitialQuota(bool isVip) const;

	const RateMeasure& GetLongTimeUploadFlow() const { return m_LongTimeUploadFlow; }
	const RateMeasure& GetLongTimeDownloadFlow() const { return m_LongTimeDownloadFlow; }

	/// �Ƿ�ܾ�tcp��������
	virtual bool IfRefuseTCPConnectionRequest() const { return false; }

	UINT GetConnectTimeout() const { return m_ConnectTimeout; }

	virtual bool SaveLog(JsonWriter& writer, UINT32 detectedIP, UINT32& totalPeercount, UINT32& savedPeercount) const;

protected:
	/// ɾ�����е�һ��Peer
	bool DoDelPeer( PeerConnection* conn, UINT16 errcode );

	bool DelPeer( PeerConnection* conn, UINT16 errcode, const serializable* errorInfo = NULL );

	/// ����GUID������ͬ��Peer
	//bool CheckConnected( const GUID& PeerGuid, bool isVip ) const;
	bool HasPeer(const GUID& peerGUID) const
	{
		return m_connectionIndex.find(peerGUID) != m_connectionIndex.end();
	}

	/// Ѱ��g_PeerInfo����ʹ�õ�λ��
	CPeerInfoItem* FindUnusedPeerInfoItem();

	/// ��ȡAnnounce�ļ��ʱ��
	virtual UINT GetAnnounceInterval() const;


	/// ͬ��RemotePeerCount��Ϣ
	void SyncConnectionInfo();


	/// �ж�һ�������Ƿ���Ա��ߵ�����vip���Ӳ��ᱻ�ϵ�,
	///! ������Ӧ����VIP���棬����������Բ�����飬������û��Ǽ��һ��
	virtual bool CanKick(const PeerConnection* conn) const;

	/// ��ȡ����ظ����Ӹ���
	virtual int GetMaxRepeatedConnectionCount(bool isVip) const;


	/// ��AdjustPeerConnection�лᱻ���ã�����MDS��صĲ���������Ƿ���Ҫ����MDS���߶Ͽ���MDS������
	virtual void CheckMDS() { }

	/// ����Ƿ���Ҫ����
	virtual void ListPeers() { }

	/// ��������
	virtual void InitiateConnections(UINT times, UINT maxTCPPendingCount, UINT maxConnectionCount) { }

	/// ������
	virtual void KickConnections(UINT times) { }

	/// ��ȡ���ı����������Ӹ���
	int GetMaxReservedDegree() const;

	/// ��������pfps��������ش���
	virtual void CalcMaxPFPSBandWidth(UINT seconds) { }


	virtual void OnSocketAccept(boost::shared_ptr<tcp_socket> newClient, const InetSocketAddress& addr);

	/// ���һ�������õ�peer����
	virtual void DoAddPeer(PeerConnection* conn);

	/// ����MDS
	void DialMDS();

	/// ����ָ����Peers
	void DialPeers(const PeerAddressCollection& peers);
	void DialPeer(const PEER_ADDRESS& peerAddr);

	/// Connect to Spark. Added by Tady, 011611
	void DialSpark(const PEER_ADDRESS& peerAddr, int connectFlags);
	void DialSparks();

	/// p2p-research�����ӿ���
	void ControllConnections();

	/// ��Զ��������������
	virtual bool ConnectToRemote(const PEER_ADDRESS& addr, const PeerConnectParam& param);

	/// ������
	void KickPeer( PeerConnection* pc, UINT32 reason );

	void DenyPeers( );
	size_t DenyPeer( const PEER_ADDRESS& addr );
	size_t DenyPeer( UINT32 ip );

	void Validate()
	{
		assert(m_IPIndex.size() == m_Connections.size());
		assert(m_connectionIndex.size() == m_Connections.size());
		assert(m_AddressIndex.size() == m_Connections.size());
		assert(m_SessionKeyIndex.size() <= m_Connections.size());
		assert(m_SessionKeyIndex.size() <= static_cast<size_t>( m_statistics.Degrees.UDPT.GetTotal() ));
	}

protected:
	/// ������AppModule
	AppModule& m_PeerModule;

	CStreamBuffer& m_streamBuffer;

	const Storage& m_storage;

	CIPPool& m_ipPool;

	boost::shared_ptr<const PeerNetInfo> m_NetInfo;
	boost::shared_ptr<PeerStatusInfo> m_StatusInfo;

	/// peer���ӽ�����
	boost::scoped_ptr<PeerConnector> m_connector;
	/// piece����ģ��
	boost::scoped_ptr<Downloader> m_downloader;
	boost::scoped_ptr<Uploader> m_uploader;

	boost::scoped_ptr<PPHuffmanAnnounce> m_huffmanAnnouncePacket;

	/// ������Ϣ
	FlowMeasure m_flow;

	/// Э��������Ϣ
	FlowMeasure m_protocolFlow;

	HistoricalRateMeasure m_externalDownloadRate;
	HistoricalRateMeasure m_externalUploadRate;

	enum PeerManagerState
	{
		st_none,		//δ����
		//st_connecting,	//��������Peer��
		st_running		//������
	} m_state;


	/// Peer���ӵļ���
	PeerConnectionCollection m_Connections;
	std::vector<PeerConnection*> m_sparkConnections;
	/// ��guidΪkey������
	PeerConnectionIndex m_connectionIndex;

	/// ��guidΪkey������
	PeerSessionKeyIndex m_SessionKeyIndex;

	/// ��peer addressΪkey������
	PeerConnectionAddressIndex m_AddressIndex;
	PeerConnectionIPIndex m_IPIndex;

	/// ��ini�м��ص�һЩ������Ϣ
	PeerManagerConfig m_config;



	PeerManagerStatistics m_statistics;

	/// VIP��IP��ַ�б�
	std::set<UINT32> m_vips;

	boost::shared_ptr<PeerInformation> m_PeerInformation;

	RateMeasure m_LongTimeUploadFlow;
	RateMeasure m_LongTimeDownloadFlow;

	UINT m_ConnectTimeout;

	/// �ϴμ������ӿ����ļ���ʱ��
	time_counter m_LastConnectionControllTime;

	boost::shared_ptr<HuffmanCoding> m_HuffmanCoding;
};


#endif
