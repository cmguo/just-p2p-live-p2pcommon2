#ifndef _LIVE_P2PCOMMON2_BASE_PPL_MSWIN_PROCESS_OBJECT_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_MAWIN_PROCESS_OBJECT_H_

#include <ppl/mswin/windows.h>
#include <ppl/diag/trace.h>
#include <boost/noncopyable.hpp>

#include <psapi.h>
#pragma comment(lib, "psapi.lib")


namespace ppl { namespace mswin {


class process_object : private boost::noncopyable
{
public:
	process_object() : m_handle(NULL), m_id(static_cast<DWORD>( -1 ))
	{
	}
	explicit process_object(DWORD id, DWORD access = PROCESS_ALL_ACCESS) : m_handle(::GetCurrentProcess()), m_id(::GetCurrentProcessId())
	{
		open(id, access);
	}
	~process_object()
	{
		close();
	}

	void attach(HANDLE handle, DWORD id)
	{
		this->close();
		m_handle = handle;
		m_id = id;
	}

	void close()
	{
		if (is_open())
		{
			::CloseHandle(m_handle);
			m_handle = NULL;
			m_id = static_cast<DWORD>( -1 );
		}
	}
	bool is_open() const
	{
		return m_handle != NULL;
	}

	bool kill(ULONG exitCode)
	{
		LIVE_ASSERT(is_open());
		return FALSE != ::TerminateProcess( m_handle, exitCode );
	}

	bool open(DWORD id, DWORD access = PROCESS_ALL_ACCESS)
	{
		close();
		m_handle = ::OpenProcess(access, FALSE, id);
		if (is_open())
		{
			m_id = id;
			//TRACE( "open process %d 0x%p\n", m_id, m_handle );
			return true;
		}
		APP_ERROR("OpenProcess " << id << " failed, error code=" << ::GetLastError());
		return false;
	}

	DWORD get_exit_code() const
	{
		if (false == is_open())
			return 0;
		DWORD exitCode = 0;
		BOOL success = ::GetExitCodeProcess(m_handle, &exitCode);
		LIVE_ASSERT(success);
		return exitCode;
	}

	bool is_alive() const
	{
		return get_exit_code() == STILL_ACTIVE;
	}

	bool get_times( FILETIME* creationTime, FILETIME* exitTime, FILETIME* kernelTime, FILETIME* userTime )
	{
		return FALSE != ::GetProcessTimes( m_handle, creationTime, exitTime, kernelTime, userTime );
	}

	bool join(DWORD milliseconds)
	{
		LIVE_ASSERT(is_open());
		LIVE_ASSERT(is_alive());
		DWORD waitResult = ::WaitForSingleObject(m_handle, milliseconds);
		if (WAIT_OBJECT_0 != waitResult && WAIT_TIMEOUT != waitResult)
		{
			TRACEOUT("process_object::join failed, handle=0x%p, id=%u, error=%d\n", m_handle, m_id, ::GetLastError());
			LIVE_ASSERT(false);
		}
		return WAIT_OBJECT_0 == waitResult;
	}

	bool join()
	{
		return join(INFINITE);
	}

#if !defined(_WIN32_WCE)
	tstring get_file_path() const
	{
		TCHAR path[MAX_PATH * 4 + 4] = { 0 };
		DWORD len = ::GetModuleFileNameEx(m_handle, NULL, path, MAX_PATH * 4);
		int errcode = ::GetLastError();
		//LIVE_ASSERT(len > 0);
		return tstring(path, len);
	}

	tstring get_basename() const
	{
		TCHAR path[MAX_PATH * 4 + 4] = { 0 };
		DWORD len = ::GetModuleBaseName(m_handle, NULL, path, MAX_PATH * 4);
		LIVE_ASSERT(len > 0);
		return tstring(path, len);
	}
#endif

	/// »ñÈ¡¾ä±ú
	HANDLE native_handle() const { return m_handle; }
	DWORD native_id() const { return m_id; }

private:
	HANDLE m_handle;
	DWORD m_id;
};


} }

#endif

