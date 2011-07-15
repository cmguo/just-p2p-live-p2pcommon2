
#ifndef _LIVE_P2PCOMMON2_NEW_COMMON_PEER_MANAGER_H_
#define _LIVE_P2PCOMMON2_NEW_COMMON_PEER_MANAGER_H_

#include <synacast/protocol/DataIO.h>
#include <boost/shared_ptr.hpp>
#include <vector>
#include <set>
#include "common/GloalTypes.h"
class InetSocketAddress;
class tcp_socket;
class AppModule;
class PeerConnection;

class FlowMeasure;
class CLiveInfo;
class CDetailedPeerInfo;
struct DEGREE_INFO;

struct UDP_PEER_PACKET_HEAD;
struct NEW_UDP_PACKET_HEAD;
struct UDPT_SESSION_HEAD;
struct UDP_SESSION_INFO;
struct PACKET_PEER_INFO;
struct NEW_UDP_PACKET_HEAD;

class PeerExchangeItem;

class PeerConnectorStatistics;
class PeerManagerStatistics;

class SimpleSocketAddress;
class PeerItem;
enum ExtraProxyEnum;
struct PEER_ADDRESS;

class JsonWriter;

typedef std::vector<PEER_ADDRESS> PeerAddressArray;
typedef std::set<PEER_ADDRESS> PeerAddressCollection;

/// peer管理器的接口类
class PeerManager
{
public:
	PeerManager()
	{
	}
	virtual ~PeerManager()
	{
	}


public:
	/// 启动函数，只进行基本的操作
	bool Start()
	{
		return DoStart();
	}

	virtual void OnPlayerBufferring( bool isBufferOK ) = 0;
	
	/// 加载输入源地址列表
	virtual void LoadInputSources( const PeerAddressArray &addresses ) = 0;

	/// 加载VIP地址列表
	virtual void LoadVIP(const PeerAddressCollection& addresses) = 0;

	/// 获取AppModule
	virtual AppModule& GetAppModule() = 0;

	/// 获取流量信息
	virtual FlowMeasure& GetFlow() = 0;

	/// 获取外网的历史最高下载速度
	virtual UINT GetMaxExternalDownloadSpeed() const = 0;
	virtual UINT GetMaxExternalUploadSpeed() const = 0;
	/// Added by Tady, 082908: 
	virtual UINT GetAvrgDownloadSpeed() const = 0;
	virtual UINT GetAvrgUploadSpeed() const = 0;

	/// 加载配置信息
	virtual void LoadConfiguration() = 0;

	/// 主定时器
	virtual void OnAppTimer(UINT times) = 0;

	/// 获取peer个数
	virtual int GetPeerCount() const = 0;

	/// 填充peer地址列表
	virtual size_t FillPeerAddresses(std::vector<PeerExchangeItem>& peers, size_t maxCount, UINT targetIP) = 0;

	/// 处理udpt报文
	virtual bool HandleUDPPacket( data_input_stream& is, const NEW_UDP_PACKET_HEAD& head, const PACKET_PEER_INFO& packetPeerInfo, const SimpleSocketAddress& sockAddr, ExtraProxyEnum extraProxy ) = 0;
	virtual bool HandleUDPSessionPacket( data_input_stream& is, const NEW_UDP_PACKET_HEAD& head, const UDP_SESSION_INFO& sessionInfo, const SimpleSocketAddress& sockAddr ) = 0;

	virtual void OnSocketAccept(boost::shared_ptr<tcp_socket> newClient, const InetSocketAddress& addr) = 0;

	virtual const PeerManagerStatistics& GetStatistics() const = 0;
	//virtual const PeerConnectorStatistics& GetConnectorStatistics() const = 0;

	/// 代替udp detect的udp连接
	virtual bool ConnectForDetect(const PeerItem& peer) = 0;

	virtual bool SaveLog(JsonWriter& writer, UINT32 detectedIP, UINT32& totalPeercount, UINT32& savedPeercount) const = 0;

protected:
	/// Start的实现
	virtual bool DoStart() = 0;

};



/// PeerManager的类工厂
class PeerManagerFactory
{
public:
	/// 创建用于peer端的PeerManager
	static PeerManager* PeerCreate(AppModule* owner);
	/// 创建用于mcc端的PeerManager
	static PeerManager* MCCCreate(AppModule* owner);
	/// 创建用于mds端的PeerManager
	static PeerManager* MDSCreate(AppModule* owner);
	/// 创建用于SimpleMDS端的PeerManager
	static PeerManager* SimpleMDSCreate(AppModule* owner);
};

#endif


