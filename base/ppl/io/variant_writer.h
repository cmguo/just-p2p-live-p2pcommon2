
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_IO_VARIANT_WRITER_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_IO_VARIANT_WRITER_H_


/************************************************************************
此文件包含一些工具类和函数，提供写入带长度的变体对象功能，例如字符串和数组
************************************************************************/


#include <ppl/io/data_output_stream.h>
#include <string>
#include <vector>

#if defined(_MSC_VER)
#pragma push_macro("max")
#undef max
#endif
#include <limits>



namespace ppl { namespace io { 


/// 对std::basic_string的writer
template <typename LengthT, typename CharT, typename TraitsT, typename AllocT>
class variant_string_writer
{
private:
	const variant_string_writer& operator=( const variant_string_writer& );
public:
	const std::basic_string<CharT, TraitsT, AllocT>& data;
	explicit variant_string_writer( const std::basic_string<CharT, TraitsT, AllocT>& s ) : data( s )
	{
		LIVE_ASSERT( data.size() < std::numeric_limits<LengthT>::max() );
	}
};

template <typename LengthT, typename CharT, typename TraitsT, typename AllocT>
inline data_output_stream& operator<<( data_output_stream& os, const variant_string_writer<LengthT, CharT, TraitsT, AllocT>& writer )
{
	LengthT len = static_cast<LengthT>( writer.data.size() );
	return os << len << writer.data;
}



/// 对std::vector的writer
template <typename LengthT, typename AllocT, typename T>
class variant_vector_writer
{
private:
	const variant_vector_writer& operator=( const variant_vector_writer& );
public:
	const std::vector<T, AllocT>& items;
	explicit variant_vector_writer( const std::vector<T, AllocT>& d ) : items( d )
	{
		LIVE_ASSERT( items.size() < std::numeric_limits<LengthT>::max() );
	}
};

template <typename LengthT, typename AllocT, typename T>
inline data_output_stream& operator<<( data_output_stream& os, const variant_vector_writer<LengthT, AllocT, T>& writer )
{
	LengthT len = static_cast<LengthT>( writer.items.size() );
	os << len;
	os.write_vector( writer.items );
	return os;
}


/// 对std::vector的writer
template <typename LengthT, typename T, typename AllocT>
class variant_buffer_writer
{
private:
	const variant_buffer_writer& operator=( const variant_buffer_writer& );
public:
	const basic_buffer<T, AllocT>& items;
	explicit variant_buffer_writer( const basic_buffer<T, AllocT>& d ) : items( d )
	{
		LIVE_ASSERT( items.size() < std::numeric_limits<LengthT>::max() );
	}
};

template <typename LengthT, typename T, typename AllocT>
inline data_output_stream& operator<<( data_output_stream& os, const variant_buffer_writer<LengthT, T, AllocT>& writer )
{
	LengthT len = static_cast<LengthT>( writer.items.size() );
	os << len;
	os.write_buffer( writer.items );
	return os;
}



template <typename LengthT>
class variant_writer
{
public:

	/// string
	template <typename CharT, typename TraitsT, typename AllocT>
	static variant_string_writer<LengthT, CharT, TraitsT, AllocT> make( const std::basic_string<CharT, TraitsT, AllocT>& data )
	{
		return variant_string_writer<LengthT, CharT, TraitsT, AllocT>( data );
	}

	/// vector
	template <typename AllocT, typename T>
	static variant_vector_writer<LengthT, AllocT, T> make( const std::vector<T, AllocT>& data )
	{
		return variant_vector_writer<LengthT, AllocT, T>( data );
	}


	/// buffer
	template <typename T, typename AllocT>
	static variant_buffer_writer<LengthT, T, AllocT> make( const basic_buffer<T, AllocT>& data )
	{
		return variant_buffer_writer<LengthT, T, AllocT>( data );
	}

};

typedef variant_writer<UINT8> variant_writer_uint8;
typedef variant_writer<UINT16> variant_writer_uint16;
typedef variant_writer<UINT32> variant_writer_uint32;



} }


#if defined(_MSC_VER)
#pragma pop_macro("max")
#endif


#endif