// src/live/p2pcommon2/base/ppl/util/log.h

#ifndef _LIVE_P2PCOMMON2_BASE_PPL_UTIL_LOG_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_UTIL_LOG_H_

#pragma once

#ifdef _PPL_PLATFORM_MSWIN
#pragma warning(disable: 4127)
#endif

#include <string>


extern void log_impl(int type, int level, const std::string& cText);




const int _PPL_NOSES	= 500;
const int _PPL_DEBUG	= 1000;
const int _PPL_INFO		= 2000;
const int _PPL_WARN		= 3000;
const int _PPL_EVENT	= 4000;
const int _PPL_ERROR	= 5000;

#include <sstream>

#define PPL_TRACEOUT(message) do{\
	std::ostringstream temposs; \
	temposs << "pplive_peer " << message << std::endl; \
	::OutputDebugStringA(temposs.str().c_str()); } while (false)

#ifdef NEED_LOG

#include <ppl/stl/stream.h>

#define PPL_LOG_IMPL(type, level, message) do{\
	std::ostringstream temposs; \
	temposs << message; \
	log_impl(type, level, temposs.str()); } while (false)


#else

#define PPL_LOG_IMPL(type, level, message)		((void)0)


#endif

#ifdef NEED_PPL_TRACE

#define PPL_TRACE(message) PPL_TRACEOUT(message)

#else //NEED_PPL_TRACE

#define PPL_TRACE(message)						((void)0)

#endif //NEED_PPL_TRACE


#define PPL_LOG(message)		PPL_LOG_IMPL(0, 0, message)



#define PPL_LOG_TYPE_LOGCORE 0

#define LOGCORE_INFO(message)		PPL_LOG_IMPL(PPL_LOG_TYPE_LOGCORE, _PPL_INFO, message)
#define LOGCORE_EVENT(message)		PPL_LOG_IMPL(PPL_LOG_TYPE_LOGCORE, _PPL_EVENT, message)
#define LOGCORE_WARN(message)		PPL_LOG_IMPL(PPL_LOG_TYPE_LOGCORE, _PPL_WARN, message)
#define LOGCORE_ERROR(message)		PPL_LOG_IMPL(PPL_LOG_TYPE_LOGCORE, _PPL_ERROR, message)
#define LOGCORE_DEBUG(message)		PPL_LOG_IMPL(PPL_LOG_TYPE_LOGCORE, _PPL_DEBUG, message)



#define PPL_LOG_TYPE_UTIL 100

#define UTIL_INFO(message)		PPL_LOG_IMPL(PPL_LOG_TYPE_UTIL, _PPL_INFO, message)
#define UTIL_EVENT(message)		PPL_LOG_IMPL(PPL_LOG_TYPE_UTIL, _PPL_EVENT, message)
#define UTIL_WARN(message)		PPL_LOG_IMPL(PPL_LOG_TYPE_UTIL, _PPL_WARN, message)
#define UTIL_ERROR(message)		PPL_LOG_IMPL(PPL_LOG_TYPE_UTIL, _PPL_ERROR, message)
#define UTIL_DEBUG(message)		PPL_LOG_IMPL(PPL_LOG_TYPE_UTIL, _PPL_DEBUG, message)


#define PPL_LOG_TYPE_APP 500

#define APP_INFO(message)		PPL_LOG_IMPL(PPL_LOG_TYPE_APP, _PPL_INFO, message)
#define APP_EVENT(message)		PPL_LOG_IMPL(PPL_LOG_TYPE_APP, _PPL_EVENT, message)
#define APP_WARN(message)		PPL_LOG_IMPL(PPL_LOG_TYPE_APP, _PPL_WARN, message)
#define APP_ERROR(message)		PPL_LOG_IMPL(PPL_LOG_TYPE_APP, _PPL_ERROR, message)
#define APP_DEBUG(message)		PPL_LOG_IMPL(PPL_LOG_TYPE_APP, _PPL_DEBUG, message)


#define PPL_LOG_TYPE_NETWORK			2000

#ifndef NETWORK_INFO
#define NETWORK_INFO(message)		PPL_LOG_IMPL(PPL_LOG_TYPE_NETWORK, _PPL_INFO, message)
#endif
#ifndef NETWORK_EVENT
#define NETWORK_EVENT(message)		PPL_LOG_IMPL(PPL_LOG_TYPE_NETWORK, _PPL_EVENT, message)
#endif
#ifndef NETWORK_WARN
#define NETWORK_WARN(message)		PPL_LOG_IMPL(PPL_LOG_TYPE_NETWORK, _PPL_WARN, message)
#endif
#ifndef NETWORK_ERROR
#define NETWORK_ERROR(message)		PPL_LOG_IMPL(PPL_LOG_TYPE_NETWORK, _PPL_ERROR, message)
#endif
#ifndef NETWORK_DEBUG
#define NETWORK_DEBUG(message)		PPL_LOG_IMPL(PPL_LOG_TYPE_NETWORK, _PPL_DEBUG, message)
#endif


#endif // BASE_PPL_UTIL_LOG_H
