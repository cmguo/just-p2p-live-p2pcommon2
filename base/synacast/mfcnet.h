
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_MFCNET_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_MFCNET_H_


/*

// Insert your headers here
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <windows.h>

#include <winsock2.h>



#if !defined(_WIN32_WCE) || defined(_PPL_FAKE_WINCE)
#pragma comment(lib, "ws2_32.lib")
#else
#pragma comment(lib, "ws2.lib")
#endif

*/



#define NEED_CORE
#define NEED_NET


//#undef NEED_LOG

#if defined(_DEBUG) && !defined(NEED_LOG)
#define NEED_LOG
#pragma message("===need log")
#else
#pragma message("===no need log")
#endif


#define PPL_EXPORT


#include <ppl/base.h>

#include <ppl/util/listener.h>


#include <ppl/mswin/mfc/timer.h>

#include <ppl/mswin/mfc/net.h>

#include <ppl/util/memory.h>


class BasicObject
{
public:
};

typedef BasicObject CBasicObject;


#include <ppl/util/guid.h>

#include <ppl/util/GuidLess.h>

#include <ppl/net/inet.h>

#include <ppl/util/loghelper.h>

#include <ppl/stl/stlutils.h>


#include <ppl/util/TimeCounter.h>

#include <ppl/mswin/module.h>
#include <ppl/io/path.h>
#include <ppl/mswin/ini.h>

#include <ppl/mswin/FileMapping.h>
#include <ppl/util/random.h>


#include <boost/array.hpp>
using boost::array;

#endif

