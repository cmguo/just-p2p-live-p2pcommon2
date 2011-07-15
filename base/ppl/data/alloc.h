
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_DATA_ALLOC_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_DATA_ALLOC_H_

#include <utility>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>


/// 使用malloc/free的内存分配器
class malloc_alloc
{
public:
#ifdef _DEBUG
	static void* allocate(size_t size, const char* filename = NULL, int line = 0)
#else
	static void* allocate(size_t size)
#endif
	{
		assert(size < 33 * 1024 * 1024);
		void* p = malloc(size);
		assert(p);
		return p;
	}

#ifdef _DEBUG
	static void deallocate(void* p, size_t size = 0, const char* filename = NULL, int line = 0)
#else
	static void deallocate(void* p, size_t size = 0)
#endif
	{
		//assert(p != NULL);
		if (p)
		{
			free(p);
		}
	}
};

#endif