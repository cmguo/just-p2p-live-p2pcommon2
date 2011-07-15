
#include "StdAfx.h"

#include "BaseInfo.h"
#include "framework/log.h"


void PeerNetInfo::SavePeerDetectedAddress( const PEER_ADDRESS& detectedAddr, const SimpleSocketAddress& sockAddr, bool isTCP, bool isInitFromRemote )
{
	// 如果对方是内网ip，则忽略
	if ( 0 == sockAddr.IP || IsPrivateIP( sockAddr.IP ) )
		return;
	// 如果探测到的ip为0或者时内网ip，则忽略
	if ( 0 == detectedAddr.IP || IsPrivateIP( detectedAddr.IP ) )
		return;

	// 如果没有记录tracker返回的DetectedIP才记录
	if ( 0 == this->m_DetectedIP )
	{
		this->SaveOuterIP( detectedAddr.IP );
	}
	if ( detectedAddr.IP != this->m_OuterAddress.IP )
	{
		// 另外一个跟DetectedIP不一致的外网ip，忽略探测到的端口
		return;
	}
	if ( isTCP )
	{
		// 对于tcp，仅当是对方主动发起连接时，tcp外部端口才有效，并过滤掉80端口
		if ( isInitFromRemote && 0 != detectedAddr.TcpPort && 80 != detectedAddr.TcpPort )
			this->m_OuterAddress.TcpPort = detectedAddr.TcpPort;
	}
	else
	{
		if ( 0 != detectedAddr.UdpPort )
			this->m_OuterAddress.UdpPort = detectedAddr.UdpPort;
	}
	VIEW_DEBUG( "SavePeerDetectedAddress " << this->m_OuterAddress << " from " << sockAddr << " with " << detectedAddr << " " << isTCP );
	//Tracer::Trace("SavePeerDetectedAddress %s\n", strings::format_object(this->OuterAddress).c_str());
	CheckPeerNetType();
}

void PeerNetInfo::SaveDetectedAddress( UINT32 detectedIP, UINT16 detectedUDPPort, const SimpleSocketAddress& trackerAddr )
{
	this->m_DetectedIP = detectedIP;
	this->SaveOuterIP( detectedIP );
	if ( detectedUDPPort != 0 )
	{
		this->m_OuterAddress.UdpPort = detectedUDPPort;
	}
	VIEW_DEBUG( "SaveDetectedAddress " << this->m_OuterAddress << " from " << trackerAddr << " with " << SimpleSocketAddress(detectedIP, detectedUDPPort) );
	CheckPeerNetType();
}

void PeerNetInfo::AddAddress( UINT32 ip )
{
	PEER_ADDRESS peerAddr = this->Address;
	peerAddr.IP = ip;
	this->m_LocalAddressArray.push_back( peerAddr );
}

void PeerNetInfo::Load( const u_long ipArray[], size_t size )
{
	if ( size > 0 )
	{
		assert( size <= 15 );
		LIMIT_MAX( size, 15 );
		this->m_LocalIPArray.clear();
		this->m_LocalAddressArray.clear();
		for ( size_t index = 0; index < size; ++index )
		{
			UINT ip = ipArray[index];
			if ( CheckIPFullyValid( ip ) )
			{
				// 过滤掉0.x.x.x和255.x.x.x，还有169.254.x.x(dhcp失败的)
				this->m_LocalIPArray.push_back( ipArray[index] );
				this->AddAddress( ipArray[index] );
			}
		}
		if ( this->m_LocalIPArray.empty() )
		{
			assert( false );
			this->m_LocalIPArray.push_back( ipArray[0] );
			this->AddAddress( ipArray[0] );
		}
		u_long ip = FindRealExternalIP(this->m_LocalIPArray);
		if ( 0 != ip )
		{
			this->m_HasExternalIP = true;
			this->SaveOuterIP( ip );
			this->m_OuterAddress.TcpPort = this->Address.TcpPort;
			this->m_OuterAddress.UdpPort = this->Address.UdpPort;
			assert( this->m_OuterAddress.IsFullyValid() );
			this->Address.IP = ip;
		}
		else
		{
			this->Address.IP = ipArray[0];
		}
		assert(this->m_HasExternalIP == (this->m_OuterAddress.IP != 0));
		VIEW_DEBUG("LoadIPArray HasExternalIP = " << (this->m_HasExternalIP ? "true" : "false"));
	}
	else
	{
		assert(false);
	}
	assert( this->Address.IsFullyValid() );
	this->CheckPeerNetType();
}

void PeerNetInfo::Clear()
{
	this->CoreInfo.Clear();
	this->Address.Clear();

	this->m_LocalIPArray.clear();

	this->m_LocalAddressArray.clear();

	this->m_NATType = STUN_TYPE_ERROR;
	this->CoreInfo.NATType = STUN_TYPE_ERROR;

	this->m_OuterAddress.Clear();
	this->m_DetectedIP = 0;
	this->m_HasExternalIP = false;
	this->m_HasOuterIPUsedForUPNP = false;
	this->m_HasUPNPBeenEnabledForTracker = false;

	m_StunServerAddress.Clear();
	m_StunDetectedAddress.Clear();

}

