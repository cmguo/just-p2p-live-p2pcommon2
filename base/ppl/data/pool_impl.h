
#include <ppl/data/memory_pool.h>

ppl::data::memory_pool s_memory_pool;

#ifdef _DEBUG
void* pool_malloc_impl(int size, const char* lpszFileName, int nLine)
{
	return s_memory_pool.allocate(size, lpszFileName, nLine);
}
void pool_free_impl(void* lpbuf, const char* lpszFileName, int nLine)
{
	s_memory_pool.deallocate(lpbuf, lpszFileName, nLine);
}
#else
void* pool_malloc_impl(int size)
{
	return s_memory_pool.allocate(size);
}
void pool_free_impl(void* lpbuf)
{
	s_memory_pool.deallocate(lpbuf);
}
#endif

