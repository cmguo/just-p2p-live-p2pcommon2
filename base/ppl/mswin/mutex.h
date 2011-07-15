#ifndef _LIVE_P2PCOMMON2_BASE_PPL_MSWIN_MUTEX_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_MAWIN_MUTEX_H_

#include <ppl/concurrent/scoped_lock.h>
#include <boost/noncopyable.hpp>

namespace ppl { namespace mswin {

class mutex : private boost::noncopyable
{
public:
	typedef ppl::concurrent::scoped_lock<mutex> scoped_lock;

	explicit mutex(LPCTSTR name = NULL)
	{
		m_handle = ::CreateMutex(NULL, FALSE, name);
	}
	~mutex()
	{
		close();
	}

	HANDLE native_handle() const
	{
		return m_handle;
	}

	bool is_open() const
	{
		return m_handle != NULL;
	}
	void close()
	{
		if (is_open())
		{
			::CloseHandle(m_handle);
			m_handle = NULL;
		}
	}

	bool acquire()
	{
		return WAIT_OBJECT_0 == ::WaitForSingleObject(m_handle, INFINITE);
	}
	bool release()
	{
		return FALSE != ::ReleaseMutex(m_handle);
	}

	bool lock()
	{
		return acquire();
	}
	bool unlock()
	{
		return release();
	}

private:
	HANDLE	m_handle;
};

} }

#endif
