#ifndef _LIVE_P2PCOMMON2_BASE_PPL_CONCURRENT_SUBPROCESS_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_CONCURRENT_SUBPROCESS_H_

#include <ppl/config.h>
#include <ppl/os/process.h>
#include <ppl/util/macro.h>
#include <ppl/util/log.h>
#include <ppl/io/stdfile.h>
#include <ppl/concurrent/thread.h>
#include <ppl/concurrent/runnable_thread.h>
#include <ppl/mswin/event.h>

#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/shared_ptr.hpp>

#include <list>
#include <string>
using std::string;




#if defined(_PPL_PLATFORM_MSWIN)

#include <io.h>
#include <fcntl.h>

#include <ppl/mswin/process.h>

/*
class win32_pipe : private boost::noncopyable
{
public:
	win32_pipe() : m_pipeRead( INVALID_HANDLE_VALUE ), m_pipeWrite( INVALID_HANDLE_VALUE )
	{

	}

private:
	HANDLE m_pipeRead;
	HANDLE m_pipeWrite;
};
*/
typedef ppl::mswin	 os_process;


namespace pplivemis_detail { 

inline bool create_pipe_api(int fds[2])
{
	//HANDLE pipeRead = INVALID_HANDLE_VALUE;
	//HANDLE pipeWrite = INVALID_HANDLE_VALUE;
	//SECURITY_ATTRIBUTES sa;
	//sa.nLength = sizeof(sa);
	//sa.bInheritHandle = TRUE;
	//sa.lpSecurityDescriptor = NULL;
	//if ( FALSE == ::CreatePipe( &pipeRead, &pipeWrite, &sa, 128 ) )
	//	return false;
	//fds[0] = _open_osfhandle( (intptr_t)pipeRead, _O_TEXT );
	//fds[1] = _open_osfhandle( (intptr_t)pipeWrite, _O_TEXT );
	//if ( fds[0] == -1 && fds[1] == -1 )
	//{
	//	::CloseHandle( pipeRead );
	//	::CloseHandle( pipeWrite );
	//	return false;
	//}
	//return true;
	return _pipe( fds, 256, _O_TEXT ) == 0;
}

}

#elif defined(_PPL_PLATFORM_LINUX)

namespace detail { 

inline bool create_pipe_api(int fds[2])
{
	return pipe(fds) == 0;
}

}


class os_process : private boost::noncopyable
{
public:
	os_process() : m_pid( 0 )
	{
	}
	~os_process()
	{
		this->close();
	}

	bool is_open() const
	{
		return m_pid > 0;
	}

	bool is_alive() const
	{
		if ( false == is_open() )
			return false;
		ULONG exitCode = 0;
		::GetExitCodeProcess( m_info.hProcess, &exitCode );
		return STILL_ACTIVE == exitCode;
	}

	bool kill(ULONG exitCode = 123)
	{
		return !!::TerminateProcess( m_info.hProcess, exitCode );
	}

	bool start( const string& cmdline, int outhandle = -1, int inhandle = -1, int errhandle = -1)
	{
		this->close();
		pid_t pid = fork ();
		if ( pid < (pid_t) 0 )
		{
			// The fork failed. 
			fprintf (stderr, "Fork failed.\n");
			return false;
		}
		if ( pid > (pid_t)0 )
		{
			// This is the parent process.Close other end first.
			return true;
		}
		// This is the child process.Close other end first.
		set_pipe_handle( outhandle, 1 );
		set_pipe_handle( inhandle, 0 );
		set_pipe_handle( errhandle, 2 );
		execl( cmdline.c_str(), cmdline.c_str(), NULL );
		return true;
	}

	bool join( ULONG milliseconds )
	{
		LIVE_ASSERT( is_open() && m_info.hProcess != NULL );
		return ::WaitForSingleObject( m_info.hProcess, milliseconds ) == WAIT_OBJECT_0;
	}

	bool join()
	{
		return this->join( INFINITE );
	}

	void close()
	{
		if ( m_opened )
		{
			if ( is_alive() )
			{
				bool success = this->kill();
				LIVE_ASSERT( success );
			}
			LIVE_ASSERT( false == this->is_alive() );
			LIVE_ASSERT( m_info.hProcess != NULL && m_info.hThread != NULL );
			LIVE_ASSERT( m_info.dwProcessId != 0 && m_info.dwThreadId != 0 );
			::CloseHandle( m_info.hProcess );
			::CloseHandle( m_info.hThread );
			FILL_ZERO( m_info );
			m_opened = false;
		}
	}

protected:
	static void set_pipe_handle( int fd, int dest )
	{
		if ( fd != -1 )
		{
			dup2( fd, dest );
		}
	}

private:
	pid_t m_pid;
};



#else

#error ? invalid platform for os_process

#endif





