
#ifndef _LIVE_P2PCOMMON2_NEW_CORE_PEER_CHANNEL_IMPL_H_
#define _LIVE_P2PCOMMON2_NEW_CORE_PEER_CHANNEL_IMPL_H_

#include "PeerChannel.h"
#include "framework/memory.h"
#include <synacast/protocol/data/SimpleSocketAddress.h>

#include <ppl/net/socketfwd.h>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <list>

class PeerConnection;
class CPeerManager;
class TCPConnectionPacketSender;
class UDPConnectionPacketSender;
class SimpleSocketAddress;


/// 数据传输通道的基础实现类
class PeerChannelImpl :  public PeerChannel, public pool_object, private boost::noncopyable
{
protected:
	explicit PeerChannelImpl(PeerConnection& pc);

	virtual bool HandleUDPSessionPacket(data_input_stream& is, const NEW_UDP_PACKET_HEAD& head, const UDP_SESSION_INFO& sessionInfo, const SimpleSocketAddress& sockAddr) { assert(false); return false; }

	/// 什么都不做，返回true，表示一切正常
	virtual bool OnAppTimer(UINT seconds) { return true; }

	virtual bool IsUDPT() const { return false; }
	virtual bool IsMultiConnection() const { return false; }
	virtual size_t GetTunnelCount() const { return 0; }
	virtual int GetSendPending() { return 0; }

	virtual bool IsLastMRPResponseReceived() { return false;}
// 	virtual size_t SendPacket(const PacketBase& packet) { return 0; }
// 	virtual size_t SendPacket(const PacketBase& packet, bool bNeedRepeatMRP){ return SendPacket(packet); }
	virtual void OnTimerForMRP() {};
protected:
	PeerConnection& m_Connection;
};


/// 用于普通tcp的peer连接的数据传输通道类
class TCPPeerChannel : public PeerChannelImpl, protected tcp_socket_listener
{
public:
	explicit TCPPeerChannel( PeerConnection& pc, boost::shared_ptr<TCPConnectionPacketSender> packetSender, tcp_socket_ptr sock );
	virtual ~TCPPeerChannel();

	bool HandlePacket(const BYTE* data, size_t size);

	virtual size_t SendPacket(const PacketBase& packet);

	virtual UINT32 GetTotalPacketCount() const { return m_TotalPacketCount; }
	virtual UINT32 GetReceivedPacketCount() const { return m_TotalPacketCount; }
	virtual UINT32 GetSentPacketCount() const { return m_SentPacketCount; }

	virtual size_t GetTunnelCount() const { return 1; }

	virtual void on_socket_receive(tcp_socket* sender, BYTE* data, size_t size);
	virtual void on_socket_receive_failed(tcp_socket* sender, int errcode);
	virtual int GetSendPending();
	
	virtual size_t SendPacket(const PacketBase& packet, bool bNeedRepeatMRP) { return 0; }
	virtual size_t SendPacket(boost::shared_ptr<SubPieceUnitRequestPacket> packet) { return 0; }
protected:
	size_t m_TotalPacketCount;
	UINT32 m_SentPacketCount;
	boost::shared_ptr<TCPConnectionPacketSender> m_PacketSender;
	tcp_socket_ptr m_Socket;
};

#define OldRequestIDArray_Len 100
/// 用于udpt的peer连接的数据传输通道类
class UDPPeerChannel : public PeerChannelImpl
{
public:
	explicit UDPPeerChannel(PeerConnection& pc, boost::shared_ptr<UDPConnectionPacketSender> packetSender, const SimpleSocketAddress& remoteSocketAddr, UINT32 localSessionKey, UINT32 remoteSessionKey);
	virtual ~UDPPeerChannel();

	virtual size_t SendPacket(const PacketBase& packet);

	virtual UINT32 GetTotalPacketCount() const { return m_TotalPacketCount; }
	virtual UINT32 GetReceivedPacketCount() const { return m_ReceivedPacketCount; }
	virtual UINT32 GetSentPacketCount() const { return m_SequenceID; }


	virtual bool HandleUDPSessionPacket(data_input_stream& is, const NEW_UDP_PACKET_HEAD& head, const UDP_SESSION_INFO& sessionInfo, const SimpleSocketAddress& sockAddr);

	// Added by Tady, 081108: For multi-request-packet.
	virtual bool IsLastMRPResponseReceived() { return m_bIsLastMRPResponseReceived; } 
	virtual size_t SendPacket(const PacketBase& packet, bool bNeedRepeatMRP);
	virtual size_t SendPacket(boost::shared_ptr<SubPieceUnitRequestPacket> packet); // Added by Tady, 081308: Just For MRP.
	virtual void OnTimerForMRP();
	struct MRPElem 
	{
		boost::shared_ptr<SubPieceUnitRequestPacket>	packet;
		UINT32		transactionID;
		UINT32		sendTimes;
	};
	typedef std::list<MRPElem> MRPList;
	MRPList m_MRPList;

protected:
	/// 对方分配给自己的key，即Key(Remote,Local)
	UINT32 m_LocalSessionKey;
	/// 自己分配给对方的key，即Key(Local,Remote)
	UINT32 m_RemoteSessionKey;
	UINT32 m_SequenceID;
	SimpleSocketAddress m_RemoteSocketAddress;
	UINT32 m_TotalPacketCount;
	UINT32 m_ReceivedPacketCount;
	boost::shared_ptr<UDPConnectionPacketSender> m_PacketSender;

	/// 上一次收到的命令字
	UINT8 m_LastRequestAction;

	/// 上一次的TransactionID
	UINT32 m_LastInRequestTransactionID;
	// Added by Tady, 081108: For multi-request-packet.
	UINT32	m_LastOutRequestTransactionID;
	UINT32	m_iOldOutRequestTransactionIDs[OldRequestIDArray_Len];
	bool	m_bIsLastMRPResponseReceived;
	UINT32  m_iOldInRequestTransactionIDs[OldRequestIDArray_Len];
};

#endif
