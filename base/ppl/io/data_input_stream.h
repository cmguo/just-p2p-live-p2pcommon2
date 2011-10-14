
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_IO_DATA_INPUT_STREAM_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_IO_DATA_INPUT_STREAM_H_

#include <ppl/io/input_stream.h>
#include <ppl/io/memory_input_stream.h>
#include <ppl/data/byte_order.h>
#include <ppl/data/int.h>
#include <ppl/data/buffer.h>
#include <ppl/util/macro.h>

#include <boost/noncopyable.hpp>

#include <vector>
#include <string>
#include <assert.h>


namespace ppl { namespace io { 


/// ���ݵ����������ṩ���������������ݵĶ�ȡ����
class data_input_stream : private boost::noncopyable
{
public:
	explicit data_input_stream( input_stream* is, bool bigEndian ) 
		: m_in( is )
		, m_byte_order( bigEndian ) 
		, m_good( is != NULL )
	{
	}

	typedef bool data_input_stream::*unspecified_bool_type;

	operator unspecified_bool_type() const
	{
		return m_good ? &data_input_stream::m_good : 0;
	}

	bool operator!() const
	{
		return ! m_good;
	}

	/// ���е�read�����������ȼ��m_good�����m_goodΪfalse����ȡʧ��
	bool good() const
	{
		return m_good;
	}
	/// ����״̬Ϊ�����⣬�Խ�ֹ�����Ķ�ȡ����
	void set_bad()
	{
		//LIVE_ASSERT(m_good);
		m_good = false;
	}

	void attach( input_stream& is )
	{
		m_in = &is;
		m_good = true;
	}

	input_stream& get_input() const
	{
		return *m_in;
	}

	bool is_big_endian() const
	{
		return m_byte_order.is_big_endian();
	}
	void set_big_endian( bool isBigEndian )
	{
		m_byte_order.set_big_endian( isBigEndian );
	}

	size_t available() const
	{
		if ( false == m_good )
			return 0;
		return m_in->available();
	}
	bool is_available( size_t size ) const
	{
		if ( false == m_good )
			return false;
		return m_in->is_available( size );
	}

	bool skip_n( size_t size )
	{
		if ( false == m_good )
			return false;
		m_good = m_in->skip_n( size );
		return m_good;
	}

	/// ��ǰ��ȡλ�ã������ڴ�����˵Ҳ���Ѿ����˵ĳ���
	size_t position() const
	{
		return m_in->position();
	}

	/// �ܳ��ȣ������ڴ�����˵����buffer���ܴ�С
	size_t total_size() const
	{
		return m_in->total_size();
	}
	const unsigned char* get_buffer()
	{
		return m_in->get_buffer();
	}

	/// try_read_nʧ�ܺ�������m_goodΪfalse������ȡstring/vector�ȶ���ʱʹ��
	bool try_read_n( size_t size )
	{
		if ( is_available( size ) )
			return true;
		m_good = false;
		LIVE_ASSERT(false);
		return false;
	}

	bool read_byte( BYTE& val )
	{
		// ���״̬�Ѿ�Ϊbad����Ϊ�ϴζ�ȡʧ�ܣ������ټ�����
		if ( false == m_good )
			return false;
		m_good = m_in->read_byte( val );
		return m_good;
	}

	bool read_n( void* buf, size_t size )
	{
		// ���״̬�Ѿ�Ϊbad����Ϊ�ϴζ�ȡʧ�ܣ������ټ�����
		if ( false == m_good )
			return false;
		m_good = m_in->read_n( buf, size );
		return m_good;
	}

	/// ����ֽ������read_raw_buffer
	bool read_raw_buffer( unsigned char* buf, size_t size )
	{
		return read_n( buf, size );
	}
	/// ����ַ������read_raw_buffer
	bool read_raw_buffer( char* buf, size_t size )
	{
		return read_n( buf, size );
	}

	/// ͨ�ð汾��read_array
	template <typename T>
	bool read_array( T* buf, size_t size )
	{
		for ( size_t index = 0; index < size; ++index )
		{
			if ( *this >> buf[index] )
			{
				continue;
			}
			else
			{
				LIVE_ASSERT(false);
				return false;
			}
		}
		return true;
	}
	/// ����ֽ������read_array
	bool read_array( unsigned char* buf, size_t size )
	{
		return read_n( buf, size );
	}
	/// ����ַ������read_array
	bool read_array( char* buf, size_t size )
	{
		return read_n( buf, size );
	}

	/// ʵ��ֻ֧��byte/char
	template<typename T, typename AllocT>
	bool read_buffer(size_t count, basic_buffer<T, AllocT>& s)
	{
		// �ȼ�������Ƿ���㣬�ٽ���resize����������count����resize�ľ��ڴ�
		// �������m_goodΪfalse,��ʹcountΪ0��Ҳ����false����ʾʧ��
		if ( false == try_read_n( count ) )
			return false;
		if ( 0 == count )
		{
			s.clear();
			return true;
		}
		s.resize( count );
		if ( read_raw_buffer( s.data(), count ) )
			return true;
		LIVE_ASSERT(false);
		return false;
	}

