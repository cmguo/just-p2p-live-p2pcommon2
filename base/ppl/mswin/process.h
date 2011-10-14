#ifndef _LIVE_P2PCOMMON2_BASE_PPL_MSWIN_PROCESS_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_MAWIN_PROCESS_H_

#include <ppl/mswin/process_object.h>
#include <ppl/util/macro.h>
#include <io.h>
#include <fcntl.h>


namespace ppl { namespace mswin {


class process : private boost::noncopyable
{
public:
	process()
	{
	}
	~process()
	{
		this->close();
	}

	static DWORD current_process_id()
	{
		return ::GetCurrentProcessId();
	}
	static DWORD current_thread_id()
	{
		return ::GetCurrentThreadId();
	}

	bool join( DWORD milliseconds )
	{
		return m_object.join(milliseconds);
	}

	bool join()
	{
		return m_object.join();
	}

	void close()
	{
		if (is_open())
		{
			if ( is_alive() )
			{
				bool success = this->kill(123);
				LIVE_ASSERT( success );
			}
		}
		m_object.close();
	}

	/// »ñÈ¡¾ä±ú
	HANDLE native_handle() const { return m_object.native_handle(); }
	DWORD native_id() const { return m_object.native_id(); }

	process_object& native_object() { return m_object; }

	bool is_open() const
	{
		return m_object.is_open();
	}

	bool is_alive() const
	{
		return m_object.is_alive();
	}

	ULONG get_exit_code() const
	{
		return m_object.get_exit_code();
	}

	bool kill(DWORD exitCode)
	{
		return m_object.kill(exitCode);
	}

	bool start( const string& cmdline, STARTUPINFO& si, bool inheritHandles = false )
	{
		LIVE_ASSERT( false == is_open() );
		LIVE_ASSERT( false == is_alive() );
		this->close();
		string cmdlinestr = cmdline;
		PROCESS_INFORMATION pi = { 0 };
		if ( FALSE == ::CreateProcess( NULL, &cmdlinestr[0], NULL, NULL, inheritHandles ? TRUE : FALSE, NORMAL_PRIORITY_CLASS|CREATE_NO_WINDOW, NULL, NULL, &si, &pi) )
		{
			UTIL_ERROR("failed to create process " << cmdlinestr << " errcode=" << ::GetLastError());
			return false;
		}
		m_object.attach(pi.hProcess, pi.dwProcessId);
		CloseHandle(pi.hThread);
		return true;
	}

	bool start( const string& cmdline, int outhandle, int inhandle = -1, int errhandle = -1 )
	{
		return this->start(cmdline, get_pipe_handle(outhandle), get_pipe_handle(inhandle), get_pipe_handle(errhandle));
	}

	bool start(const string& cmdline, HANDLE outhandle = INVALID_HANDLE_VALUE, HANDLE inhandle = INVALID_HANDLE_VALUE, HANDLE errhandle = INVALID_HANDLE_VALUE)
	{
		STARTUPINFO si;
		FILL_ZERO( si );
		si.cb = sizeof( si );
		bool inheritHandles = false;
		if (INVALID_HANDLE_VALUE != outhandle)
		{
			si.dwFlags = STARTF_USESTDHANDLES;
			si.hStdOutput = outhandle;
			inheritHandles = true;
		}
		else
		{
			//si.hStdOutput = ::GetStdHandle( STD_OUTPUT_HANDLE );
		}
		if (INVALID_HANDLE_VALUE != inhandle)
		{
			si.dwFlags = STARTF_USESTDHANDLES;
			si.hStdInput = inhandle;
			inheritHandles = true;
		}
		else
		{
			//si.hStdInput = ::GetStdHandle( STD_INPUT_HANDLE );
		}
		if (INVALID_HANDLE_VALUE != errhandle)
		{
			si.dwFlags = STARTF_USESTDHANDLES;
			si.hStdError = errhandle;
			inheritHandles = true;
		}
		else
		{
			//si.hStdError = ::GetStdHandle( STD_ERROR_HANDLE );
		}
		return this->start( cmdline, si, inheritHandles );
	}

	static HANDLE get_pipe_handle( int fd )
	{
		if (-1 == fd)
			return INVALID_HANDLE_VALUE;
		HANDLE h = reinterpret_cast<HANDLE>( _get_osfhandle( fd ) );
		SetHandleInformation( h, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT );
		//::DuplicateHandle( )
		return h;
	}

protected:

private:
	process_object m_object;
};

class Process : private boost::noncopyable
{
public:
	static bool Create(STARTUPINFO& startupInfo, PROCESS_INFORMATION& processInfo, LPCTSTR appname, LPTSTR cmdline, DWORD flags)
	{
		BOOL res = ::CreateProcess(appname, cmdline, NULL, NULL, FALSE, flags, NULL, NULL, &startupInfo, &processInfo);
		return (FALSE != res);
	}
};

} }

#endif
