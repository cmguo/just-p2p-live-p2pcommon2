
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_NET_ASIO_HTTP_ACCEPTOR_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_NET_ASIO_HTTP_ACCEPTOR_H_

#include <ppl/net/asio/tcp_acceptor.h>
#include <ppl/net/asio/http_socket.h>

class http_acceptor : public tcp_acceptor
{
public:
	http_acceptor(boost::asio::io_service& ioservice = io_service_provider::default_service()) : tcp_acceptor(ioservice)
	{
	}
	virtual ~http_acceptor()
	{
	}

protected:
	virtual tcp_socket* new_client(boost::shared_ptr<tcp_socket_impl> sock)
	{
		return new http_socket(sock);
	}
};


#endif
