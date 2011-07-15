#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_LOG_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_LOG_H_

#ifdef NEED_APACHE_LOG
// apatch log
#include <log4cxx/level.h>
#include <log4cxx/logger.h>
using namespace log4cxx;

#define SOURCE_LIB_LOG_IMPL(level,message)	LOG4CXX_LOG(Logger::getLogger(String(_T( "SourceReaderLib" ))),level,message)  

#define SOURCE_LIB_DEBUG(message)		SOURCE_LIB_LOG_IMPL(::log4cxx::Level::DEBUG,message)
//#define SOURCE_LIB_DEBUG(message)		LOG4CXX_LOG(Logger::getLogger(String(_T( "SourceReaderLib" ))),Level::DEBUG,message)
#define SOURCE_LIB_INFO(message)		SOURCE_LIB_LOG_IMPL(::log4cxx::Level::INFO,message)
#define SOURCE_LIB_WARN(message)		SOURCE_LIB_LOG_IMPL(::log4cxx::Level::WARN,message)
#define SOURCE_LIB_ERROR(message)		SOURCE_LIB_LOG_IMPL(::log4cxx::Level::ERROR,message)
#define SOURCE_LIB_FATAL(message)		SOURCE_LIB_LOG_IMPL(::log4cxx::Level::FATAL,message)

#else

#define SOURCE_LIB_DEBUG(message)
#define SOURCE_LIB_INFO(message)
#define SOURCE_LIB_WARN(message)
#define SOURCE_LIB_ERROR(message)
#define SOURCE_LIB_FATAL(message)

#endif

/*

#include <sstream>

#define __INFO			1000
#define __EVENT			2000
#define __WARN			3000
#define __ERROR			4000
#define __DEBUG			5000
#define __NOSEE			6000

extern void log_impl(unsigned long type, unsigned long level, const char* cText);

#ifdef NEED_LOG


#define SOURCE_LIB_LOG_IMPL(level, type, message) do{\
	std::ostringstream os; \
	os << message; \
	log_impl(level, type, os.str().c_str()); } while (false)

#else

#define SOURCE_LIB_LOG_IMPL(level, type, message)

#endif

// 	#define SOURCE_LIB_INFO(message)		SOURCE_LIB_LOG_IMPL(SOURCE_LOG_TYPE, __INFO, message)
// 	#define SOURCE_LIB_EVENT(message)		SOURCE_LIB_LOG_IMPL(SOURCE_LOG_TYPE, __EVENT, message)
// 	#define SOURCE_LIB_WARN(message)		SOURCE_LIB_LOG_IMPL(SOURCE_LOG_TYPE, __WARN, message)
// 	#define SOURCE_LIB_ERROR(message)		SOURCE_LIB_LOG_IMPL(SOURCE_LOG_TYPE, __ERROR, message)
// 	#define SOURCE_LIB_DEBUG(message)		SOURCE_LIB_LOG_IMPL(SOURCE_LOG_TYPE, __DEBUG, message)


// #define PPL_LOG(message)		SOURCE_LIB_IMPL(0, 0, message)
*/

#endif