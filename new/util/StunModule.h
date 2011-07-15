
#ifndef _LIVE_P2PCOMMON2_NEW_UTIL_STUN_MODULE_H_
#define _LIVE_P2PCOMMON2_NEW_UTIL_STUN_MODULE_H_

#include "framework/timer.h"

#include <synacast/protocol/vod.h>
#include <synacast/protocol/nat.h>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <vector>

class PeerInformation;

class StunClientThread;
class UDPSender;
typedef boost::shared_ptr<UDPSender> UDPSenderPtr;

class StunTypeResolver : private boost::noncopyable, public boost::enable_shared_from_this<StunTypeResolver>
{
public:
	typedef boost::function<void (MY_STUN_NAT_TYPE)> CallbackType;

	StunTypeResolver();
	~StunTypeResolver();

	void Resolve(CallbackType callback);
	void Cancel();

private:
	void HandleNATType(MY_STUN_NAT_TYPE type) 
	{
		m_bIsHandling = true;
		if (m_callback)
			m_callback(type);
		m_bIsHandling = false;
	}
	boost::shared_ptr<StunClientThread> m_thread;
	CallbackType m_callback;
	bool	m_bIsHandling;
};


class StunModule : private boost::noncopyable
{
public:
	typedef boost::function<void (BYTE*, size_t, const InetSocketAddress&)> TransmitCallbackType;
	typedef boost::function<void ()> LoginCallbackType;

	explicit StunModule( const std::vector<InetSocketAddress>& stunServers, UDPSenderPtr sender, boost::shared_ptr<const PeerInformation> peerInformation, UINT16 peerVersion );
	~StunModule();

	void Start();

	bool Transmit(const BYTE* data, size_t size, const SimpleSocketAddress& targetAddr, const SimpleSocketAddress& stunServerAddress);

	bool HandlePacket(BYTE* data, size_t size, const InetSocketAddress& remoteAddr);

	PEER_ADDRESS GetDetectedAddress() const { return CPeerAddrUtil::ToPeerAddress( m_DetectedAddress ); }
	const InetSocketAddress& GetServerAddress() const
	{
		assert(!m_ServerList.empty());
		assert( m_CurrentServer < m_ServerList.size() );
		return m_ServerList[m_CurrentServer];
	}

	void SetCallbacks( LoginCallbackType loginCallback, TransmitCallbackType transmitCallback );

protected:

	bool IsLoginned() const { return m_Loginned; }

	void OnLoginSucceeded();
	void OnLoginFailed();

	bool NeedStun() const;

	bool Login();
	bool KeepAlive();

	//void StartReLoginTimer();

protected:
	CPeerAddr GetLocalAddress() const;

	void OnLoginTimer()
	{
		m_LoginTimeoutTimer.stop();
		this->Login();
	}

private:
	UDPSenderPtr m_PacketSender;
	boost::shared_ptr<const PeerInformation> m_PeerInformation;
	StunRequestPacketBuilder m_PacketBuilder;
	CPeerAddr m_DetectedAddress;
	std::vector<InetSocketAddress> m_ServerList;
	size_t m_CurrentServer;
	bool m_Loginned;
	periodic_timer m_LoginTimer;
	periodic_timer m_KeepAliveTimer;
	once_timer m_LoginTimeoutTimer;
	size_t m_LoginFailedTimes;

	LoginCallbackType m_LoginCallback;
	TransmitCallbackType m_TransmitCallback;
};

#endif
