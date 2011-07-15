
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_NET_ASIO_DETAIL_TIMER_IMPL_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_NET_ASIO_DETAIL_TIMER_IMPL_H_

#if defined(_PPL_PLATFORM_MSWIN) && !defined(_PPL_ASIO_USE_POSIX_TIMER)

#include <ppl/net/asio/detail/tick_count_timer.h>
typedef tick_count_timer my_timer_type;
typedef tick_count_time_traits::duration_type my_time_duration_type;

#else

#include <boost/asio/deadline_timer.hpp>
typedef boost::asio::deadline_timer my_timer_type;
typedef boost::posix_time::milliseconds my_time_duration_type;

#endif

#include <ppl/util/log.h>

#include <boost/asio/placeholders.hpp>
#include <boost/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

class timer_impl : private boost::noncopyable, public boost::enable_shared_from_this<timer_impl>
{
public:
	typedef boost::function<void ()> callback_type;

	explicit timer_impl(boost::asio::io_service& inIOService, callback_type callback) 
		: m_impl( inIOService )
		, m_callback( callback )
		, m_detached( false )
		, m_started( false )
	{
		UTIL_ERROR("timer_impl::timer_impl " << this);
	}

	~timer_impl()
	{
		assert(false == m_started);
		assert(m_detached);
		this->stop();
		UTIL_ERROR("timer_impl::~timer_impl " << this);
	}

	void start_from_now(UINT elapse)
	{
		assert(false == this->is_started());
		UTIL_ERROR("timer_impl::start_from_now " << ppl::make_tuple(this, m_started, m_detached, elapse));
		m_impl.expires_from_now(my_time_duration_type(elapse), m_lasterror);
		this->async_wait();
	}
	void start_at(const my_timer_type::time_type& expireTime)
	{
		assert(false == this->is_started());
		UTIL_ERROR("timer_impl::start_at " << ppl::make_tuple(this, m_started, m_detached));
		m_impl.expires_at(expireTime, m_lasterror);
		this->async_wait();
	}
	void stop()
	{
		UTIL_ERROR("timer_impl::stop " << ppl::make_tuple(this, m_started, m_detached));
		if ( m_started )
		{
			m_impl.cancel(m_lasterror);
			m_started = false;
		}
	}

	void detach()
	{
		UTIL_ERROR("timer_impl::detach " << ppl::make_tuple(this, m_started, m_detached));
		m_detached = true;
	}

	bool is_started() const { return m_started; }

protected:
	void async_wait()
	{
		m_impl.async_wait(
			boost::bind(
			&timer_impl::on_timer_message, 
			this->shared_from_this(), 
			boost::asio::placeholders::error
			)
			);
		m_started = true;
	}
	void on_timer_message( const boost::system::error_code& err )
	{
		UTIL_ERROR("timer_impl::on_timer_message " << ppl::make_tuple(this, m_started, m_detached, err.value()));
		if ( m_detached )
		{
			UTIL_ERROR("on_timer_message, timer_impl is detached, error is " << err << " started=" << m_started);
			assert(false == m_started);
			return;
		}
		if ( err )
		{
			assert( boost::asio::error::operation_aborted == err );
			return;
		}
		if ( false == m_started )
		{
			//assert( m_started );
			return;
		}
		m_started = false;
		m_callback();
	}

protected:
	//tick_count_timer m_impl;
	my_timer_type m_impl;
	callback_type m_callback;
	boost::system::error_code m_lasterror;
	bool m_detached;
        bool m_started;
};

#endif
