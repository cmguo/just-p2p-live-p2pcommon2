
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_MSWIN_THREAD_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_MAWIN_THREAD_H_

//#include <ppl/mswin/event.h>
#include <ppl/util/log.h>
#include <ppl/diag/trace.h>

#include <boost/noncopyable.hpp>
#include <process.h>


namespace ppl { namespace mswin { 


/// win32平台的线程基类
class thread_base : private boost::noncopyable
{
public:
	thread_base() : m_handle(NULL), m_id(0)
	{
	}
	virtual ~thread_base()
	{
		assert(!is_alive());
		close();
	}

	HANDLE native_handle() const { return m_handle; }
	DWORD native_id() const { return m_id; }

	bool is_open() const { return m_handle != NULL; }

	bool is_alive() const { return is_open() && get_exit_code() == STILL_ACTIVE; }

//	struct ThreadParam
//	{
//		Thread* Target;
//		ManualResetEvent Notifier;
//	};

	/// 启动线程
	bool start()
	{
		assert(!this->is_alive());
		//assert(!this->is_started());
		stop();
		close();
		if (!create_thread())
			return false;
		//m_startEvent.Wait();
		return true;
	}
	/// 停止线程
	void stop(DWORD milliseconds = INFINITE)
	{
		interrupt();
		ensure_stopped( milliseconds );
	}

	DWORD get_exit_code() const
	{
		DWORD code = 0;
		BOOL res = ::GetExitCodeThread(m_handle, &code);
		assert(res);

		TRACE("thread_base::get_exit_code 0x%p %d %d\n", m_handle, res, code);
		return code;
	}

	bool join(DWORD milliseconds)
	{
		if ( false == is_alive() )
			return true;
		DWORD waitResult = ::WaitForSingleObject(m_handle, milliseconds);
		return ( WAIT_OBJECT_0 == waitResult );
	}

	bool join()
	{
		return join(INFINITE);
	}

	bool ensure_stopped(UINT milliseconds)
	{
		if ( false == is_alive() )
		{
			return false;
		}

		if ( false == this->join( milliseconds ) )
		{
			UTIL_ERROR("thread_base::do_stop failed to join, then kill " << make_tuple(milliseconds, ::GetLastError()));
			this->kill();
		}

		assert(false == is_alive());
		return true;
	}
	bool ensure_stopped()
	{
		return ensure_stopped( INFINITE );
	}

	void close()
	{
		if (!is_open())
			return;
		if (!::CloseHandle(m_handle))
		{
			UTIL_ERROR("thread_base::close CloseHandle failed " << make_tuple(m_handle, ::GetLastError()));
			assert(false);
		}
		m_handle = NULL;
		m_id = 0;
	}

	bool kill()
	{
		if ( false == is_alive() )
			return false;
		BOOL success = ::TerminateThread(m_handle, 110);
		//UTIL_DEBUG("thread_base::kill 0x%p %lu %d\n", m_handle, m_id, success);
		assert(success);
		return ( FALSE != success );
	}

	bool interrupt()
	{
		if ( false == is_alive() )
			return false;
		do_interrupt();
		return true;
	}

	static void sleep( long millis )
	{
		::Sleep( millis );
	}

	static void yield()
	{
		::Sleep( 0 );
	}


protected:
	void attach(HANDLE handle, DWORD id)
	{
		assert(!is_open());
		m_handle = handle;
		m_id = id;
	}

	void safe_run()
	{
		__try
		{
			do_run();
		}
		__except (my_except_filter(GetExceptionInformation()))
		{
		}
	}
	LONG my_except_filter(PEXCEPTION_POINTERS excepInfo)
	{
		__try
		{
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			::OutputDebugString(TEXT("error occured in thread\n"));
		}
		return EXCEPTION_CONTINUE_SEARCH;
	}

	virtual bool create_thread() = 0;

protected:
	static void run_thread(void* param)
	{
		if (param == NULL)
		{
			assert(false);
			return;
		}
	//	ThreadParam* tp = static_cast<ThreadParam*>(param);
	//	thread_base* target = tp->Target;
		thread_base* target = static_cast<thread_base*>(param);
		//target->m_startEvent.Signal();
		//target->m_startEvent.close();
		target->do_run();
	}
	virtual void do_run() = 0;
	virtual void do_interrupt() = 0;

private:
	/// 线程句柄
	HANDLE m_handle;
	/// 线程ID
	unsigned long m_id;

	/// 用于启动线程的事件对象
	//manual_reset_event m_startEvent;
};


#if defined(_AFX)

/// mfc程序中使用的线程类(使用AfxBeginThread创建线程)
class thread : public thread_base
{
public:
	thread()
	{

	}
	virtual ~thread()
	{
	}

protected:
	virtual bool create_thread()
	{
		unsigned int id = 0;
		CWinThread* threadobj = AfxBeginThread(&thread::thread_entry_point, this);
		if (threadobj == NULL)
		{
			UTIL_ERROR("AfxThread::create_thread AfxBeginThread failed " << ::GetLastError());
			assert(false);
			return false;
		}
		assert(threadobj->m_hThread != NULL && threadobj->m_nThreadID != 0);
		HANDLE hThread = NULL;
		BOOL res = ::DuplicateHandle( GetCurrentProcess(), threadobj->m_hThread, GetCurrentProcess(), &hThread, THREAD_ALL_ACCESS, FALSE, DUPLICATE_SAME_ACCESS );
		assert( res );
		attach(hThread, threadobj->m_nThreadID);
		return true;
	}
	static unsigned int __cdecl thread_entry_point(void* param)
	{
		thread_base::run_thread(param);
		return 0;
	}

};

#elif !defined(_WIN32_WCE)

/// 一般c++程序中使用的线程类(使用_beginthreadex创建线程)
class thread : public thread_base
{
protected:
	virtual bool create_thread()
	{
		unsigned int id = 0;
		HANDLE hThread = reinterpret_cast<HANDLE>(_beginthreadex(NULL, 0, &thread::thread_entry_point, this, 0, &id));
		if (hThread == NULL)
		{
			UTIL_ERROR("Win32Thread::create_thread _beginthreadex failed " << errno);
			assert(false);
			return false;
		}
		assert(id != 0);
		attach(hThread, id);
		return true;
	}
	static unsigned int __stdcall thread_entry_point(void* param)
	{
		thread_base::run_thread(param);
		return 0;
	}
};

#else

/// windows ce下使用的线程类(使用CreateThread API创建线程)
class thread : public thread_base
{
protected:
	virtual bool create_thread()
	{
		unsigned long id = 0;
		HANDLE hThread = ::CreateThread(NULL, 0, &WinCEThread::thread_entry_point, this, 0, &id);
		if (hThread == NULL)
		{
			UTIL_ERROR("thread_base::create_thread _beginthreadex failed");
			assert(false);
			return false;
		}
		assert(id != 0);
		attach(hThread, id);
		return true;
	}
	static unsigned long __stdcall thread_entry_point(void* param)
	{
		thread_base::run_thread(param);
		return 0;
	}
};

#endif




} }

#endif


