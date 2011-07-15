#ifndef _LIVE_P2PCOMMON2_NEW_COMMON_IPPool_H_
#define _LIVE_P2PCOMMON2_NEW_COMMON_IPPool_H_


struct PEER_ADDRESS;
struct CANDIDATE_PEER_INFO;
struct PACKET_PEER_INFO;
class SimpleSocketAddress;

//typedef set<PEER_ADDRESS> PeerAddressCollection;

class PeerItem;

#include "IPInfo.h"
#include <boost/shared_ptr.hpp>
#include <set>

#define BLOCKED_TIMEOUT 100*1000



class IPPoolStatistics;
class IPPoolConfig;

class PeerNetInfo;
class JsonWriter;
class PeerExchangeItem;


/// peer地址池
class CIPPool
{
public:
	CIPPool() { }

	virtual ~CIPPool() { }

	virtual void SetConfig(const IPPoolConfig& config) = 0;

	virtual void OnTimer(UINT seconds) = 0;

	/// 添加一个地址到IPPool以供探测
	virtual bool AddCandidate(const PEER_ADDRESS& addr, const PEER_CORE_INFO& coreInfo, CandidatePeerTypeEnum FromWhere) = 0;

	/// 添加一个地址到IPPool以供探测
	virtual bool AddCandidate(const CANDIDATE_PEER_INFO& addr, CandidatePeerTypeEnum FromWhere) { return false; }


	virtual bool AddInnerCandidate(const INNER_CANDIDATE_PEER_INFO& addr, CandidatePeerTypeEnum FromWhere) = 0;

	/// 添加一个地址到IPPool以供探测
	virtual void AddCandidate(size_t count, const CANDIDATE_PEER_INFO addr[], CandidatePeerTypeEnum FromWhere) = 0;

	/// 添加通过peer-exchange获取到的peers
	virtual void AddExchangedPeers(size_t count, const PeerExchangeItem* peers, CandidatePeerTypeEnum FromWhere) = 0;

	/// 添加一个探测过的
	virtual bool AddDetected(const CANDIDATE_PEER_INFO& pInfo, UINT DetectedRTT, bool isUDP) = 0;

	virtual bool AddEchoDetected(const CANDIDATE_PEER_INFO& pInfo, UINT DetectedRTT, bool isUDP) { return this->AddDetected(pInfo, DetectedRTT, isUDP); }

		
	/// 添加一个正在连接的
	virtual bool AddConnecting(const PEER_ADDRESS& addr) = 0;

	/// 添加一个连接成功的地址
	virtual bool AddConnected(const PEER_ADDRESS& addr, UINT16 realPort, UINT8 connectFlags, bool isLAN) = 0;

	/// 添加一个连接成功的地址
	virtual bool AddConnectFailed(const PEER_ADDRESS& addr, bool isTCP) = 0;

	/// 添加一个连接断开了的地址
	virtual bool AddDisconnected(const PEER_ADDRESS& addr, UINT connectionTime, long errcode, UINT downloadSpeed, UINT uploadSpeed, UINT64 downloadBytes, UINT64 uploadBytes) = 0;


	/// 获取地址以供探测
	virtual bool GetForHello(PeerItem& addr) = 0;

	/// 获取地址以供探测
	virtual bool GetForDetect(PeerItem& addr) = 0;

	/// 获取地址以供连接
	virtual bool GetForConnect(PeerItem& addr) = 0;

	/// 获取IPPool总的大小
	virtual size_t GetSize() const { return 0; }



	/// 获得外部IP的时候，根据外部IP设置所有IPPool节点的地域信息
	virtual void SetExternalIP(UINT externalIP) = 0;
	
	/// 设置成为 不可能连接的 节点
	virtual void AddCannotConnectNode( UINT ip, USHORT port ) = 0;

	/// 查找IPPool中存不存在这个信息
	virtual bool FindAddress(const PEER_ADDRESS& addr) = 0;

	/// 是否需要探测
	//virtual bool NeedDetect() = 0;

	/// 是否需要到tracker上获取peer
	virtual bool NeedDoList() = 0;

//	virtual bool AddConnectedIP(const PEER_ADDRESS& addr, UINT rtt) = 0;

	/// 获取统计数据
	virtual const IPPoolStatistics* GetStatistics() const = 0;

	virtual bool DeleteDuplicatedNAT(const SimpleSocketAddress& sockAddr, const PEER_ADDRESS& keyAddr, const PACKET_PEER_INFO& packetPeerInfo) = 0;

