
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


/// 检查异步的tcp连接和读写被防火墙以同步方式处理的问题，改为使用udp方式进行p2p传输
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
	/// 单位：秒
	UINT MaxIdleTime;

	/// 百分比
	UINT PrelocateDensePosition;

	/// 百分比
	UINT SourceMinMaxReferRangePercent;
	UINT SourceMinMaxReferGap;

	PeerConnectionConfig();

	void Load(ini_file& ini);
};


class UploaderConfig
{
public:
	/// 最大上传速度，用于上传限速(单位：KB/s，配置文件中为Kb/s)
	/// 最大上传速度为0，表示上传速度无效，按照上传SubPiece上传次数限制
	/// 默认值为0，即不按照速度限制
	UINT MaxUploadSpeed;

	/// 初始的每个sub-piece最大上传次数
	/// 如果最大上传速度为0，就按照次数限制
	/// 默认值1000，即1000倍上传
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


/// 连接相关的配置项
class PeerConnectorConfig
{
public:
	/// 延迟发起连接的时间，单位是单位时间的次数
	UINT DelayTimes;

	/// 间隔时间，表示多少个单位时间(1/4秒)
	UINT Interval;

	/// 每次最多发起的连接个数
	int MaxConnectionInitiateEveryTime;

	/// 短的连接超时时间，单位：秒
	UINT ShortConnectionTimeout;

	/// 中等的连接超时时间，单位：秒
	UINT NormalConnectionTimeout;

	/// 长的连接超时时间，单位：秒
	UINT LongConnectionTimeout;

	/// vip的连接超时时间，单位：秒
	UINT VIPConnectionTimeout;

	/// 握手超时，单位：毫秒
	UINT HandshakeTimeout;

	/// udpt握手超时，单位：毫秒
	UINT UDPHandshakeTimeout;

	/// 刚开始时多发起一些链接
	UINT StartConnectIncrement;

	/// 是否拒绝udpt连接请求
	bool IsRefuseUDPConnectionRequest;

	/// 是否只允许mds
	bool IfOnlyAcceptMDS;
	bool DisableDirectConnection;

	PeerConnectorConfig();

	void Load(ini_file& ini);
};


/// p2p-research用的配置项
class P2PResearchConfig
{
public:
	/// 黑名单：禁止连接的节点
	PeerAddressCollection DeniedPeers;
	/// 黑名单：禁止连接的节点ip
	std::set<UINT32> DeniedIPs;

	/// 白名单：允许连接的节点，不会主动踢掉
	//PeerAddressCollection AllowedPeers;

	/// 优先连接的节点
	PeerAddressCollection PreferredPeers;

	/// 设定的上传速度
	UINT16 FixedUploadSpeed;
	bool UseFixedUploadSpeed;
	/// p2p-research用的连接控制文件的加载间隔时间（单位：秒）
	UINT ConnectionControllInterval;

	P2PResearchConfig() { Clear(); }
	void Clear();
	void Load( ini_file& ini );
};


/// 用于PeerManager部分的配置信息
class PeerManagerConfig
{
public:
	/// 估计的资源比较密集的位置，单位：百分比，0-100
	UINT DenseResourcePosition;

	/// 最高下载速度
	UINT MaxDownloadSpeed;

	/// 启动时的参考peer个数
	UINT StartupReferPeerCount;
	/// 启动时的参考时间
	UINT StartupReferTime;

	/// 定位起始下载位置时相对max的距离
	UINT LocateFromMaxTimeSpan;

	UINT RequestStoppedTimeout;
	UINT PrelocateInterval;


	UINT AnnounceInterval;

	/// 保留连接数的比例，单位：百分比，0-100
	UINT ReservedDegree;
	UINT ReservedDegreeMin;
	UINT ReservedDegreeMax;

	UINT MinPeerCountForGoodStatus;
	UINT LANMinPeerCountForGoodStatus;
	/// 单位：秒
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

	/// 连接相关的配置
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



/// 最小保留的空闲连接个数
const int MIN_RESERVED_DEGREE = 10;



/// peer管理器的实现类
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

	/// 获取连接相关的配置
	const PeerConnectorConfig& GetConnectorConfig() const { return m_config.ConnectorConfig; }

