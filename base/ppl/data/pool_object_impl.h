#ifndef _LIVE_P2PCOMMON2_BASE_PPL_DATA_POOL_OBJECT_IMPL_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_DATA_POOL_OBJECT_IMPL_H_

#include <stddef.h>


template <typename AllocT>
class pool_object_impl
{
protected:
	pool_object_impl() { }
	~pool_object_impl() { }

public:
	static void* operator new(size_t size)
	{
		return AllocT::allocate(size);
	}
	static void operator delete(void* p)
	{
		AllocT::deallocate(p);
	}
	static void* operator new(size_t size, void* p)
	{
		return p;
	}
	static void operator delete(void* p, void*)
	{
	}

#if defined(_PPL_ENABLE_REDEFINE_NEW)
	static void* operator new(size_t size, int, const char*, int)
	{
		return AllocT::allocate(size);
	}
	static void operator delete(void* p, int, const char*, int)
	{
		AllocT::deallocate(p);
	}
#endif
};

#endif

