
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_DATA_POOL_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_DATA_POOL_H_
#include <boost/noncopyable.hpp>


#ifdef _DEBUG
extern void* pool_malloc_impl(int size, const char* lpszFileName, int nLine);
extern void pool_free_impl(void* lpbuf, const char* lpszFileName, int nLine);
#else
extern void* pool_malloc_impl(int size);
extern void pool_free_impl(void* lpbuf);
#endif

#include <ppl/data/allocator_impl.h>
//#include <ppl/data/pool_allocator.h>
#include <ppl/data/pool_object_impl.h>

#include <ppl/data/buffer.h>
#include <string>


class pool_alloc : boost::noncopyable
{
public:
#ifdef _DEBUG
	static void* allocate(size_t size, const char* filename = NULL, int line = 0)
	{
		assert(size < 33 * 1024 * 1024);
		void* p = pool_malloc_impl(size, filename, line);
		assert(p);
		return p;
	}
	static void deallocate(void* p, size_t size = 0, const char* filename = NULL, int line = 0)
	{
		//assert(p != NULL);
		if (p)
		{
			pool_free_impl(p, filename, line);
		}
	}
#else
	static void* allocate(size_t size)
	{
		assert(size < 33 * 1024 * 1024);
		void* p = pool_malloc_impl(size);
		assert(p);
		return p;
	}
	static void deallocate(void* p, size_t size = 0)
	{
		//assert(p != NULL);
		if (p)
		{
			pool_free_impl(p);
		}
	}
#endif
};


template <typename T>
class pool_allocator
{
public:
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef T value_type;

    pool_allocator() { }
    pool_allocator(const pool_allocator<T>&) { }

    template<class _Other>
    pool_allocator(const pool_allocator<_Other>&) { }

    template<class _Other>
    pool_allocator & operator=(const pool_allocator<_Other>&) { return (*this);}  

    bool operator==(const pool_allocator &) const { return true;}  

    pointer address(reference x) const { return &x; }
    const_pointer address(const_reference x) const { return &x; }

    pointer allocate(size_type n, const void* p = 0)
    {
        return static_cast<pointer>(pool_alloc::allocate(sizeof(T) * n));
    }
    char* _Charalloc(size_type n)
    {
        return static_cast<char*>(pool_alloc::allocate(sizeof(T) * n));
    }
    void deallocate(void* p, size_type)
    {
        pool_alloc::deallocate(p);
    }
    void construct(pointer p, const T& val)
    {
        ppl::construct(p, val);
    }
    void destroy(pointer p)
    {
        ppl::destruct(p);
    }
    size_t max_size() const
    {
        size_t n = (size_t)(-1) / sizeof (T);
        return (0 < n ? n : 1);
    }

    template<typename _Other>
    struct rebind
    {	// convert an allocator<_Ty> to an allocator <_Other>
        typedef pool_allocator<_Other> other;
    };

};

typedef pool_object_impl<pool_alloc> pool_object;

class noncopyable_pool_object : public pool_object, private boost::noncopyable
{
};

typedef basic_buffer<char, pool_alloc> pool_char_buffer;
typedef basic_buffer<unsigned char, pool_alloc> pool_byte_buffer; 

typedef std::basic_string<char, std::char_traits<char>, pool_allocator<char> > pool_string;
typedef std::basic_string<unsigned char, std::char_traits<unsigned char>, pool_allocator<unsigned char> > pool_byte_string;

#endif