	/// 主定时器
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


	/// 将外网流量的历史最大下载速度清零
	void ResetMaxExternalSpeed()
	{
		m_externalDownloadRate.Reset();
		// 上传速度的历史记录保留
		//m_externalUploadRate.Reset();
	}

	/// 增加一个peer
	PeerConnection* AddUDPTPeer(boost::shared_ptr<UDPPeerConnectionInfo> connInfo, const PeerHandshakeInfo& handshakeInfo);


	/// 增加一个peer
	bool AddPeer(boost::shared_ptr<TCPPeerConnectionInfo> connInfo, const PeerHandshakeInfo& handshakeInfo);

	/// 获取连接集合
	const PeerConnectionCollection& GetConnections() const { return m_Connections; }


	/// 确定是否需要接收其它地方的数据
	virtual bool NeedData() const { return true; }

	/// 判断一个连接是否需要解析其资源位图－－即从收到的announce报文中解压缩BitMap
	virtual bool NeedResource(const PeerConnection* pc) const;

	/// source的启动函数，只进行基本的操作
	virtual bool DoStart();

	/// 停止
	bool Stop();

	/// 检查peer是否已经连接
	virtual bool IsPeerConnected(const GUID& peerGuid, bool isVIP, bool isInitFromRemote) const;

	/// 依据IP和端口信息查找相同的Peer
	bool CheckConnected( UINT Ip, unsigned short TcpPort, bool isVip ) const;

	/// 加载配置信息
	virtual void LoadConfiguration();

	/// 获取配置信息
	const PeerManagerConfig& GetConfig() const { return m_config; }

	/// 判断一个连接是否可以用来下载数据－－对于MDS，只有vip连接才可以用来下载
	virtual bool CanDownload(const PeerConnection* conn) const { return true; } 

	/// 获得PeerConnection的个数
	int GetPeerCount() const;
	int GetLocalPeerCount() const;
	int GetRemotePeerCount() const;



	virtual bool HandleUDPPacket( data_input_stream& is, const NEW_UDP_PACKET_HEAD& head, const PACKET_PEER_INFO& packetPeerInfo, const SimpleSocketAddress& sockAddr, ExtraProxyEnum extraProxy );
	virtual bool HandleUDPSessionPacket(data_input_stream& is, const NEW_UDP_PACKET_HEAD& head, const UDP_SESSION_INFO& sessionInfo, const SimpleSocketAddress& sockAddr);

	/// 是否可以接收某个ip来的连接请求
	virtual bool CanAcceptConnection( UINT ip ) const;

	/// 检查是否可以接受握手请求
	virtual UINT32 CheckAcceptHandshake(bool isVIP, UINT32 externalIP, const PeerHandshakeInfo& handshakeInfo) const
	{
		// 默认可以接受握手请求
		return 0;
	}

	/// 获取初始的配额
	double GetInitialQuota(bool isVip) const;

	const RateMeasure& GetLongTimeUploadFlow() const { return m_LongTimeUploadFlow; }
	const RateMeasure& GetLongTimeDownloadFlow() const { return m_LongTimeDownloadFlow; }

	/// 是否拒绝tcp连接请求
	virtual bool IfRefuseTCPConnectionRequest() const { return false; }

	UINT GetConnectTimeout() const { return m_ConnectTimeout; }

	virtual bool SaveLog(JsonWriter& writer, UINT32 detectedIP, UINT32& totalPeercount, UINT32& savedPeercount) const;

protected:
	/// 删除已有的一个Peer
	bool DoDelPeer( PeerConnection* conn, UINT16 errcode );

	bool DelPeer( PeerConnection* conn, UINT16 errcode, const serializable* errorInfo = NULL );

	/// 依据GUID查找相同的Peer
	//bool CheckConnected( const GUID& PeerGuid, bool isVip ) const;
	bool HasPeer(const GUID& peerGUID) const
	{
		return m_connectionIndex.find(peerGUID) != m_connectionIndex.end();
	}

	/// 寻找g_PeerInfo可以使用的位置
	CPeerInfoItem* FindUnusedPeerInfoItem();

