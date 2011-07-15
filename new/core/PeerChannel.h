
#ifndef _LIVE_P2PCOMMON2_NEW_CORE_PEER_CHANNEL_H_
#define _LIVE_P2PCOMMON2_NEW_CORE_PEER_CHANNEL_H_

#include <synacast/protocol/DataIO.h>
#include <ppl/net/socketfwd.h>
#include <boost/shared_ptr.hpp>


class PacketBase;
class PeerConnectionInfo;
class PeerHandshakeInfo;
class PeerConnection;
struct UDP_PEER_PACKET_HEAD;
struct UDPT_HEAD_INFO;
class UDPConnectionPacketSender;
class TCPConnectionPacketSender;

class SubPieceUnitRequestPacket;

struct NEW_UDP_PACKET_HEAD;
struct UDP_SESSION_INFO;
class SimpleSocketAddress;


/// ���ݴ���ͨ���Ľӿ�
class PeerChannel
{
public:
	virtual ~PeerChannel() { }

	/// ���ͱ���
	virtual size_t SendPacket(const PacketBase& packet) = 0;

	/// ������һЩ����
	virtual bool OnAppTimer(UINT seconds) = 0;

	/// �Ƿ���udpt����
	virtual bool IsUDPT() const = 0;

	/// �Ƿ��Ƕ�����
	virtual bool IsMultiConnection() const = 0;

	virtual UINT32 GetTotalPacketCount() const = 0;
	virtual UINT32 GetReceivedPacketCount() const = 0;
	virtual UINT32 GetSentPacketCount() const = 0;

	virtual size_t GetTunnelCount() const = 0;

	virtual int GetSendPending() = 0;

	virtual bool HandleUDPSessionPacket(data_input_stream& is, const NEW_UDP_PACKET_HEAD& head, const UDP_SESSION_INFO& sessionInfo, const SimpleSocketAddress& sockAddr) = 0;
	// Added by Tady, 081108: For multi-request-packet.
	virtual bool IsLastMRPResponseReceived() = 0;
	virtual size_t SendPacket(const PacketBase& packet, bool bNeedRepeatMRP) = 0;
	virtual size_t SendPacket(boost::shared_ptr<SubPieceUnitRequestPacket> packet) = 0;
	virtual void OnTimerForMRP() = 0; // For MRP repeat. step is 250ms
};



/// PeerChannel���๤��
class PeerChannelFactory
{
public:
	static PeerChannel* CreateTCP( PeerConnection& pc, boost::shared_ptr<TCPConnectionPacketSender> packetSender, tcp_socket_ptr sock );

	static PeerChannel* CreateUDP( PeerConnection& pc, boost::shared_ptr<UDPConnectionPacketSender> packetSender, const SimpleSocketAddress& remoteSocketAddr, UINT32 localSessionKey, UINT32 remoteSessionKey );
};

#endif


