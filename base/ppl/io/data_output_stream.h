
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_IO_DATA_OUTPUT_STREAM_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_IO_DATA_OUTPUT_STREAM_H_

#include <ppl/io/output_stream.h>
#include <ppl/io/memory_output_stream.h>
#include <ppl/data/byte_order.h>
#include <ppl/data/int.h>
#include <ppl/data/buffer.h>

#include <boost/noncopyable.hpp>

#include <vector>
#include <string>
#include <assert.h>


namespace ppl { namespace io { 


/// 数据的输入流，提供各种整数类型数据的读取功能
class data_output_stream : private boost::noncopyable
{
public:
	explicit data_output_stream( output_stream* os, bool bigEndian ) 
		: m_out( os )
		, m_byte_order( bigEndian )//, m_good( true )
	{
	}

	void attach( output_stream& out )
	{
		m_out = &out;
	}

	output_stream& get_output() const
	{
		return *m_out;
	}

	bool is_big_endian() const
	{
		return m_byte_order.is_big_endian();
	}
	void set_big_endian( bool isBigEndian )
	{
		m_byte_order.set_big_endian( isBigEndian );
	}

	size_t position() const
	{
		return m_out->position();
	}

	//typedef bool data_output_stream::*unspecified_bool_type;

	//operator unspecified_bool_type() const
	//{
	//	return m_good ? &data_output_stream::m_good : 0;
	//}


	void write_byte( BYTE val )
	{
		m_out->write_byte( val );
	}

	void write_n( const void* buf, size_t size )
	{
		m_out->write_n( buf, size );
	}

	template<typename CharT, typename TraitsT, typename AllocT>
	void write_string( const std::basic_string<CharT, TraitsT, AllocT>& s )
	{
		this->write_raw_buffer( s.data(), s.size() );
	}

	template<typename T, typename AllocT>
	void write_buffer( const basic_buffer<T, AllocT>& s )
	{
		this->write_raw_buffer( s.data(), s.size() );
	}

	template<typename T, typename AllocT>
	void write_vector( const std::vector<T, AllocT>& items )
	{
		if ( items.size() > 0 )
		{
			write_array( &items[0], items.size() );
		}
	}


	void write_uint8( BYTE val )
	{
		write_byte( val );
	}

	void write_uint16( unsigned short val )
	{
		UINT16 x = m_byte_order.convert_word( val );
		write_n( &x, 2 );
	}

	void write_uint32( unsigned long val )
	{
		UINT32 x = m_byte_order.convert_dword( val );
		write_n( &x, 4 );
	}

	void write_uint64( UINT64 val )
	{
		UINT64 x = m_byte_order.convert_qword( val );
		write_n( &x, 8 );
	}

	void write_int8( signed char val )
	{
		write_uint8( static_cast<BYTE>( val ) );
	}

	void write_int16( short val )
	{
		write_uint16( static_cast<unsigned short>( val ) );
	}

	void write_int32( long val )
	{
		write_uint32( static_cast<unsigned long>( val ) );
	}

	void write_int64( INT64 val )
	{
		write_uint64( static_cast<UINT64>( val ) );
	}

	/// 通用版本，必须放在特化版本前面，否则在vc6下编译可能报错
	template <typename T>
	void write_array( const T* data, size_t size )
	{
		for ( size_t index = 0; index < size; ++index )
		{
			*this << data[index];
		}
	}
	/// 针对字节数组的优化版本
	void write_array( const unsigned char* data, size_t size )
	{
		this->write_n( data, size );
	}
	/// 针对字符数组的优化版本
	void write_array( const char* data, size_t size )
	{
		this->write_n( data, size );
	}

	/// 针对字节数组的write_raw_buffer
	void write_raw_buffer( const unsigned char* data, size_t size )
	{
		this->write_n( data, size );
	}
	/// 针对字符数组的write_raw_buffer
	void write_raw_buffer( const char* data, size_t size )
	{
		this->write_n( data, size );
	}

private:
	output_stream* m_out;
	byte_order m_byte_order;
	//bool m_good;
};


inline data_output_stream& operator<<( data_output_stream& os, signed char val )
{
	os.write_int8( val );
	return os;
}

inline data_output_stream& operator<<( data_output_stream& os, char val )
{
	os.write_int8( val );
	return os;
}

inline data_output_stream& operator<<( data_output_stream& os, BYTE val )
{
	os.write_uint8( val );
	return os;
}

inline data_output_stream& operator<<( data_output_stream& os, signed short val )
{
	os.write_int16( val );
	return os;
}

inline data_output_stream& operator<<( data_output_stream& os, unsigned short val )
{
	os.write_uint16( val );
	return os;
}

inline data_output_stream& operator<<( data_output_stream& os, int val )
{
	os.write_int32( val );
	return os;
}

inline data_output_stream& operator<<( data_output_stream& os, unsigned int val )
{
	os.write_uint32( val );
	return os;
}

inline data_output_stream& operator<<( data_output_stream& os, long val )
{
	os.write_int32( val );
	return os;
}

inline data_output_stream& operator<<( data_output_stream& os, unsigned long val )
{
	os.write_uint32( val );
	return os;
}

inline data_output_stream& operator<<( data_output_stream& os, INT64 val )
{
	os.write_int64( val );
	return os;
}

inline data_output_stream& operator<<( data_output_stream& os, UINT64 val )
{
	os.write_uint64( val );
	return os;
}

template<typename TraitsT, typename AllocT>
inline data_output_stream& operator<<( data_output_stream& os, const std::basic_string<char, TraitsT, AllocT>& s )
{
	os.write_string( s );
	return os;
}

template<typename TraitsT, typename AllocT>
inline data_output_stream& operator<<( data_output_stream& os, const std::basic_string<unsigned char, TraitsT, AllocT>& s )
{
	os.write_string( s );
	return os;
}

template <typename T, typename AllocT>
inline data_output_stream& operator<<( data_output_stream& os, const std::vector<T, AllocT>& data )
{
	os.write_vector( data );
	return os;
}

template <typename AllocT>
inline data_output_stream& operator<<( data_output_stream& os, const basic_buffer<unsigned char, AllocT>& data )
{
	os.write_buffer( data );
	return os;
}

template <typename AllocT>
inline data_output_stream& operator<<( data_output_stream& os, const basic_buffer<char, AllocT>& data )
{
	os.write_buffer( data );
	return os;
}


class memory_data_output_stream : public data_output_stream
{
public:
	memory_data_output_stream( void* data, size_t size, bool bigEndian ) : data_output_stream( NULL, bigEndian ), m_mis( data, size )
	{
		this->attach( m_mis );
	}

private:
	memory_output_stream m_mis;
};

template <typename Type>
void write_memory(void * buf, Type const & t)
{
    memory_data_output_stream os(buf, sizeof(t), false);
    os << t;
}


} }

#endif