
#include "StdAfx.h"

#include "NetTypedIPPoolImpl.h"
#include "iptable.h"
#include "framework/log.h"

#include <synacast/protocol/PeerError.h>
#include <synacast/protocol/PacketHead.h>

#include "common/BaseInfo.h"
#include "util/testlog.h"

#include <ppl/io/stdfile.h>
#include <ppl/data/stlutils.h>

#include <ppl/text/json/writer.h>
#include <ppl/data/numeric.h>
#include <ppl/os/paths.h>

#include <assert.h>

/// 最大的rtt值
//const UINT DETECTED_MAX_RTT = 5000;


CIPPool* IPPoolFactory::PeerCreateNetTyped(boost::shared_ptr<PeerNetInfo> netInfo, const tstring& baseDir, const std::set<PEER_ADDRESS>& mdsPool, IPTable& iptable)
{
	return new NetTypedIPPoolImpl(netInfo, baseDir, mdsPool, iptable);
}



class IPPoolIndexUpdating : private boost::noncopyable
{
public:
	IPPoolIndexUpdating(NetTypedIPInfoPtr ipInfo, NetTypedIPPoolImpl* ipPool) : m_ipPool(*ipPool), m_ipInfo(ipInfo)
	{
		LIVE_ASSERT(ipPool);
		LIVE_ASSERT(m_ipInfo);
		m_ipPool.DeleteIndex(m_ipInfo);
	}
	~IPPoolIndexUpdating()
	{
		if (m_ipInfo)
		{
			m_ipPool.AddIndex(m_ipInfo);
		}
	}

	void Disable()
	{
		m_ipInfo.reset();
	}

private:
	NetTypedIPPoolImpl& m_ipPool;
	NetTypedIPInfoPtr m_ipInfo;
};


//////////////////////////////////////////////////////////////////////////
//					NetTypedIPInfo
//////////////////////////////////////////////////////////////////////////

NetTypedIPInfo::NetTypedIPInfo(const PEER_ADDRESS& addr) : CIPInfo(addr)
{
	Clear();
	this->Info.Address = addr;
}

void NetTypedIPInfo::Clear()
{
	memset(this, 0, sizeof(NetTypedIPInfo));
	ConnectFlags = IPPOOL_CONNECT_NONE;
	ConnectRank = 20;
	this->CanDetect = false;
	this->RTT = UINT_MAX;
}

void NetTypedIPInfo::CalcAddressDistance(const NET_TYPE& localNetType, IPTable& iptable)
{
	NET_TYPE netType = iptable.LocateIP(this->GetAddress().IP);
	this->NetworkType = netType;
	this->AddressDistance = CalcNetTypeDistance(localNetType, netType);
}

NetTypedIPPoolImpl::NetTypedIPPoolImpl(boost::shared_ptr<const PeerNetInfo> netInfo, const tstring& baseDir, const std::set<PEER_ADDRESS>& mdsPool, IPTable& iptable) 
	: m_mdsPool(mdsPool)
        , m_NetInfo( netInfo )
        , m_localAddress( netInfo->Address )
        , m_BaseDirectory(baseDir)
	, m_iptable(iptable)
{
	m_externalIP = m_localAddress.IP;
}

NetTypedIPPoolImpl::~NetTypedIPPoolImpl()
{
	Clear();
}

void NetTypedIPPoolImpl::Clear()
{
	m_peers.clear();
}

void NetTypedIPPoolImpl::AddIndex(NetTypedIPInfoPtr ipInfo)
{
	maps::insert(m_HelloIndex, ipInfo->GetHelloIndicator(), ipInfo);
	maps::insert(m_ActiveIndex, ipInfo->GetActiveIndicator(), ipInfo);
	maps::insert(m_ConnectIndex, ipInfo->GetConnectIndicator(), ipInfo);
	maps::insert(m_DetectIndex, ipInfo->GetDetectIndicator(), ipInfo);
}
void NetTypedIPPoolImpl::DeleteIndex(NetTypedIPInfoPtr ipInfo)
{
	maps::erase(m_DetectIndex, ipInfo->GetDetectIndicator(), ipInfo);
	maps::erase(m_ConnectIndex, ipInfo->GetConnectIndicator(), ipInfo);
	maps::erase(m_ActiveIndex, ipInfo->GetActiveIndicator(), ipInfo);
	maps::erase(m_HelloIndex, ipInfo->GetHelloIndicator(), ipInfo);
}

void NetTypedIPPoolImpl::SetExternalIP(UINT externalIP)
{
	if (m_externalIP == externalIP)
		return;
	bool isOldIPPrivate = IsPrivateIP(m_externalIP);
	m_externalIP = externalIP;
	m_localNetType = m_iptable.LocateIP(m_externalIP);
	bool isNewIPPrivate = IsPrivateIP(m_externalIP);
	if (isOldIPPrivate && !isNewIPPrivate)
	{
		IPPOOL_INFO( "LocalExternalIP " << 
				" ("<<m_localNetType.ISP<<":"<<m_localNetType.Country<<":"<<m_localNetType.Province<<":"<<m_localNetType.City<<") End");
		m_DetectIndex.clear();
		m_ConnectIndex.clear();
		STL_FOR_EACH_CONST(NetTypedPeerInfoCollection, m_peers, iter)
		{
			NetTypedIPInfoPtr ipInfo = iter->second;
			IPPoolIndexUpdating indexUpdating(ipInfo, this);
			ipInfo->CalcAddressDistance(m_localNetType, m_iptable);

			IPPOOL_INFO( "RefreshAddressDistance " << ipInfo->Info.Address << " " << ipInfo->RTT << " " << ipInfo->AddressDistance << 
				" ("<<ipInfo->NetworkType<<") End");
		}
	}
}

