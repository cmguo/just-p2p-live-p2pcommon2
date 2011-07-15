
#ifndef _LIVE_P2PCOMMON2_NEW_FRAMEWORK_LOG_H_
#define _LIVE_P2PCOMMON2_NEW_FRAMEWORK_LOG_H_

#include <ppl/util/log.h>


#define MANAGER_INFO(x)		PEERMANAGER_INFO(x)
#define MANAGER_EVENT(x)	PEERMANAGER_EVENT(x)
#define MANAGER_WARN(x)		PEERMANAGER_WARN(x)
#define MANAGER_ERROR(x)	PEERMANAGER_ERROR(x)
#define MANAGER_DEBUG(x)	PEERMANAGER_DEBUG(x)



#define PEERCON_INFO(x)		CONNECTION_INFO(x)
#define PEERCON_EVENT(x)	CONNECTION_EVENT(x)
#define PEERCON_WARN(x)		CONNECTION_WARN(x)
#define PEERCON_ERROR(x)	CONNECTION_ERROR(x)
#define PEERCON_DEBUG(x)	CONNECTION_DEBUG(x)


#define PPL_LOG_TYPE_STREAMBUFFER 4800

#define STREAMBUFFER_INFO(message)		LOG(PPL_LOG_TYPE_STREAMBUFFER, __INFO, message)
#define STREAMBUFFER_EVENT(message)		LOG(PPL_LOG_TYPE_STREAMBUFFER, __EVENT, message)
#define STREAMBUFFER_WARN(message)		LOG(PPL_LOG_TYPE_STREAMBUFFER, __WARN, message)
#define STREAMBUFFER_ERROR(message)		LOG(PPL_LOG_TYPE_STREAMBUFFER, __ERROR, message)
#define STREAMBUFFER_DEBUG(message)		LOG(PPL_LOG_TYPE_STREAMBUFFER, __DEBUG, message)



#define PPL_LOG_TYPE_VIEW 5000

#define VIEW_INFO(message)		PPL_LOG_IMPL(PPL_LOG_TYPE_VIEW, __INFO, message)
#define VIEW_EVENT(message)		PPL_LOG_IMPL(PPL_LOG_TYPE_VIEW, __EVENT, message)
#define VIEW_WARN(message)		PPL_LOG_IMPL(PPL_LOG_TYPE_VIEW, __WARN, message)
#define VIEW_ERROR(message)		PPL_LOG_IMPL(PPL_LOG_TYPE_VIEW, __ERROR, message)
#define VIEW_DEBUG(message)		PPL_LOG_IMPL(PPL_LOG_TYPE_VIEW, __DEBUG, message)



#define PPL_LOG_TYPE_UDPT 5500

#define UDPT_INFO(message)		PPL_LOG_IMPL(PPL_LOG_TYPE_UDPT, __INFO, message)
#define UDPT_EVENT(message)		PPL_LOG_IMPL(PPL_LOG_TYPE_UDPT, __EVENT, message)
#define UDPT_WARN(message)		PPL_LOG_IMPL(PPL_LOG_TYPE_UDPT, __WARN, message)
#define UDPT_ERROR(message)		PPL_LOG_IMPL(PPL_LOG_TYPE_UDPT, __ERROR, message)
#define UDPT_DEBUG(message)		PPL_LOG_IMPL(PPL_LOG_TYPE_UDPT, __DEBUG, message)


#define PPL_LOG_TYPE_UPLOAD 5600

#define UPLOAD_INFO(message)		PPL_LOG_IMPL(PPL_LOG_TYPE_UPLOAD, __INFO, message)
#define UPLOAD_EVENT(message)		PPL_LOG_IMPL(PPL_LOG_TYPE_UPLOAD, __EVENT, message)
#define UPLOAD_WARN(message)		PPL_LOG_IMPL(PPL_LOG_TYPE_UPLOAD, __WARN, message)
#define UPLOAD_ERROR(message)		PPL_LOG_IMPL(PPL_LOG_TYPE_UPLOAD, __ERROR, message)
#define UPLOAD_DEBUG(message)		PPL_LOG_IMPL(PPL_LOG_TYPE_UPLOAD, __DEBUG, message)






#ifdef _PPL_RELEASE_LOG

#pragma message("------use log4cxx")

#ifdef LOG4CXX_EXPORT
#error "------error for log4cxx"
#endif

#ifndef LOG4CXX_STATIC
#define LOG4CXX_STATIC
#pragma message("------use statically linked log4cxx")
#endif


#include <log4cxx/level.h>
#include <log4cxx/logger.h>
#include <log4cxx/helpers/properties.h>
#include <log4cxx/propertyconfigurator.h>

//using namespace log4cxx;

#define SOURCENEW_LOG_IMPL(level,message)	LOG4CXX_LOG(::log4cxx::Logger::getLogger(::log4cxx::String(_T( "SourceNew" ))),level,message)  

#define SOURCENEW_DEBUG(message)		SOURCENEW_LOG_IMPL(::log4cxx::Level::DEBUG,message)
#define SOURCENEW_INFO(message)		SOURCENEW_LOG_IMPL(::log4cxx::Level::INFO,message)
#define SOURCENEW_WARN(message)		SOURCENEW_LOG_IMPL(::log4cxx::Level::WARN,message)
#define SOURCENEW_ERROR(message)		SOURCENEW_LOG_IMPL(::log4cxx::Level::ERROR,message)
#define SOURCENEW_FATAL(message)		SOURCENEW_LOG_IMPL(::log4cxx::Level::FATAL,message)

#else


#define SOURCENEW_DEBUG(message)		SOURCE_DEBUG(message)
#define SOURCENEW_INFO(message)			SOURCE_INFO(message)
#define SOURCENEW_WARN(message)			SOURCE_WARN(message)
#define SOURCENEW_ERROR(message)		SOURCE_ERROR(message)
#define SOURCENEW_FATAL(message)		SOURCE_ERROR(message)


#endif


#endif