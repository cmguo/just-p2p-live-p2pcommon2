
#ifndef _LIVE_P2PCOMMON2_NEW_COMMON_PACKET_SENDER_H_
#define _LIVE_P2PCOMMON2_NEW_COMMON_PACKET_SENDER_H_

#include "util/flow.h"
#include <ppl/net/socketfwd.h>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

class UDPSender;
typedef boost::shared_ptr<UDPSender> UDPSenderPtr;

class PacketBase;
class UDPPacketBuilder;
class SecureTrackerRequestPacketBuilder;

class UDPConnectionlessPacketBuilder;
class UDPConnectionPacketBuilder;

class TCPConnectionlessPacketBuilder;
class TCPConnectionPacketBuilder;

class InetSocketAddress;
class SimpleSocketAddress;

class PeerInformation;
class IDGenerator;
struct PEER_ADDRESS;


/// 报文发送的基类
class PacketSenderBase : private boost::noncopyable
{
public:
	PacketSenderBase() : m_Flow( NULL ), m_TotalFlow( NULL ) { }
	virtual ~PacketSenderBase() { }

	RateMeasure& GetFlow() { return *m_Flow; }
	const RateMeasure& GetFlow() const { return *m_Flow; }
	void UpdateFlow() { m_Flow->Update(); }
	UINT64 GetTotalBytes() const { return m_Flow->GetTotalBytes(); }

protected:
	RateMeasure* m_Flow;
	RateMeasure* m_TotalFlow;
};

/// 报文发送的模板实现类
template <typename PacketBuilderT>
class PacketSenderImpl : public PacketSenderBase
{
public:
	PacketSenderImpl() { }

	boost::shared_ptr<PacketBuilderT> GetBuilder() { return m_Builder; }

protected:
	boost::shared_ptr<PacketBuilderT> m_Builder;
};


class TrackerPacketSender : public PacketSenderBase
{
public:
	TrackerPacketSender( boost::shared_ptr<UDPPacketBuilder> builder, boost::shared_ptr<SecureTrackerRequestPacketBuilder> secureBuilder, 
		UDPSenderPtr normalSender, UDPSenderPtr tcpProxySender, UDPSenderPtr httpProxySender, 
		boost::shared_ptr<PeerInformation> peerInformation, RateMeasure& flow, RateMeasure& totalFlow );

	size_t Send( const PacketBase& body, const InetSocketAddress& sockAddr, UINT8 proxyType, bool isSecure );

	UINT32 GetTransactionID() const;

protected:
       UDPSenderPtr m_NormalSender;	
       UDPSenderPtr m_TCPProxySender;
       UDPSenderPtr m_HTTPProxySender;
       boost::shared_ptr<PeerInformation> m_PeerInformation;
       boost::shared_ptr<UDPPacketBuilder> m_Builder;
       boost::shared_ptr<SecureTrackerRequestPacketBuilder> m_SecureBuilder;
       boost::shared_ptr<IDGenerator> m_TransactionID;
};



/// 无连接udp报文发送
class UDPConnectionlessPacketSender : public PacketSenderImpl<UDPConnectionlessPacketBuilder>
{
public:
	UDPConnectionlessPacketSender( boost::shared_ptr<UDPConnectionlessPacketBuilder> builder, UDPSenderPtr sender, boost::shared_ptr<PeerInformation> peerInformation, RateMeasure& flow, RateMeasure& totalFlow );

	size_t Send( const PacketBase& body, UINT32 transactionID, const SimpleSocketAddress& sockAddr );

	void UpdateInformation( const PEER_ADDRESS& detectedAddr );

	UDPSenderPtr GetUDPSender() { return m_Sender; }

private:
	UDPSenderPtr m_Sender;
	boost::shared_ptr<PeerInformation> m_PeerInformation;
};


/// 有连接udp报文发送
class UDPConnectionPacketSender : public PacketSenderImpl<UDPConnectionPacketBuilder>
{
public:
	UDPConnectionPacketSender( boost::shared_ptr<UDPConnectionPacketBuilder> builder, UDPSenderPtr sender, RateMeasure& flow, RateMeasure& totalFlow );

	size_t Send( const PacketBase& body, UINT32 transactionID, UINT32 sequenceID, UINT32 sessionKey, const SimpleSocketAddress& sockAddr );
	UINT32   GetCurTransactionID(); // Added by Tady, 081208: For MRP.
private:
	UDPSenderPtr m_Sender;
};

/// 连接tcp报文发送
class TCPConnectionlessPacketSender : public PacketSenderImpl<TCPConnectionlessPacketBuilder>
{
public:
	explicit TCPConnectionlessPacketSender( boost::shared_ptr<TCPConnectionlessPacketBuilder> builder, boost::shared_ptr<PeerInformation> peerInformation, RateMeasure& flow, RateMeasure& totalFlow );

	size_t Send( const PacketBase& body, tcp_socket_ptr sock, const InetSocketAddress& remoteSockAddr );

	void UpdateInformation( const PEER_ADDRESS& detectedAddr );

protected:
	boost::shared_ptr<PeerInformation> m_PeerInformation;
};

/// 有连接tcp报文发送
class TCPConnectionPacketSender : public PacketSenderImpl<TCPConnectionPacketBuilder>
{
public:
	explicit TCPConnectionPacketSender( boost::shared_ptr<TCPConnectionPacketBuilder> builder, RateMeasure& flow, RateMeasure& totalFlow );

	size_t Send( const PacketBase& body, tcp_socket_ptr sock );
};

#endif




