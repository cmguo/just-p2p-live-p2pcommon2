#ifndef _LIVE_P2PCOMMON2_NEW_FRAMEWORK_TIMER_H_
#define _LIVE_P2PCOMMON2_NEW_FRAMEWORK_TIMER_H_

/* 
包含p2p模块中跟socket相关的定义
如果使用ten.dll，包含synacast/net目录下的文件
如果使用asio，包含synacast/asio
如果使用mfc，可以包含synacast/mfcnet.h和ppl/mswin/mfc/timer.h
*/


#include <ppl/config.h>



//#if defined(_PPL_PLATFORM_LINUX) || defined(_PPL_USE_ASIO)


#include <ppl/net/asio/timer.h>
//#include <ppl/net/asio/tcp_socket.h>
//#include <ppl/net/asio/message_tcp_socket.h>
//#include <ppl/net/asio/http_client.h>


/*
#elif defined(_PPL_PLATFORM_MSWIN)

#include <synacast/net/timer.h>


#else

#error "invalid platform for timer.h"


#endif

*/


#endif
