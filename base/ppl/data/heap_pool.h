
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_DATA_HEAP_POOL_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_DATA_HEAP_POOL_H_

#include <ppl/concurrent/lightweight_mutex.h>
#include <ppl/diag/trace.h>
#include <boost/noncopyable.hpp>
#include <utility>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>



class heap_pool : private boost::noncopyable
{
	typedef ppl::concurrent::lightweight_mutex::scoped_lock AutoLightMutex;

public:
	heap_pool()
	{
		this->heap_ = ::HeapCreate(0, 0, 0);
		TRACE("mempool MyHeap is %p\n", heap_);
	}
	~heap_pool()
	{
		::HeapDestroy(this->heap_);
		this->heap_ = NULL;
	}

	void* allocate(int size, const char* filename = NULL, int line = 0)
	{
		AutoLightMutex lock(lock_);
		void* p = ::HeapAlloc(this->heap_, 0, size);
		return p;
	}

	void deallocate(void * lpbuf, const char* filename = NULL, int line = 0)
	{
		AutoLightMutex lock(lock_);
		BOOL res = ::HeapFree(this->heap_, 0, lpbuf);
#ifdef _DEBUG
		if (TRUE != res)
		{
			TRACE("mempool pool_free is failed %p %p %s %d\n", res, lpbuf, filename, line);
		}
#endif
		assert(res);
	}

	void clear()
	{
	}

private:
	ppl::concurrent::lightweight_mutex lock_;
	HANDLE heap_;
};


#endif