
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_IO_MEMORY_OUTPUT_STREAM_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_IO_MEMORY_OUTPUT_STREAM_H_

#include <ppl/io/output_stream.h>
#include <ppl/data/int.h>
#include <ppl/util/macro.h>

#include <boost/noncopyable.hpp>

#include <vector>
#include <string>
#include <assert.h>
#include <stdexcept>

namespace ppl { namespace io { 


class memory_output_stream : public output_stream, private boost::noncopyable
{
public:
	explicit memory_output_stream(void* buf, size_t capacity) : m_buffer(static_cast<BYTE*>(buf)), m_capacity(capacity), m_size(0)
	{
		LIVE_ASSERT(m_buffer != NULL && m_capacity > 0);
		LIVE_ASSERT( ! ::IsBadWritePtr(m_buffer, m_capacity));
	}

	size_t capacity() const { return m_capacity; }
	size_t size() const { return m_size; }

	virtual void write_n( const void* data, size_t size )
	{
		this->write_bytes( data, size );
	}

	virtual void write_byte( BYTE val )
	{
		check_available( 1 );
		m_buffer[m_size] = val;
		m_size += 1;
	}

	virtual size_t position() const
	{
		return m_size;
	}

	void check_available(size_t size)
	{
		if ( false == this->is_available(size) )
			throw std::overflow_error("memory output stream overflow");
	}

	bool is_available(size_t size)
	{
		return (m_size + size <= m_capacity);
	}

	//template <typename ValueT>
	//void write_value(ValueT val)
	//{
	//	check_available(sizeof(ValueT));
	//	WRITE_MEMORY(m_buffer + m_size, val, ValueT);
	//	m_size += sizeof(ValueT);
	//}

	void write_bytes(const void* data, size_t size)
	{
		check_available(size);
		if (size > 0)
		{
			memcpy(m_buffer + m_size, data, size);
			m_size += size;
		}
	}


	template <typename ValueT>
	void write_array(const ValueT* vals, size_t count)
	{
		write_bytes(vals, sizeof(ValueT) * count);
	}

	template <typename ValueT, typename AllocatorT>
	void write_vector(const std::vector<ValueT, AllocatorT>& vals)
	{
		if (!vals.empty())
		{
			write_array(&vals[0], vals.size());
		}
	}

	//template <typename StructT>
	//void write_struct(const StructT& val)
	//{
	//	write_bytes(&val, sizeof(StructT));
	//}

	//template <typename CharT, typename CharTraitsT, typename AllocatorT>
	//void write_string(const std::basic_string<CharT, CharTraitsT, AllocatorT>& s)
	//{
	//	this->write_bytes( s.data(), s.size() * sizeof( CharT ) );
	//}

	//void write_string(const char* s)
	//{
	//	this->write_bytes( s, strlen(s) );
	//}

	//void write_char(char val) { write_value(val); }
	//void write_short(short val) { write_value(val); }
	//void write_int(int val) { write_value(val); }
	//void write_long(long val) { write_value(val); }
	//void write_longlong(longlong val) { write_value(val); }

	////void write_byte(BYTE val) { write_value(val); }
	//void write_word(unsigned short val) { write_value(val); }
	//void write_dword(unsigned long val) { write_value(val); }
	//void write_qword(ulonglong val) { write_value(val); }

	//void write_int8(signed char val) { write_value(val); }
	//void write_uint8(BYTE val) { write_value(val); }

	//void write_int16(short val) { write_value(val); }
	//void write_uint16(unsigned short val) { write_value(val); }

	//void write_int32(long val) { write_value(val); }
	//void write_uint32(unsigned long val) { write_value(val); }

	//void write_int64(longlong val) { write_value(val); }
	//void write_uint64(ulonglong val) { write_value(val); }

	//template <typename ObjT>
	//void write_object( const ObjT& obj )
	//{
	//	obj.write_object( *this );
	//}

private:
	BYTE* m_buffer;
	size_t m_capacity;
	size_t m_size;
};



} }


#endif