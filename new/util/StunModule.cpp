
#include "StdAfx.h"

#include "StunModule.h"
#include "common/BaseInfo.h"
#include "common/UDPSender.h"
#include "framework/log.h"

#include <ppl/util/random.h>

#include <boost/static_assert.hpp>
#include <synacast/protocol/base.h>

BOOST_STATIC_ASSERT( sizeof(bool) == 1 );

#include <synacast/util/IDGenerator.h>

#include <ppl/concurrent/thread.h>
#include "util/StunClient.h"

#include <ppl/io/data_input_stream.h>
#include <ppl/io/data_output_stream.h>

ppl::io::data_input_stream & operator>>(ppl::io::data_input_stream & os, CPeerAddr & addr)
{
	os.read_n(&addr, sizeof(addr));
        return os;
}

class StunClientThread : public ppl::concurrent::thread
{
public:
	explicit StunClientThread(StunTypeResolver::CallbackType callback) : m_callback(callback), m_NATType(STUN_TYPE_ERROR)
	{

	}

protected:
	virtual void do_run()
	{
		m_NATType = m_StunClient.StartGetNatTpye();
		m_callback(m_NATType);
		StunTypeResolver::CallbackType null;
		m_callback.swap(null); 
	}

	virtual void do_interrupt()
	{
		m_StunClient.Stop();
	}

private:
	StunTypeResolver::CallbackType m_callback;
	CStunClient m_StunClient;
	MY_STUN_NAT_TYPE m_NATType;
};


StunTypeResolver::StunTypeResolver()
{
	m_callback = 0;
	m_bIsHandling = false;
}

StunTypeResolver::~StunTypeResolver()
{
	if ( m_thread )
	{
		m_thread->stop(0);
		m_thread.reset();
	}

}

void StunTypeResolver::Resolve( CallbackType callback )
{
	m_callback = callback;
	if ( m_thread )
	{
		m_thread->stop(0);
	}
	m_thread.reset( new StunClientThread(CallbackType(boost::bind(&StunTypeResolver::HandleNATType, this->shared_from_this(), _1))) );
	m_thread->start();
}

void StunTypeResolver::Cancel()
{
// 	StunTypeResolver::CallbackType null;
// 	m_callback.swap(null); 
	m_callback.clear();
	
	int iCount(30);
	while(m_bIsHandling && iCount > 0)
	{
#ifdef WIN32
    ::Sleep(100);
#else
        sleep(100);
#endif
		
		--iCount;
	}
	if ( m_bIsHandling )
	{
		m_thread->stop(1000);
	}
}


StunModule::StunModule( const std::vector<InetSocketAddress>& stunServers, UDPSenderPtr sender, boost::shared_ptr<const PeerInformation> peerInformation, UINT16 peerVersion ) 
	: m_PacketSender( sender )
	, m_PeerInformation( peerInformation )
	, m_PacketBuilder( boost::shared_ptr<IDGenerator>( new IDGenerator(RandomGenerator().NextIn(USHRT_MAX) + 0x1111) ), peerVersion )
	, m_CurrentServer( 0 )
        , m_Loginned( false )
    	, m_LoginFailedTimes( 0 )
{
	m_KeepAliveTimer.set_callback(boost::bind(&StunModule::KeepAlive, this));
	m_LoginTimer.set_callback(boost::bind(&StunModule::OnLoginTimer, this));
	m_LoginTimeoutTimer.set_callback(boost::bind(&StunModule::OnLoginFailed, this));

	m_ServerList.clear();
	if ( stunServers.empty() )
	{
		//m_ServerList.push_back(InetSocketAddress(ResolveHostName("59.151.34.54"), 7107));
		m_ServerList.push_back(InetSocketAddress(ResolveHostName("124.225.112.214"), 7107));
		m_ServerList.push_back(InetSocketAddress(ResolveHostName("218.61.6.139"), 7107));
		//m_ServerList.push_back(InetSocketAddress(ResolveHostName("127.0.0.1"), 7107));
		//m_ServerList.push_back(InetSocketAddress(ResolveHostName("61.155.8.27"), 7107));

		//	m_ServerList.push_back(InetSocketAddress(ResolveHostName("192.168.21.205"), 7107));

		//	m_ServerList.push_back(InetSocketAddress(ResolveHostName("59.151.34.54"), 7207));
		//	m_ServerList.push_back(InetSocketAddress(ResolveHostName("59.151.34.54"), 7217));
		//	m_ServerList.push_back(InetSocketAddress(ResolveHostName("59.151.34.54"), 7227));
		//	m_ServerList.push_back(InetSocketAddress(ResolveHostName("59.151.34.54"), 7237));
		//	m_ServerList.push_back(InetSocketAddress(ResolveHostName("59.151.34.54"), 7247));
		//	m_ServerList.push_back(InetSocketAddress(ResolveHostName("59.151.34.54"), 7257));
	}
	else
	{
		m_ServerList = stunServers;
	}
	m_CurrentServer = RandomGenerator().Next() % m_ServerList.size();
	LIVE_ASSERT(m_CurrentServer < m_ServerList.size());

}

