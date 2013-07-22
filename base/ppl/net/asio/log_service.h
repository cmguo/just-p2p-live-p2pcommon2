
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_NET_ASIO_LOGSERVICE_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_NET_ASIO_LOGSERVICE_H_

#include <ppl/config.h>

#include <ppl/net/asio/io_service_runner.h>
#include <ppl/net/asio/timer.h>

#include <ppl/io/stdfile.h>
#include <ppl/os/paths.h>
#include <ppl/os/file_system.h>
#include <ppl/os/module.h>
#include <ppl/data/stlutils.h>
#include <ppl/util/time_counter.h>
#include <ppl/data/time.h>
#include <ppl/data/int.h>
#include <ppl/util/log.h>

#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
using boost::shared_ptr;
using boost::asio::io_service;

#include <fstream>
#include <assert.h>

#if defined(_PPL_PLATFORM_LINUX)
#include <ext/hash_map>
using __gnu_cxx::hash_map;
#else
#include <hash_map>
using stdext::hash_map;
#endif

class log_service : private boost::noncopyable
{
protected:
	log_service() 
		: m_ioServiceRunner( new io_service_runner )
		, m_logLevel( 0)
		, m_dirty( false ) 
	{
		ref_service_initialized() = true;
		m_maxLogSize = 512 * 1024 * 1024;
		m_timer.reset( new periodic_timer( *m_ioServiceRunner->get_service() ) );
		m_timer->set_callback( boost::bind( &log_service::on_timer, this ) );
	}
	~log_service()
	{
		ref_service_initialized() = false;
	}

	static bool& ref_service_initialized()
	{
		// 用于控制是否post log到io_service
		static bool isInited = false;
		return isInited;
	}

public:
	typedef hash_map<int, string> value_string_table;

	static bool service_initialized()
	{
		return ref_service_initialized();
	}

	class service_stopper : private boost::noncopyable
	{
	public:
		~service_stopper()
		{
			log_service::instance().stop();
		}
	};

	static log_service& instance()
	{
		static log_service theLog;
		return theLog;
	}

	/// 非线程安全，请在启动日志模块之前调用
	void init( const string& logDir, const string& logName, size_t maxSize = 512 * 1024 * 1024 )
	{
		m_dir = logDir;
		m_name = logName;
		LIVE_ASSERT( ppl::os::file_system::directory_exists( m_dir.c_str() ) );
		m_maxLogSize = maxSize;
	}

	bool start(const string& logDir, const string& logName, size_t maxSize = 512 * 1024 * 1024)
	{
		init(logDir, logName, maxSize);
		return start();
	}

	bool start()
	{
		// 如果没有设置输出路径和日志名，或者打开文件失败，则不输出到文件
		if ( false == m_dir.empty() && false == m_name.empty() )
		{
			open_log( date_time::now() );
		}
		m_ioServiceRunner->start();
		register_level( _PPL_INFO, "INFO" );
		register_level( _PPL_EVENT, "EVENT" );
		register_level( _PPL_WARN, "WARN" );
		register_level( _PPL_ERROR, "ERROR" );
		register_level( _PPL_DEBUG, "DEBUG" );
		register_type( PPL_LOG_TYPE_APP, "APP" );
		register_type( PPL_LOG_TYPE_UTIL, "UTIL" );
		register_type( PPL_LOG_TYPE_LOGCORE, "LOG" );
		m_timer->start( 3000 );
		post_log(0, _PPL_EVENT, "Start.");
		return true;
	}
	void stop()
	{
		ref_service_initialized() = false;
		m_fout.close();
		m_timer->stop();
		m_ioServiceRunner->stop(100);
	}

	void register_type( int typeval, const string& typestr )
	{
		m_ioServiceRunner->get_service()->post( boost::bind( &log_service::on_register_type, this, typeval, typestr) );
	}

	void register_level( int levelval, const string& levelstr )
	{
		m_ioServiceRunner->get_service()->post( boost::bind( &log_service::on_register_level, this, levelval, levelstr) );
	}

	void set_log_level( int logLevel )
	{
		m_logLevel = logLevel;
	}
	void log_on( int logType )
	{
		this->set_log_type( logType, true );
	}
	void log_off( int logType )
	{
		this->set_log_type( logType, false );
	}

