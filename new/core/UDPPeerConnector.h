
#ifndef _LIVE_P2PCOMMON2_NEW_CORE_UDPPEER_CONNECTOR_H_
#define _LIVE_P2PCOMMON2_NEW_CORE_UDPPEER_CONNETTOR_H_

#include <synacast/util/IDGenerator.h>
#include <synacast/protocol/DataIO.h>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <map>
#include <common/GloalTypes.h>
struct PEER_ADDRESS;
struct PACKET_PEER_INFO;
struct NEW_UDP_PACKET_HEAD;
struct UDP_SESSION_INFO;

class SimpleSocketAddress;


class PeerConnector;
class CPeerManager;
class CIPPool;
class AppModule;
class PeerConnection;

class PeerNetInfo;
class PeerStatusInfo;
class CLiveInfo;

class UDPPeerConnectionInfo;
class UDPTHandshakePacketInfo;

class PeerConnectorStatistics;
class PeerConnectParam;
enum ExtraProxyEnum;

class UDPPendingPeer;

typedef boost::shared_ptr<UDPPendingPeer> UDPPendingPeerPtr;
typedef std::map<SimpleSocketAddress, UDPPendingPeerPtr> UDPPendingPeerCollection;
typedef std::map<UINT32, UDPPendingPeerPtr> UDPPendingPeerKeyIndex;


class UDPPeerConnector : private boost::noncopyable
{
public:
	explicit UDPPeerConnector( PeerConnector& owner );

	bool Connect(const PEER_ADDRESS& addr, const PeerConnectParam& baseInfo);

	void HandleUDPTConnectFail(boost::shared_ptr<UDPPeerConnectionInfo> connInfo, UINT16 errcode, UINT32 refuseReason);

	void OnUDPTHandshakeError( UDPPendingPeerPtr peer, UINT16 errcode, UINT32 refuseReason);

	size_t GetPendingPeerCount() const
	{
		assert(m_IndexedPeers.size() <= m_udptHandshakingPeers.size());
		return m_udptHandshakingPeers.size();
	}


	UDPPendingPeerPtr CreateUDPTPendingPeer(const SimpleSocketAddress& sockAddr, boost::shared_ptr<UDPPeerConnectionInfo> connInfo);
	void AddUDPTPendingPeer(UDPPendingPeerPtr peer);


	PeerConnection* OnUDPTPeerHandshaked(UDPPendingPeerPtr peer);

	/// 处理udpt报文
	bool HandlePacket(data_input_stream& is, const NEW_UDP_PACKET_HEAD& head, const PACKET_PEER_INFO& packetPeerInfo, const SimpleSocketAddress& sockAddr, ExtraProxyEnum extraProxy);
	bool HandleSessionPacket(data_input_stream& is, const NEW_UDP_PACKET_HEAD& head, const UDP_SESSION_INFO& sessionInfo, const SimpleSocketAddress& sockAddr);

	void HandleUDPTError(const BYTE* body, size_t size, const PACKET_PEER_INFO& packetPeerInfo, const NEW_UDP_PACKET_HEAD& head, const SimpleSocketAddress& sockAddr);
	void HandleUDPTAnnounce(const BYTE* body, size_t size, const PACKET_PEER_INFO& packetPeerInfo, const NEW_UDP_PACKET_HEAD& head, UDPPendingPeerPtr peer);
	void HandleUDPTHandshake(const BYTE* body, size_t size, const PACKET_PEER_INFO& packetPeerInfo, const NEW_UDP_PACKET_HEAD& head, const SimpleSocketAddress& sockAddr);

	/// 返回错误码和bool值表示是否是request
	//pair<long, bool> DoHandleHandshake( const BYTE* body, size_t size, const SimpleSocketAddress& sockAddr, const PACKET_PEER_INFO& packetPeerInfo, const UDP_PACKET_HEAD& head );

	bool HandleUDPTHandshakeRequest(const UDPTHandshakePacketInfo& handshakePacketInfo, const PACKET_PEER_INFO& packetPeerInfo, const NEW_UDP_PACKET_HEAD& head, const SimpleSocketAddress& sockAddr);
	bool HandleUDPTHandshakeResponse(const UDPTHandshakePacketInfo& handshakePacketInfo, const PACKET_PEER_INFO& packetPeerInfo, const NEW_UDP_PACKET_HEAD& head, UDPPendingPeerPtr peer);

	/// peer握手超时
	void OnUDPTPeerHandshakeTimeout(UDPPendingPeerPtr peer);

	bool Contains(const SimpleSocketAddress& sockAddr) const;

	size_t GetConnectingPeerCount() const { return m_udptHandshakingPeers.size(); }

	CPeerManager& GetPeerManager() { return m_PeerManager; }
	AppModule& GetAppModule() { return m_AppModule; }
	PeerConnector& GetOwner() { return m_Owner; }

	bool Refuse( UINT32 refuseReason, UINT32 transactionID, const SimpleSocketAddress& sockAddr );

	UINT32 GetNewSessionKey();

private:
	PeerConnector& m_Owner;
	AppModule& m_AppModule;
	CPeerManager& m_PeerManager;
	CIPPool& m_IPPool;
	CLiveInfo& m_LiveInfo;

	/// 正在握手的udpt连接
	UDPPendingPeerCollection m_udptHandshakingPeers;

	UDPPendingPeerKeyIndex m_IndexedPeers;

	IDGenerator m_ConnectionID;

	UINT32 m_MySessionKey;

	PeerConnectorStatistics& m_statistics;

	//friend class PeerConnector;
};


#endif