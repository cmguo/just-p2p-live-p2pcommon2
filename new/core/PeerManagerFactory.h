
#ifndef _LIVE_P2PCOMMON2_NEW_CORE_PEER_MANAGER_FACTORY_H_
#define _LIVE_P2PCOMMON2_NEW_CORE_PEER_MANAGER_FACTORY_H_

#include "PeerManagerImpl.h"
#include <synacast/protocol/data/PeerAddress.h>
#include <map>
#include <set>

class Downloader;
class PeerConnector;





/// 客户端用的peer管理器
class ClientPeerManager : public CPeerManager
{
public:
	explicit ClientPeerManager(AppModule* lpPeerModule);
	virtual ~ClientPeerManager();

	virtual bool ConnectForDetect(const PeerItem& peer);

	virtual void OnPlayerBufferring( bool isBufferOK );

protected:
	virtual bool DoStart();

	virtual void InitiateConnections(UINT times, UINT maxTCPPendingCount, UINT maxConnectionCount);
	virtual void ListPeers();
	virtual void KickConnections(UINT seconds);
			void KickConnections2(UINT seconds);
	virtual void CheckMDS();

	/// peer连接状态是否有效
	bool IsStatusOK() const;

	/// 连接局域网内的peer
	void DialLAN();


	/// 是否需要连接MDS
	bool NeedMDSSupport() const;

	virtual int GetMaxLocalPeerCount() const;


	virtual void CalcMaxPFPSBandWidth(UINT seconds);


	virtual void DoAddPeer(PeerConnection* conn);

private:

	/// 上次List操作的时间
	time_counter m_LastTickCountDoList;
	time_counter m_startTime;

	/// List操作的间隔时间
	UINT m_DoListInterval;
	
	// max data time. Added by Tady, 091108
	UINT m_maxPrepaDataTime;
	time_counter m_lastTickDoConnect;
	bool m_bNeedConnectForDetect;
};






/// 用于Source端的PeerManager
class SourcePeerManager : public CPeerManager
{
public:
	SourcePeerManager(AppModule* owner) : CPeerManager(owner)
	{
	}

	/// source不需要从别的地方下载数据
	virtual bool NeedData() const { return false; }

	/// source也不需要其它peer的minmax信息
	virtual bool NeedResource(const PeerConnection* pc) const { return false; }

	virtual bool ConnectForDetect(const PeerItem& peer)
	{
		return false;
	}

protected:
	virtual bool CanDownload(const PeerConnection* conn) const;

	virtual bool CanAcceptConnection( UINT ip ) const;

	/// source需要尽量快的announce
	virtual UINT GetAnnounceInterval() const { return 1; }

	/// source需要不同的踢连接的策略
	virtual void KickConnections(UINT seconds);

	/// 进行实际的踢连接
	void DoKickConnections(UINT seconds);

	virtual int GetMaxRepeatedConnectionCount(bool isVip) const;

	/// 获取公网peer个数
	size_t GetExternalPeerCount() const;

	virtual UINT32 CheckAcceptHandshake(bool isVIP, UINT32 externalIP, const PeerHandshakeInfo& handshakeInfo) const;

	virtual void LoadConfiguration();

	virtual void CheckMDS()
	{
		// source增加主动发起连接的功能，以实现从内网向外推流的功能
		DialMDS();
	}

private:
	/// 踢内网peer
	void KickInternalPeers();

	/// 踢出现异常情况的peer
	void KickBadPeers();

};








/// 用于mds的连接配置信息
class MdsConfiguration
{
public:
	MdsConfiguration();


	void Load(ini_file& ini);
	void Save(ini_file& ini);

public:
	/// 最小服务时间
	UINT	MinServeTime;
	/// 最大服务时间
	UINT	MaxServeTime;
	/// 保留的连接比率
	UINT	ReservedDegreeRatio;
	/// 是否需要限制最大服务时间
	bool	NeedLimitMaxServeTime;
};



typedef std::multimap<UINT, PeerConnection*> PeerConnectionQOSCollection;

///! 内外网地址信息
struct PairedPeerAddress
{
	PairedPeerAddress(PEER_ADDRESS innerAddress, PEER_ADDRESS outterAddress) :
		InnerAddress( innerAddress ), OutterAddress( outterAddress )
		{
		}

	PEER_ADDRESS InnerAddress;
	PEER_ADDRESS OutterAddress;
};

inline bool operator<( const PairedPeerAddress & address1, const PairedPeerAddress address2 )
{
	if ( address1.OutterAddress != address2.OutterAddress )
	{
		return address1.OutterAddress < address2.OutterAddress;
	}

	return address1.InnerAddress < address2.InnerAddress;
}


typedef std::set<PairedPeerAddress> InputSourceCollection;

/// 用于SourceAgent端的PeerManager
class SourceAgentPeerManager : public ClientPeerManager
{
public:
	SourceAgentPeerManager(AppModule* owner);

	/// mds需要从别的地方下载数据
	virtual bool NeedData() const { return true; }
	virtual bool NeedResource(const PeerConnection* pc) const;

	virtual void LoadConfiguration();

	/// 加载输入源地址信息，MDS参数中，0以前的就是Publisher地址，地址成对，内网在前，外网在后
	virtual void LoadInputSources( const PeerAddressArray &addresses );

	/// 过滤掉输入源的连接
	virtual bool ConnectToRemote(const PEER_ADDRESS& addr, const PeerConnectParam& param);
	/// 不允许Kick输入源
	virtual bool CanKick(const PeerConnection* conn) const;

	virtual bool IsInputSourceIP( UINT ip ) const;

	virtual bool ConnectForDetect(const PeerItem& peer)
	{
		return false;
	}

protected:
	virtual void KickConnections(UINT seconds);
	virtual UINT GetAnnounceInterval() const { return 1; }
	virtual void CheckMDS();

	virtual void InitiateConnections(UINT times) { }
	virtual void ListPeers() { }

	virtual bool CanDownload(const PeerConnection* conn) const;
	virtual int GetMaxRepeatedConnectionCount(bool isVip) const;

	/// 检查所有的peer，踢掉超过最大服务时间的peer，其余多余的peer根据qos来排序
	size_t CheckExtraPeers(PeerConnectionQOSCollection& qosColl, size_t extraCount);
	/// 清除PeerConnectionQOSCollection中最多maxCount个peer
	size_t DeleteExtraPeers(const PeerConnectionQOSCollection& qosColl, size_t maxCount);
	/// 添加一个多余的peer到PeerConnectionQOSCollection中
	void AddExtraPeer(PeerConnectionQOSCollection& qosColl, PeerConnection* pc, size_t maxCount);

	/// 是否已经提供了过长时间的服务(超过最大服务时间)
	bool HasServedTooLongTime(DWORD serveTime) const;
	/// 是否需要继续服务(小于最小服务时间)
	bool NeedContinueServing(DWORD serveTime) const;

	/// 是否可以调整连接
	bool CanAdjustConnection();

	/// 计算需要删除的peer个数
	int CalcExtraCount() const;

	virtual bool CanAcceptConnection( UINT ip ) const { return true; }

	/// mds相关的配置
	MdsConfiguration m_mdsConfig;

	/// 输入源不允许主动连接，不允许发起数据请求，但是应该发送Announce，可以发送Error Leave
	InputSourceCollection m_InputSources;

private:
	bool IsInputSource( const PEER_ADDRESS & outterAddress ) const { return this->IsInputSourceIP(outterAddress.IP); };
	bool IsInputSource( UINT outIP, const PEER_ADDRESS & innerAddress ) const;

};

#endif