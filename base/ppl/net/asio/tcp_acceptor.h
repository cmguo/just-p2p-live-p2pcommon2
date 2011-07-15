
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_NET_ASIO_TCP_ACCEPTOR_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_NET_ASIO_TCP_ACCEPTOR_H_

#include <ppl/net/asio/socket_base.h>
#include <ppl/net/asio/detail/tcp_acceptor_impl.h>
#include <ppl/net/asio/io_service_provider.h>

#include <ppl/net/asio/timer.h>
#include <ppl/net/asio/tcp_socket.h>

#include <ppl/net/socketfwd.h>
#include <ppl/net/inet.h>
#include <ppl/util/listenable.h>
#include <ppl/data/buffer.h>
#include <ppl/boostlib/endpoint.h>


using boost::system::error_code;


class tcp_acceptor : public socket_base, public ppl::util::listenable<tcp_acceptor_listener, trivial_tcp_acceptor_listener>, public boost::enable_shared_from_this<tcp_acceptor>
{
public:
	explicit tcp_acceptor(boost::asio::io_service& ioservice = io_service_provider::default_service())
	{
		m_acceptor.reset(new tcp_acceptor_impl(ioservice, boost::bind(&tcp_acceptor::on_accept, this, _1, _2)));

	}
	virtual ~tcp_acceptor()
	{
		m_acceptor->detach();
		this->close();
	}

	void close()
	{
		//this->acceptor_.cancel(m_last_error);
		this->m_acceptor->get_socket().close(m_last_error);
	}

	bool is_open() const
	{
		return this->m_acceptor->get_socket().is_open();
	}

	InetSocketAddress local_address() const
	{
		boost::asio::ip::tcp::endpoint ep = m_acceptor->get_socket().local_endpoint(m_last_error);
		if (m_last_error)
			return InetSocketAddress();
		return ppl::boostlib::from_endpoint(ep);
	}

	bool open( u_short port )
	{
		boost::system::error_code& err = m_last_error;
		this->m_acceptor->get_socket().open(boost::asio::ip::tcp::v4(), m_last_error);
		if ( err )
		{
			this->close();
			assert(false);
			return false;
		}


		// no delay
		boost::asio::ip::tcp::no_delay option(true);
		this->m_acceptor->get_socket().set_option(option);
		// 复用地址
//		this->m_acceptor->get_socket().set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
		// 设置接受缓冲区为0 ，防止内存拷贝
		this->m_acceptor->get_socket().set_option(boost::asio::ip::tcp::socket::receive_buffer_size(0));
		// 设置发送缓冲区为0 ，防止内存拷贝
		this->m_acceptor->get_socket().set_option(boost::asio::ip::tcp::socket::send_buffer_size(0));    
		// 设置linger为false,要求closesocket后立即关闭socket，防止socket出现TIME_WAIT状态
		this->m_acceptor->get_socket().set_option( boost::asio::ip::tcp::socket::linger(true,0));

		this->m_acceptor->get_socket().bind(
			boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port), 
			m_last_error
			);
		if ( err )
		{
			//assert(false);
			this->close();
			return false;
		}
		this->m_acceptor->get_socket().listen(0, m_last_error);
		if ( err )
		{
			this->close();
			assert(false);
			return false;
		}
		this->accept_once();
		return true;
	}

protected:
	void accept_once()
	{
		boost::shared_ptr<tcp_socket_impl> sock(new tcp_socket_impl(m_acceptor->get_socket().get_io_service()));
		m_acceptor->accept(sock);
	}

	void on_accept(const boost::system::error_code& err, boost::shared_ptr<tcp_socket_impl> sock)
	{
		if ( m_acceptor->get_socket().is_open() )
		{
			// issue another accept if acceptor is not closed
			this->accept_once();
		}
		if ( err )
		{
			this->get_listener()->on_socket_accept_failed(this, err.value());
			return;
		}
		tcp_socket* client = new_client(sock);
		this->get_listener()->on_socket_accept( this, client, ppl::boostlib::from_endpoint(sock->get_remote_address()));
	}
	virtual tcp_socket* new_client(boost::shared_ptr<tcp_socket_impl> sock)
	{
		return new tcp_socket(sock);
	}

protected:
	boost::shared_ptr<tcp_acceptor_impl> m_acceptor;
};
#endif