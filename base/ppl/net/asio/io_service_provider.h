
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_NET_ASIO_IO_SERVICE_PROVIDER_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_NET_ASIO_IO_SERVICE_PROVIDER_H_


#include <boost/asio/io_service.hpp>
using boost::asio::io_service;

#include <boost/shared_ptr.hpp>
typedef boost::shared_ptr<boost::asio::io_service> io_service_ptr;



/// �����ṩĬ�ϵ�io_service������Ҫ���ڹ���socket��timer����
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
	/// ��ȡĬ��service������ָ�����ã�����������ָ����ʱ���ͷ�io_service����
	static io_service_ptr& ref_default()
	{
		static io_service_ptr theService(new io_service);
//		static io_service_ptr* theSer = new io_service_ptr(new io_service);
		return theService;
	}

};

#endif