NetTypedIPInfoPtr NetTypedIPPoolImpl::Find(const PEER_ADDRESS& PeerAddr)
{
	NetTypedPeerInfoCollection::const_iterator iter = m_peers.find(PeerAddr);
	if (iter == m_peers.end())
		return NetTypedIPInfoPtr();
	LIVE_ASSERT(iter->second);
	return iter->second;
}

bool NetTypedIPPoolImpl::AddInnerCandidate( const INNER_CANDIDATE_PEER_INFO& addr, CandidatePeerTypeEnum FromWhere )
{
	if (!IsValidAddress(addr.DetectedAddress))
		return false;

	NetTypedIPInfoPtr ipInfo = GetPeerInfo(addr.DetectedAddress);
	ipInfo->Info.CoreInfo = addr.CoreInfo;
	ipInfo->StunServerAddress = addr.StunServerAddress;
	IPPoolIndexUpdating indexUpdating(ipInfo, this);

	// 计算AddressDistance
	ipInfo->CalcAddressDistance(m_localNetType, m_iptable);
	ipInfo->LastActiveTime = GetTimeCount();

	//! why? 这个和CR数学模型有关。
	if( FromWhere == CANDIDATE_FROM_TRACKER)
	{
		if( ipInfo->ConnectRank == 20 )
			ipInfo->ConnectRank = 50;
	}
	if( ipInfo->FromWhere != CANDIDATE_FROM_TRACKER )
	{
		ipInfo->FromWhere = FromWhere;
	}


	return true;
}

bool NetTypedIPPoolImpl::AddCandidate(const PEER_ADDRESS& addr, const PEER_CORE_INFO& coreInfo, CandidatePeerTypeEnum FromWhere)
{
	if (!IsValidAddress(addr))
		return false;

	NetTypedIPInfoPtr ipInfo = GetPeerInfo(addr);
	ipInfo->Info.CoreInfo = coreInfo;
	IPPoolIndexUpdating indexUpdating(ipInfo, this);

	// 计算AddressDistance
	ipInfo->CalcAddressDistance(m_localNetType, m_iptable);
	ipInfo->LastActiveTime = GetTimeCount();

	//! why? 这个和CR数学模型有关。
	if( FromWhere == CANDIDATE_FROM_TRACKER)
	{
		if( ipInfo->ConnectRank == 20 )
			ipInfo->ConnectRank = 50;
	}
	if( ipInfo->FromWhere != CANDIDATE_FROM_TRACKER )
	{
		ipInfo->FromWhere = FromWhere;
	}

	
	return true;
}

bool NetTypedIPPoolImpl::AddCandidate(const CANDIDATE_PEER_INFO& PeerAddr, CandidatePeerTypeEnum FromWhere)
{
	return AddCandidate(PeerAddr.Address, PeerAddr.CoreInfo, FromWhere);
}

void NetTypedIPPoolImpl::AddCandidate(size_t count, const CANDIDATE_PEER_INFO addrs[], CandidatePeerTypeEnum FromWhere)
{
	LIVE_ASSERT(count < UCHAR_MAX);
	for (size_t i = 0; i < count; ++i)
	{
		VIEW_INFO("AddCandidate " << i << " " << addrs[i].Address 
			<< " NetType:NATType " << make_tuple(addrs[i].CoreInfo.PeerNetType, addrs[i].CoreInfo.NATType) 
			<< " from " << FromWhere);
		AddCandidate(addrs[i], FromWhere);
	}
}

void NetTypedIPPoolImpl::AddExchangedPeers( size_t count, const PeerExchangeItem* peers, CandidatePeerTypeEnum FromWhere )
{
	LIVE_ASSERT(count < UCHAR_MAX);

	CANDIDATE_PEER_INFO peerInfo;
	FILL_ZERO(peerInfo);
	for (size_t i = 0; i < count; ++i)
	{
		peerInfo.Address = peers[i].Address;
		peerInfo.CoreInfo.PeerType = peers[i].PeerType;
		VIEW_INFO("AddExchangedPeer " << i << " " << peerInfo.Address 
			<< " NetType:NATType " << make_tuple(peerInfo.CoreInfo.PeerNetType, peerInfo.CoreInfo.NATType) 
			<< " from " << FromWhere);
		AddCandidate(peerInfo, FromWhere);
		TEST_LOG_OUT("echo peer " << (int)index << ": " << peerInfo.Address);
	}
}

bool NetTypedIPPoolImpl::OnUdpDetect(const NetTypedIPInfoPtr& ipInfo)
{
	IPPoolIndexUpdating indexUpdating(ipInfo, this);
	ipInfo->DetectTimes ++;
	if( ipInfo->IsDetecting == true )
		ipInfo->ContinuousDetectFailedTimes ++;
	ipInfo->LastDetectTime = GetTimeCount();
	ipInfo->IsDetecting = true;
	IPPOOL_INFO( "SendUdpDetect " << ipInfo->Info.Address << " " << ipInfo->FromWhere << " " << ipInfo->AddressDistance << 
		" ("<<ipInfo->NetworkType<<" End");
	return true;
}

