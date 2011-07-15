
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_DATA_FAKE_POOL_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_DATA_FAKE_POOL_H_


#include <ppl/data/allocator_impl.h>
#include <ppl/data/pool_object_impl.h>
#include <ppl/data/alloc.h>
#include <ppl/data/buffer.h>
#include <string>

typedef malloc_alloc pool_alloc;

template <typename T>
class pool_allocator : public std::allocator<T>
{ };

typedef pool_object_impl<malloc_alloc> pool_object;


typedef char_buffer pool_char_buffer;
typedef byte_buffer pool_byte_buffer; 

typedef std::basic_string<char, std::char_traits<char>, pool_allocator<char> > pool_string;
typedef std::basic_string<BYTE, std::char_traits<BYTE>, pool_allocator<BYTE> > pool_byte_string;

#endif