StunModule::~StunModule()
{

}

bool StunModule::NeedStun() const
{
	return m_PeerInformation->NetInfo->IsNAT() && m_PeerInformation->NetInfo->GetPeerType() == NORMAL_PEER;
}

void StunModule::Start()
{
	m_LoginTimer.start(3 * 60 * 1000);
	LIVE_ASSERT( false == m_LoginTimeoutTimer.is_started() );
	m_LoginTimeoutTimer.stop();

	if ( false == NeedStun() )
		return;

	this->Login();
}

CPeerAddr StunModule::GetLocalAddress() const
{
	const PeerNetInfo& netInfo = *m_PeerInformation->NetInfo;
	return CPeerAddrUtil::FromPeerAddress(netInfo.Address, netInfo.IsNAT(), netInfo.GetNATType());
}

bool StunModule::Login()
{
	if ( false == this->NeedStun() )
		return false;
	if ( m_LoginTimeoutTimer.is_started() )
	{
		LIVE_ASSERT( false );
		return false;
	}
	m_LoginTimeoutTimer.stop();
	m_LoginTimeoutTimer.start( 6 * 1000 );
	CPeerAddr localAddr = this->GetLocalAddress();
	m_PacketBuilder.BuildLoginPacket( localAddr );
	return m_PacketSender->Send(m_PacketBuilder.GetData(), m_PacketBuilder.GetSize(), this->GetServerAddress());
}

bool StunModule::KeepAlive()
{
	if ( false == this->NeedStun() )
		return false;
	CNetHeader header;
	m_PacketBuilder.InitStunNetHeader(header);
	header.m_usNetType = NHTYPE_CONFUSE_KEEPALIVE;
	VIEW_INFO("nat keepalive to " << this->GetServerAddress());
	return m_PacketSender->Send( &header, sizeof(CNetHeader), this->GetServerAddress() );
}

bool StunModule::Transmit( const BYTE* data, size_t size, const SimpleSocketAddress& targetAddr, const SimpleSocketAddress& stunServerAddress )
{
//	if ( false == this->NeedStun() )
//		return false;
	const PeerNetInfo& netInfo = *m_PeerInformation->NetInfo;
	CPeerAddr localAddr = m_DetectedAddress;
	if ( 0 == localAddr.m_uIP || 0 == localAddr.m_usPort )
	{
		CPeerAddrUtil::AssignFromPeerAddress(localAddr, netInfo.GetProperOuterAddress());
	}
	CPeerAddr targetPeerAddr = CPeerAddrUtil::FromSimpleSocketAddress(targetAddr, false, 0);
	m_PacketBuilder.BuildTransmitPacket(data, size, localAddr, targetPeerAddr);
	//m_PacketBuilder.BuildTransmitPacket(data, size, localAddr, localAddr);
	InetSocketAddress serverAddr(stunServerAddress.IP, stunServerAddress.Port);
	VIEW_INFO("nat transmit packet to " << targetAddr << " via " << stunServerAddress);
	return m_PacketSender->Send(m_PacketBuilder.GetData(), m_PacketBuilder.GetSize(), serverAddr);
}

