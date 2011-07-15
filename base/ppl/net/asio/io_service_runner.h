
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_NET_ASIO_IO_SERVICE_RUNNER_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_NET_ASIO_IO_SERVICE_RUNNER_H_

#include <boost/version.hpp>

#include <ppl/util/random.h>
#include <ppl/concurrent/runnable_thread.h>
#include <ppl/diag/trace.h>

#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>


class io_service_runner;
typedef boost::shared_ptr<io_service_runner> io_service_runner_ptr;


class io_service_runner : private boost::noncopyable
{
public:
	typedef boost::shared_ptr<boost::asio::io_service> io_service_ptr;

	explicit io_service_runner() : m_ioservice(new boost::asio::io_service) { }
	explicit io_service_runner(io_service_ptr ioservice) : m_ioservice(ioservice)
	{
		assert(m_ioservice);
	}

	io_service_ptr get_service()
	{
		return m_ioservice;
	}

	bool is_started()
	{
		return m_thread.is_alive();
	}

	void start()
	{
		m_work.reset(new boost::asio::io_service::work(*this->get_service()));
		m_thread.start(boost::bind(&io_service_runner::run, this), boost::bind(&io_service_runner::interrupt, this));
	}
	void stop(UINT milliseconds)
	{
		m_thread.stop(milliseconds);
	}
	bool join(UINT milliseconds)
	{
		return m_thread.join(milliseconds);
	}
	void interrupt()
	{
		m_work.reset();
#if BOOST_VERSION >= 103500
		m_ioservice->stop();
#else
		try
		{
			m_ioservice->interrupt();
		}
		catch (std::exception& e)
		{
			printf("failed to interrupt\n");
		}
#endif
	}

private:
	void run()
	{
		OldRandomSeed setRandomSeed;
		//TRACE("run io service at thread %lu 0x%p\n", ::GetCurrentThreadId(), this);
		boost::system::error_code errcode;
		m_ioservice->run(errcode);
		//TRACE("run io service ok at thread %lu 0x%p, errcode=%d\n", ::GetCurrentThreadId(), this, errcode.value());
	}

private:
	io_service_ptr	m_ioservice;
	boost::shared_ptr<boost::asio::io_service::work> m_work;
	ppl::concurrent::runnable_thread m_thread;
};

class threading_io_service_runner : public io_service_runner
{
public:
	threading_io_service_runner() : io_service_runner()
	{
	}
};

#endif

