
#ifndef _LIVE_P2PCOMMON2_NEW_COMMON_NET_TYPED_IPPOOL_IMPL_H_
#define _LIVE_P2PCOMMON2_NEW_COMMON_NET_TYPED_IPPOOL_IMPL_H_


#include "IpPool.h"
#include "IPPoolStatistics.h"
#include "IPInfo.h"
#include <synacast/protocol/data/NetType.h>

#include <ppl/data/tstring.h>
#include <ppl/util/time_counter.h>

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <map>
#include <set>

struct DetectIndicator;
struct ConnectIndicator;

class NetTypedIPInfo;
class IPTable;
class PeerNetInfo;


inline bool CompareIndicator(UINT x, const PEER_ADDRESS& keyX, UINT y, const PEER_ADDRESS& keyY)
{
	if (x == y)
		return keyX < keyY;
	return x < y;
}


/// 洪泛相关的索引键
struct HelloIndicator
{
	PEER_ADDRESS Key;
	UINT AddressDistance;
	UINT HelloTimes;

	HelloIndicator(const PEER_ADDRESS& key, UINT _addressDistance, UINT _helloTimes) : AddressDistance(_addressDistance), HelloTimes(_helloTimes)
	{
		this->Key = key;
	}
};

inline bool operator<(const HelloIndicator& x, const HelloIndicator& y)
{
	if (x.HelloTimes == y.HelloTimes)
	{
		//return x.AddressDistance < y.AddressDistance;
		return CompareIndicator(x.AddressDistance, x.Key, y.AddressDistance, y.Key);
	}
	else
	{
		return x.HelloTimes < y.HelloTimes;
	}
}

/// 探测相关的索引键
struct DetectIndicator
{
	PEER_ADDRESS Key;
	UINT RTT;

	UINT AddressDistance;

	UINT LastDetectTime;

	UINT ContinuousDetectFailedTimes;

	CandidatePeerTypeEnum FromWhere;

	DetectIndicator( const PEER_ADDRESS& key, UINT _RTT, UINT _AddressDistance, UINT _LastDetectTime, UINT _ContinuousDetectFailedTimes, CandidatePeerTypeEnum _FromWhere )
	{
		this->Key = key;
		RTT = _RTT;
		AddressDistance = _AddressDistance;
		LastDetectTime = _LastDetectTime;
		ContinuousDetectFailedTimes = _ContinuousDetectFailedTimes;
		FromWhere = _FromWhere;
	}

	inline int GetRank() const
	{
//		if ( ContinuousDetectFailedTimes >= 2 )
//		{	
//			// 说明已经连续两次探测失败，说明其可探测性极差,故将其放在最后
//			return 5;
//		}
//		else if( LastDetectTime == 0 )
//		{	// 还没有探测 并且 不 从Tracker获得的)
//			return 20;
//		}
//		else
//		{	// 已经探测　不管探测成功，还是失败
//			return 10;
//		}
		return 10;
	}
};

/// 比较两个DetectIndicator，用于构造map
inline bool operator<(const DetectIndicator& x, const DetectIndicator& y)
{
//	if( x.GetRank() != y.GetRank() )
//	{	
//		return x.GetRank() > y.GetRank();
//	}
//	
//	int rank = x.GetRank();
//	if( rank == 40 )
//	{	// 探测成功的 RTT<200ms  按照RTT 有小到大排列
//		return x.RTT < y.RTT;
//	}
//	else if( rank == 30 || rank == 20 )
//	{	// 还没有探测 按照AD*1000 + Rand(1000) 重大到小排列
//		return x.AddressDistance > y.AddressDistance;
//	}
//	else if( rank == 10 || rank == 5 )
//	{	// 已经探测　(不管探测成功，还是失败) 按照上一次发起 Detect时间来
//		return x.LastDetectTime < y.LastDetectTime;
//	}
//	assert(0);
//	return true;
	//return x.LastDetectTime < y.LastDetectTime;
	return CompareIndicator(x.LastDetectTime, x.Key, y.LastDetectTime, y.Key);
}


/// 连接相关的索引键
struct ConnectIndicator
{
	PEER_ADDRESS Key;
	UINT RTT;

	UINT AddressDistance;

	UINT ConnectRank;

	UINT Speed;

	DWORD LastConnectTime;

	CandidatePeerTypeEnum FromWhere;

