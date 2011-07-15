
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_PACKET_BASE_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_PACKET_BASE_H_

/**
 * @file
 * @brief 包含报文封包的一些基础类
 */

#include <synacast/protocol/PacketStream.h>
#include <ppl/io/serializable.h>
#include <ppl/data/pool.h>
#include <boost/noncopyable.hpp>

using ppl::io::serializable;
using ppl::io::data_input_stream;
using ppl::io::data_output_stream;




/// 带类型的报文
class PacketBase : public pool_object, private boost::noncopyable, public serializable
{
public:

	explicit PacketBase( UINT8 action ) : m_action(action)
	{
	}
	virtual ~PacketBase()
	{
	}

	//virtual bool Parse(const BYTE* data, size_t size)
	//{
	//	assert(false);
	//	return false;
	//}

	UINT8 GetAction() const { return m_action; }

	size_t GetSize() const
	{
		return get_object_size();
	}

	void Write(data_output_stream& os) const
	{
		this->write_object(os);
	}

	void Write(void* buffer, size_t capacity) const
	{
		PacketOutputStream os(buffer, capacity);
		this->Write(os);
		assert(os.position() == get_object_size());
	}

	/// 从内存缓冲区解析报文
	bool Read( const unsigned char* data, size_t size )
	{
		PacketInputStream in( data, size );
		bool res = this->read_object( in );
		if ( res )
		{
			assert( in.position() == get_object_size() );
		}
		return res;
	}

	bool Read(data_input_stream& is)
	{
		return this->read_object(is);
	}

	virtual bool read_object( data_input_stream& is )
	{
		assert( false );
		return false;
	}

	/// 将对象数据输出到流
	virtual void write_object( data_output_stream& os ) const
	{
		assert(false);
	}

	/// 获取对象大小
	virtual size_t get_object_size() const
	{
		assert(false);
		return 0;
	}

protected:
	void SetAction(UINT8 action)
	{
		assert(action > 0);
		m_action = action;
	}

private:
	/// 命令字
	UINT8 m_action;
};

class EmptyPacket : public PacketBase
{
public:
	explicit EmptyPacket(UINT8 action) : PacketBase(action)
	{
	}

	virtual bool read_object( ppl::io::data_input_stream& is )
	{
		return true;
	}

	virtual void write_object( ppl::io::data_output_stream& os ) const
	{
	}

	virtual size_t get_object_size() const
	{
		return 0;
	}
};

/// 使用临时缓冲区的报文
class TemporaryPacket : public PacketBase
{
public:
	explicit TemporaryPacket( UINT8 action, const BYTE* data, size_t size ) : PacketBase( action ), m_TempData( data ), m_TempSize( size )
	{

	}

public:
	virtual size_t get_object_size() const
	{
		return this->m_TempSize;
	}

	virtual void write_object(data_output_stream& os) const
	{
		os.write_n( m_TempData, m_TempSize );
	}

private:
	const BYTE* m_TempData;
	size_t m_TempSize;
};

#endif
