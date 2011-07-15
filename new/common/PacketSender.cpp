
#include "StdAfx.h"

#include "PacketSender.h"
#include "UDPSender.h"
#include "BaseInfo.h"

#include "framework/socket.h"
#include <synacast/protocol/PacketBuilder.h>


inline void CheckSocketAddressValid(UINT32 ip, UINT16 port)
{
	assert(ip != 0 && ip != INADDR_NONE && ip != INADDR_ANY);
	assert(port != 0);
	//assert( CheckIPValid( ip ) );
}

inline void CheckSocketAddressValid(const InetSocketAddress& sockAddr)
{
	CheckSocketAddressValid(sockAddr.GetIP(), sockAddr.GetPort());
}

inline void CheckSocketAddressValid(const SimpleSocketAddress& sockAddr)
{
	CheckSocketAddressValid(sockAddr.IP, sockAddr.Port);
}




TrackerPacketSender::TrackerPacketSender( 
	boost::shared_ptr<UDPPacketBuilder> builder, boost::shared_ptr<SecureTrackerRequestPacketBuilder> secureBuilder, 
	UDPSenderPtr normalSender, UDPSenderPtr tcpProxySender, UDPSenderPtr httpProxySender, 
	boost::shared_ptr<PeerInformation> peerInformation, RateMeasure& flow, RateMeasure& totalFlow )
	: m_NormalSender( normalSender ), m_TCPProxySender( tcpProxySender ), m_HTTPProxySender( httpProxySender ), 
	  m_PeerInformation( peerInformation ), m_Builder( builder ), m_SecureBuilder( secureBuilder )
{
	m_Flow = &flow;
	m_TotalFlow = &totalFlow;
	m_TransactionID = builder->GetTransactionID();
}

UINT32 TrackerPacketSender::GetTransactionID() const
{
	return m_TransactionID->Current();
}

size_t TrackerPacketSender::Send(const PacketBase& body, const InetSocketAddress& sockAddr, UINT8 proxyType, bool isSecure)
{
	CheckSocketAddressValid(sockAddr);

	const BYTE* data = NULL;
	size_t size = 0;
	bool res = false;
	if ( isSecure )
	{
		m_SecureBuilder->UpdateCoreInfo( m_PeerInformation->NetInfo->CoreInfo );
		m_SecureBuilder->Build( body, 0 );
		data = m_SecureBuilder->GetData();
		size = m_SecureBuilder->GetSize();
	}
	else
	{
		m_Builder->Build(body, 0);
		data = m_Builder->GetData();
		size = m_Builder->GetSize();
	}

	assert( data != NULL && size > 0 && FALSE == ::IsBadReadPtr(data, size) );

	if (proxyType == PROXY_UDP)
	{
		res = m_NormalSender->Send(data, size, sockAddr);
	}
	else if (proxyType == PROXY_TCP)
	{
		res = m_TCPProxySender->Send(data, size, sockAddr);
	}
	else if (proxyType == PROXY_HTTP)
	{
		res = m_HTTPProxySender->Send(data, size, sockAddr);
	}
	else
	{
		res = false;
		APP_ERROR("Unsupported proxy type. " << proxyType << " to " << sockAddr << " with " << make_buffer_pair(data, size));
		assert(!"Unsupported proxy type.");
	}

	if (res)
	{
		m_Flow->Record((UINT)size);
		m_TotalFlow->Record( (UINT) size );
	}
	return res ? size : 0;
}




size_t UDPConnectionPacketSender::Send( const PacketBase& body, UINT32 transactionID, UINT32 sequenceID, UINT32 sessionKey, const SimpleSocketAddress& sockAddr )
{
	CheckSocketAddressValid(sockAddr);
	assert( sessionKey != 0 );
	m_Builder->Build( body, transactionID, sequenceID, sessionKey );
	size_t size = m_Builder->GetSize();
	assert( size > 3 );
	const BYTE* data = m_Builder->GetData();
	assert( data[0] != 0xE9 || data[1] != 0x03 );
	if ( m_Sender->Send( m_Builder->GetData(), size, sockAddr.ToInetSocketAddress() ) )
	{
		m_Flow->Record((UINT)size);
		m_TotalFlow->Record( (UINT) size );
		return size;
	}

	return 0;
}

UINT32 UDPConnectionPacketSender::GetCurTransactionID()
{
	return GetBuilder()->GetTransactionID()->Current();
}

UDPConnectionPacketSender::UDPConnectionPacketSender( boost::shared_ptr<UDPConnectionPacketBuilder> builder, UDPSenderPtr sender, RateMeasure& flow, RateMeasure& totalFlow ) 
	: m_Sender( sender )
{
	m_Flow = &flow;
	m_TotalFlow = &totalFlow;
	m_Builder = builder;
}
size_t UDPConnectionlessPacketSender::Send( const PacketBase& body, UINT32 transactionID, const SimpleSocketAddress& sockAddr )
{
	CheckSocketAddressValid( sockAddr );
	//m_Builder->SetPeerAddress( peerAddr, peerAddr );
	PEER_ADDRESS detectedAddr;
	detectedAddr.IP = sockAddr.IP;
	detectedAddr.UdpPort = sockAddr.Port;
	detectedAddr.TcpPort = 0;

	this->UpdateInformation( detectedAddr );
//	m_Builder->SetPeerAddress( m_PeerInformation->NetInfo->Address );
//	m_Builder->SetDegrees( m_PeerInformation->StatusInfo->Degrees );
//	m_Builder->SetOuterPeerAddress( m_PeerInformation->NetInfo->OuterAddress );

	m_Builder->Build( body, transactionID );
	size_t size = m_Builder->GetSize();
	assert( size > 3 );
	const BYTE* data = m_Builder->GetData();
	assert( data[0] != 0xE9 || data[1] != 0x03 );
	if ( m_Sender->Send( m_Builder->GetData(), size, sockAddr.ToInetSocketAddress() ) )
	{
		m_Flow->Record( (UINT)size );
		m_TotalFlow->Record( (UINT) size );
		return size;
	}
	return 0;
}