	void set_log_type( int type, bool isLogOn )
	{
		m_ioServiceRunner->get_service()->post( boost::bind( &log_service::on_set_log_type, this, type, isLogOn ) );
	}

	void post_log( int type, int level, string text )
	{
		date_time dt = date_time::now();
		m_ioServiceRunner->get_service()->post( boost::bind( &log_service::on_log, this, dt, type, level, text ) );
	}

	bool can_log( int type, int level )
	{
		if ( false == m_fout.is_open() )
			return false;
		if ( level < m_logLevel )
			return false;
		return containers::contains( m_allowedTypes, type );
	}


protected:
	void on_log( date_time dt, int type, int level, string text )
	{
		// 如果文件没有打开，则只输出到控制台
		//if ( false == m_fout.IsOpen() )
		//	return;
		if ( level < m_logLevel )
			return;
		if ( type != 0 && false == containers::contains( m_allowedTypes, type ) )
			return;

		string timestr = dt.str();
		string typestr = get_value_string( type, m_typeStrings );
		string levelstr = get_value_string( level, m_levelStrings );
//		output( stdout, timestr, typestr, levelstr, text );
		if ( m_fout.is_open() )
		{
			m_dirty = true;
			output( m_fout.native_handle(), timestr, typestr, levelstr, text );
			if ( m_lastFlushTime.elapsed32() > 5 * 1000 )
			{
				// 1分钟flush一次
				m_dirty = false;
				m_fout.flush();
				m_lastFlushTime.sync();
				if ( m_maxLogSize > 0 && m_fout.get_position() > m_maxLogSize )
				{
					this->open_log( dt );
				}
			}
		}
	}
	void output( FILE* fp, const string& timestr, const string& type, const string& level, const string& text )
	{
		fprintf( fp, "%s %-5s %5s %s\n", timestr.c_str(), type.c_str(), level.c_str(), text.c_str() );
	}

	void on_timer()
	{
		if ( m_dirty && m_fout.is_open() )
		{
			m_dirty = false;
			m_fout.flush();
			m_lastFlushTime.sync();
		}
	}

	bool open_log( const date_time& dt )
	{
		m_dirty = false;
		string timestr;
		string filename;
		if (m_maxLogSize > 0)
		{
			timestr = dt.format( "_%04u%02u%02u_%02u%02u%02u" );
		}
		filename = m_name + timestr + ".log";
		string filepath = ppl::os::paths::combine( m_dir, filename );
		m_fout.close();
		if ( false == m_fout.open_text(filepath.c_str()) )
		{
			printf("failed to open log file %s\n", filepath.c_str());
			//LIVE_ASSERT(false);
			return false;
		}
		return true;
	}

	void on_set_log_type( int type, bool isLogOn )
	{
		if ( isLogOn )
		{
			m_allowedTypes.insert( type );
		}
		else
		{
			m_allowedTypes.erase( type );
		}
	}

	void on_register_type( int typeval, string typestr )
	{
		m_typeStrings[ typeval ] = typestr;
	}

	void on_register_level( int levelval, string levelstr )
	{
		m_levelStrings[ levelval ] = levelstr;
	}

	static string get_value_string( int val, const value_string_table& strs )
	{
		value_string_table::const_iterator iter = strs.find( val );
		if ( iter != strs.end() )
			return iter->second;
		return strings::format( "%lu", val );
	}

private:
	shared_ptr<io_service_runner> m_ioServiceRunner;
	value_string_table m_typeStrings;
	value_string_table m_levelStrings;
	ppl::io::stdfile_writer m_fout;
	time_counter m_lastFlushTime;
	int m_logLevel;
	std::set<int> m_allowedTypes;

	string m_dir;
	string m_name;
	size_t m_maxLogSize;
	bool m_dirty;

	/// 用指针式因为需要延迟构造以便使用指定的io_service
	boost::shared_ptr<periodic_timer> m_timer;
};


inline void log_impl(int type, int level, const string& text)
{
	if (log_service::service_initialized())
	{
		log_service::instance().post_log( type, level, text );
	}
	else
	{
		date_time dt = date_time::now();
		string timestr = dt.str();
		//printf("%s %04u %04u %s\n", timestr.c_str(), type, level, text.c_str());
	}
}

#endif