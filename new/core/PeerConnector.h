
#ifndef _LIVE_P2PCOMMON2_NEW_CORE_PEER_CONNECTOR_H_
#define _LIVE_P2PCOMMON2_NEW_CORE_PEER_CONNECTOR_H_

#include <synacast/protocol/DataIO.h>
#include <ppl/data/tuple.h>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>


class TCPPeerConnector;
class UDPPeerConnector;

class AppModule;
class CPeerManager;
class PeerConnectorStatistics;
class CIPPool;
class PeerConnection;

class PeerNetInfo;
class PeerStatusInfo;
class PeerInformation;
class PeerConnectParam;
class CLiveInfo;

struct PEER_ADDRESS;
struct PEER_CORE_INFO;
struct PACKET_PEER_INFO;


class TCPConnectionlessPacketSender;
class UDPConnectionlessPacketSender;


inline triple<int, int, double> CalcRatio(int total, int part)
{
	double ratio = 0;
	if (total > 0)
	{
		ratio = part * 100.0 / total;
	}
	return ppl::make_tuple(total, part, ratio);
}



/// ���������ӣ����ҹ���û����ɵ�����
class PeerConnector : private boost::noncopyable//, private TCPClientSocketListener
{
public:
	explicit PeerConnector(CPeerManager& peerManager, boost::shared_ptr<PeerInformation> peerInformation, boost::shared_ptr<TCPConnectionlessPacketSender> tcpPacketSender, boost::shared_ptr<UDPConnectionlessPacketSender> udpPacketSender);
	~PeerConnector();

	AppModule& GetAppModule()
	{
		return m_AppModule;
	}

	boost::shared_ptr<TCPPeerConnector> GetTCPConnector() { return m_TCPConnector; }
	boost::shared_ptr<UDPPeerConnector> GetUDPConnector() { return m_UDPConnector; }

	boost::shared_ptr<TCPConnectionlessPacketSender> GetTCPPacketSender() { return m_TCPPacketSender; }
	boost::shared_ptr<UDPConnectionlessPacketSender> GetUDPPacketSender() { return m_UDPPacketSender; }

	boost::shared_ptr<const PeerInformation> GetPeerInformation() { return m_PeerInformation; }

	void HandleNoDegree( data_input_stream& is, const PACKET_PEER_INFO& packetPeerInfo, bool isTCP );

	size_t GetTotalPendingPeerCount() const;

	/// ��ȡ�������ӵ�peer����
	size_t GetConnectingPeerCount();

	/// ��ȡ�������ֵ�peer����
	size_t GetHandshakingPeerCount();

	/// ����һ������
	bool Connect(const PEER_ADDRESS& addr, const PeerConnectParam& param);


	/// �Ͽ�����
	//bool Disconnect(TCPClientSocketPtr sock, long errcode);

	/// tcp���������Ƿ��Ѿ�����
	bool IsBusy() const;


	/// ��ȡ������
	CPeerManager& GetPeerManager() { return m_PeerManager; }

	PeerConnectorStatistics& GetStatistics() { return m_statistics; }

	void SyncPendingConnectionInfo();

	bool ConnectTCP(const PEER_ADDRESS& addr, UINT16 realPort, const PeerConnectParam& param);

	bool ConnectUDP(const PEER_ADDRESS& addr, const PeerConnectParam& param);

protected:
	bool CheckConnected(const PEER_ADDRESS& addr, bool isVip) const;



public:
	bool IfOnlyAcceptMDS() const;
	bool IfRefusePeer(const PEER_CORE_INFO& coreInfo) const;

//	friend class TCPPeerConnector;
//	friend class UDPPeerConnector;

private:
	/// ������
	CPeerManager& m_PeerManager;

	CLiveInfo& m_LiveInfo;

	/// ������AppModule
	AppModule& m_AppModule;

	CIPPool& m_ipPool;

	boost::shared_ptr<TCPPeerConnector> m_TCPConnector;
	boost::shared_ptr<UDPPeerConnector> m_UDPConnector;

	boost::shared_ptr<const PeerNetInfo> m_NetInfo;

	boost::shared_ptr<TCPConnectionlessPacketSender> m_TCPPacketSender;
	boost::shared_ptr<UDPConnectionlessPacketSender> m_UDPPacketSender;

	/// �������ӵ�����peer
//	ConnectingPeerCollection m_connectingPeers;

	/// �������ֵ�����peer
//	HandshakingPeerCollection m_handshakingPeers;

	/// �������ֵ�udpt����
//	UDPTPendingPeerCollection m_udptHandshakingPeers;

	PeerConnectorStatistics& m_statistics;

	UINT m_maxConnectionID;

	boost::shared_ptr<const PeerInformation> m_PeerInformation;

};

#endif

