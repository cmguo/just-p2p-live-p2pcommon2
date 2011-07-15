
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_NET_ASIO_TCP_SOCKET_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_NET_ASIO_TCP_SOCKET_H_

#include <ppl/net/asio/socket_base.h>
#include <ppl/net/asio/detail/tcp_socket_impl.h>
#include <ppl/net/asio/io_service_provider.h>
#include <ppl/net/asio/timer.h>

#include <ppl/net/socketfwd.h>
#include <ppl/net/inet.h>
#include <ppl/util/listenable.h>
#include <ppl/data/buffer.h>
#include <ppl/boostlib/endpoint.h>

#include <boost/asio/placeholders.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>


#include <list>
#include <ostream>

// Added for test.
#include <ppl/diag/trace.h>

class tcp_socket : public socket_base, public ppl::util::listenable<tcp_socket_listener, trivial_tcp_socket_listener>
{
public:
	explicit tcp_socket(boost::asio::io_service& ioservice = io_service_provider::default_service()) : m_impl(new tcp_socket_impl(ioservice))
	{
		this->init();
	}
	explicit tcp_socket(boost::shared_ptr<tcp_socket_impl> sock) : m_impl(sock)
	{
		this->init();
	}
	~tcp_socket()
	{
		m_impl->detach();
		this->close();
	}

	boost::asio::ip::tcp::socket& native_socket()
	{
		return m_impl->get_socket();
	}

	bool is_open() const
	{
		return m_impl->get_socket().is_open();
	}

	bool create()
	{
		this->close();
		this->m_impl->get_socket().open( boost::asio::ip::tcp::v4(), m_last_error );
		return !m_last_error;
	}

	virtual bool close()
	{
		m_impl->get_socket().close(m_last_error);
		m_SendQueue.clear();
		return !m_last_error;
	}
	virtual bool shut_down()
	{
		m_impl->get_socket().shutdown(boost::asio::ip::tcp::socket::shutdown_send, m_last_error);
		return !m_last_error;
	}

	bool connect(u_long ip, u_short port)
	{
		return this->connect(InetSocketAddress(ip, port));
	}
	bool connect(const InetSocketAddress& addr)
	{
		this->close();
		if (false == create())
			return false;
		assert( this->is_open() );
		m_ConnectTimer.start(m_ConnectTimeout * 1000);
		m_impl->connect(addr);
		return true;
	}

	virtual bool send(const void* data, size_t size)
	{
		tcp_socket_impl::byte_buffer_ptr buf(new byte_buffer(size));
		memcpy( buf->data(), data, size );
		this->do_send(buf);
		return true;
	}

	virtual bool send2(const void* data, size_t size)
	{
		tcp_socket_impl::byte_buffer_ptr buf(new byte_buffer(size));
		memcpy( buf->data(), data, size );
		return this->do_send2(buf);
	}

	bool receive_n(size_t size)
	{
		assert(size > 0);
		if ( size > 10 * 1024 * 1024 )
		{
			assert(false);
			return false;
		}
		
#ifdef _SINGLE_THREAD
		static tcp_socket_impl::byte_buffer_ptr buf(new byte_buffer());
		buf->resize(size);
#else
		tcp_socket_impl::byte_buffer_ptr buf(new byte_buffer(size));
#endif 
		return receive_buffer_n(buf);
	}

	virtual bool receive()
	{
		assert(false);
		return false;
	}

	InetSocketAddress remote_address() const
	{
		boost::system::error_code err;
		boost::asio::ip::tcp::endpoint ep = this->m_impl->get_socket().remote_endpoint(err);
		if ( err )
			return InetSocketAddress();
		return ppl::boostlib::from_endpoint(ep);
	}

	InetSocketAddress local_address() const
	{
		boost::system::error_code err;
		boost::asio::ip::tcp::endpoint ep = this->m_impl->get_socket().local_endpoint(err);
		if ( err )
			return InetSocketAddress();
		return ppl::boostlib::from_endpoint(ep);
	}

	long sending_count()
	{
		return m_SendQueue.size();
	}

	static int pending_connection_count()
	{
		return tcp_socket_impl::pending_connection_count();
	}

	void set_connect_timeout(int timeout)
	{
		m_ConnectTimeout = timeout;
	}
	
	void set_send_timeout(int timeout)
	{
//		if (m_impl) m_impl->set_send_timeout(timeout);
		m_SendTimeout = timeout;
	}
protected:
	bool receive_buffer_n(const boost::shared_ptr<byte_buffer>& buf)
	{
		m_impl->receive_n(buf);
		return true;
	}
	void init()
	{
		assert(m_impl);
		m_ConnectTimeout = 60;m_SendTimeout = 0;
		m_ConnectTimer.set_callback(boost::bind(&tcp_socket::on_connect_timeout, this));
		m_impl->set_connect_callback(boost::bind(&tcp_socket::on_connect, this, _1));
		m_impl->set_receive_callback(boost::bind(&tcp_socket::on_receive, this, _1, _2, _3));
		m_impl->set_send_callback(boost::bind(&tcp_socket::on_send, this, _1, _2, _3));
	}