	virtual bool SaveLog(JsonWriter& writer, UINT32 detectedIP, UINT32& totalPeercount, UINT32& savedPeercount) const = 0;
};



/// 什么事情都不做的peer地址池，主要用于source、mds等模块
class TrivialIPPool : public CIPPool
{
public:
	virtual void SetConfig(const IPPoolConfig& config) { }
	virtual void OnTimer(UINT seconds) { }

	/// 添加一个地址到IPPool以供探测
	virtual bool AddCandidate(const PEER_ADDRESS& addr, const PEER_CORE_INFO& coreInfo, CandidatePeerTypeEnum FromWhere) { return false; }

	virtual bool AddInnerCandidate(const INNER_CANDIDATE_PEER_INFO& addr, CandidatePeerTypeEnum FromWhere) { return false; }

	/// 添加一个地址到IPPool以供探测
	virtual bool AddCandidate(const CANDIDATE_PEER_INFO& addr, CandidatePeerTypeEnum FromWhere) { return false; }

	/// 添加一个地址到IPPool以供探测
	virtual void AddCandidate(size_t count, const CANDIDATE_PEER_INFO addr[], CandidatePeerTypeEnum FromWhere) { }

	virtual void AddExchangedPeers(size_t count, const PeerExchangeItem* peers, CandidatePeerTypeEnum FromWhere) { }

	/// 添加一个探测过的
	virtual bool AddDetected(const CANDIDATE_PEER_INFO& pInfo, UINT DetectedRTT, bool isUDP) { return false; }

	/// 添加一个正在连接的
	virtual bool AddConnecting(const PEER_ADDRESS& addr) { return false; }

	/// 添加一个连接成功的地址
	virtual bool AddConnected(const PEER_ADDRESS& addr, UINT16 realPort, UINT8 connectFlags, bool isLAN) { return false; }

	/// 添加一个连接成功的地址
	virtual bool AddConnectFailed(const PEER_ADDRESS& addr, bool isTCP) { return false; }

	/// 添加一个连接断开了的地址
	virtual bool AddDisconnected(const PEER_ADDRESS& addr, UINT connectionTime, long errcode, UINT downloadSpeed, UINT uploadSpeed, UINT64 downloadBytes, UINT64 uploadBytes) { return false; }



	/// 获取地址以供探测
	virtual bool GetForHello(PeerItem& addr) { return false; }

	/// 获取地址以供探测
	virtual bool GetForDetect(PeerItem& addr) { return false; }

	/// 获取地址以供连接
	virtual bool GetForConnect(PeerItem& addr) { return false; }

	/// 获取IPPool总的大小
	virtual size_t GetSize() const { return 0; }



	/// 获得外部IP的时候，根据外部IP设置所有IPPool节点的地域信息
	virtual void SetExternalIP(UINT externalIP) { }
	
	/// 设置成为 不可能连接的 节点
	virtual void AddCannotConnectNode( UINT ip, USHORT port ) { }

	/// 查找IPPool中存不存在这个信息
	virtual bool FindAddress(const PEER_ADDRESS& addr) { return false; }

	/// 是否需要探测
	//virtual bool NeedDetect() { return false; }

	/// 是否需要到tracker上获取peer
	virtual bool NeedDoList() { return false; }

//	virtual bool AddConnectedIP(const PEER_ADDRESS& addr, UINT rtt) { return false; }

	/// 获取统计数据
	virtual const IPPoolStatistics* GetStatistics() const { return NULL; }

	virtual bool DeleteDuplicatedNAT(const SimpleSocketAddress& sockAddr, const PEER_ADDRESS& keyAddr, const PACKET_PEER_INFO& packetPeerInfo) { return false; }

	virtual bool SaveLog(JsonWriter& writer, UINT32 detectedIP, UINT32& totalPeercount, UINT32& savedPeercount) const { return false; }
};



class IPTable;


class IPPoolFactory
{
public:
	/// 创建什么事情都不做的IPPool
	static CIPPool* CreateTrivial()
	{
		return new TrivialIPPool();
	}

//	static CIPPool* PeerCreate(const PEER_ADDRESS& localAddress);
	static CIPPool* PeerCreateNetTyped(boost::shared_ptr<PeerNetInfo> netInfo, const tstring& baseDir, const std::set<PEER_ADDRESS>& mdsPool, IPTable& iptable);
//	static CIPPool* PeerCreate(AppModule* module);
};


#endif