bool NetTypedIPPoolImpl::OnUdpHello(const NetTypedIPInfoPtr& ipInfo)
{
	IPPoolIndexUpdating indexUpdating(ipInfo, this);
	ipInfo->HelloTimes ++;
	ipInfo->LastHelloTime = GetTimeCount();
	IPPOOL_INFO( "SendUdpHello " << ipInfo->Info.Address << " " << ipInfo->HelloTimes << " " << ipInfo->AddressDistance << " " << ipInfo->LastHelloTime << " End");
	return true;
}

bool NetTypedIPPoolImpl::AddEchoDetected(const CANDIDATE_PEER_INFO& info, UINT DetectedRTT, bool isUDP)
{
	if (!isUDP)
		return false;
	if (!IsValidAddress(info.Address))
		return false;

	NetTypedIPInfoPtr ipInfo = GetPeerInfo(info.Address);
	// 只在Udp探测的时候 更新下面的信息
	IPPoolIndexUpdating indexUpdating(ipInfo, this);
	// 加入到DetectedPool
	// 如果新的rtt更小或者原来没有探测成功过
	if (ipInfo->RTT > DetectedRTT || ipInfo->RTT == 0)
	{
		if( DetectedRTT == 0 ) DetectedRTT = 1;
		LIVE_ASSERT( DetectedRTT > 0 );
		ipInfo->RTT = DetectedRTT;
		if( DetectedRTT < m_config.ValidDetectedRTT && 
			(ipInfo->ConnectRank==20 || ipInfo->ConnectRank==50) )
		{
			ipInfo->ConnectRank = 90;
		}
	}
	ipInfo->LastActiveTime = GetTimeCount();
	IPPOOL_INFO("EchoDetectSucced "<<info.Address<<" "<<DetectedRTT<<" "<<isUDP<<" End");
	return true;
}

bool NetTypedIPPoolImpl::AddDetected(const CANDIDATE_PEER_INFO& info, UINT DetectedRTT, bool isUDP)
{
	if (!isUDP)
		return false;
	if (!IsValidAddress(info.Address))
		return false;

	NetTypedIPInfoPtr ipInfo = GetPeerInfo(info.Address);

	// 只在Udp探测的时候 更新下面的信息
	IPPoolIndexUpdating indexUpdating(ipInfo, this);
	// 加入到DetectedPool
	if( DetectedRTT == 0 ) DetectedRTT = 1;
	LIVE_ASSERT( DetectedRTT > 0 );
	ipInfo->RTT = DetectedRTT;
	if( DetectedRTT < m_config.ValidDetectedRTT && 
		(ipInfo->ConnectRank==20 || ipInfo->ConnectRank==50) )
	{
		ipInfo->ConnectRank = 90;
	}
	ipInfo->DetectSucceededTimes ++;
	ipInfo->ContinuousDetectFailedTimes = 0;
	ipInfo->LastActiveTime = GetTimeCount();
	
	ipInfo->IsDetecting = false;
	ipInfo->CanDetect = true;

	IPPOOL_INFO("DetectSucced "<<info.Address<<" "<<DetectedRTT<<" "<<isUDP<<" End");
	return true;
}

bool NetTypedIPPoolImpl::AddConnecting(const PEER_ADDRESS& addr)
{
	if (!IsValidAddress(addr))
		return false;

	NetTypedIPInfoPtr ipInfo = GetPeerInfo(addr);

	IPPoolIndexUpdating indexUpdating(ipInfo, this);
	ipInfo->ConnectTimes++;
	ipInfo->LastConnectTime = GetTimeCount();
	ipInfo->IsConnection = true;
	IPPOOL_INFO( "TcpConnect " << addr << " " << ipInfo->FromWhere << " " << ipInfo->ConnectRank << " " << ipInfo->ConnectFailedTimes << " " << ipInfo->RTT << " " << ipInfo->AddressDistance << 
		" (" << ipInfo->NetworkType << " End");
	return true;
}

bool NetTypedIPPoolImpl::AddConnected(const PEER_ADDRESS& addr, UINT16 realPort, UINT8 connectFlags, bool isLAN)
{
	if (!IsValidAddress(addr))
		return false;
	
	if( m_peers.find(addr) == m_peers.end() )
	{
		IPPOOL_INFO( "IpPoolCouldNotFindWhenConnected " << addr << " End" );
	}

	NetTypedIPInfoPtr ipInfo = GetPeerInfo(addr);
	IPPOOL_INFO("ConnectSucced " << addr << " " << realPort << " " << ipInfo->FromWhere << " " << ipInfo->RTT << " " << ipInfo->AddressDistance 
		<< "(" << ipInfo->NetworkType << " End ");
	
	IPPoolIndexUpdating indexUpdating(ipInfo, this);

	if( ipInfo->RTT > 0 && ipInfo->RTT < m_config.ValidDetectedRTT )
	{
		ipInfo->ConnectRank = 90;	// 初值
	}
	else
	{
		ipInfo->ConnectRank = 50;	//
	}
	ipInfo->ConnectSucceededTimes++;
	ipInfo->ConnectFailedTimes = 0;
	ipInfo->LastActiveTime = GetTimeCount();

	// 检查已经连上的端口，如果是80端口，则记个标志
//	if (addr.TcpPort != 80 && realPort == 80)
//	{
//		ipInfo->ConnectFlags |= IPPOOL_CONNECT_80;
//	}
//	else
//	{
//		ipInfo->ConnectFlags |= IPPOOL_CONNECT_NORMAL;
//	}
	if (connectFlags != IPPOOL_CONNECT_NONE)
	{
		ipInfo->ConnectFlags = connectFlags;
	}
	ipInfo->IsLAN = isLAN;
	return true;
}