	ConnectIndicator(const PEER_ADDRESS& key, UINT _RTT, UINT _AddressDistance, UINT _ConnectRank, UINT _DownloadSeped, UINT _UploadSpeed, DWORD _LastConnectTime, CandidatePeerTypeEnum _FromWhere)
	{
		this->Key = key;
		RTT = _RTT;
		AddressDistance = _AddressDistance;
		ConnectRank = _ConnectRank;
		Speed = _DownloadSeped + _UploadSpeed;
		LastConnectTime = _LastConnectTime;
		FromWhere = _FromWhere;
	}
};

/// 比较两个ConnectIndicator，用于构造map
inline bool operator<(const ConnectIndicator& x, const ConnectIndicator& y)
{
	if( x.ConnectRank != y.ConnectRank )
	{	
		//return x.ConnectRank > y.ConnectRank;
		return CompareIndicator(y.ConnectRank, y.Key, x.ConnectRank, x.Key);
	}

	UINT ConnectRank = x.ConnectRank;
	if( ConnectRank == 90 )
	{	// 能够探测成功的节点
		assert( x.RTT > 0 && y.RTT > 0);
		//return x.RTT < y.RTT;
		return CompareIndicator(x.RTT, x.Key, y.RTT, y.Key);
	}
	else if( ConnectRank == 50 || ConnectRank == 20 )
	{	// 不能探测成功 或者 探测的时间太长
		//return x.AddressDistance > y.AddressDistance;
		return CompareIndicator(y.AddressDistance, y.Key, x.AddressDistance, x.Key);
	}
	else if( ConnectRank == 100 || ConnectRank == 60 || ConnectRank == 40 )
	{	// 在已经断开的连接中发起连接的时候，重点参考 上一次的成功连接的 上传速度 和 下载速度，而不参考 RTT和AD
		//return x.Speed > y.Speed;
		return CompareIndicator(y.Speed, y.Key, x.Speed, x.Key);
	}
	else if( ConnectRank == 30 || ConnectRank == 10 || ConnectRank == 0)
	{	// iii.	几乎不可能被连接的节点 CR=10 按　LastConnectTime 从小到大排序
		//return x.LastConnectTime < y.LastConnectTime;
		return CompareIndicator(x.LastConnectTime, x.Key, y.LastConnectTime, y.Key);
	}
	assert(0);
	//return true;
	return x.Key < y.Key;
}

/// 活跃相关的索引键
struct ActiveIndicator
{
	PEER_ADDRESS Key;
	UINT ActiveTime;

	ActiveIndicator(const PEER_ADDRESS& key, UINT _RTT, UINT _LastActiveTime)
	{
		this->Key = key;
		if( _RTT == 0 || _RTT > 10*60*1000 )
		{
			ActiveTime = _LastActiveTime + 10*60*1000;
		}
		else
		{
			ActiveTime = _LastActiveTime + 20*60*1000 - _RTT*60;
		}
	}

};

inline bool operator<(const ActiveIndicator& x, const ActiveIndicator& y)
{
	//return x.ActiveTime < y.ActiveTime;
	return CompareIndicator(x.ActiveTime, x.Key, y.ActiveTime, y.Key);
}

/// peer地址信息项
class NetTypedIPInfo : public CIPInfo
{
public:
	UINT64 TotalDownloadBytes;
	UINT64 TotalUploadBytes;

	NET_TYPE NetworkType;

	/// 地域值
	UINT AddressDistance;

	/// 连接时的优先级别
	UINT ConnectRank;

	/// 上一次活跃时间
	UINT LastActiveTime;

	/// 上一次GetForConnetct的时间
	UINT LastGetForConnectTime;

	/// 连续探测失败的次数
	UINT ContinuousDetectFailedTimes;

	/// 上一次连接过程中的下载速度
	UINT DownloadSpeed;

	/// 上一次连接过程中的上传速度
	UINT UploadSpeed;

	/// 洪泛的次数
	UINT HelloTimes;

	/// 上一次洪泛的时间
	UINT LastHelloTime;

	// 上一次IP从什么地方获得的，1-从Tracker获得 2-从UdpDetect获得 3-从TcpDetect获得
	CandidatePeerTypeEnum FromWhere;

	/// 是否正在探测中
	bool IsDetecting;

	/// 是否正在连接中
	bool IsConnection;

	/// 是否是局域网连接
	bool IsLAN;

	explicit NetTypedIPInfo(const PEER_ADDRESS& addr);
	//NetTypedIPInfo(const CANDIDATE_PEER_INFO& info);

