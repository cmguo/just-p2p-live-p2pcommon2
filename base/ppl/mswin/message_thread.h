
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_MSWIN_MESSAGE_THREAD_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_MAWIN_MESSAGE_THREAD_H_

#include <ppl/mswin/thread.h>
#include <ppl/mswin/windows.h>

#include <boost/function.hpp>
#include <boost/bind.hpp>


namespace ppl { namespace mswin { 


/// 处理windows消息的线程类
class message_thread : public ppl::mswin::thread
{
public:
	typedef boost::function0<bool> initialize_function_type;
	typedef boost::function0<void> uninitialize_function_type;

	message_thread()
	{
	}
	~message_thread()
	{
		this->stop(10);
	}

	void set_initializer(initialize_function_type initOP)
	{
		m_init = initOP;
	}

	void set_uninitializer(uninitialize_function_type uninitOP)
	{
		m_uninit = uninitOP;
	}

protected:
	virtual void do_run()
	{
		if (false == do_initialize())
		{
			::OutputDebugString(_T("failed to initialize message thread\n"));
			assert(false);
			return;
		}
		MSG msg;
		while ( ::GetMessage( &msg, NULL, 0, 0 ) )
		{
			::TranslateMessage( &msg );
			::DispatchMessage( &msg );
		}
		do_uninitialize();
	}

	virtual void do_interrupt()
	{
		::PostThreadMessage( native_id(), WM_QUIT, 0, 0 );
	}

	virtual bool do_initialize()
	{
		if (m_init.empty())
			return true;
		return m_init();
	}

	virtual void do_uninitialize()
	{
		if (m_uninit.empty())
			return;
		m_uninit();
	}

private:
	initialize_function_type m_init;
	uninitialize_function_type m_uninit;
};


} }


#endif