bool NetTypedIPPoolImpl::AddConnectFailed(const PEER_ADDRESS& addr, bool isTCP)
{
	if (!IsValidAddress(addr))
		return false;
	
	if( m_peers.find(addr) == m_peers.end() )
	{
		IPPOOL_INFO( "IpPoolCouldNotFindWhenConnectFailed " << addr << " End" );
	}

	NetTypedIPInfoPtr ipInfo = GetPeerInfo(addr);
	IPPOOL_INFO("ConnectFailed " << addr << " " << ipInfo->FromWhere << " " << ipInfo->RTT << " " << ipInfo->AddressDistance 
		<< "(" << ipInfo->NetworkType << " End ");

	IPPoolIndexUpdating indexUpdating(ipInfo, this);

	ipInfo->LastConnectFailedTime = GetTimeCount();
	ipInfo->ConnectFailedTimes ++;
	if ( isTCP )
		ipInfo->TCPConnectFailedTimes++;
	else
		ipInfo->UDPConnectFailedTimes++;
	ipInfo->IsConnection = false;
	
	if( ipInfo->ConnectRank == 30 || ipInfo->ConnectRank == 20)
	{	// 上一次连接失败，则再降一挡次
		ipInfo->ConnectRank = 10;
	}
	else if( ipInfo->ConnectRank == 10 )
	{	// 连续两次 或者 两次以上 连接失败
		if( ipInfo->ConnectFailedTimes >= m_config.MaxConnectFailedTimes)
		{	// 置黑名单 给10分钟的 复活机会 直接进入黑名单
			IPPOOL_INFO("IppoolDelete By Connect Error "<<addr<<" "<<ipInfo->ConnectRank<<" "<<ipInfo->RTT<<" "<<ipInfo->ConnectFailedTimes);
			DeleteItem(ipInfo);
			indexUpdating.Disable();
			return true;
		}
	}
	else 
	{	// 第一次连接 或者 上一次成功 : 连接不上，则降一档次
		ipInfo->ConnectRank = 30;
	}
	
	return true;
}

inline bool IsDisconnectEqual( int errcode, int reason )
{
	return (reason - errcode ) % 100 == 0;
}


enum DISCONNECTED_RANK
{
	RANK_LONG = 1,		// 等待较长时间
	RANK_SHORT = 2, 	// 等待较短时间
	RANK_IMMEDIATE = 3,	// 立即
	RANK_BLOCKED = 4,	// 放入黑名单
	RANK_IGNORED = 5,	// 忽略
};
/*
const DISCONNECTED_RANK ERROR_RANKS[] = 
{
	RANK_LONG, // -1
	RANK_BLOCKED, // -2
	RANK_BLOCKED, // -3
	RANK_SHORT, // -4
	RANK_LONG, // -5
	RANK_SHORT, // -6
	RANK_IGNORED, // -7
	RANK_LONG, // -8
	RANK_BLOCKED, // -9
	RANK_SHORT, // -10
	RANK_IGNORED, // -11
	RANK_LONG, // -12
	RANK_BLOCKED, // -13

	RANK_IMMEDIATE,	// -14
	RANK_IGNORED,	// -15
	RANK_IGNORED,	// -16
	RANK_IGNORED,	// -17
	RANK_IGNORED,	// -18
	RANK_IGNORED,	// -19

	RANK_BLOCKED, // -20
	RANK_IMMEDIATE, // -21
	RANK_LONG, // -22
	RANK_LONG, // -23
	RANK_LONG, // -24
};*/


DISCONNECTED_RANK GetDisconnectedRank(long errcode)
{
	DISCONNECTED_RANK rank = RANK_LONG;
	switch ( errcode )
	{
	case PP_LEAVE_SELF_TO_SELF:
	case PP_LEAVE_BAD_CHANNEL:
	case PP_LEAVE_VERSION_NOT_SUPPORT:
	case PP_LEAVE_QUIT:
		rank = RANK_BLOCKED;
		break;
	case PP_LEAVE_DUPLICATE_CONNECTION:
	case PP_LEAVE_REFUSE:
	case PP_LEAVE_NO_CONNECTION:
		rank = RANK_IGNORED;
		break;
	case PP_LEAVE_KICKED:
	case PP_LEAVE_NETWORK_ERROR:
		rank = RANK_SHORT;
		break;
	case PP_LEAVE_LONG_IDLE:
		rank = RANK_SHORT;
		break;
	case PP_LEAVE_HANDSHAKE_TIMEOUT:
		rank = RANK_LONG;
		break;
	}
	return rank;
}