	/// 获取Announce的间隔时间
	virtual UINT GetAnnounceInterval() const;


	/// 同步RemotePeerCount信息
	void SyncConnectionInfo();


	/// 判断一个连接是否可以被踢掉－－vip连接不会被断掉,
	///! 输入流应该在VIP里面，所以这里可以不做检查，不过最好还是检查一下
	virtual bool CanKick(const PeerConnection* conn) const;

	/// 获取最大重复连接个数
	virtual int GetMaxRepeatedConnectionCount(bool isVip) const;


	/// 在AdjustPeerConnection中会被调用，进行MDS相关的操作，检查是否需要连接MDS或者断开到MDS的连接
	virtual void CheckMDS() { }

	/// 检查是否需要发起
	virtual void ListPeers() { }

	/// 发起连接
	virtual void InitiateConnections(UINT times, UINT maxTCPPendingCount, UINT maxConnectionCount) { }

	/// 踢连接
	virtual void KickConnections(UINT times) { }

	/// 获取最大的保留空闲连接个数
	int GetMaxReservedDegree() const;

	/// 计算用于pfps的最大下载带宽
	virtual void CalcMaxPFPSBandWidth(UINT seconds) { }


	virtual void OnSocketAccept(boost::shared_ptr<tcp_socket> newClient, const InetSocketAddress& addr);

	/// 添加一个创建好的peer连接
	virtual void DoAddPeer(PeerConnection* conn);

	/// 连接MDS
	void DialMDS();

	/// 连接指定的Peers
	void DialPeers(const PeerAddressCollection& peers);
	void DialPeer(const PEER_ADDRESS& peerAddr);

	/// Connect to Spark. Added by Tady, 011611
	void DialSpark(const PEER_ADDRESS& peerAddr, int connectFlags);
	void DialSparks();

	/// p2p-research的连接控制
	void ControllConnections();

	/// 向远端主机发起连接
	virtual bool ConnectToRemote(const PEER_ADDRESS& addr, const PeerConnectParam& param);

	/// 踢连接
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
	/// 所属的AppModule
	AppModule& m_PeerModule;

	CStreamBuffer& m_streamBuffer;

	const Storage& m_storage;

	CIPPool& m_ipPool;

	boost::shared_ptr<const PeerNetInfo> m_NetInfo;
	boost::shared_ptr<PeerStatusInfo> m_StatusInfo;

	/// peer连接接受器
	boost::scoped_ptr<PeerConnector> m_connector;
	/// piece下载模块
	boost::scoped_ptr<Downloader> m_downloader;
	boost::scoped_ptr<Uploader> m_uploader;

	boost::scoped_ptr<PPHuffmanAnnounce> m_huffmanAnnouncePacket;

	/// 流量信息
	FlowMeasure m_flow;

	/// 协议流量信息
	FlowMeasure m_protocolFlow;

	HistoricalRateMeasure m_externalDownloadRate;
	HistoricalRateMeasure m_externalUploadRate;

	enum PeerManagerState
	{
		st_none,		//未启动
		//st_connecting,	//正在连接Peer中
		st_running		//已启动
	} m_state;


	/// Peer连接的集合
	PeerConnectionCollection m_Connections;
	std::vector<PeerConnection*> m_sparkConnections;
	/// 以guid为key的索引
	PeerConnectionIndex m_connectionIndex;

	/// 以guid为key的索引
	PeerSessionKeyIndex m_SessionKeyIndex;

	/// 以peer address为key的索引
	PeerConnectionAddressIndex m_AddressIndex;
	PeerConnectionIPIndex m_IPIndex;

	/// 从ini中加载的一些配置信息
	PeerManagerConfig m_config;



	PeerManagerStatistics m_statistics;

	/// VIP的IP地址列表
	std::set<UINT32> m_vips;

	boost::shared_ptr<PeerInformation> m_PeerInformation;

	RateMeasure m_LongTimeUploadFlow;
	RateMeasure m_LongTimeDownloadFlow;

	UINT m_ConnectTimeout;

	/// 上次加载连接控制文件的时间
	time_counter m_LastConnectionControllTime;

	boost::shared_ptr<HuffmanCoding> m_HuffmanCoding;
};


#endif
