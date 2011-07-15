
#ifndef _LIVE_P2PCOMMON2_NEW_COMMON_BASEINFO_H_
#define _LIVE_P2PCOMMON2_NEW_COMMON_BASEINFO_H_

#include "base/SourceResource.h"
#include <synacast/protocol/nat.h>
#include <synacast/protocol/base.h>
#include <synacast/protocol/data/NetInfo.h>
#include <synacast/protocol/data/PeerAddress.h>
#include <synacast/protocol/data/PeerMinMax.h>
#include <synacast/protocol/data/PeerStatus.h>
#include <ppl/util/time_counter.h>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <vector>

class PeerAuthInfo
{
public:
	/// 表示是否使用防盗链功能
	DWORD CookieType;

	/// 使用防盗链功能时的cookie值
	UINT CookieValue;

	string Username;
	string Password;

	PeerAuthInfo()
	{
		this->CookieType = COOKIE_NONE;
		this->CookieValue = 0;
	}
};



class PeerNetInfo : public PEER_NET_INFO
{
public:
	/// 公网连接节点，可以在每次PeerExchange时顺便更新
//	std::vector<PEER_ADDRESS> PublicHosts;

protected:
	/// tracker看到的ip
	UINT32 m_DetectedIP;

	/// 外网地址
	PEER_ADDRESS m_OuterAddress;


	/// 本地ip列表
	std::vector<UINT32> m_LocalIPArray;

	/// 本地ip列表
	std::vector<PEER_ADDRESS> m_LocalAddressArray;

	MY_STUN_NAT_TYPE m_NATType;

	/// 是否有外网ip
	bool m_HasExternalIP;


	/// 登录tracker时upnp是否启用
	mutable bool m_HasUPNPBeenEnabledForTracker;

	/// 登录tracker时是否通过外网ip进行登录
	mutable bool m_HasOuterIPUsedForUPNP;

	PEER_ADDRESS m_StunServerAddress;
	PEER_ADDRESS m_StunDetectedAddress;
 

public:
	PeerNetInfo()
	{
		this->Clear();
	}
	virtual ~PeerNetInfo()
	{
	}

	/// 清空
	void Clear();

	/// 加载本地ip列表
	void Load(const u_long ipArray[], size_t size);

	/// 获取peer类型
	UINT8 GetPeerType() const { return this->CoreInfo.PeerType; }

	UINT32 GetOuterIP() const { return this->Address.IP; }
	UINT32 GetDetectedIP() const { return this->m_DetectedIP; }

	void SetNATType( MY_STUN_NAT_TYPE natType )
	{
		this->m_NATType = natType;
		this->CoreInfo.NATType = natType;
	}
	MY_STUN_NAT_TYPE GetNATType() const { return m_NATType; }
	

	bool IsFullConeNAT() const { return m_NATType == STUN_TYPE_FULLCONENAT; }

	const PEER_ADDRESS& GetStunServerAddress() const { return m_StunServerAddress; }
	const PEER_ADDRESS& GetStunDetectedAddress() const { return m_StunDetectedAddress; }
	void SetStunAddress(const PEER_ADDRESS& serverAddress, const PEER_ADDRESS& detectedAddress);

	/// 检查ip是否是公网ip
	bool IsExternalIP() const
	{
		return this->m_DetectedIP == this->Address.IP;
	}

	/// 获取本地ip数组
	std::vector<UINT32> GetIPArray() const;
	/// 或缺本地PeerAddress数组
	std::vector<PEER_ADDRESS> GetLocalAddresses() const;

	/// 获取合适的外部地址
	PEER_ADDRESS GetProperOuterAddress() const
	{
		return this->IsUPNPEnabled() ? this->GetUPNPAddress() : this->m_OuterAddress;
	}

	/// 保存别的peer探测到的地址
	void SavePeerDetectedAddress( const PEER_ADDRESS& detectedAddr, const SimpleSocketAddress& sockAddr, bool isTCP, bool isInitFromRemote );

	/// 保存tracker探测到的ip和udp端口
	void SaveDetectedAddress(UINT32 detectedIP, UINT16 detectedUDPPort, const SimpleSocketAddress& trackerAddr);

	/// 是否是公网IP
	bool IsExposedIP() const { return this->IsExternalIP() || this->IsUPNPEnabled() || this->IsFullConeNAT() || this->IsPublicNAT(); }
	bool IsNAT() const { return ! IsExposedIP(); }
	bool NeedNAT() const { return NORMAL_PEER == this->GetPeerType() && false == IsExposedIP(); }

	bool IsPublicNAT() const { return this->GetNATType() == STUN_TYPE_PUBLIC; }

	/// 是否因为upnp启用需要重新登录tracker
	bool NeedReLoginForUPNP() const
	{
		return (false == this->m_HasUPNPBeenEnabledForTracker) && this->IsUPNPEnabled();
	}