	/// ʵ��ֻ֧��byte/char
	template<typename CharT, typename TraitsT, typename AllocT>
	bool read_string(size_t count, std::basic_string<CharT, TraitsT, AllocT>& s)
	{
		// �ȼ�������Ƿ���㣬�ٽ���resize����������count����resize�ľ��ڴ�
		// �������m_goodΪfalse,��ʹcountΪ0��Ҳ����false����ʾʧ��
		if ( false == try_read_n( count ) )
			return false;
		if ( 0 == count )
		{
			s.erase();
			return true;
		}
		s.resize( count );
		if ( read_raw_buffer( &s[0], count ) )
			return true;
		LIVE_ASSERT(false);
		return false;
	}

	/// ��ȡһ��vector��itemSize��ʵ�ʶ�ȡ��item�Ĵ�С
	template<typename T, typename AllocT>
	bool read_vector(size_t count, std::vector<T, AllocT>& items, size_t itemSize)
	{
		LIVE_ASSERT( itemSize > 0 );
		// �ȼ�������Ƿ���㣬�ٽ���resize����������count����resize�ľ��ڴ�
		// �������m_goodΪfalse,��ʹcountΪ0��Ҳ����false����ʾʧ��
		if ( false == try_read_n( count * itemSize ) )
			return false;
		if ( 0 == count )
		{
			items.clear();
			return true;
		}
		items.resize( count );
		return read_array( &items[0], count );
	}


	bool read_uint8( BYTE& val )
	{
		return read_byte( val );
	}

	bool read_uint16( unsigned short& val )
	{
		UINT16 x;
		if ( false == read_n( &x, 2 ) )
			return false;
		val = m_byte_order.convert_word( x );
		return true;
	}

	bool read_uint32( unsigned long& val )
	{
		UINT32 x;
		if ( false == read_n( &x, 4 ) )
			return false;
		val = m_byte_order.convert_dword( x );
		return true;
	}

	bool read_uint32( unsigned int& val )
	{
		UINT32 x;
		if ( false == read_n( &x, 4 ) )
			return false;
		val = m_byte_order.convert_dword( x );
		return true;
	}

	bool read_uint64( UINT64& val )
	{
		UINT64 x;
		if ( false == read_n( &x, 8 ) )
			return false;
		val = m_byte_order.convert_qword( x );
		return true;
	}

	bool read_int8( signed char& val )
	{
		return read_uint8( reinterpret_cast<BYTE&>( val ) );
	}

	bool read_int16( short& val )
	{
		return read_uint16( reinterpret_cast<unsigned short&>( val ) );
	}

	bool read_int32( long& val )
	{
		return read_uint32( reinterpret_cast<unsigned long&>( val ) );
	}

	bool read_int32( int& val )
	{
		return read_uint32( reinterpret_cast<unsigned int&>( val ) );
	}

	bool read_int64( INT64& val )
	{
		return read_uint64( reinterpret_cast<UINT64&>( val ) );
	}

private:
	input_stream* m_in;
	byte_order m_byte_order;
	/// ״̬�Ƿ�Ϊgood�����Ϊfalse����ʾ�ϴζ�ȡ�Ѿ�ʧ�ܣ������Ķ�ȡ��ʧ��
	bool m_good;
};


inline data_input_stream& operator>>( data_input_stream& is, signed char& val )
{
	is.read_int8( val );
	return is;
}

inline data_input_stream& operator>>( data_input_stream& is, char& val )
{
	is.read_int8( reinterpret_cast<signed char&>( val ) );
	return is;
}

inline data_input_stream& operator>>( data_input_stream& is, BYTE& val )
{
	is.read_uint8( val );
	return is;
}

inline data_input_stream& operator>>( data_input_stream& is, signed short& val )
{
	is.read_int16( val );
	return is;
}

inline data_input_stream& operator>>( data_input_stream& is, unsigned short& val )
{
	is.read_uint16( val );
	return is;
}

inline data_input_stream& operator>>( data_input_stream& is, int& val )
{
	is.read_int32( reinterpret_cast<long&>( val ) );
	return is;
}

inline data_input_stream& operator>>( data_input_stream& is, unsigned int& val )
{
	is.read_uint32( reinterpret_cast<unsigned long&>( val ) );
	return is;
}

inline data_input_stream& operator>>( data_input_stream& is, long& val )
{
	is.read_int32( val );
	return is;
}

inline data_input_stream& operator>>( data_input_stream& is, unsigned long& val )
{
	is.read_uint32( val );
	return is;
}

inline data_input_stream& operator>>( data_input_stream& is, INT64& val )
{
	is.read_int64( val );
	return is;
}

inline data_input_stream& operator>>( data_input_stream& is, UINT64& val )
{
	is.read_uint64( val );
	return is;
}


class memory_data_input_stream : public data_input_stream
{
public:
	memory_data_input_stream( const void* data, size_t size, bool bigEndian ) : data_input_stream( NULL, bigEndian ), m_mis( data, size )
	{
		this->attach( m_mis );
	}

private:
	memory_input_stream m_mis;
};

template <typename Type>
Type read_memory(void const * buf)
{
    memory_data_input_stream is(buf, sizeof(Type), false);
    Type t;
    is >> t;
    return t;
}

} }


#endif