bool NetTypedIPPoolImpl::AddDisconnected(const PEER_ADDRESS& addr, UINT connectionTime, long errcode, UINT downloadSpeed, UINT uploadSpeed, UINT64 downloadBytes, UINT64 uploadBytes)
{
	if (!IsValidAddress(addr))
		return false;
	
	if( m_peers.find(addr) == m_peers.end() )
	{
		IPPOOL_INFO( "IpPoolCouldNotFindWhenDisconnected " << addr << " End" );
	}

	NetTypedIPInfoPtr ipInfo = GetPeerInfo(addr);

	IPPoolIndexUpdating indexUpdating(ipInfo, this);

	DetectIndicator oldIndicator = ipInfo->GetDetectIndicator();

	ipInfo->DisconnectTimes++;
	ipInfo->DownloadSpeed = downloadSpeed;
	ipInfo->UploadSpeed = uploadSpeed;
	ipInfo->ConnectFailedTimes = 0;
	ipInfo->LastActiveTime = GetTimeCount();
	ipInfo->LastDisconnectTime = GetTimeCount();
	ipInfo->IsConnection = false;

	ipInfo->TotalDownloadBytes += downloadBytes;
	ipInfo->TotalUploadBytes += uploadBytes;

	// 不同的断开码 对应不同的 ConnectRank
	DISCONNECTED_RANK rank = GetDisconnectedRank(errcode);
	if (rank == RANK_BLOCKED)
	{
		// 直接进入黑名单
		DeleteItem(ipInfo);
		indexUpdating.Disable();
		IPPOOL_INFO("IppoolDelete By Disconnect "<<addr<<" "<<ipInfo->ConnectRank<<" "<<ipInfo->RTT<<" "<<errcode<<" "<<downloadSpeed<<" "<<uploadSpeed);
		return true;
	}
	else if (rank == RANK_IMMEDIATE)
	{
		ipInfo->ConnectRank = 100;
	}
	else if (rank == RANK_SHORT)
	{
		ipInfo->ConnectRank = 60;
	}
	else if (rank == RANK_LONG)
	{
		ipInfo->ConnectRank = 40;
	}

	IPPOOL_INFO( "AddDisconnected " << addr << " " << connectionTime << " " << errcode << " " << downloadSpeed << " " << uploadSpeed << " " << ipInfo->ConnectRank << " End" );

	LIVE_ASSERT(ipInfo->LastDisconnectTime > 0);

	DetectIndicator newIndicator = ipInfo->GetDetectIndicator();
	LIVE_ASSERT(newIndicator.RTT == oldIndicator.RTT);
	LIVE_ASSERT(newIndicator.AddressDistance == oldIndicator.AddressDistance);
	LIVE_ASSERT(newIndicator.ContinuousDetectFailedTimes == oldIndicator.ContinuousDetectFailedTimes);
	LIVE_ASSERT(newIndicator.LastDetectTime == oldIndicator.LastDetectTime);
	LIVE_ASSERT(((oldIndicator < newIndicator) == false) && ((newIndicator < oldIndicator) == false));
	return true;
}


bool NetTypedIPPoolImpl::GetForHello(PeerItem& addr)
{
	UINT now = GetTimeCount();
	//! why? 为什么要使用 for
	for ( HelloIndex::iterator iter = m_HelloIndex.begin(); iter != m_HelloIndex.end(); ++iter)
	{
		NetTypedIPInfoPtr ipInfo = iter->second;
		if (ipInfo->LastHelloTime != 0 && now - ipInfo->LastHelloTime < m_config.HelloInterval * 1000)
			return false;
		addr = *ipInfo;
		OnUdpHello(ipInfo);
		return true;
	}
	return false;
}

bool NetTypedIPPoolImpl::GetForDetect(PeerItem& addr)
{
	UINT now = GetTimeCount();
	
	UINT DetectContinueCount = 0;
	//UINT DetectContinueCountForLastDetectTime = 0;
	
	//! why? 为什么要使用 for
	for( DetectIndex::iterator iter = m_DetectIndex.begin(); iter != m_DetectIndex.end(); iter ++)
	{
		NetTypedIPInfoPtr ipInfo = iter->second;

		if (ipInfo->LastDetectTime != 0 && now - ipInfo->LastDetectTime < m_config.DetectInterval * 1000)
		{	// 一个Peer不能
			return false;
		}

		addr = *ipInfo;
		OnUdpDetect(ipInfo);

		return true;
	}
	m_statistics.DetectCheckCount = (WORD)DetectContinueCount;
	return false;
}

bool NetTypedIPPoolImpl::GetForConnect(PeerItem& addr)
{
	UINT ConnectContinueCount = 0;
	UINT ConnectContinueCountForLastConnectTime = 0;

	for( ConnectIndex::iterator iter = m_ConnectIndex.begin(); iter != m_ConnectIndex.end(); iter ++)
	{
		NetTypedIPInfoPtr ipInfo = iter->second;

		if( ipInfo->IsConnection == true )
		{	// 正在连接，或者已经连上的节点不再连接
			ConnectContinueCount ++;
			continue;
		}

		if( ipInfo->LastGetForConnectTime > 0 && ipInfo->LastGetForConnectTime + m_config.ConnectInterval * 1000 > GetTimeCount() )
		{	// 不到30s时间又GetForConnect了一次
			ConnectContinueCount ++;
			ConnectContinueCountForLastConnectTime ++;
			if( ConnectContinueCountForLastConnectTime >= m_config.MaxContinuousConnectCheck )
			{
				break;
			}
			continue;
		}
	
		SimpleSocketAddress Address;
		Address.IP = ipInfo->Info.Address.IP;
		Address.Port = ipInfo->Info.Address.TcpPort;

		if( m_CannotConnectPeers.find(Address) != m_CannotConnectPeers.end() )
		{	// 遇到不可能连上的节点
			ConnectContinueCount ++;
			continue;
		}

		// 将 ConnectContinueCount 写入共享内存以便于检查问题
		m_statistics.ConnectCheckCount = (WORD)ConnectContinueCount;

		addr = *ipInfo;
		ipInfo->LastGetForConnectTime = GetTimeCount();

//		const PEER_ADDRESS& peerAddr = addr.Info.Address;
//		LIVE_ASSERT(peerAddr.IP != m_localAddress.IP || (peerAddr.TcpPort != m_localAddress.TcpPort && peerAddr.UdpPort != m_localAddress.UdpPort));

        VIEW_DEBUG( "IPPool::GetForConnect " << addr.Info.Address << " " << make_tuple( addr.ConnectTimes, addr.ConnectFlags, addr.CanDetect ) );
		return true;
	}
	m_statistics.ConnectCheckCount = (WORD)ConnectContinueCount;
	return false;
}