	/// 是否因为外部ip被查询到需要重新登录tracker
	bool NeedReLoginForUPNPOuterIP() const
	{
		return (false == this->m_HasOuterIPUsedForUPNP) && this->IsUPNPEnabled() && (this->m_OuterAddress.IP != 0);
	}


	// virtual methods

	/// 检查更新peer网络类型
	virtual void CheckPeerNetType() = 0;

	/// upnp是否启用
	virtual bool IsUPNPEnabled() const = 0;

	/// 用于汇报给tracker的地址，如果启用了upnp，应该跟upnp的地址一样（实际只使用了2个端口）
	virtual PORT_PAIR GetOuterPorts() const = 0;

	/// 获取Upnp的peer地址
	virtual PEER_ADDRESS GetUPNPAddress() const = 0;

protected:
	void SaveOuterIP( UINT32 ip );

	void AddAddress( UINT32 ip );
};


class PeerStatusInfo : public PEER_STATUS_INFO
{
protected:
	boost::shared_ptr<SourceResource> m_SourceResource;

public:
	DEGREE_INFO Degrees;
	UINT64 CurrentTimeStamp;
	UINT BufferTime;
	UINT16 FixedQOS;
	bool UseFixedQOS;

public:
	PeerStatusInfo() : m_SourceResource( new SourceResource )
	{
		this->Clear();
	}

	void Clear()
	{
		PEER_STATUS_INFO::Clear();
		this->m_SourceResource->Clear();
		this->Degrees.Clear();
		this->BufferTime = 0;
		this->CurrentTimeStamp = 0;
		this->FixedQOS = 0;
		this->UseFixedQOS = false;
	}

	void Update(const PEER_STATUS_INFO& src)
	{
		this->Status = src.Status;
		this->MinMax = src.MinMax;
	}

	boost::shared_ptr<SourceResource> GetSourceResource()
	{
		return m_SourceResource;
	}
	boost::shared_ptr<const SourceResource> GetSourceResource() const
	{
		return m_SourceResource;
	}

	PeerStatusEx GetStatusEx() const
	{
		return PeerStatusEx( this->Status, this->Degrees );
	}
};


class PeerInformation : private boost::noncopyable
{
public:
	PeerInformation(const GUID& channelGUID, const GUID& peerGUID, UINT32 appVersion, UINT16 appVersion16, PeerNetInfo* netInfo)
		:	ChannelGUID(channelGUID), PeerGUID(peerGUID), AppVersion(appVersion), AppVersionNumber16(appVersion16), 
			NetInfo(netInfo), StatusInfo(new PeerStatusInfo()), AuthInfo(new PeerAuthInfo())
	{
	}
	virtual ~PeerInformation() { }

	UINT32 GetStartedSeconds() const
	{
		double seconds = this->StartTime.elapsed() / 1000.0;
		return static_cast<UINT>( seconds );
	}

	bool IsNormalPeer() const
	{
		return NORMAL_PEER == this->NetInfo->GetPeerType();
	}

public:
	const GUID ChannelGUID;
	const GUID PeerGUID;
	const UINT32 AppVersion;
	const UINT16 AppVersionNumber16;
	const boost::shared_ptr<PeerNetInfo> NetInfo;
	const boost::shared_ptr<PeerStatusInfo> StatusInfo;
	const boost::shared_ptr<PeerAuthInfo> AuthInfo;

	const time_counter StartTime;
};

/*
inline void ReviseOuterPeerAddress( PEER_ADDRESS& addr, const SimpleSocketAddress& sockAddr, bool isTCP, bool isInitFromRemote )
{
	assert( sockAddr.IP != 0 && sockAddr.Port != 0 && sockAddr.IP != INADDR_BROADCAST );
	if ( 0 == addr.IP )
	{
		assert( false );
		addr.IP = sockAddr.IP;
	}
	else
	{
		//assert( sockAddr.IP == addr.IP );
	}
	if ( isTCP )
	{
		// 对于tcp，如果是主动发起的连接，则需要考虑检查对方的tcp端口
		if ( false == isInitFromRemote )
		{
			if ( 0 == addr.TcpPort && 80 != sockAddr.Port )
			{
				assert( false );
				addr.TcpPort = sockAddr.Port;
			}
			else
			{
				//assert( 80 == sockAddr.Port || sockAddr.Port == addr.TcpPort );
			}
		}
	}
	else
	{
		if ( 0 == addr.UdpPort )
		{
			assert( false );
			addr.UdpPort = sockAddr.Port;
		}
		else
		{
			//assert( sockAddr.Port == addr.UdpPort );
		}
	}
}*/


/// 基于udp的socket地址获取合适的可用作key的peer地址
PEER_ADDRESS GetProperUDPKeyPeerAddress( const PEER_ADDRESS& peerAddr, const SimpleSocketAddress& sockAddr );


/// 获取合适的可用作key的peer地址
PEER_ADDRESS GetProperKeyPeerAddress( const PEER_ADDRESS& peerAddr, const SimpleSocketAddress& sockAddr, bool isUDP, bool isInitFromRemote, const PEER_ADDRESS& outerAddr );

#endif
