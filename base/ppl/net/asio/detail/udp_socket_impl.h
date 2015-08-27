#ifndef _LIVE_P2PCOMMON2_BASE_PPL_NET_ASIO_DETAIL_UDP_SOCKET_IMPL_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_NET_ASIO_DETAIL_UDP_SOCKET_IMPL_H_

#include <ppl/util/log.h>
#include <ppl/data/buffer.h>

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include <boost/asio/ip/udp.hpp>
#include <boost/asio/placeholders.hpp>

using boost::system::error_code;


class udp_socket_impl : private boost::noncopyable, public boost::enable_shared_from_this<udp_socket_impl>
{
public:
typedef boost::system::error_code error_code;
	struct udp_receive_session
	{
		byte_buffer buffer;
		boost::asio::ip::udp::endpoint remote_endpoint;
	};
	typedef boost::shared_ptr<udp_receive_session> udp_receive_session_ptr;

	typedef boost::function<void (const error_code&, size_t, udp_receive_session_ptr)> callback_type;


	explicit udp_socket_impl(boost::asio::io_service& ioservice, callback_type callback) : m_impl(ioservice), m_detached(false), m_callback(callback)
	{
	}

	/// 切断联系
	void detach()
	{
		m_detached = true;
	}

	boost::asio::ip::udp::socket& get_socket() { return m_impl; }

	void receive(udp_receive_session_ptr session)
	{
		LIVE_ASSERT(session);
		LIVE_ASSERT(session->buffer.size() > 0);
		m_impl.async_receive_from(
			boost::asio::buffer(session->buffer.data(), session->buffer.size()), 
			session->remote_endpoint, 
			boost::bind(
				&udp_socket_impl::on_receive, 
				this->shared_from_this(), 
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred, 
				session
				)
			);
	}

private:
	void on_receive(const error_code& err, size_t bytes, udp_receive_session_ptr session)
	{
		//		TRACE("TCPSocketImpl::HandleLengthEvent %p %d %d\n", this, m_closed, m_test);
		if ( m_detached )
		{
			NETWORK_ERROR("handle_receive, socket_impl is detached, error is " << err);
			return;
		}
		m_callback(err, bytes, session);
	}

private:
	boost::asio::ip::udp::socket m_impl;
	/// 是否有listener关联，如果没有，就不会做事件回调
	bool m_detached;
	callback_type m_callback;
};

#endif