std::vector<UINT32> PeerNetInfo::GetIPArray() const
{
	if ( this->IsUPNPEnabled() && this->m_OuterAddress.IP != 0 )
	{
		this->m_HasOuterIPUsedForUPNP = true;
		std::vector<UINT32> ipArray;
		ipArray.push_back( this->m_OuterAddress.IP );
		return ipArray;
	}
	return this->m_LocalIPArray;
}

std::vector<PEER_ADDRESS> PeerNetInfo::GetLocalAddresses() const
{
	assert(m_LocalAddressArray.size() <= 15);
	if ( this->IsUPNPEnabled() && this->m_OuterAddress.IP != 0 )
	{
		this->m_HasOuterIPUsedForUPNP = true;
		this->m_HasUPNPBeenEnabledForTracker = true;

		std::vector<PEER_ADDRESS> addrs;
		addrs.push_back( this->GetUPNPAddress() );
		return addrs;
	}
	return this->m_LocalAddressArray;
}

void PeerNetInfo::SaveOuterIP( UINT32 ip )
{
	assert( 0 != ip );
	this->m_OuterAddress.IP = ip;

	if ( std::find(this->m_LocalIPArray.begin(), this->m_LocalIPArray.end(), ip) != this->m_LocalIPArray.end() )
	{
		this->Address.IP = ip;
	}
}

void PeerNetInfo::SetStunAddress( const PEER_ADDRESS& serverAddress, const PEER_ADDRESS& detectedAddress )
{
	if ( serverAddress.IsUDPValid() )
	{
		m_StunServerAddress = serverAddress;
	}
	if ( detectedAddress.IsUDPValid() )
	{
		m_StunDetectedAddress = detectedAddress;
	}
}

class NormalPeerNetInfo : public PeerNetInfo
{
public:
	virtual void CheckPeerNetType() { }

	virtual bool IsUPNPEnabled() const { return false; }

	virtual PORT_PAIR GetOuterPorts() const
	{
		return this->Address.GetPorts();
	}

	virtual PEER_ADDRESS GetUPNPAddress() const
	{
		assert( this->IsUPNPEnabled() );
		return this->Address;
	}
};




PEER_ADDRESS GetProperUDPKeyPeerAddress( const PEER_ADDRESS& peerAddr, const SimpleSocketAddress& sockAddr )
{
	//	assert( peerAddr.IP == sockAddr.IP || IsPrivateIP( peerAddr.IP ) );
	if ( peerAddr.IP == sockAddr.IP && IsPrivateIP( sockAddr.IP ) )
		return peerAddr;
	PEER_ADDRESS keyAddress;
	keyAddress.IP = sockAddr.IP;
	keyAddress.TcpPort = peerAddr.TcpPort;
	keyAddress.UdpPort = sockAddr.Port;
	return keyAddress;
}


PEER_ADDRESS GetProperKeyPeerAddress( const PEER_ADDRESS& peerAddr, const SimpleSocketAddress& sockAddr, bool isUDP, bool isInitFromRemote, const PEER_ADDRESS& outerAddr )
{
	if ( false == isInitFromRemote )
	{
		// 本地发起的连接，直接返回peerAddr
		assert( peerAddr.IP == sockAddr.IP );
		if ( isUDP )
		{
			assert( peerAddr.UdpPort == sockAddr.Port );
		}
		else
		{
			assert( peerAddr.TcpPort == sockAddr.Port || 80 == sockAddr.Port || peerAddr.TcpPort == 80 );
		}
		return peerAddr;
	}
	// 远程发起的连接，参考sockAddr信息
	PEER_ADDRESS keyAddress;
	//	assert( peerAddr.IP == sockAddr.IP || IsPrivateIP( peerAddr.IP ) );
	keyAddress.IP = sockAddr.IP;
	if ( isUDP )
	{
		if ( sockAddr.IP == peerAddr.IP )
		{
			keyAddress = peerAddr;
		}
		else if ( outerAddr.IsFullyValid() )
		{
			keyAddress = outerAddr;
		}
		else
		{
			keyAddress.TcpPort = peerAddr.TcpPort;
			keyAddress.UdpPort = sockAddr.Port;
		}
	}
	else
	{
		keyAddress.UdpPort = peerAddr.UdpPort;
		if ( 80 == sockAddr.Port )
		{
			// 如果对方是80端口，则使用peerAddr中的tcp端口
			keyAddress.TcpPort = peerAddr.TcpPort;
		}
		else
		{
			keyAddress.TcpPort = sockAddr.Port;
		}
	}
	return keyAddress;
}





#ifdef _PPL_RUN_TEST

class NetInfoTestCase : public ppl::util::test_case
{
	virtual void DoRun()
	{
		UINT ip = ResolveHostName( "0.1.0.4" );
		assert( false == CheckIPValid( ip ) );
		assert( false == CheckIPFullyValid( ip ) );
	}
};

CPPUNIT_TEST_SUITE_REGISTRATION( NetInfoTestCase );

#endif