bool NetTypedIPPoolImpl::IsValidAddress(const PEER_ADDRESS& addr) const
{
	if (false == addr.IsValid())
	{
		IPPOOL_ERROR("Ignore add an error peer " << addr);
		return false;
	}
	if ((m_localAddress == addr))
	{
		IPPOOL_INFO("Ignore add myself into IPPool!");
		return false;//如果是本机的地址，那么就不添加
	}
	if (m_NetInfo->GetProperOuterAddress().ToUDPAddress() == addr.ToUDPAddress())
	{
		IPPOOL_INFO("Ignore add myself outer into IPPool!");
		return false;//如果是本机的地址，那么就不添加
	}
	// 如果是mds pool中的地址，则不能加到其它pool中
	if (containers::contains(m_mdsPool, addr))
	{
		IPPOOL_INFO("the IP is MDS!" << addr);
		return false;
	}
//	LIVE_ASSERT(addr.IP != m_localAddress.IP || (addr.TcpPort != m_localAddress.TcpPort && addr.UdpPort != m_localAddress.UdpPort));
	return true;
}

bool NetTypedIPPoolImpl::NeedDoList()
{
	//? 需要调整和进一步的考虑
	return (GetSize() <  m_config.MaxPoolSizeNeedDoList); 
}

void NetTypedIPPoolImpl::UpdateIPPoolInfo()
{
	//m_statistics.TotalPoolSize = m_peers.size();
}

NetTypedIPInfoPtr NetTypedIPPoolImpl::GetPeerInfo(const PEER_ADDRESS& addr)
{
	//LIVE_ASSERT( CheckIPValid( addr.IP ) );
	//LIVE_ASSERT( false == IsPrivateIP( addr.IP ) );
	NetTypedIPInfoPtr& ipInfo = m_peers[addr];
	if (!ipInfo)
	{
		ipInfo.reset(new NetTypedIPInfo(addr));
		ipInfo->StartTime = GetTimeCount();
	}
	return ipInfo;
}

DWORD NetTypedIPPoolImpl::GetTimeCount() const
{
	return m_StartTime.elapsed32();
}

DWORD NetTypedIPPoolImpl::CalcTimeSpan(DWORD startingTime) const
{
	return m_StartTime.elapsed32() - startingTime;
}


size_t NetTypedIPPoolImpl::GetSize() const
{
	return m_peers.size();
}

void NetTypedIPPoolImpl::DeleteItem(NetTypedIPInfoPtr ipInfo)
{
	DeleteIndex(ipInfo);
	m_peers.erase(ipInfo->GetAddress());
}

void NetTypedIPPoolImpl::OnTimer(UINT seconds)
{
	IPPOOL_INFO("NetTypedIPPoolImpl::OnTimer Begin " << seconds
		<< " size: " << m_peers.size()
		<< " detect_index: " << m_DetectIndex.size()
		<< " connect_index: " << m_ConnectIndex.size()
		<< " active_index: " << m_ActiveIndex.size()
		<< " cannot: " << m_CannotConnectPeers.size() << " End");

	LIVE_ASSERT( m_DetectIndex.size() <= m_peers.size() );
	LIVE_ASSERT( m_ConnectIndex.size() <= m_peers.size() );
	LIVE_ASSERT( m_ActiveIndex.size() <= m_peers.size() );
	LIVE_ASSERT( m_HelloIndex.size() <= m_peers.size() );

	UINT now = GetTimeCount();
	int DeleteCount = 0;
	for(ActiveIndex::iterator iter = m_ActiveIndex.begin();
		iter != m_ActiveIndex.end();  )
	{
		ActiveIndicator indicator = iter->first;
		NetTypedIPInfoPtr ipInfo = iter->second;
		iter ++;
		if( indicator.ActiveTime >= now )
		{	// 如果过期
			break;
		}
		else
		{
			LIVE_ASSERT(1);
		}
		DeleteItem(ipInfo);
		DeleteCount ++;
	}
	m_statistics.DeleteCheckCount = (WORD)DeleteCount;
	UpdateIPPoolInfo();

	m_statistics.TotalPoolSize = (UINT)m_peers.size();
	m_statistics.UnconnectablePoolSize = (UINT)m_CannotConnectPeers.size();
	m_statistics.DetectIndexSize = (UINT)m_DetectIndex.size();
	m_statistics.ConnectIndexSize = (UINT)m_ConnectIndex.size();
	m_statistics.ActiveIndexSize = (UINT)m_ActiveIndex.size();
	m_statistics.HelloIndexSize = (UINT)m_HelloIndex.size();

	if( seconds % 20 == 4 )
	{	// 20 秒钟才计算一次,因为要遍历,所以用来记录
		int DetectedPoolCount = 0;
		int DetectSendedCount = 0;
		int ConnectionPoolCount = 0;
		int CandidatePoolCount = 0;
		int DetectingCount = 0;
 
		for(NetTypedPeerInfoCollection::iterator itr = m_peers.begin(); itr != m_peers.end(); itr ++ ) 
		{
			NetTypedIPInfoPtr info = itr->second;
			LIVE_ASSERT( info );
			if( !info ) continue;
			if( info->IsConnection )
			{
				ConnectionPoolCount ++;
			}
			if( info->LastDetectTime > 0 )
			{
				DetectingCount ++;
			}
			if( info->RTT > 0 )
			{
				DetectedPoolCount ++;
			}
			if( info->LastDetectTime > 0 )
			{
				DetectSendedCount ++;
			}
			if( info->ContinuousDetectFailedTimes < 2 && info->RTT == 0 )
			{
				CandidatePoolCount ++;
			}
		}
		m_statistics.DetectedPoolSize = DetectedPoolCount;
		m_statistics.CandidatePoolSize = CandidatePoolCount;
		m_statistics.ConnectionPoolSize = ConnectionPoolCount;
		m_statistics.DetectingCount = (WORD)DetectingCount;
	}

#ifdef _DEBUG
	OutputDebugData(seconds);
#endif
	IPPOOL_INFO("NetTypedIPPoolImpl::OnTimer End " << seconds << " size: " << DeleteCount << " End" );
}


