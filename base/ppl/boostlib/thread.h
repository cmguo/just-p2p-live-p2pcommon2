
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_BOOSTLIB_THREAD_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_BOOSTLIB_THREAD_H_

#include <ppl/config.h>

#include <ppl/boostlib/threads.h>

#include <boost/scoped_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>


namespace ppl { namespace boostlib {


/// 基于boost.thread实现的线程类
class thread : private boost::noncopyable
{
public:
	thread()
	{
	}
	~thread()
	{
		LIVE_ASSERT(false == is_alive());
	}

	/// 启动
	void start()
	{
		LIVE_ASSERT(false == is_started());
		m_thread.reset( new boost::thread( boost::bind( &thread::do_run, this ) ) );
	}

	/// 停止线程，如果在指定的时间内没有停止，则强制杀掉
	void stop(UINT milliseconds)
	{
		interrupt();
		ensure_stopped( milliseconds );
	}
	/// 停止线程，无限时等待其结束
	void stop()
	{
		interrupt();
		ensure_stopped();
	}

	bool join()
	{
		LIVE_ASSERT(m_thread);
		if ( !m_thread )
			return true;
		m_thread->join();
		return true;
	}
	bool join(UINT milliseconds)
	{
		LIVE_ASSERT(m_thread);
		if ( !m_thread )
			return true;
		return m_thread->timed_join(boost::posix_time::milliseconds(milliseconds));
	}
	bool interrupt()
	{
		if ( !m_thread )
			return false;
		do_interrupt();
		return true;
	}

	bool ensure_stopped(UINT milliseconds)
	{
		if ( !m_thread )
			return false;
		if ( false == this->join( milliseconds ) )
		{
			this->kill();
		}
		m_thread.reset();
		return true;
	}
	bool ensure_stopped()
	{
		if ( !m_thread )
			return false;
		if ( false == this->join() )
		{
			this->kill();
		}
		m_thread.reset();
		return true;
	}

	void kill()
	{
		if ( m_thread )
		{
			threads::kill(*m_thread, 321);
		}
	}

	bool is_started() const
	{
		return m_thread;
	}
	bool is_alive() const
	{
		if ( !m_thread )
			return false;
		return threads::kill( *m_thread, 321 );
	}

	static void sleep( long millis )
	{
		LIVE_ASSERT( millis >= 0 );
		static long const nanoseconds_per_millisecond = 1000L*1000L;
		long secs = millis / 1000;
		boost::xtime::xtime_nsec_t nsecs = ( millis % 1000 ) * nanoseconds_per_millisecond;
		boost::xtime xt;
		boost::xtime_get(&xt, boost::TIME_UTC);
		xt.sec += secs;
		xt.nsec += nsecs;
		boost::thread::sleep(xt);
	}

	static void yield()
	{
		boost::thread::yield();
	}


protected:
	virtual void do_run() = 0;
	virtual void do_interrupt() = 0;


private:
	boost::scoped_ptr<boost::thread> m_thread;
};


} }


#endif
