
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
	/// ��ʾ�Ƿ�ʹ�÷���������
	DWORD CookieType;

	/// ʹ�÷���������ʱ��cookieֵ
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
	/// �������ӽڵ㣬������ÿ��PeerExchangeʱ˳�����
//	std::vector<PEER_ADDRESS> PublicHosts;

protected:
	/// tracker������ip
	UINT32 m_DetectedIP;

	/// ������ַ
	PEER_ADDRESS m_OuterAddress;


	/// ����ip�б�
	std::vector<UINT32> m_LocalIPArray;

	/// ����ip�б�
	std::vector<PEER_ADDRESS> m_LocalAddressArray;

	MY_STUN_NAT_TYPE m_NATType;

	/// �Ƿ�������ip
	bool m_HasExternalIP;


	/// ��¼trackerʱupnp�Ƿ�����
	mutable bool m_HasUPNPBeenEnabledForTracker;

	/// ��¼trackerʱ�Ƿ�ͨ������ip���е�¼
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

	/// ���
	void Clear();

	/// ���ر���ip�б�
	void Load(const u_long ipArray[], size_t size);

	/// ��ȡpeer����
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

	/// ���ip�Ƿ��ǹ���ip
	bool IsExternalIP() const
	{
		return this->m_DetectedIP == this->Address.IP;
	}

	/// ��ȡ����ip����
	std::vector<UINT32> GetIPArray() const;
	/// ��ȱ����PeerAddress����
	std::vector<PEER_ADDRESS> GetLocalAddresses() const;

	/// ��ȡ���ʵ��ⲿ��ַ
	PEER_ADDRESS GetProperOuterAddress() const
	{
		return this->IsUPNPEnabled() ? this->GetUPNPAddress() : this->m_OuterAddress;
	}

	/// ������peer̽�⵽�ĵ�ַ
	void SavePeerDetectedAddress( const PEER_ADDRESS& detectedAddr, const SimpleSocketAddress& sockAddr, bool isTCP, bool isInitFromRemote );

	/// ����tracker̽�⵽��ip��udp�˿�
	void SaveDetectedAddress(UINT32 detectedIP, UINT16 detectedUDPPort, const SimpleSocketAddress& trackerAddr);

	/// �Ƿ��ǹ���IP
	bool IsExposedIP() const { return this->IsExternalIP() || this->IsUPNPEnabled() || this->IsFullConeNAT() || this->IsPublicNAT(); }
	bool IsNAT() const { return ! IsExposedIP(); }
	bool NeedNAT() const { return NORMAL_PEER == this->GetPeerType() && false == IsExposedIP(); }

	bool IsPublicNAT() const { return this->GetNATType() == STUN_TYPE_PUBLIC; }

	/// �Ƿ���Ϊupnp������Ҫ���µ�¼tracker
	bool NeedReLoginForUPNP() const
	{
		return (false == this->m_HasUPNPBeenEnabledForTracker) && this->IsUPNPEnabled();
	}

	/// �Ƿ���Ϊ�ⲿip����ѯ����Ҫ���µ�¼tracker
	bool NeedReLoginForUPNPOuterIP() const
	{
		return (false == this->m_HasOuterIPUsedForUPNP) && this->IsUPNPEnabled() && (this->m_OuterAddress.IP != 0);
	}


	// virtual methods

	/// ������peer��������
	virtual void CheckPeerNetType() = 0;

	/// upnp�Ƿ�����
	virtual bool IsUPNPEnabled() const = 0;

	/// ���ڻ㱨��tracker�ĵ�ַ�����������upnp��Ӧ�ø�upnp�ĵ�ַһ����ʵ��ֻʹ����2���˿ڣ�
	virtual PORT_PAIR GetOuterPorts() const = 0;

	/// ��ȡUpnp��peer��ַ
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
		// ����tcp�������������������ӣ�����Ҫ���Ǽ��Է���tcp�˿�
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


/// ����udp��socket��ַ��ȡ���ʵĿ�����key��peer��ַ
PEER_ADDRESS GetProperUDPKeyPeerAddress( const PEER_ADDRESS& peerAddr, const SimpleSocketAddress& sockAddr );


/// ��ȡ���ʵĿ�����key��peer��ַ
PEER_ADDRESS GetProperKeyPeerAddress( const PEER_ADDRESS& peerAddr, const SimpleSocketAddress& sockAddr, bool isUDP, bool isInitFromRemote, const PEER_ADDRESS& outerAddr );

#endif
