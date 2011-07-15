
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_IO_MEMORY_INPUT_STREAM_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_IO_MEMORY_INPUT_STREAM_H_

#include <ppl/io/input_stream.h>
#include <ppl/data/int.h>
#include <ppl/util/macro.h>

#include <boost/noncopyable.hpp>

#include <vector>
#include <string>
#include <assert.h>

namespace ppl { namespace io { 


class memory_input_stream : public input_stream, private boost::noncopyable
{
public:
	explicit memory_input_stream(const void* buf, size_t size) : m_buffer(static_cast<const BYTE*>(buf)), m_size(size), m_position(0)
	{
		//assert(m_buffer != NULL);
		//assert( ! ::IsBadReadPtr(m_buffer, m_size));
	}

	//size_t position() const { return m_position; }
	//size_t size() const { return m_size; }

	virtual bool read_n( void* buf, size_t size )
	{
		return this->read_bytes( buf, size );

	}
	virtual bool read_byte( BYTE& val )
	{
		if ( false == is_available( 1 ) )
			return false;
		val = m_buffer[m_position];
		++m_position;
		return true;
	}

	virtual size_t available() const
	{
		if ( m_size >= m_position )
			return m_size - m_position;
		return 0;
	}
	virtual bool is_available( size_t size ) const
	{
		return (m_position + size <= m_size);
	}

	virtual bool skip_n( size_t size )
	{
		if ( false == is_available( size ) )
			return false;
		m_position += size;
		return true;
	}

	virtual size_t position() const
	{
		return m_position;
	}

	virtual size_t total_size() const
	{
		return m_size;
	}
	virtual const unsigned char* get_buffer()
	{
		return m_buffer;
	}

	//void check_available(size_t size)
	//{
	//	assert( is_available( size ) );
	//}

	//template <typename ValueT>
	//bool read_value(ValueT& val)
	//{
	//	if ( false == is_available( sizeof(ValueT) ) )
	//		return false;
	//	val = READ_MEMORY(m_buffer + m_position, ValueT);
	//	m_position += sizeof(ValueT);
	//	return true;
	//}

	bool read_bytes(void* data, size_t size)
	{
		if ( false == is_available(size) )
			return false;
		if (size > 0)
		{
			memcpy(data, m_buffer + m_position, size);
			m_position += size;
		}
		return true;
	}

	//template <typename T, typename AllocT>
	//void write_buffer(const basic_buffer<T, AllocT>& buf)
	//{
	//	this->write_array(buf.data(), buf.size());
	//}


	template <typename ValueT>
	bool read_array(ValueT* vals, size_t count)
	{
		if ( 0 == count )
			return true;
		return read_bytes(vals, sizeof(ValueT) * count);
	}

	template <typename ValueT, typename AllocatorT>
	bool read_vector(size_t count, std::vector<ValueT, AllocatorT>& vals)
	{
		if ( 0 == count )
			return true;
		if ( false == is_available( sizeof(ValueT) * count ) )
		{
			assert(false);
			return false;
		}
		// 先检查数据是否充足，再进行resize操作，避免count过大，resize耗尽内存
		vals.resize( count );
		if ( read_array( &vals[0], count ) )
			return true;
		vals.clear();
		return false;
	}

	template <typename StructT>
	bool read_struct(StructT& val)
	{
		return read_bytes(&val, sizeof(StructT));
	}

	template <typename CharT, typename CharTraitsT, typename AllocatorT>
	bool read_string(size_t count, std::basic_string<CharT, CharTraitsT, AllocatorT>& s)
	{
		if ( 0 == count )
		{
			s.clear();
			return true;
		}
		// 先检查数据是否充足，再进行resize操作，避免count过大，resize耗尽内存
		if ( false == is_available( count * sizeof(CharT) ) )
		{
			assert(false);
			return false;
		}
		s.resize( count );
		if ( read_array( &s[0], count ) )
			return true;
		s.clear();
		return false;
	}

	//bool read_char(char& val) { return read_value(val); }
	//bool read_short(short& val) { return read_value(val); }
	//bool read_int(int& val) { return read_value(val); }
	//bool read_long(long& val) { return read_value(val); }
	//bool read_longlong(longlong& val) { return read_value(val); }

	//bool read_uchar(BYTE& val) { return read_value(val); }
	//bool read_ushort(unsigned short& val) { return read_value(val); }
	//bool read_uint(unsigned int& val) { return read_value(val); }
	//bool read_ulong(unsigned long& val) { return read_value(val); }
	//bool read_ulonglong(ulonglong& val) { return read_value(val); }

	////bool read_byte(BYTE& val) { return read_value(val); }
	//bool read_word(unsigned short& val) { return read_value(val); }
	//bool read_dword(unsigned long& val) { return read_value(val); }
	//bool read_qword(ulonglong& val) { return read_value(val); }

	//bool read_int8(signed char& val) { return read_value(val); }
	//bool read_uint8(BYTE& val) { return read_value(val); }

	//bool read_int16(short& val) { return read_value(val); }
	//bool read_uint16(unsigned short& val) { return read_value(val); }

	//bool read_int32(int& val) { return read_value(val); }
	//bool read_uint32(unsigned int& val) { return read_value(val); }

	//bool read_int64(longlong& val) { return read_value(val); }
	//bool read_uint64(ulonglong& val) { return read_value(val); }

	template <typename ObjT>
	bool read_object( ObjT& obj )
	{
		return obj.read_object( *this );
	}

private:
	const BYTE* m_buffer;
	const size_t m_size;
	size_t m_position;
};


} }

#endif