	/// 转换为字符串
	string ToString() const
	{
		char str[1025] = { 0 };
		_snprintf(str, 1024, 
			" %u %u %u %u %u %u %d %d %d", 
			this->AddressDistance, this->ConnectRank, this->LastActiveTime, this->ContinuousDetectFailedTimes, 
			this->DownloadSpeed, this->UploadSpeed, 
			this->IsDetecting, this->IsConnection, this->FromWhere);
		return CIPInfo::ToString() + str;
	}


	void CalcAddressDistance(const NET_TYPE& localNetType, IPTable& iptable);


	DetectIndicator GetDetectIndicator() const { return DetectIndicator(this->GetAddress(), RTT,AddressDistance,LastDetectTime,ContinuousDetectFailedTimes,FromWhere); }
	ConnectIndicator GetConnectIndicator() const { return ConnectIndicator(this->GetAddress(), RTT,AddressDistance,ConnectRank,DownloadSpeed,UploadSpeed,ConnectRank,FromWhere); }
	ActiveIndicator GetActiveIndicator() const { return ActiveIndicator(this->GetAddress(), RTT, LastActiveTime); }
	HelloIndicator GetHelloIndicator() const { return HelloIndicator(this->GetAddress(), this->AddressDistance, this->HelloTimes); }

	DetectIndicator GetIndicator(DetectIndicator*) const { return GetDetectIndicator(); }
	ConnectIndicator GetIndicator(ConnectIndicator*) const { return GetConnectIndicator(); }
	ActiveIndicator GetIndicator(ActiveIndicator*) const { return GetActiveIndicator(); }
	HelloIndicator GetIndicator(HelloIndicator*) const { return GetHelloIndicator(); }


private:
	///清除信息
	void Clear();
};


typedef boost::shared_ptr<NetTypedIPInfo> NetTypedIPInfoPtr;

/// 以peer地址为key的peer信息集合
typedef std::map<PEER_ADDRESS, NetTypedIPInfoPtr> NetTypedPeerInfoCollection;

/// 以tcp连接信息为key的Peer信息集合
typedef std::set<SimpleSocketAddress> TcpAddressCollection;

/// peer地址的集合
//typedef set<PEER_ADDRESS> PeerAddressCollection;

/// peer地址的集合
//typedef set<PEER_ADDRESS> PeerAddressCollection;


typedef std::map<DetectIndicator, NetTypedIPInfoPtr> DetectIndex;
typedef std::map<ConnectIndicator, NetTypedIPInfoPtr> ConnectIndex;
typedef std::map<ActiveIndicator, NetTypedIPInfoPtr> ActiveIndex;
typedef std::map<HelloIndicator, NetTypedIPInfoPtr> HelloIndex;



/// 真正的IpPool  包含以上所有的Pool
class NetTypedIPPoolImpl : public CIPPool, private boost::noncopyable
{
public:
	explicit NetTypedIPPoolImpl(boost::shared_ptr<const PeerNetInfo> netInfo, const tstring& baseDir, const std::set<PEER_ADDRESS>& mdsPool, IPTable& iptable);
	~NetTypedIPPoolImpl();

	virtual void SetConfig(const IPPoolConfig& config) { m_config = config; }

	virtual void SetNoUDP(const PEER_ADDRESS& addr);

	// OnNew 用于添加CandidatePeer
	bool AddCandidate(const PEER_ADDRESS& addr, const PEER_CORE_INFO& coreInfo, CandidatePeerTypeEnum FromWhere);
	bool AddCandidate(const CANDIDATE_PEER_INFO& addr, CandidatePeerTypeEnum FromWhere);
	void AddCandidate(size_t count, const CANDIDATE_PEER_INFO addrs[], CandidatePeerTypeEnum FromWhere);

	virtual void AddExchangedPeers(size_t count, const PeerExchangeItem* peers, CandidatePeerTypeEnum FromWhere);

	virtual bool AddInnerCandidate(const INNER_CANDIDATE_PEER_INFO& addr, CandidatePeerTypeEnum FromWhere);

	// OnUdpDetect	
	bool OnUdpDetect(const NetTypedIPInfoPtr& ipInfo);		// 当发起探测的时候 调用的
	bool OnUdpHello(const NetTypedIPInfoPtr& ipInfo);		// 当发起洪范的时候 调用的

	//	当洪范收到RTT的时候 调用的 
	virtual bool AddEchoDetected(const CANDIDATE_PEER_INFO& pInfo, UINT DetectedRTT, bool isUDP);

