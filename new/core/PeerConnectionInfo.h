
#ifndef _LIVE_P2PCOMMON2_NEW_CORE_PEER_CONNECTION_INFO_H_
#define _LIVE_P2PCOMMON2_NEW_CORE_PEER_CONNECTION_INFO_H_

#include "common/IPInfo.h"

#include <synacast/protocol/data/DegreeInfo.h>
#include <synacast/protocol/base.h>
#include <synacast/protocol/nat.h>
#include <ppl/util/time_counter.h>
#include <ppl/net/socketfwd.h>
#include <boost/noncopyable.hpp>


const UINT32 SESSION_KEY_START = 0;
const UINT32 SESSION_KEY_END = (UINT32)-1;


/// ��ȡ���ʵĿ�����key��peer��ַ
PEER_ADDRESS GetProperKeyPeerAddress( const PEER_ADDRESS& peerAddr, const SimpleSocketAddress& sockAddr, bool isUDP, bool isInitFromRemote, const PEER_ADDRESS& outerAddr );


class PeerConnectParam : public PeerItem
{
public:
	bool IsConnectForDetect;
        UINT Timeout;
	bool IsVIP;
	bool IsSpark;


	PeerConnectParam(const PeerItem& peer, bool isConnectForDetect, UINT timeout, bool isVIP, bool isSpark = false) 
		: IsConnectForDetect( isConnectForDetect )
		, Timeout( timeout )
		, IsVIP( isVIP )
		, IsSpark( isSpark )
	{
		assert(!(IsSpark && IsVIP));

		PeerItem& thisPeer = *this;
		thisPeer = peer;
	}

	bool IsAccessibleNAT() const
	{
		UINT8 natType = this->Info.CoreInfo.NATType;
		return STUN_TYPE_PUBLIC == natType || STUN_TYPE_FULLCONENAT == natType;
	}

	bool IsAccessibleNetType() const
	{
		UINT8 netType = this->Info.CoreInfo.PeerNetType;
		return PNT_OUTER == netType || PNT_UPNP == netType;
	}

	bool IsAccessible() const
	{
		return this->IsAccessibleNAT() || this->IsAccessibleNetType();
	}

};



//#pragma warning(push)
//#pragma warning(disable:4512)

/// �������ӵ�socket��Ϣ
class PeerConnectionInfo : private boost::noncopyable
{
public:
	virtual ~PeerConnectionInfo() { }

        SimpleSocketAddress RemoteSocketAddress;	
        /// ���ַ���ʱ��
        time_counter HandshakeStartTime;
        bool IsInitFromRemote;
         /// �����ӵĵ�ַ
        PEER_ADDRESS RemoteAddress;
        // ʵ�����ӵĶ˿�
        const UINT16 RealPort;
         /// ���Ӻ�ʱ
        UINT ConnectUsedTime;
        PeerConnectParam ConnectParam;
        const bool IsUDP;
	PEER_ADDRESS OuterAddress;
        bool IsThroughNAT;

	/// ��Ϊkey��peer��ַ
	PEER_ADDRESS KeyPeerAddress;

	/// ���ӷ���ʱ��
	time_counter ConnectStartTime;
//	const UINT ConnectFlags;
//	const bool CanDetect;
//	const UINT RTT;
	/// �Ƿ�ΪVIP����
	//const bool IsVIP;

	explicit PeerConnectionInfo(const SimpleSocketAddress& sockAddr, bool isInitFromRemote, bool isUDP, 
		const PEER_ADDRESS& remoteAddr, UINT16 realPort, const PeerConnectParam& param, const PEER_ADDRESS& outerAddr) 
			: 
		RemoteSocketAddress(sockAddr), 
		HandshakeStartTime(0), 
                IsInitFromRemote(isInitFromRemote),		
                RemoteAddress(remoteAddr), 
		RealPort(realPort), 
		ConnectUsedTime((UINT)-1), 
		ConnectParam( param ),
		IsUDP(isUDP), 
		OuterAddress(outerAddr), 
		IsThroughNAT( false) 
	{
		this->UpdateKeyPeerAddress();
	}

