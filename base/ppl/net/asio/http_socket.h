
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_NET_ASIO_HTTP_SOCKET_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_NET_ASIO_HTTP_SOCKET_H_

#include <ppl/net/asio/tcp_socket.h>
#include <ppl/net/asio/timer.h>
#include <ppl/net/asio/detail/tcp_socket_impl.h>

#include <boost/asio/read_until.hpp>
#include <boost/asio/streambuf.hpp>


class http_socket : public tcp_socket
{
public:
	const static size_t HTTP_HEADER_MAX_SIZE = 10*1024;

	explicit http_socket(boost::shared_ptr<tcp_socket_impl> sock) : tcp_socket(sock)
	{
		m_receive_timeout = 60000;
		m_receive_timeout_timer.set_callback(boost::bind(&http_socket::on_receive_timeout, this));
	}
	explicit http_socket() 
	{
		m_receive_timeout = 60000;
		m_receive_timeout_timer.set_callback(boost::bind(&http_socket::on_receive_timeout, this));
	}

	virtual ~http_socket()
	{
	}

	virtual bool receive()
	{
		return this->http_receive_headers();
	}

	bool receive(size_t size)
	{

		if (request_.size() == 0)
		{
			receive_n(size);
			m_receive_timeout_timer.start(m_receive_timeout);
		}
		else
		{
			if (size <= request_.size())
			{
				io_service_provider::get_default()->post(boost::bind(&http_socket::on_http_left_received, this, size));
			}
			else
			{
				receive_n(size - request_.size());
				m_receive_timeout_timer.start(m_receive_timeout);
			}
		}
		return true;
	}

protected:
	void on_receive_timeout()
	{
		this->close();
	}

	bool http_receive_headers()
	{
		boost::asio::async_read_until(
			m_impl->get_socket(), 
			request_, 
			//boost::regex("\r\n\r\n"),
			"\r\n\r\n",
			boost::bind(
				&tcp_socket_impl::on_lengthed_event, 
				m_impl, 
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred, 
				tcp_socket_impl::lengthed_event_callback_type(boost::bind( &http_socket::on_http_headers_received, this, _1, _2 ))
			)
			);
		m_receive_timeout_timer.start(m_receive_timeout);
		return true;
	}

protected:
	void on_http_headers_received(const boost::system::error_code& err, size_t bytes)
	{
		m_receive_timeout_timer.stop();
		if (err)
		{
			close();
			this->get_listener()->on_socket_receive_failed(this, err.value());
			return;
		}
		byte_buffer buf;
		if (bytes > 0)
		{
			buf.resize(bytes);
			std::istream request_stream(&request_);
			request_stream.read(reinterpret_cast<char*>(buf.data()), bytes);
		}
		this->get_listener()->on_socket_receive(this, buf.data(), bytes);
	}

	virtual void on_receive(const error_code& err, size_t bytes, tcp_socket_impl::byte_buffer_ptr buf)
	{
		m_receive_timeout_timer.stop();
		if ( err )
		{
			if (bytes > 0)
			{
				LIVE_ASSERT(buf->size() >= bytes);
				if (request_.size() > 0)
				{
					size_t head_left_bytes = request_.size();
					byte_buffer bigBuf;
					bigBuf.resize(head_left_bytes + bytes);

					std::istream request_stream(&request_);
					request_stream.read(reinterpret_cast<char*>(bigBuf.data()), head_left_bytes);
					memcpy(bigBuf.data() + head_left_bytes, buf->data(), bytes);
					get_listener()->on_socket_receive(this, bigBuf.data(), head_left_bytes + bytes);
				}
				else
					get_listener()->on_socket_receive(this, buf->data(), bytes);
			}
			else
				get_listener()->on_socket_receive_failed(this, err.value());
			return;
		}
		LIVE_ASSERT(buf->size() >= bytes);
		LIVE_ASSERT(buf->size() == bytes);
		if (request_.size() > 0)
		{
			size_t head_left_bytes = request_.size();
			byte_buffer bigBuf;
			bigBuf.resize(head_left_bytes + bytes);

			std::istream request_stream(&request_);
			request_stream.read(reinterpret_cast<char*>(bigBuf.data()), head_left_bytes);
			memcpy(bigBuf.data() + head_left_bytes, buf->data(), bytes);
			get_listener()->on_socket_receive(this, bigBuf.data(), head_left_bytes + bytes);
		}
		else
			get_listener()->on_socket_receive(this, buf->data(), bytes);
	}

	void on_http_left_received(size_t bytes)
	{
		byte_buffer buf;
		if (bytes > 0)
		{
			buf.resize(bytes);
			std::istream request_stream(&request_);
			request_stream.read(reinterpret_cast<char*>(buf.data()), bytes);
		}
		this->get_listener()->on_socket_receive(this, buf.data(), bytes);
	}

protected:
	boost::asio::streambuf request_;
	once_timer m_receive_timeout_timer;
	UINT m_receive_timeout;

};

#endif


