
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_NET_ASIO_DETAIL_TCP_SOCKET_IMPL_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_NET_ASIO_DETAIL_TCP_SOCKET_IMPL_H_

#include <ppl/util/log.h>
#include <ppl/data/buffer.h>
#include <ppl/boostlib/endpoint.h>

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_array.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/asio/deadline_timer.hpp>
//#include <boost/asio.hpp>


using boost::system::error_code;


class tcp_socket_impl : private boost::noncopyable, public boost::enable_shared_from_this<tcp_socket_impl>
{
public:
	typedef boost::shared_ptr<byte_buffer> byte_buffer_ptr;

	typedef boost::function<void (const error_code&)> event_callback_type;
	typedef boost::function<void (const error_code&, size_t)> lengthed_event_callback_type;

	typedef boost::function<void (const error_code&)> connect_callback_type;
	typedef boost::function<void (const error_code&, size_t, byte_buffer_ptr)> receive_callback_type;
	typedef boost::function<void (const error_code&, size_t, byte_buffer_ptr)> send_callback_type;

	static int& ref_pending_connection_count()
	{
		static int pendingConnectionCount = 0;
		return pendingConnectionCount;
	}
 
	static int pending_connection_count() { return ref_pending_connection_count(); }

	explicit tcp_socket_impl(boost::asio::io_service& ioservice) 
		: m_impl(ioservice), m_detached(false)/*, m_alarm(ioservice), m_is_remote_timeout(false), m_timeout_expire(0)*/
	{
	}
	~tcp_socket_impl() 
	{
	}

	void set_receive_callback(receive_callback_type receiveCallback) 
	{
		m_receive_callback = receiveCallback;
	}

	void set_send_callback(send_callback_type sendCallback) 
	{
		m_send_callback = sendCallback;
	}

	void set_connect_callback(connect_callback_type callback)
	{
		m_connect_callback = callback;
	}

	void detach()
	{
		m_detached = true;
	}

	boost::asio::ip::tcp::socket& get_socket() { return m_impl; }
	boost::asio::ip::tcp::endpoint& get_remote_address() { return m_remote_endpoint; }

	void receive_n(byte_buffer_ptr buf)
	{
		assert(buf->size() > 0);
		assert(m_receive_callback);
		boost::asio::async_read(
			m_impl, 
			boost::asio::buffer( buf->data(), buf->size() ),
			boost::asio::transfer_at_least(buf->size()),
			boost::bind( 
				&tcp_socket_impl::on_receive, 
				this->shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred, 
				buf
				)
			);
	}

	void send_n(byte_buffer_ptr buf)
	{
		assert(buf->size() > 0);
		assert(m_send_callback);
		boost::asio::async_write(
			m_impl,
			boost::asio::buffer( buf->data(), buf->size() ),
			boost::asio::transfer_at_least(buf->size()),
			boost::bind( 
				&tcp_socket_impl::on_send, 
				this->shared_from_this(), 
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred, 
				buf
				)
			);
//		if (m_timeout_expire > 0) 
//			set_expired(m_timeout_expire);
	}

	bool connect(const InetSocketAddress& addr)
	{
		assert(addr.GetIP() != 0 && addr.GetPort() != 0);
		assert(m_connect_callback);
		assert( m_impl.is_open() );
		tcp_endpoint ep = ppl::boostlib::make_tcp_endpoint( addr );

		m_impl.async_connect(
			ep,
			boost::bind( 
				&tcp_socket_impl::on_connect, 
				this->shared_from_this(), 
				boost::asio::placeholders::error
				)
			);
		++tcp_socket_impl::ref_pending_connection_count();
		return true;
	}

	void on_event(const error_code& err, event_callback_type callback)
	{
		if ( m_detached )
		{
			NETWORK_ERROR("on_callback, socket_impl is detached, error=" << err);
			return;
		}
		callback(err);
	}
	void on_lengthed_event(const error_code& err, size_t bytes, lengthed_event_callback_type callback)
	{
		if ( m_detached )
		{
			NETWORK_ERROR("on_lengthed_callback, socket_impl is detached, error=" << err);
			return;
		}
		callback(err, bytes);
	}
/*
	void set_send_timeout(int timeout)
	{
		m_timeout_expire = timeout;
	}
*/
private:
	void on_connect(const error_code& err)
	{
		--tcp_socket_impl::ref_pending_connection_count();
		if ( m_detached )
		{
			NETWORK_ERROR("on_callback, socket_impl is detached, error=" << err);
			return;
		}
		m_connect_callback(err);
	}
	void on_receive(const error_code& err, size_t bytes, byte_buffer_ptr buf)
	{
		if ( m_detached )
		{
			NETWORK_ERROR("on_receive, socket_impl is detached, error=" << err << " bytes=" << bytes);
			return;
		}
		m_receive_callback(err, bytes, buf);
	}
	void on_send(const error_code& err, size_t bytes, byte_buffer_ptr buf)
	{
		if ( m_detached )
		{
			NETWORK_ERROR("on_send, socket_impl is detached, error=" << err << " bytes=" << bytes);
			return;
		}
// 		if (m_timeout_expire > 0 && m_is_remote_timeout == true)
// 			m_send_callback(boost::asio::error::operation_aborted, bytes, buf);
// 		else
		{
// 			if (m_timeout_expire > 0) 
// 				set_expired(0);
			m_send_callback(err, bytes, buf);
		}
	}
/*
	// Added by Tady, 060509: For VistaHttpNetWriter. The Asf-reader in DShow is a freak!
	void set_expired(int timeout_expire)
	{
		m_is_remote_timeout = false;
		m_alarm.cancel();
		if (timeout_expire > 0)
		{
			m_alarm.expires_from_now(boost::posix_time::seconds(timeout_expire));
			m_alarm.async_wait(boost::bind(&tcp_socket_impl::handle_connection_over_time, shared_from_this(), boost::asio::placeholders::error));
		}
	}

	void handle_connection_over_time(const boost::system::error_code& error)
	{
		if (error != boost::asio::error::operation_aborted)
		{
//			TRACE("Tady-> CSimpleServiceSession: handle_connection_over_time.");
			m_is_remote_timeout = true;
			m_alarm.cancel();
			boost::system::error_code ec;
			m_impl.close(ec);
		}
	}
*/
private:
	boost::asio::ip::tcp::socket m_impl;
	boost::asio::ip::tcp::endpoint m_remote_endpoint;
	connect_callback_type m_connect_callback;
	receive_callback_type m_receive_callback;
	send_callback_type m_send_callback;
	bool m_detached;
/*
	boost::asio::deadline_timer m_alarm;
	//bool m_test;
	bool  m_is_remote_timeout;
	int   m_timeout_expire;
*/
};

#endif
