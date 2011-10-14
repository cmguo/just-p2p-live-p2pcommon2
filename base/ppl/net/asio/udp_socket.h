
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_NET_ASIO_UDP_SOCKET_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_NET_ASIO_UDP_SOCKET_H_

#include <ppl/config.h>

#include <ppl/net/asio/io_service_provider.h>
#include <ppl/net/asio/socket_base.h>
#include <ppl/net/asio/detail/udp_socket_impl.h>

#include <ppl/boostlib/endpoint.h>
#include <ppl/util/listenable.h>
#include <ppl/net/socketfwd.h>


/// udp socket类，提供异步发送和接收功能
class udp_socket : public socket_base, public ppl::util::listenable<udp_socket_listener, trivial_udp_socket_listener>
{
public:
	udp_socket(boost::asio::io_service& ioservice = *io_service_provider::get_default())
	{
		m_impl.reset(new udp_socket_impl(ioservice, boost::bind(&udp_socket::on_receive, this, _1, _2, _3)));

	}

	virtual ~udp_socket()
	{
		this->close();
		// 切断跟m_impl之间的联系，m_impl可能继续在asio的任务队列中存活一小段时间
		m_impl->detach();
	}

	InetSocketAddress local_address() const
	{
		if ( false == m_impl->get_socket().is_open() )
			return InetSocketAddress();
		return ppl::boostlib::from_endpoint( m_impl->get_socket().local_endpoint(m_last_error) );
	}

	bool close()
	{
		//m_impl->get_socket().cancel(m_last_error);
		m_impl->get_socket().close(m_last_error);
		LIVE_ASSERT( !m_last_error );
		return !m_last_error;
	}

	bool open( u_short port )
	{
		this->close();

		boost::asio::ip::udp::endpoint ep( boost::asio::ip::udp::v4(), port );

		this->m_impl->get_socket().open( boost::asio::ip::udp::v4(), m_last_error );
		if ( m_last_error )
			return false;
		this->m_impl->get_socket().bind( ep, m_last_error );
		if ( m_last_error )
			return false;
		boost::asio::detail::io_control::non_blocking_io iocmd(true);
		this->m_impl->get_socket().io_control(iocmd, m_last_error);
		return !m_last_error;
	}

	bool receive( size_t maxSize )
	{
		udp_socket_impl::udp_receive_session_ptr session(new udp_socket_impl::udp_receive_session);
		session->buffer.resize(maxSize);
		m_impl->receive(session);
		return true;
	}

	/// 发送报文到指定的socket地址
	bool send(const void* data, size_t size, const InetSocketAddress& addr)
	{
		boost::asio::ip::udp::endpoint ep = ppl::boostlib::make_udp_endpoint( addr );
		m_impl->get_socket().send_to(
			boost::asio::buffer(data, size), 
			ep,
			0, 
			m_last_error
			);

		// error为0
		if ( !m_last_error )
			return true;

#if defined(_PPL_PLATFORM_MSWIN)
		if ( boost::asio::error::would_block == m_last_error || boost::asio::error::in_progress == m_last_error )
#elif defined(_PPL_PLATFORM_LINUX)
		if ( boost::asio::error::try_again == m_last_error )
#else
#error "invalid platform for sendto"
#endif
		{
			return true;
		}
		return false;
	}

	bool is_open() const
	{
		return m_impl->get_socket().is_open();
	}

protected:
	void on_receive(const error_code& error, std::size_t bytes, udp_socket_impl::udp_receive_session_ptr session)
	{
		if (error)
		{
			// 失败
			this->get_listener()->on_socket_receive_failed( this, error.value() );
		}
		else
		{
			LIVE_ASSERT( bytes > 0 );
			// 成功
			InetSocketAddress addr = ppl::boostlib::from_endpoint( session->remote_endpoint );
			this->get_listener()->on_socket_receive(this, addr, session->buffer.data(), bytes );
		}
	}


protected:
	boost::shared_ptr<udp_socket_impl> m_impl;
};

#endif