
#ifndef _LIVE_P2PCOMMON2_NEW_CORE_TCPPEER_CONNECTOR_H_
#define _LIVE_P2PCOMMON2_NEW_CORE_TCPPEER_CONNECTOR_H_

#include <synacast/protocol/DataIO.h>
#include <ppl/net/socketfwd.h>
#include <boost/noncopyable.hpp>
#include <map>

struct PEER_ADDRESS;

class SimpleSocketAddress;

class PeerConnector;
class CPeerManager;
class CIPPool;

class PeerNetInfo;
class PeerStatusInfo;

class AppModule;
class PeerConnector;

class PeerHandshakeInfo;
class TCPPeerConnectionInfo;

class CLiveInfo;
class TCPPendingPeer;
typedef TCPPendingPeer ConnectingPeer;
typedef TCPPendingPeer HandshakingPeer;
//class HandshakingPeer;

typedef boost::shared_ptr<ConnectingPeer> ConnectingPeerPtr;
typedef boost::shared_ptr<HandshakingPeer> HandshakingPeerPtr;

typedef boost::shared_ptr<TCPPendingPeer> TCPPendingPeerPtr;

/// 正在连接的peer集合
typedef std::map<SimpleSocketAddress, TCPPendingPeerPtr> TCPPendingPeerCollection;

/// 正在握手的peer集合
typedef std::map<SimpleSocketAddress, HandshakingPeerPtr> HandshakingPeerCollection;

class PacketHandleResult;

class PeerConnectorStatistics;
class PeerConnectParam;


class TCPPeerConnector : private boost::noncopyable
{
public:
	explicit TCPPeerConnector(PeerConnector& owner);

	bool Connect(const PEER_ADDRESS& addr, UINT16 realPort, const PeerConnectParam& param);

	void OnSocketAccept(tcp_socket_ptr newClient, const InetSocketAddress& addr);

	//void OnAcceptConnection(TCPClientSocket* newClient, BYTE* data, size_t size);

	bool Contains(const PEER_ADDRESS& addr) const;

	/// 获取正在连接的peer个数
	size_t GetConnectingPeerCount() const { return m_ConnectingPeers.size(); }

	/// 获取正在握手的peer个数
	size_t GetHandshakingPeerCount() const { return m_HandshakingPeers.size(); }

	CPeerManager& GetPeerManager() { return m_PeerManager; }
	AppModule& GetAppModule() { return m_AppModule; }

	/// peer连接
	void OnPeerConnected(TCPPendingPeerPtr peer);

	/// peer连接失败
	void HandleConnectError( TCPPendingPeerPtr peer, int errcode );
	void HandleHandshakeError( TCPPendingPeerPtr peer, int errcode );

	/// peer握手失败
	void OnPeerSocketReciveFailed(TCPPendingPeerPtr peer, int errcode);

	/// peer握手超时
	void OnPeerHandshakeTimeout(TCPPendingPeerPtr peer);

	void HandlePeerHandshake( TCPPendingPeerPtr peer, int errcode );

	PeerConnector& GetOwner() { return m_Owner; }

	size_t GetPendingPeerCount() const { return m_ConnectingPeers.size() + m_HandshakingPeers.size(); }

private:
//	PacketHandleResult HandleHandshake(bool isVIP, bool isInitFromRemote, const BYTE* data, size_t size, PeerHandshakeInfo& handshakeInfo);

	//PacketHandleResult HandleConnectionRequest(const InetSocketAddress& remoteAddress, const BYTE* data, size_t size, bool& isVIP, PeerHandshakeInfo& handshakeInfo);

	//void HandlePacket( const InetSocketAddress& remoteAddress, const BYTE* data, size_t size );

	void HandleTCPConnectFail(boost::shared_ptr<TCPPeerConnectionInfo> connInfo);

	/// 处理peer握手错误
	void HandlePeerHandshakeError( TCPPendingPeerPtr peer, int errcode );

	/// peer握手成功
	void OnPeerHandshaked(TCPPendingPeerPtr peer, const PeerHandshakeInfo& handshakeInfo);

	boost::shared_ptr<TCPPeerConnectionInfo> DeleteConnectingPeer( TCPPendingPeerPtr peer );
	boost::shared_ptr<TCPPeerConnectionInfo> DeleteHandshakingPeer( TCPPendingPeerPtr peer );

private:
	PeerConnector& m_Owner;
	AppModule& m_AppModule;
	CPeerManager& m_PeerManager;
	CIPPool& m_IPPool;

	CLiveInfo& m_LiveInfo;

	/// 正在连接的所有peer
	TCPPendingPeerCollection m_ConnectingPeers;

	TCPPendingPeerCollection m_HandshakingPeers;

	PeerConnectorStatistics& m_statistics;

	//friend class PeerConnector;
};


#endif