
#ifndef _LIVE_P2PCOMMON2_NEW_COMMON_APPCORE_H_
#define _LIVE_P2PCOMMON2_NEW_COMMON_APPCORE_H_



/// 主定时器每半秒钟触发的次数
const int APP_TIMER_TIMES_PER_HALF_SECOND = 2;

/// 主定时器每秒钟触发的次数
const int APP_TIMER_TIMES_PER_SECOND = APP_TIMER_TIMES_PER_HALF_SECOND * 2;

/// 主定时器的间隔时间
const int APP_TIMER_INTERVAL = 1000 / APP_TIMER_TIMES_PER_SECOND;

#endif

