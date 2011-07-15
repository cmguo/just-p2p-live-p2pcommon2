
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_NET_ASIO_DETAIL_TCP_ACCEPTOR_IMPL_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_NET_ASIO_DETAIL_TCP_ACCEPTOR_IMPL_H_

#include <ppl/net/asio/detail/tcp_socket_impl.h>

#include <ppl/util/log.h>
#include <ppl/data/buffer.h>

#include <boost/function.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include <boost/asio/ip/udp.hpp>
#include <boost/asio/placeholders.hpp>


class tcp_acceptor_impl : private boost::noncopyable, public boost::enable_shared_from_this<tcp_acceptor_impl>
{
public:
	typedef boost::shared_ptr<tcp_socket_impl> tcp_socket_impl_ptr;

	typedef boost::function<void (const error_code&, tcp_socket_impl_ptr)> callback_type;


	explicit tcp_acceptor_impl(boost::asio::io_service& ioservice, callback_type callback) : m_impl(ioservice), m_detached(false), m_callback(callback)
	{
	}

	/// 切断联系
	void detach()
	{
		m_detached = true;
	}

	boost::asio::ip::tcp::acceptor& get_socket() { return m_impl; }

	void accept(tcp_socket_impl_ptr sock)
	{
		m_impl.async_accept( 
			sock->get_socket(),
			sock->get_remote_address(),
			boost::bind(
				&tcp_acceptor_impl::on_accept, 
				this->shared_from_this(), 
				boost::asio::placeholders::error, 
				sock
				)
			);
	}

private:
	void on_accept(const boost::system::error_code& err, tcp_socket_impl_ptr sock)
	{
		if ( m_detached )
		{
			UTIL_ERROR("on_accept, socket_impl is detached, error is " << err);
			return;
		}
		m_callback(err, sock);
	}

private:
	boost::asio::ip::tcp::acceptor m_impl;
	/// 是否有listener关联，如果没有，就不会做事件回调
	bool m_detached;
	callback_type m_callback;
};

#endif