	// OnUdpDetectSucced  当探测收到RTT的时候 调用的
	bool AddDetected(const CANDIDATE_PEER_INFO& pInfo,UINT DetectedRTT, bool isUDP);

	// OnConnect 当发起连接的时候 调用的
	bool AddConnecting(const PEER_ADDRESS& addr);

	// OnConnectSucced	当连接成功的时候 调用的
	bool AddConnected(const PEER_ADDRESS& addr, UINT16 realPort, UINT8 connectFlags, bool isLAN);

	// OnConnectFailed	当连接失败的时候 调用的
	bool AddConnectFailed(const PEER_ADDRESS& addr, bool isTCP);
	
	// OnDisConnect		当断开连接的时候 调用的
	bool AddDisconnected(const PEER_ADDRESS& addr, UINT connectionTime, long errcode, UINT downloadSpeed, UINT uploadSpeed, UINT64 downloadBytes, UINT64 uploadBytes);

	// 获得需要发出探测的 CandidatePeer 
	bool GetForDetect(PeerItem& addr);

	// 获得需要发出洪范的 CandidatePeer
	virtual bool GetForHello(PeerItem& addr);

	// 获得需要发出连接的 CandidatePeer
	bool GetForConnect(PeerItem& addr);

	bool NeedDoList();

	virtual void SetExternalIP(UINT externalIP);

	size_t GetSize() const;

	/// 设置成为 不可能连接的 节点
	virtual void AddCannotConnectNode( UINT ip, USHORT port );

	/// 查找IPPool中存不存在这个信息
	virtual bool FindAddress(const PEER_ADDRESS& addr);

	/// 清空IPPool
	void Clear();

	void AddIndex(NetTypedIPInfoPtr ipInfo);
	void DeleteIndex(NetTypedIPInfoPtr ipInfo);

	virtual bool DeleteDuplicatedNAT(const SimpleSocketAddress& sockAddr, const PEER_ADDRESS& keyAddr, const PACKET_PEER_INFO& packetPeerInfo);

	virtual bool SaveLog(JsonWriter& writer, UINT32 detectedIP, UINT32& totalPeercount, UINT32& savedPeercount) const;

protected:
	/// 根据地址查找ip信息项
	NetTypedIPInfoPtr Find(const PEER_ADDRESS& PeerAddr);

	/// 将统计数据更新到共享内存
	void UpdateIPPoolInfo();

	/// 检查地址是否有效
	bool IsValidAddress(const PEER_ADDRESS& addr) const;

	/// 获取一个Peer信息项，如果对应地址的项不存在，则创建一个新的
	NetTypedIPInfoPtr GetPeerInfo(const PEER_ADDRESS& addr);

	/// 获取从开始到现在的时间(NetTypedIPInfo中的时间计数都使用这种时间而不是GetTickCount，以减少因为TickCount过大带来的问题)
	DWORD GetTimeCount() const;

	/// 计算从给定时间到现在的时间差
	DWORD CalcTimeSpan(DWORD startingTime) const;

	/// 计算探测的大致间隔时间
	UINT CalcDetectInterval() const;

	/// 删除地址
	void DeleteItem(NetTypedIPInfoPtr ipInfo);

	virtual const IPPoolStatistics* GetStatistics() const { return &m_statistics; }

	virtual void OnTimer(UINT seconds);

	void OutputDebugData(UINT seconds);

private:
	/// Detect 索引
	DetectIndex m_DetectIndex;				// 用于探测的索引
	ConnectIndex m_ConnectIndex;			// 用于连接的索引
	ActiveIndex m_ActiveIndex;				// 用于删除的索引
	HelloIndex m_HelloIndex;				// 用于洪范的索引

	/// 统计数据
	IPPoolStatistics m_statistics;

	/// 总的peer信息项集合
	NetTypedPeerInfoCollection m_peers;

	/// 连接黑名单,进入这里的节点,将不可能再被连接
	TcpAddressCollection m_CannotConnectPeers;

	/// mds的peer地址集合
	const std::set<PEER_ADDRESS>& m_mdsPool;


	boost::shared_ptr<const PeerNetInfo> m_NetInfo;
	/// 本地peer地址
	const PEER_ADDRESS& m_localAddress;


	UINT32 m_externalIP;

	tstring m_BaseDirectory;

	/// 启动的时间，是NetTypedIPInfo中记录的几个时间值的基准点
	time_counter m_StartTime;

	NET_TYPE m_localNetType;

	IPTable& m_iptable;

	IPPoolConfig m_config;

};

#endif
