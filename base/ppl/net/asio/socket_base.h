
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_NET_ASIO_SOCKET_BASE_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_NET_ASIO_SOCKET_BASE_H_

#include <boost/noncopyable.hpp>
#include <boost/system/error_code.hpp>


class socket_base : private boost::noncopyable
{
public:
	virtual ~socket_base()
	{

	}

	int last_error() const
	{
		return m_last_error.value();
	}


protected:
	mutable boost::system::error_code m_last_error;
};

#endif