void NetTypedIPPoolImpl::AddCannotConnectNode( UINT ip, USHORT port )
{
	SimpleSocketAddress Address;
	Address.IP = ip;
	Address.Port = port;
	IPPOOL_INFO( "AddCannotConnectNode " << Address << " End" );
	m_CannotConnectPeers.insert(Address);
}


bool NetTypedIPPoolImpl::FindAddress(const PEER_ADDRESS& addr)
{
	return m_peers.find(addr) != m_peers.end();
}


#ifdef _DEBUG
void OutputIPInfo(ppl::io::stdfile_writer& fout, const NetTypedIPInfo* ipInfo)
{
	const PEER_ADDRESS& addr = ipInfo->GetAddress();
	const PEER_ADDRESS& ntsaddr = ipInfo->StunServerAddress;
	string ipstr = AnsiFormatIPAddress(addr.IP);
	string ntsipstr = AnsiFormatIPAddress(ntsaddr.IP);
	fout.write_format("%8u  %16s:%5u:%5u  %5hu:%5hu:%5hu:%8u  %3u:%5hu:%5hu:%5hu:%5hu:%8hu:%8hu:%8hu  %5hu:%8u  %5u   n1=%d:n2=%d nts=%16s:%5u:%5u w=%d\r\n", 
		ipInfo->StartTime, 
		ipstr.c_str(), addr.TcpPort, addr.UdpPort, 
		ipInfo->DetectTimes, ipInfo->DetectSucceededTimes, ipInfo->RTT, ipInfo->LastDetectTime, 
		ipInfo->ConnectRank, ipInfo->ConnectTimes, ipInfo->ConnectSucceededTimes, 
		ipInfo->ConnectFailedTimes, ipInfo->TCPConnectFailedTimes, ipInfo->UDPConnectFailedTimes, 
		ipInfo->LastConnectTime, ipInfo->LastConnectFailedTime, 
		ipInfo->DisconnectTimes, ipInfo->LastDisconnectTime, ipInfo->ConnectFlags, 
		ipInfo->Info.CoreInfo.NATType, ipInfo->Info.CoreInfo.PeerNetType, 
		ntsipstr.c_str(), ntsaddr.TcpPort, ntsaddr.UdpPort, 
		ipInfo->FromWhere );
}

void NetTypedIPPoolImpl::OutputDebugData(UINT seconds)
{
#ifdef _DEBUG
	UINT timeUnit = 30;
	if (m_peers.size() > 2000)
		timeUnit = 120;
	else if (m_peers.size() > 1000)
		timeUnit = 60;
	if (seconds % timeUnit != 10)
		return;

	tstring path = ppl::os::paths::combine( m_BaseDirectory, _T("ipdat.txt") );
	ppl::io::stdfile_writer fout;
	if (!fout.open_binary(path.c_str()))
	{
		//	LIVE_ASSERT(false);
		return;
	}
	time_t currentTime = time(NULL);
	fout.write_format("%s\r\n", asctime(localtime(&currentTime)));
	fout.write_format("StartTime  \t  IP:TCP:UDP  \t\t\t\t  DetectTimes:S:RTT:L  \t  ConnectRank:ConnectTimes:S:F:TCP:UDP:L:LF  \t\t\t\t\t  DisconnectTimes:L CF\r\n");
	int count = 0;
	STL_FOR_EACH_CONST(NetTypedPeerInfoCollection, m_peers, iter)
	{
		if (count > 1000)
			break;
		NetTypedIPInfoPtr ipInfo = iter->second;
		OutputIPInfo(fout, ipInfo.get());
		++count;
	}
#endif
}

#endif


void NetTypedIPPoolImpl::SetNoUDP( const PEER_ADDRESS& addr )
{
	if (!IsValidAddress(addr))
		return;

	NetTypedPeerInfoCollection::iterator iter = m_peers.find( addr );
	if( iter == m_peers.end() )
	{
		IPPOOL_INFO( "IpPoolCouldNotFindWhenSetNoUDP " << addr << " End" );
		return;
	}

	NetTypedIPInfoPtr ipInfo = iter->second;
//	ipInfo->ConnectFlags = IPPOOL_CONNECT_NORMAL;
}

