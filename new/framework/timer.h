#ifndef _LIVE_P2PCOMMON2_NEW_FRAMEWORK_TIMER_H_
#define _LIVE_P2PCOMMON2_NEW_FRAMEWORK_TIMER_H_

/* 
����p2pģ���и�socket��صĶ���
���ʹ��ten.dll������synacast/netĿ¼�µ��ļ�
���ʹ��asio������synacast/asio
���ʹ��mfc�����԰���synacast/mfcnet.h��ppl/mswin/mfc/timer.h
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
