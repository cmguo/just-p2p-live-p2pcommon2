
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_IO_VARIANT_READER_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_IO_VARIANT_READER_H_


/************************************************************************
此文件包含一些工具类和函数，提供读取带长度的变体对象功能，例如字符串和数组
也就是一个长度标记（可能是UINT8/UINT16/UINT32的），然后跟着一段变体数据
************************************************************************/


#include <ppl/io/data_input_stream.h>
#include <ppl/data/buffer.h>
#include <ppl/data/int.h>
#include <string>
#include <vector>


namespace ppl { namespace io { 


/// 对std::basic_string的reader
template <typename LengthT, typename CharT, typename TraitsT, typename AllocT>
class variant_string_reader
{
private:
	const variant_string_reader& operator=( const variant_string_reader& );
public:
	std::basic_string<CharT, TraitsT, AllocT>& data;
	size_t max_count;
	explicit variant_string_reader( std::basic_string<CharT, TraitsT, AllocT>& s, size_t maxCount ) : data( s ), max_count( maxCount ) { }
};


/// 对std::vector的reader
template <typename LengthT, typename T, typename AllocT>
class variant_vector_reader
{
private:
	const variant_vector_reader& operator=( const variant_vector_reader& );
public:
	std::vector<T, AllocT>& items;
	size_t max_count;
	/// 需要提供item_size，以优化性能并避免异常情况（根据item_size来检查变体的实际尺寸）
	size_t item_size;
	size_t block_size;
	explicit variant_vector_reader( std::vector<T, AllocT>& d, size_t itemSize, size_t blockSize, size_t maxCount ) 
		: items( d ),max_count( maxCount ), item_size( itemSize ), block_size( blockSize)
	{
		assert( 0 == block_size || block_size >= item_size );
	}
};


/// 对std::basic_string的reader
template <typename LengthT, typename T, typename AllocT>
class variant_buffer_reader
{
private:
	const variant_buffer_reader& operator=( const variant_buffer_reader& );
public:
	basic_buffer<T, AllocT>& data;
	size_t max_count;
	explicit variant_buffer_reader( basic_buffer<T, AllocT>& s, size_t maxCount  ) : data( s ), max_count( maxCount ) { }
};


class variant_reader_util
{
public:
	/// 读取一个vector，itemSize是实际读取的item的大小，blockSize是每一块的大小（item位于每一个块的开头，也就是blockSize>=itemSize）
	template<typename T, typename AllocT>
	static bool read_padded_vector(data_input_stream& is, size_t count, std::vector<T, AllocT>& items, size_t itemSize, size_t blockSize)
	{
		items.resize(0);
		if ( 0 == blockSize )
		{
			blockSize = itemSize;
		}
		// if itemSize is 0, then size of T is inconstant
		if ( 0 == itemSize )
		{
			for (size_t index = 0; index < count; ++index)
			{
				T obj;
				if (is >> obj)
				{
					items.push_back(obj);
				}
				else
				{
					return false;
				}
			}
			return is;
		}
		assert( itemSize > 0 );
		assert( itemSize <= blockSize );
		size_t skippedSize = blockSize - itemSize;
		// 先检查数据是否充足，再进行resize操作，避免count过大，resize耗尽内存
		// 并且如果m_good为false,即使count为0，也返回false，表示失败
		if ( false == is.try_read_n( count * itemSize ) )
			return false;
		if ( 0 == count )
		{
			items.resize(0);
			return true;
		}
		items.resize( count );
		if ( 0 == skippedSize )
		{
			return is.read_array( &items[0], count );
		}
		for ( size_t index = 0; index < count; ++index )
		{
			if ( is >> items[index] && is.skip_n( skippedSize ) )
			{
				continue;
			}
			else
			{
				assert(false);
				return false;
			}
		}
		return true;
	}

};


template <typename LengthT>
class variant_reader
{
public:
	/// string
	template <typename CharT, typename TraitsT, typename AllocT>
	static variant_string_reader<LengthT, CharT, TraitsT, AllocT> make( std::basic_string<CharT, TraitsT, AllocT>& data, size_t maxCount = 0 )
	{
		return variant_string_reader<LengthT, CharT, TraitsT, AllocT>( data, maxCount );
	}

	/// vector
	template <typename T, typename AllocT>
	static variant_vector_reader<LengthT, T, AllocT> make( std::vector<T, AllocT>& data, size_t itemSize, size_t blockSize = 0, size_t maxCount = 0 )
	{
		return variant_vector_reader<LengthT, T, AllocT>( data, itemSize, blockSize, maxCount );
	}

	/// 基于byte的buffer
	template <typename T, typename AllocT>
	static variant_buffer_reader<LengthT, T, AllocT> make( basic_buffer<T, AllocT>& data, size_t maxCount = 0 )
	{
		return variant_buffer_reader<LengthT, T, AllocT>( data, maxCount );
	}

};

typedef variant_reader<UINT8> variant_reader_uint8;
typedef variant_reader<UINT16> variant_reader_uint16;
typedef variant_reader<UINT32> variant_reader_uint32;


template <typename LengthT, typename CharT, typename TraitsT, typename AllocT>
inline data_input_stream& operator>>( data_input_stream& is, const variant_string_reader<LengthT, CharT, TraitsT, AllocT>& reader )
{
	LengthT len = 0;
	if ( is >> len )
	{
		if ( reader.max_count > 0 && len > reader.max_count )
		{
			is.set_bad();
			return is;
		}
		is.read_string( len, reader.data );
	}
	return is;
}

template <typename LengthT, typename T, typename AllocT>
inline data_input_stream& operator>>( data_input_stream& is, const variant_buffer_reader<LengthT, T, AllocT>& reader )
{
	LengthT len = 0;
	if ( is >> len )
	{
		if ( reader.max_count > 0 && len > reader.max_count )
		{
			is.set_bad();
			return is;
		}
		is.read_buffer( len, reader.data );
	}
	return is;
}

template <typename LengthT, typename T, typename AllocT>
inline data_input_stream& operator>>( data_input_stream& is, const variant_vector_reader<LengthT, T, AllocT>& reader )
{
	LengthT len = 0;
	if ( is >> len )
	{
		if ( reader.max_count > 0 && len > reader.max_count )
		{
			is.set_bad();
			return is;
		}
		variant_reader_util::read_padded_vector( is, len, reader.items, reader.item_size, reader.block_size);
	}
	return is;
}



} }

#endif