bool NetTypedIPPoolImpl::DeleteDuplicatedNAT( const SimpleSocketAddress& sockAddr, const PEER_ADDRESS& keyAddr, const PACKET_PEER_INFO& packetPeerInfo )
{
	if ( packetPeerInfo.Address.IP == keyAddr.IP )
		return false;
	NetTypedPeerInfoCollection::iterator iterInner = m_peers.find( packetPeerInfo.Address );
	// 如果IPPool里面没有内网地址，不需要block掉keyAddr
	if ( iterInner == m_peers.end() )
		return false;
	// 如果IPPool里面没有keyAddr，不需要block掉keyAddr
	NetTypedPeerInfoCollection::iterator iterKey = m_peers.find( keyAddr );
	if ( iterKey != m_peers.end() )
		return false;

	NetTypedIPInfoPtr ipInfo = iterKey->second;
	IPPoolIndexUpdating indexUpdating(ipInfo, this);
	ipInfo->ConnectFlags = IPPOOL_CONNECT_BLOCKED;
	ipInfo->ConnectRank = 0;
	return true;
}
/*
bool NetTypedIPPoolImpl::SaveLog( JsonWriter& writer, UINT32 detectedIP ) const
{
	const int max_ip_count = 5000;
	UINT totalCount = 0;
	UINT realCount = 0;
	UINT totalUp(0), totalDown(0), rTotalUp(0), rTotalDown(0);
	writer.WriteString("peers");
	writer.WriteColon();
	{
		JsonWriter::ArrayWriterEntry arrayEntry(writer.GetStream());
		STL_FOR_EACH_CONST(NetTypedPeerInfoCollection, m_peers, iter)
		{
			LIVE_ASSERT( iter->second );
			const NetTypedIPInfo& ipInfo = *(iter->second);
			// 转换为KB
			UINT downloadBytes = static_cast<UINT>( ipInfo.TotalDownloadBytes );
			UINT uploadBytes = static_cast<UINT>( ipInfo.TotalUploadBytes);
			totalDown += downloadBytes;
			totalUp   += uploadBytes;
			downloadBytes /= 1000;
			uploadBytes /= 1000;
			if ( downloadBytes > 0 || uploadBytes > 0 )
			{
				// 忽略小于1K的
				if (totalCount < max_ip_count)
				{
					if ( totalCount > 0 )
					{
						writer.WriteComma();
					}
					UINT32 ip = ( ipInfo.IsLAN ) ? detectedIP : ipInfo.Info.Address.IP;
					{
						JsonWriter::ObjectWriterEntry objectEntry(writer.GetStream());
						writer.WriteJsonString("IP", AnsiFormatIPAddress(ip));
						writer.WriteComma();
						writer.WriteJsonNumber("Down", numeric<UINT>::format(uploadBytes));
						writer.WriteComma();
						writer.WriteJsonNumber("Up", numeric<UINT>::format(downloadBytes));

						rTotalUp += uploadBytes;
						rTotalDown += downloadBytes;
					}
					++realCount;
				}
				++totalCount;
			}
		}
	}
	writer.WriteComma();
	writer.WriteJsonNumber("tpeers", numeric<UINT>::format(totalCount));
//	writer.WriteComma();
//	writer.WriteJsonNumber("realcount", numerics::format_uint(realCount));
	UINT peerPercent = realCount * 100.0 / totalCount;
	writer.WriteComma();
	writer.WriteJsonNumber("peerpercent", numeric<UINT>::format(peerPercent));
	return true;
}
*/
bool NetTypedIPPoolImpl::SaveLog( JsonWriter& writer, UINT32 detectedIP, UINT32& totalPeercount, UINT32& savedPeercount ) const
{
	const int max_ip_count = 5000;
	UINT totalUp(0), totalDown(0), rTotalUp(0), rTotalDown(0);

	STL_FOR_EACH_CONST(NetTypedPeerInfoCollection, m_peers, iter)
	{
		LIVE_ASSERT( iter->second );
		const NetTypedIPInfo& ipInfo = *(iter->second);
		// 转换为KB
		UINT downloadBytes = static_cast<UINT>( ipInfo.TotalDownloadBytes );
		UINT uploadBytes = static_cast<UINT>( ipInfo.TotalUploadBytes);
		totalDown += downloadBytes;
		totalUp   += uploadBytes;
		downloadBytes /= 1024;
		uploadBytes /= 1024;
		if ( downloadBytes > 0 || uploadBytes > 0 )
		{
			// 忽略小于1K的
			if (totalPeercount <(size_t) max_ip_count)
			{
				if ( savedPeercount > 0 )
				{
					writer.WriteComma();
				}
				UINT32 ip = ( ipInfo.IsLAN ) ? detectedIP : ipInfo.Info.Address.IP;
				{
					JsonWriter::ObjectWriterEntry objectEntry(writer.GetStream());
					writer.WriteJsonString("IP", AnsiFormatIPAddress(ip));
					writer.WriteComma();
					writer.WriteJsonNumber("Down", numeric<UINT>::format(uploadBytes));
					writer.WriteComma();
					writer.WriteJsonNumber("Up", numeric<UINT>::format(downloadBytes));

					rTotalUp += uploadBytes;
					rTotalDown += downloadBytes;
				}
				++savedPeercount;
			}
			++totalPeercount;
		}
	}

	return true;
}