class os_pipe : private boost::noncopyable
{
public:
	os_pipe()
	{
		m_fds[0] = -1;
		m_fds[1] = -1;
	}
	~os_pipe()
	{
		this->close();
	}

	bool open()
	{
		this->close();
		if ( false == pplivemis_detail::create_pipe_api( m_fds ) )
			return false;
		return true;
	}

	void close()
	{
		this->close_read();
		this->close_write();
	}

	int get_read() const
	{
		return m_fds[0];
	}
	int get_write() const
	{
		return m_fds[1];
	}

	void close_read()
	{
		if ( m_fds[0] != -1 )
		{
			_close( m_fds[0] );
			m_fds[0] = -1;
		}
	}

	void close_write()
	{
		if ( m_fds[1] != -1 )
		{
			_close( m_fds[1] );
			m_fds[1] = -1;
		}
	}

	void reset_read()
	{
		m_fds[0] = -1;
	}
	void reset_write()
	{
		m_fds[1] = -1;
	}

private:
	int m_fds[2];
};

#if defined(_PPL_PLATFORM_MSWIN)

class os_pipe_reader : private boost::noncopyable
{
public:
	os_pipe_reader() : m_in( INVALID_HANDLE_VALUE ), m_good( false )
	{
	}

	bool valid() const
	{
		return m_in != INVALID_HANDLE_VALUE;
	}
	bool good() const
	{
		return m_good;
	}
	void attach( HANDLE in )
	{
		m_in = in;
		m_good = true;
	}
	void attach_descriptor( int in )
	{
		m_in = reinterpret_cast<HANDLE>( _get_osfhandle( in ) );
		m_good = true;
	}
	void detach()
	{
		m_in = INVALID_HANDLE_VALUE;
		m_good = false;
	}
	bool read_line( string& s )
	{
		this->try_read();
		if ( m_lines.empty() )
			return false;
		s = m_lines.front();
		m_lines.pop_front();
		return true;
	}

private:
	bool try_read()
	{
		DWORD totalBytes = 0;
		if ( FALSE == ::PeekNamedPipe( m_in, NULL, 0, NULL, &totalBytes, NULL ) )
		{
			if ( false == m_extra.empty() )
			{
				m_lines.push_back( m_extra );
				m_extra.clear();
			}
			m_good = false;
			return false;
		}
		if ( 0 == totalBytes )
			return true;
		DWORD bytes = 0;
		size_t oldSize = m_extra.size();
		m_extra.resize( oldSize + totalBytes );
		if ( FALSE == ::ReadFile( m_in, &m_extra[oldSize], totalBytes, &bytes, NULL ) )
		{
			if ( false == m_extra.empty() )
			{
				m_lines.push_back( m_extra );
				m_extra.clear();
			}
			m_good = false;
			return false;
		}
		m_extra.resize( oldSize + bytes );
		std::list<string> lines;
		boost::split( lines, m_extra, boost::is_any_of("\r\n") );
		m_lines.splice( m_lines.end(), lines );
		if ( false == m_lines.empty() )
		{
			m_extra = m_lines.back();
			m_lines.pop_back();
		}
		return true;
	}

private:
	HANDLE m_in;
	std::list<string> m_lines;
	string m_extra;
	bool m_good;
};

#elif defined(_PPL_PLATFORM_LINUX)

#endif


#if 0
inline string run_process( const string& cmdlinestr )
{
	string cmdline = cmdlinestr;

	SECURITY_ATTRIBUTES secattr; 
	ZeroMemory(&secattr,sizeof(secattr));
	secattr.nLength = sizeof(secattr);
	secattr.bInheritHandle = TRUE;


	HANDLE rPipe, wPipe;
	HANDLE rPipe2, wPipe2;

	//Create pipes to write and read data

	HANDLE ph = ::GetCurrentProcess();
	CreatePipe(&rPipe,&wPipe,&secattr,0);
	//DuplicateHandle( ph, rPipe, ph, &rPipe2, 0, TRUE, DUPLICATE_SAME_ACCESS );
	//DuplicateHandle( ph, wPipe, ph, &wPipe2, 0, TRUE, DUPLICATE_SAME_ACCESS );
	//

	STARTUPINFO sInfo; 
	ZeroMemory(&sInfo,sizeof(sInfo));
	PROCESS_INFORMATION pInfo; 
	ZeroMemory(&pInfo,sizeof(pInfo));
	sInfo.cb=sizeof(sInfo);
	sInfo.dwFlags=STARTF_USESTDHANDLES;
	sInfo.hStdInput=NULL; 
	sInfo.hStdOutput=wPipe; 
	sInfo.hStdError=NULL;

	//Create the process here.

	CreateProcess(NULL, &cmdline[0],0,0,TRUE,
		NORMAL_PRIORITY_CLASS|CREATE_NO_WINDOW,0,0,&sInfo,&pInfo);
	CloseHandle(wPipe);

	//now read the output pipe here.

	int pfd = _open_osfhandle( (intptr_t)rPipe, _O_TEXT );
	char buf[1024];
	ULONG len; 
	string s;
	string s2;
	BOOL res;
	do
	{
		//res=::ReadFile(rPipe,buf,1024,&reDword,0);
		len = ::_read( pfd, buf, 16 );
		if ( len <= 0 )
		{
			break;
		}
		buf[len] = '\0';
		s2 = string( buf, len );
		fprintf(stdout, "%s", buf);
		fflush(stdout);
		s += s2;
	}while(1);

	return s;
}
#endif


