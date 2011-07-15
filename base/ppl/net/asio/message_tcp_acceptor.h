
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_NET_ASIO_MESSAGE_TCP_ACCEPTOR_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_NET_ASIO_MESSAGE_TCP_ACCEPTOR_H_

#include <ppl/net/asio/tcp_acceptor.h>
#include <ppl/net/asio/message_tcp_socket.h>

class message_tcp_acceptor : public tcp_acceptor
{
public:
	message_tcp_acceptor(boost::asio::io_service& ioservice = io_service_provider::default_service()) : tcp_acceptor(ioservice)
	{
	}
	virtual ~message_tcp_acceptor()
	{
	}

protected:
	virtual tcp_socket* new_client(boost::shared_ptr<tcp_socket_impl> sock)
	{
		return new message_tcp_socket(sock);
	}
};


#endif
