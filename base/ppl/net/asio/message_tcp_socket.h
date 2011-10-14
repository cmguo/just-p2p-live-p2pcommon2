
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_NET_ASIO_MESSAGE_TCP_SOCKET_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_NET_ASIO_MESSAGE_TCP_SOCKET_H_

#include <ppl/net/asio/tcp_socket.h>
#include <ppl/data/byte_order.h>
#include <ppl/data/bit_converter.h>

#include <boost/asio/streambuf.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <memory>

#include <ppl/io/data_input_stream.h>
#include <ppl/io/data_output_stream.h>


/// message-based tcp socket
class message_tcp_socket : public tcp_socket
{
	typedef tcp_socket_impl::byte_buffer_ptr byte_buffer_ptr;
public:
	explicit message_tcp_socket(boost::asio::io_service& ioservice = io_service_provider::default_service()) : tcp_socket(ioservice)
	{
		init_message();
	}
	explicit message_tcp_socket(boost::shared_ptr<tcp_socket_impl> sock) : tcp_socket(sock)
	{
		init_message();
	}
	~message_tcp_socket()
	{
		this->close();
	}

	static void set_big_endian(bool bigEndian)
	{
		ref_big_endian() = bigEndian;
	}
	static void max_receiving_message_size(size_t size)
	{
		LIVE_ASSERT(size > 0 && size < 32 * 1024 * 1024);
		if (size > 0)
		{
			ref_max_receiving_message_size() = size;
		}
	}

	static void max_sending_message_size(size_t size)
	{
		LIVE_ASSERT(size > 0 && size < 32 * 1024 * 1024);
		if (size > 0)
		{
			ref_max_sending_message_size() = size;
		}
	}

	virtual bool close()
	{
		reset_state();
		return tcp_socket::close();
	}

	virtual bool send(const void* data, size_t size)
	{
		if (size >= ref_max_sending_message_size())
		{
			LIVE_ASSERT( false );
			return false;
		}
		byte_buffer_ptr buf(new byte_buffer(size + sizeof(UINT32)));
		//UINT32 packetSize = byte_order(ref_big_endian()).convert_dword(size);
		WRITE_MEMORY(buf->data(), size, UINT32);

		memcpy(buf->data() + sizeof(UINT32), data, size );
		this->do_send(buf);
		return true;
	}

	virtual bool receive()
	{
		return receive_message();
	}

	bool receive_message()
	{
		if (m_head_ok)
		{
			LIVE_ASSERT(false);
			return false;
		}
		LIVE_ASSERT(m_head->size() == sizeof(UINT32));
		return receive_buffer_n(m_head);
	}

protected:
	void init_message()
	{
		reset_state();
		m_head.reset(new byte_buffer(sizeof(UINT32)));
		m_body.reset(new byte_buffer(128));
		m_buffer.reset(new byte_buffer(128));
	}

	virtual void on_receive(const error_code& err, size_t bytes, tcp_socket_impl::byte_buffer_ptr buf)
	{
		if ( err )
		{
			reset_state();
			get_listener()->on_socket_receive_failed(this, err.value());
			return;
		}
		if (m_head_ok)
		{
			// body received
			LIVE_ASSERT(buf == m_body);
			LIVE_ASSERT(m_body->size() == bytes);
			reset_state();
			// m_body和m_buffer交换，然后将m_buffer交给上层处理，m_body则可以使用新的空闲的缓冲区进行接收
			m_body.swap(m_buffer);
			get_listener()->on_socket_receive(this, m_buffer->data(), m_buffer->size());
		}
		else
		{
			// head received
			LIVE_ASSERT(buf == m_head);
			LIVE_ASSERT(m_head->size() == bytes);
			UINT32 msgSize = READ_MEMORY(m_head->data(), UINT32);
			if (msgSize > ref_max_receiving_message_size())
			{
				// msg size is too large, report error
				reset_state();
				get_listener()->on_socket_receive_failed(this, EFAULT);
				return;
			}
			// prepare buffer to receive body
			m_body->ensure_size(msgSize);
			m_head_ok = true;
			receive_buffer_n(m_body);
		}
	}

	void reset_state()
	{
		m_head_ok = false;
	}

	static bool& ref_big_endian()
	{
		static bool isBigEndian = false;
		return isBigEndian;
	}

	static size_t& ref_max_receiving_message_size()
	{
		static size_t maxRecvMsgSize = 63 * 1024;
		return maxRecvMsgSize;
	}

	static size_t& ref_max_sending_message_size()
	{
		static size_t maxSendMsgSize = 63 * 1024;
		return maxSendMsgSize;
	}

protected:
	u_int m_length;
	byte_buffer_ptr m_head;
	byte_buffer_ptr m_body;
	/// 用来存储最终收到的数据，并给上层使用
	byte_buffer_ptr m_buffer;

	/// 是否在接收过程中，也就是是否已经收到头部
	bool m_head_ok;
};

#endif

