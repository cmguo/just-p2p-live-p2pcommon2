
#ifndef _LIVE_P2PCOMMON2_NEW_APP_CONTROLLER_H_
#define _LIVE_P2PCOMMON2_NEW_APP_CONTROLLER_H_

#include <ppl/config.h>

#include "app/channel.h"
#include "common/GloalTypes.h"

#include <ppl/util/test_case.h>
#include <ppl/net/asio/io_service_runner.h>
#include <ppl/net/asio/io_service_provider.h>

#ifdef NEED_LOG
#include <ppl/net/asio/log_service.h>
#endif

#include <ppl/os/shell.h>
#include <ppl/os/file_system.h>
#include <ppl/os/paths.h>
#include <ppl/os/module.h>

#include <boost/interprocess/shared_memory_object.hpp>

#include <map>

class LiveChannel;
typedef boost::shared_ptr<LiveChannel> LiveChannelPtr;
typedef std::map<LiveChannel*, LiveChannelPtr> LiveChannelCollection;


/***********************************************************************
每个频道使用独立的一套tcport和udpport，但使用公共的消息队列和对应的线程
对于asio，使用同一个的io_service和相应的线程，即默认的io_service
(这样便于timer和socket的构造，让它们不需要知道io_service的存在)
***********************************************************************/


class LiveChannelController : private boost::noncopyable
{
public:
	LiveChannelController(int node_type = 0) 
		: m_runner(io_service_provider::get_default()),m_startup_times(0),m_node_type(node_type)
	{
	}
	~LiveChannelController()
	{
	}
 
	bool Start(int node_type)
	{
		::InterlockedIncrement(&m_startup_times);
		if (m_runner.is_started())
			return true;

#if 0
		// for linux
		tstring path = ppl::os::paths::combine( configDir, _T("live.cfg") );
		IniFile ini;
		ini.SetFileName(path);
		ini.SetSection(_T("network"));
		UINT tcpPort = ini.GetInt(_T("tcp"), 0);
		if ( 0 == tcpPort )
		{
			tcpPort = rnd.NextIn(5500) + 2111;
			ini.SetInt("tcp", tcpPort);
		}
		UINT udpPort = ini.GetInt(_T("udp"), 0);
		if ( 0 == udpPort )
		{
			udpPort = rnd.NextIn(5500) + 2111;
			ini.SetInt(_T("udp"), udpPort);
		}
#endif

#if defined(_PPL_PLATFORM_MSWIN)
		ppl::os::module localModule;
		localModule.attach_to_self();
		string baseDir = localModule.get_file_directory();
		string configDir = ppl::os::paths::combine( ShellFolders::CommonAppDataDirectory(), _T("PPLive\\Core") );
#elif defined(_PPL_PLATFORM_LINUX)
        string baseDir = ppl::os::paths::combine(boost::interprocess::detail::get_temporary_path(), "live");
		string configDir = ppl::os::paths::combine(baseDir, "config");
#else
#error invalid platform-------------------
#endif
		bool success = ppl::os::file_system::ensure_directory_exists(configDir.c_str());
		TRACEOUT("live:config directory is %s %s %d\n", configDir.c_str(), baseDir.c_str(), success);
		LIVE_ASSERT(success);
		LIVE_ASSERT(ppl::os::file_system::directory_exists(baseDir.c_str()));

		m_configdir = configDir;
		m_basedir = baseDir;

#ifdef NEED_LOG
		log_service::instance().start(baseDir, "pplive.exe", 0);

		//ppl::os::module thisModule;
		//thisModule.attach_to_self();
		//LoadLogSetting(thisModule.get_file_directory());
#endif

#ifdef _DEBUG
		ppl::util::run_tests();
#endif

		m_runner.start();

		m_node_type = node_type;
		return true;
	}
	void Stop()
	{
		if (::InterlockedDecrement(&m_startup_times) > 0)
			return;

		printf("clear channels %u\n", m_channels.size());
		m_channels.clear();
		printf("live cleanup %u\n", m_channels.size());
		m_runner.stop(1000);
		printf("live cleanup ok\n");
#ifdef NEED_LOG
		log_service::instance().stop();
#endif
	}

	LiveChannelPtr StartChannel(const tstring& urlstr, int tcpPort, int udpPort)
	{
		LiveChannelPtr controller(new LiveChannel(m_node_type));
		if ( false == controller->Init(urlstr, tcpPort, udpPort, m_configdir, m_basedir) )
			return LiveChannelPtr();
		controller->Start();
		m_channels[controller.get()] = controller;
		return controller;
	}

	bool StopChannel(void* key)
	{
		if ( NULL == key )
			return false;
		LiveChannel* channel = static_cast<LiveChannel*>( key );
		LiveChannelCollection::iterator iter = m_channels.find(channel);
		if (iter == m_channels.end() )
			return false;
		LIVE_ASSERT(iter->second);
		LIVE_ASSERT(iter->second.get() == channel);
		TRACEOUT("Tady --> ### Begin Stop ### ");
		iter->second->Stop();
		TRACEOUT("Tady --> ### End Stop ### ");
		m_channels.erase(iter);
		TRACEOUT("Tady --> @@@@@ Finish Stop Channel @@@@@");
		return true;
	}

	LiveChannel* GetChannel(void* key)
	{
		if ( NULL == key )
			return NULL;
		LiveChannel* channel = static_cast<LiveChannel*>( key );
		LiveChannelCollection::const_iterator iter = m_channels.find(channel);
		if ( iter == m_channels.end() )
			return NULL;
		LIVE_ASSERT(iter->second);
		LIVE_ASSERT(iter->second.get() == channel);
		return iter->second.get();
	}

private:
	io_service_runner m_runner;
	LiveChannelCollection m_channels;
	tstring m_basedir;
	tstring m_configdir;
	long m_startup_times;

	int	m_node_type;
};

#endif