	void on_connect_timeout()
	{
		//this->on_connect(boost::asio::error::timed_out);
		//assert(m_SendQueue.empty());
		// 关闭socket，会触发连接失败
		m_impl->get_socket().close(m_last_error);
	}

	virtual void on_connect(const error_code& err)
	{
		m_ConnectTimer.stop();
		if ( err )
		{
			if(err.value() == 2) 
				printf("on_connect errcode [0x2]: %s", err.message().c_str());
			this->get_listener()->on_socket_connect_failed( this, err.value() );
		}
		else
		{
			this->get_listener()->on_socket_connect( this );
		}
	}
	virtual void on_receive(const error_code& err, size_t bytes, tcp_socket_impl::byte_buffer_ptr buf)
	{
		if ( err )
		{
			if(err.value() == 2) 
				printf("on_receive errcode [0x2]: %s", err.message().c_str());
			if (bytes > 0)
			{
				assert(buf->size() >= bytes);
				get_listener()->on_socket_receive(this, buf->data(), bytes);
			}
			get_listener()->on_socket_receive_failed(this, err.value());
			return;
		}
		assert(buf->size() >= bytes);
		assert(buf->size() == bytes);
		get_listener()->on_socket_receive(this, buf->data(), bytes);
	}

	void on_send(const error_code& err, size_t bytes, tcp_socket_impl::byte_buffer_ptr buf)
	{
		if (m_SendTimeout > 0) m_ConnectTimer.stop(); // Borrow ConnectTimer.
		if ( m_SendQueue.empty() )
		{
			NETWORK_ERROR("tcp_socket::on_send invalid empty buffer " << make_tuple( err.value(), bytes, m_SendQueue.size() ));
			assert(false == m_impl->get_socket().is_open());
			return;
		}
		assert(buf == m_SendQueue.front());
		// previous sending is complete, remove it
		m_SendQueue.pop_front();
		if ( err )
		{
			if(err.value() == 2) 
				printf("on_send errcode [0x2]: %s", err.message().c_str());
			// 发送失败
			NETWORK_ERROR("tcp_socket::on_send error " << make_tuple( err.value(), bytes, m_SendQueue.size() ));
			this->get_listener()->on_socket_send_failed(this, err.value());
		}
		else
		{
			if ( m_SendQueue.empty() )
			{
				return;
			}
			else
			{
				// send the first item in sending-queue
				this->send_buffer_n(m_SendQueue.front());
			}
		}
	}

	/// 调用do_send的线程必须跟io_service是同一线程
	void do_send(tcp_socket_impl::byte_buffer_ptr buf)
	{
		// queue sending item
		if (m_SendQueue.size() < 5000)
		{
			m_SendQueue.push_back(buf);
// 			if (m_SendQueue.size() >= 20)
// 			{
// 				//TRACEOUT_T("Tady -> !!!!! SendQueue size [%d] !!!!!", m_SendQueue.size());
// 			}
		}
		else
		{
//			bNeedTrace = false;
//			m_SendQueue.clear();
//			TRACEOUT_T("Tady -> >>>>>>>>> !!SendQueue Clear!! <<<<<<<<<<<", m_SendQueue.size());
		}

		if ( 1 == m_SendQueue.size() )
		{
			// if only 1 item queued, means no pending sending, so send it immediately
			this->send_buffer_n(buf);
		}
	}

	bool do_send2(tcp_socket_impl::byte_buffer_ptr buf)
	{
		// queue sending item
		if (m_SendQueue.size() < 5000)
		{
			m_SendQueue.push_back(buf);
			// 			if (m_SendQueue.size() >= 20)
			// 			{
			// 				//TRACEOUT_T("Tady -> !!!!! SendQueue size [%d] !!!!!", m_SendQueue.size());
			// 			}
		}
		else
		{
			//			bNeedTrace = false;
			//			m_SendQueue.clear();
			//			TRACEOUT_T("Tady -> >>>>>>>>> !!SendQueue Clear!! <<<<<<<<<<<", m_SendQueue.size());
			return false;
		}

		if ( 1 == m_SendQueue.size() )
		{
			// if only 1 item queued, means no pending sending, so send it immediately
			this->send_buffer_n(buf);
		}
		return true;
	}
	void send_buffer_n(tcp_socket_impl::byte_buffer_ptr tcpdata)
	{
		m_impl->send_n(tcpdata);
		if (m_SendTimeout > 0) m_ConnectTimer.start(m_SendTimeout * 1000); // Borrow ConnectTimer.
	}

public:
	UINT32 GetSendQueueSize() { return m_SendQueue.size(); }


protected:
	boost::shared_ptr<tcp_socket_impl> m_impl;
	once_timer m_ConnectTimer;
	UINT m_ConnectTimeout;
	UINT m_SendTimeout;

	/// the first item in send-queue is the buffer being sending, the rest are to be sent
	std::list<tcp_socket_impl::byte_buffer_ptr> m_SendQueue;
};


typedef boost::shared_ptr<tcp_socket> tcp_socket_ptr;


inline std::ostream& operator<<(std::ostream& os, const tcp_socket& sock)
{
	os << "tcp_socket: " << sock.remote_address();
	return os;
}

#endif