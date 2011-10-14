
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_CONCURRENT_RUNNABLE_THREAD_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_CONCURRENT_RUNNABLE_THREAD_H_

#include <ppl/concurrent/thread.h>
#include <boost/function.hpp>


namespace ppl { namespace concurrent { 


class runnable_thread : public thread
{
public:
	typedef boost::function0<void> runner_function;
	typedef boost::function0<void> stopper_function;

	runnable_thread() { }
	~runnable_thread()
	{
		this->stop(500);
	}


	void start( runner_function runner, stopper_function stopper )
	{
		this->stop( 0 );
		m_runner = runner;
		m_stopper = stopper;
		LIVE_ASSERT( false == m_runner.empty() );
		LIVE_ASSERT( false == m_stopper.empty() );
		thread::start();
	}

protected:
	virtual void do_run()
	{
		LIVE_ASSERT( false == m_runner.empty() );
		m_runner();
	}
	virtual void do_interrupt()
	{
		LIVE_ASSERT( false == m_stopper.empty() );
		if ( false == m_stopper.empty() )
		{
			m_stopper();
		}
	}

private:
	runner_function m_runner;
	stopper_function m_stopper;
};

} }

#endif
