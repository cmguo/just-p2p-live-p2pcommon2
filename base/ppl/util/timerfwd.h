
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_UTIL_TIMERFWD_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_UTIL_TIMERFWD_H_

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

#include <assert.h>



class timer_base : private boost::noncopyable
{
public:
	typedef boost::function0<void> timer_callback_type;

	timer_base()
	{
		m_callback = timer_callback_type(&timer_base::trivial_handler);
	}
	virtual ~timer_base() { }

	void set_callback(timer_callback_type listener)
	{
		m_callback = listener;
	}

	void set_listener(timer_callback_type listener)
	{
		m_callback = listener;
	}

protected:
	static void trivial_handler()
	{
		assert(!"Unhandled Timer Message.");
	}

protected:
	timer_callback_type m_callback;
};

typedef timer_base TimerBase;
#endif