/// 启动子进程
class subprocess : private boost::noncopyable
{
public:
	typedef boost::function<void (subprocess*, const string&)> status_handler_type;
	typedef boost::function<void (subprocess*, UINT)> finish_handler_type;

	class pipe_param : private boost::noncopyable
	{
	public:
		os_pipe stdout_pipe;
		os_pipe stdin_pipe;
		os_pipe_reader in;
		//StdFileReader in;
		//StdFileWriter out;
	};
	typedef boost::shared_ptr<pipe_param> pipe_param_ptr;

	subprocess()
	{
	}
	~subprocess()
	{
		this->stop();
	}

	void stop()
	{
		m_process.close();
		m_thread.stop(100);
		if ( m_thread.is_alive() )
		{
			TRACE("subprocess::stop %d\n", m_thread.get_id());
			LIVE_ASSERT(false);
		}
		if ( m_param )
		{
			m_param->in.detach();
			m_param->stdout_pipe.close();
			m_param->stdin_pipe.close();
			m_param.reset();
		}
	}

	bool start( const string& cmdline, status_handler_type statusHandler, finish_handler_type finishHandler )
	{
		m_statusHandler = statusHandler;
		m_finishHandler = finishHandler;
		pipe_param_ptr param( new pipe_param );
		m_param = param;
		if ( false == param->stdout_pipe.open() || false == param->stdin_pipe.open() )
			return false;
		//if ( false == param->in.OpenDescriptor( param->stdout_pipe.get_read() ) )
		//	return false;
		//param->stdout_pipe.reset_read();
		//if ( false == param->out.OpenDescriptor( param->stdin_pipe.get_write() ) )
		//	return false;
		//param->stdin_pipe.reset_write();

		param->in.attach_descriptor( param->stdout_pipe.get_read() );

		if ( false == m_process.start( cmdline, param->stdout_pipe.get_write(), param->stdin_pipe.get_read(), -1 ) )
			return false;
		param->stdout_pipe.close_write();
		param->stdin_pipe.close_read();

		m_thread.start( 
			boost::bind( &subprocess::run_thread, this, cmdline, param ), 
			boost::bind( &subprocess::request_stop_thread, this ) );
		return true;
	}

protected:
	void run_thread( string cmdline, pipe_param_ptr param )
	{
		//if ( false == param->stdout_pipe.open() )
		//	return;
		//if ( false == param->in.OpenDescriptor( param->stdout_pipe.get_read() ) )
		//	return;
		//param->stdout_pipe.reset_read();

		//if ( false == m_process.start( cmdline, param->stdout_pipe.get_write(), param->stdin_pipe.get_read(), -1 ) )
		//	return;
		//param->stdout_pipe.close_write();

		//if ( false == m_process.start( cmdline, param->stdout_pipe.get_write(), param->stdin_pipe.get_read(), -1 ) )
		//	return;
		//param->stdout_pipe.close_write();
		//param->stdin_pipe.close_read();

		bool canceled = false;
		string line;
		for ( ;; )
		{
			DWORD waitResult = m_event.wait( 100 );
			if ( WAIT_OBJECT_0 == waitResult )
			{
				canceled = true;
				break;
			}
			while ( param->in.read_line(line) && false == line.empty() )
			{
				//m_handler( this, string(buf, len) );
				m_statusHandler( this, line );
			}
			if ( false == param->in.good() )
				break;
		}
		m_finishHandler( this, canceled ? -1122 : m_process.get_exit_code() );

	}
	void request_stop_thread()
	{
		m_event.signal();
		//::CloseHandle( (HANDLE)_get_osfhandle( _fileno( m_param->in.GetHandle() ) ) );
		// 想子进程写入
		//m_param->out.WriteByte( '\x03' );
		//m_param->out.WriteByte( '\x03' );
		//m_pipe.close();
		//m_in.Close();
	}

private:
	ppl::concurrent::runnable_thread m_thread;
	os_process m_process;
	status_handler_type m_statusHandler;
	finish_handler_type m_finishHandler;
	pipe_param_ptr m_param;
	auto_reset_event m_event;
};


#endif