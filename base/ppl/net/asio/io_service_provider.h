
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_NET_ASIO_IO_SERVICE_PROVIDER_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_NET_ASIO_IO_SERVICE_PROVIDER_H_


#include <boost/asio/io_service.hpp>
using boost::asio::io_service;

#include <boost/shared_ptr.hpp>
typedef boost::shared_ptr<boost::asio::io_service> io_service_ptr;



/// 负责提供默认的io_service对象，主要用于构造socket、timer对象
class io_service_provider
{
public:
	static boost::asio::io_service& default_service()
	{
		return *ref_default();
	}
	static io_service_ptr get_default()
	{
		return ref_default();
	}
	/// 获取默认service的智能指针引用，可以用来在指定的时刻释放io_service对象
	static io_service_ptr& ref_default()
	{
		static io_service_ptr theService(new io_service);
//		static io_service_ptr* theSer = new io_service_ptr(new io_service);
		return theService;
	}

};

#endif