UDPConnectionlessPacketSender::UDPConnectionlessPacketSender( boost::shared_ptr<UDPConnectionlessPacketBuilder> builder, UDPSenderPtr sender, boost::shared_ptr<PeerInformation> peerInformation, RateMeasure& flow, RateMeasure& totalFlow ) 
	: m_Sender( sender ), m_PeerInformation( peerInformation )
{
	m_Flow = &flow;
	m_TotalFlow = &totalFlow;
	m_Builder = builder;
}

void UDPConnectionlessPacketSender::UpdateInformation( const PEER_ADDRESS& detectedAddr )
{
	const PeerNetInfo& netInfo = *m_PeerInformation->NetInfo;
	m_Builder->PeerInfo.Update( netInfo.Address, netInfo.GetProperOuterAddress(), m_PeerInformation->StatusInfo->Degrees );
	m_Builder->PeerInfo.UpdateCoreInfo( netInfo.CoreInfo );
	m_Builder->PeerInfo.SetDetectedAddress( detectedAddr );
}

size_t TCPConnectionlessPacketSender::Send( const PacketBase& body, tcp_socket_ptr sock, const InetSocketAddress& remoteSockAddr )
{
	//m_Builder->SetPeerAddress( peerAddr, peerAddr );
	CheckSocketAddressValid( remoteSockAddr );
	PEER_ADDRESS detectedAddr;
	detectedAddr.IP = remoteSockAddr.GetRawIP();
	detectedAddr.TcpPort = remoteSockAddr.GetPort();
	detectedAddr.UdpPort = 0;

	this->UpdateInformation( detectedAddr );

	//InetSocketAddress localSockAddr = sock->GetLocalAddress();
	//PEER_ADDRESS localPeerAddr;
	//localPeerAddr.IP = localSockAddr.GetRawIP();
	//localPeerAddr.TcpPort = localSockAddr.GetPort();
	//localPeerAddr.UdpPort = m_PeerInformation->NetInfo->Address.UdpPort;

//	m_Builder->SetPeerAddress( m_PeerInformation->NetInfo->Address );
//	m_Builder->SetDegrees( m_PeerInformation->StatusInfo->Degrees );
	m_Builder->PeerInfo.SetDetectedAddress( detectedAddr );
//	m_Builder->SetOuterPeerAddress( m_PeerInformation->NetInfo->OuterAddress );

	m_Builder->Build( body );
	size_t size = m_Builder->GetSize();
	assert( size > 0 );
	assert( size > 3 );
	const BYTE* data = m_Builder->GetData();
	assert( data[0] != 0xE9 || data[1] != 0x03 );
	if ( sock->send( m_Builder->GetData(), size ) )
	{
		m_Flow->Record( (UINT)size );
		m_TotalFlow->Record( (UINT) size );
		return size;
	}
	return 0;
}

TCPConnectionlessPacketSender::TCPConnectionlessPacketSender( boost::shared_ptr<TCPConnectionlessPacketBuilder> builder, boost::shared_ptr<PeerInformation> peerInformation, RateMeasure& flow, RateMeasure& totalFlow ) 
	: m_PeerInformation( peerInformation )
{
	m_Flow = &flow;
	m_TotalFlow = &totalFlow;
	m_Builder = builder;
}

void TCPConnectionlessPacketSender::UpdateInformation( const PEER_ADDRESS& detectedAddr )
{
	const PeerNetInfo& netInfo = *m_PeerInformation->NetInfo;
	m_Builder->PeerInfo.Update( netInfo.Address, netInfo.GetProperOuterAddress(), m_PeerInformation->StatusInfo->Degrees );
	m_Builder->PeerInfo.UpdateCoreInfo( netInfo.CoreInfo );
	m_Builder->PeerInfo.SetDetectedAddress( detectedAddr );
}


size_t TCPConnectionPacketSender::Send( const PacketBase& body, tcp_socket_ptr sock )
{
	m_Builder->Build( body );
	size_t size = m_Builder->GetSize();
	assert( size > 0 );
	if ( sock->send( m_Builder->GetData(), size ) )
	{
		m_Flow->Record( (UINT)size );
		m_TotalFlow->Record( (UINT) size );
		return size;
	}
	return 0;
}

TCPConnectionPacketSender::TCPConnectionPacketSender( boost::shared_ptr<TCPConnectionPacketBuilder> builder, RateMeasure& flow, RateMeasure& totalFlow )
{
	m_Flow = &flow;
	m_TotalFlow = &totalFlow;
	m_Builder = builder;
}