	void UpdateKeyPeerAddress()
	{
		this->KeyPeerAddress = GetProperKeyPeerAddress( this->RemoteAddress, this->RemoteSocketAddress, this->IsUDP, this->IsInitFromRemote, this->OuterAddress );
	}

	void UpdateSocketAddress(const SimpleSocketAddress& sockAddr)
	{
		this->RemoteSocketAddress = sockAddr;
		this->RemoteAddress.IP = sockAddr.IP;
		this->RemoteAddress.UdpPort = sockAddr.Port;
		if ( false == this->IsInitFromRemote )
		{
			// ����Ǳ��ط�������ӣ�����Ҫ����OuterAddress
			this->OuterAddress = this->RemoteAddress;
		}
		this->UpdateKeyPeerAddress();
	}
};


class PeerHandshakeInfo : public pool_object
{
public:
	/// Ƶ����ʾ
	GUID ChannelGUID;

	/// peer��ʶ
	GUID PeerGUID;

	/// peer��ַ
	PEER_ADDRESS Address;
	PEER_ADDRESS OuterAddress;
	PEER_ADDRESS DetectedRemoteAddress;

	/// peer�汾��
	UINT AppVersion;

	/// �Ƿ���Ƕ��ʽϵͳ
	bool IsEmbedded;

	PEER_CORE_INFO CoreInfo;
	bool MustBeVIP;

	DEGREE_INFO Degrees;

	PeerHandshakeInfo()
	{
		FILL_ZERO(*this);
	}
};





/// �������ӵ�socket��Ϣ
class UDPPeerConnectionInfo : public PeerConnectionInfo
{
public:
//	UDPT_HEAD_INFO UDPTHead;
//	UDPT_HEAD_INFO RemoteUDPTHead;

	//PEER_SOCKET_ADDRESS RemoteSocketAddress;
	//SimpleSocketAddress RemoteSocketAddress;
	DEGREE_INFO Degrees;

	/// �Է�������Լ���key����Key(Remote,Local)
	UINT32 LocalSessionKey;
	/// �Լ�������Է���key����Key(Local,Remote)
	UINT32 RemoteSessionKey;


	explicit UDPPeerConnectionInfo(const SimpleSocketAddress& sockAddr, bool isInitFromRemote, 
		const PEER_ADDRESS& remoteAddr, const PeerConnectParam& param, const PEER_ADDRESS& outerAddr) 
		: PeerConnectionInfo(sockAddr, isInitFromRemote, true, remoteAddr, remoteAddr.UdpPort, param, outerAddr), 
			LocalSessionKey(SESSION_KEY_START), RemoteSessionKey(SESSION_KEY_START)
	{
		FILL_ZERO(this->Degrees);
	}

	static bool IsValidSessionKey(UINT32 sessionKey)
	{
		return sessionKey != SESSION_KEY_START && sessionKey != SESSION_KEY_END;
	}

	bool IsBothSessionKeyValid() const
	{
		return IsValidSessionKey(LocalSessionKey) && IsValidSessionKey(RemoteSessionKey);
	}


};



class TCPPeerConnectionInfo : public PeerConnectionInfo
{
public:
	/// Socket����ָ��
	tcp_socket_ptr Socket;
	bool IsInputSource;

	explicit TCPPeerConnectionInfo(tcp_socket_ptr sock, const SimpleSocketAddress& sockAddr, bool isInitFromRemote, 
		const PEER_ADDRESS& remoteAddr, UINT16 realPort, const PeerConnectParam& param)
		: PeerConnectionInfo(sockAddr, isInitFromRemote, false, remoteAddr, realPort, param, remoteAddr)
		, Socket(sock), IsInputSource(false)
	{
	}
};

//#pragma warning(pop)
#endif


