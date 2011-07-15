
#ifndef _LIVE_P2PCOMMON2_NEW_APP_CHANNEL_H_
#define _LIVE_P2PCOMMON2_NEW_APP_CHANNEL_H_

#include "common/AppParam.h"
#include "common/GloalTypes.h"
#include <synacast/pub/peerdll.h>
#include <ppl/os/memory_mapping.h>
#include <ppl/net/socketfwd.h>
#include <ppl/net/asio/timer.h>
#include <boost/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>

class UDPSender;
class LiveAppModuleImpl;
class udp_socket;
class message_tcp_acceptor;

class LiveChannel : public boost::enable_shared_from_this<LiveChannel>, public udp_socket_listener, public tcp_acceptor_listener, private boost::noncopyable
{
public:
	explicit LiveChannel(int node_type);
	virtual ~LiveChannel();

	bool Init(const tstring& urlstr, int tcpPort, int udpPort, const tstring& configDir, const string& baseDir);
	void Start();
	void Stop();

	virtual void on_socket_receive(udp_socket* sender, const InetSocketAddress& sockAddr, BYTE* data, size_t size);
	virtual void on_socket_receive_failed(udp_socket* sender, int errcode);
	virtual void on_socket_accept(tcp_acceptor* sender, tcp_socket* client, const InetSocketAddress& addr);
	virtual void on_socket_accept_failed(tcp_acceptor* sender, int errcode);

	void OnAppTimer();

	void ReceiveUDP();

	static void AppendTrackerAddresses(std::vector<TRACKER_LOGIN_ADDRESS>& trackers, const TRACKERADDR* srcAddresses, UINT count, UINT maxCount);

	CLiveInfo* GetInfo();
	SYSINFO* GetSysInfo();

	void OnUDPSenderResponse(BYTE* data, size_t size, const InetSocketAddress& sockAddr, UINT proxyType);

	void SetPlayerStatus(int pstatus);
	void SetPlayerCallback(FUNC_CallBack callbac, unsigned int channelHandle);

protected:
	void DoStart();
	void DoStop();
	
	void HanlePlayerClientError(int errcode);
protected:
	tstring m_urlstr;
	LiveAppModuleCreateParam m_param;

	boost::shared_ptr<message_tcp_acceptor> m_tcp;
        boost::shared_ptr<udp_socket> m_udp;

	UDPSenderPtr m_normalUDPSender;
	UDPSenderPtr m_tcpProxyUDPSender;
	UDPSenderPtr m_httpProxyUDPSender;

	ppl::os::memory_mapping m_mapping;

	periodic_timer m_timer;

	boost::shared_ptr<LiveAppModuleImpl> m_AppModule;

	FUNC_CallBack m_callback;

	unsigned int m_channel_handle;

	bool m_playing;

	bool m_started;
	int	 m_node_type;
};

#endif

