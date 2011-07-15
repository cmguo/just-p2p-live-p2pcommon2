
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_STD_CLASS_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_STD_CLASS_H_




namespace ppl {


template <typename T>
static T* construct(void* p)
{
	return new(p) T();
}

template <typename T>
static T* construct(void* p, const T& val)
{
	return new(p) T(val);
}

template <typename T>
static void destruct(T* obj)
{
	obj->~T();
}


}

#endif