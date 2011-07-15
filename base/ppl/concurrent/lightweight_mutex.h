
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_CONCURRENT_LIGHT_WEIGHT_MUTEX_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_CONCURRENT_LIGHT_WEIGHT_MUTEX_H_

#include <ppl/config.h>
#include <ppl/concurrent/scoped_lock.h>
#include <boost/noncopyable.hpp>
#include <boost/config.hpp>


#if defined(_PPL_PLATFORM_MSWIN)

#include <ppl/mswin/windows.h>

namespace ppl { namespace concurrent { 


class lightweight_mutex : private boost::noncopyable
{
public:
	typedef ppl::concurrent::scoped_lock<lightweight_mutex> scoped_lock;

	lightweight_mutex()
	{
		::InitializeCriticalSection(&m_cs);
	}
	~lightweight_mutex()
	{
		::DeleteCriticalSection(&m_cs);
	}

	bool lock()
	{
		//__try
		{
			::EnterCriticalSection(&m_cs);
		}
		//__except (EXCEPTION_EXECUTE_HANDLER)
		//{
		//	return false;
		//}
		return true;
	}
	bool unlock()
	{
		::LeaveCriticalSection(&m_cs);
		return true;
	}

private:
	CRITICAL_SECTION m_cs;
};


} }

#elif defined(_PPL_PLATFORM_POSIX)

namespace ppl { namespace concurrent { 

class lightweight_mutex : private boost::noncopyable
{
public:
    typedef ppl::concurrent::scoped_lock<lightweight_mutex> scoped_lock;

	lightweight_mutex()
	{
		// HPUX 10.20 / DCE has a nonstandard pthread_mutex_init

#if defined(__hpux) && defined(_DECTHREADS_)
		pthread_mutex_init(&m_, pthread_mutexattr_default);
#else
		pthread_mutex_init(&m_, 0);
#endif
	}

	~lightweight_mutex()
	{
		pthread_mutex_destroy(&m_);
	}

	bool lock()
	{
		pthread_mutex_lock(&m_);
		return true;
	}

	bool unlock()
	{
		pthread_mutex_unlock(&m_);
		return true;
	}

private:
	pthread_mutex_t m_;
};

} }

#else

#error invalid platform for lightweight_mutex

#endif

#endif