void StunModule::OnLoginSucceeded()
{
	m_LoginFailedTimes = 0;
	m_Loginned = true;
	m_LoginTimeoutTimer.stop();
	m_KeepAliveTimer.stop();
	m_KeepAliveTimer.start(10 * 1000);
}

void StunModule::OnLoginFailed()
{
	++m_LoginFailedTimes;
	m_KeepAliveTimer.stop();
	LIVE_ASSERT( false == m_LoginTimeoutTimer.is_started() );
	m_LoginTimeoutTimer.stop();
	m_Loginned = false;
	if ( m_LoginFailedTimes >= 3 )
	{
		// 切换到下一个nat服务器
		m_LoginFailedTimes = 0;
		m_CurrentServer++;
		if ( m_CurrentServer >= m_ServerList.size() )
		{
			m_CurrentServer = 0;
		}
	}
	this->Login();
}

static inline boost::uint16_t little_endian_to_host_short(
    boost::uint16_t v)
{
    char * ch = (char *)&v;
    return (boost::uint16_t)ch[1] << 8 
        | (boost::uint16_t)ch[0];
}


bool StunModule::HandlePacket( BYTE* data, size_t size, const InetSocketAddress& remoteAddr )
{
	if ( m_PeerInformation->NetInfo->GetPeerType() != NORMAL_PEER )
		return false;

	const size_t total_header_size = sizeof(CNetHeader) + sizeof(ShortP2PProtocolHeader);
	if ( size < total_header_size )
	{
		return false;
	}

	CNetHeader* pstNetHeader = (CNetHeader*)data;
	if ( NHTYPE_CONFUSE_PFS_SHORT_P2PHEADER != little_endian_to_host_short(pstNetHeader->m_usNetType ))
		return false;

	//去除混淆协议
	StunRequestPacketBuilder::unConfuse((INT8*)data, size,NHTYPE_CONFUSE_PFS_SHORT_P2PHEADER);

	LPSHORTP2PPROTOCOLHEADER lpCmdPLHeader = (LPSHORTP2PPROTOCOLHEADER)(data+NETHEADER_LENGTH);
	BYTE* realData = reinterpret_cast<BYTE*>( lpCmdPLHeader + 1 );
	size_t realSize = size - total_header_size;
	const size_t transmit_info_size = sizeof(CPeerAddr) * 2;
	switch(lpCmdPLHeader->protocolType)
	{
	case RTN_PUB_PORT:
		if ( realSize >= sizeof(CPeerAddr) )
		{
			//HandleReqPubAddressRequest(pstTransport);
			VIEW_INFO("stun RTN_PUB_PORT ok. " << remoteAddr);
			m_DetectedAddress = READ_MEMORY(lpCmdPLHeader + 1, CPeerAddr);
			m_LoginCallback();
			return true;
		}
		break;
	case RTN_TRANSMIT_PACKEG:
		if ( realSize > transmit_info_size)
		{
			VIEW_INFO("stun RTN_TRANSMIT_PACKEG ok. " << remoteAddr);
			CPeerAddr srcAddr = READ_MEMORY(realData, CPeerAddr);
			m_TransmitCallback(realData + transmit_info_size, realSize - transmit_info_size, CPeerAddrUtil::ToSocketAddress(srcAddr));
			return true;
		}
		break;
	default:
		{
			// 释放无法识别的网络包
			//CloseTransport(pstTransport);
			break;
		}
	}
	return false;
}

void StunModule::SetCallbacks( LoginCallbackType loginCallback, TransmitCallbackType transmitCallback )
{
	m_LoginCallback = loginCallback;
	m_TransmitCallback = transmitCallback;
}

