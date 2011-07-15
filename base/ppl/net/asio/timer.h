
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_NET_ASIO_TIMER_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_NET_ASIO_TIMER_H_

#include <ppl/config.h>

#include <ppl/net/asio/io_service_provider.h>
#include <ppl/net/asio/detail/timer_impl.h>


#include <ppl/data/int.h>
#include <ppl/util/timerfwd.h>

//using boost::asio::deadline_timer;
//using boost::posix_time::milliseconds;

using boost::system::error_code;

#include <boost/noncopyable.hpp>
#include <boost/function.hpp>

#include <boost/asio/placeholders.hpp>





class timer : public timer_base
{
public:
	explicit timer(boost::asio::io_service& inIOSerivce)
	{
		m_impl.reset( new timer_impl(inIOSerivce, timer_impl::callback_type(boost::bind(&timer::on_timer, this))) );
	}
	virtual ~timer()
	{
		m_impl->detach();
	}


	/// 启动定时器
	bool start(UINT elapse)
	{
		//assert( false == this->IsStarted() );
		this->stop();
		return do_start(elapse);
	}

	/// 停止或取消定时器
	void stop()
	{
		this->do_stop();
	}

	/// 是否已启动
	virtual bool is_started() const = 0;

	//void SetListener( TimerListener* listener )
	//{
	//	if (0 == listener)
	//	{
	//		m_callback = timer_callback_type(&timer_base::trivial_handler);
	//		return;
	//	}
	//	this->set_callback(boost::bind(&TimerListener::OnTimerElapsed, listener, this));
	//}

protected:
	/// 启动定时器
	virtual bool do_start(UINT elapse) = 0;

	/// 停止或取消定时器
	virtual void do_stop() = 0;

	virtual void on_timer() = 0;

protected:
	boost::shared_ptr<timer_impl> m_impl;
};

class once_timer : public timer
{
public:
	explicit once_timer(boost::asio::io_service& inIOSerivce = io_service_provider::default_service())
		: timer(inIOSerivce)
	{
	}
	virtual ~once_timer()
	{
		this->stop();
		assert( false == this->is_started() );
	}

	/// 是否已启动
	virtual bool is_started() const
	{
		return m_impl->is_started();
	}

protected:
	/// 启动定时器
	virtual bool do_start(UINT elapse)
	{
		//	TRACE("OnceTimer::Start %p %d\n", this, elapse);
		this->m_impl->start_from_now(elapse);
		return true;
	}

	/// 停止或取消定时器
	virtual void do_stop()
	{
		//	TRACE("OnceTimer::DoStop %p %d\n", this, this->m_Started);
		this->m_impl->stop();
	}

	virtual void on_timer()
	{
		//	TRACE("Timer::OnTimer %p \n", this);
		this->m_callback();
	}

private:
};

class periodic_timer : public timer
{
public:
	explicit periodic_timer(boost::asio::io_service& inIOSerivce = io_service_provider::default_service())
		: timer(inIOSerivce),m_interval( 0 ),m_times( 0 ),m_started(false)
	{
	}
	virtual ~periodic_timer()
	{
		this->stop();
		assert( false == this->is_started() );
	}

	/// 是否已启动
	virtual bool is_started() const
	{
		return m_started;
	}

	UINT get_interval() const { return m_interval; }

	/// 获取定时器事件触发的次数
	UINT get_times() const { return m_times; }

	/// 获取定时器的流逝时间
	UINT64 elapsed_time() const { return static_cast<UINT64>(m_times) * m_interval; }

protected:
	/// 启动定时器
	virtual bool do_start(UINT elapse)
	{
		assert(elapse >= 50 );
		//	TRACE("PeriodicTimer::Start %p %d\n", this, elapse);
		//	m_starttime = my_timer_type::traits_type::now();
		m_firetime = my_timer_type::traits_type::now();
		m_interval = elapse;
		m_times = 0;
		this->do_start_once();
		m_started = true;
		return true;
	}

	/// 停止或取消定时器
	virtual void do_stop()
	{
		m_started = false;
		this->m_impl->stop();
	}

	virtual void on_timer()
	{
		++m_times;
		//	TRACE("Timer::OnTimer %p \n", this);
		assert(m_started);
		if ( m_started )
		{
			this->do_start_once();
			this->m_callback();
		}
	}

	void do_start_once()
	{
		m_firetime = my_timer_type::traits_type::add(m_firetime, my_time_duration_type(m_interval));
		this->m_impl->start_at(m_firetime);
	}

protected:
//	my_timer_type::time_type m_starttime;
	my_timer_type::time_type m_firetime;
	UINT m_interval;
	UINT m_times;
	bool m_started;
};



#endif
