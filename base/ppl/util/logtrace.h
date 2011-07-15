#ifndef _LIVE_P2PCOMMON2_BASE_PPL_UTIL_LOGTRACE_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_UTIL_LOGTRACE_H_

/**
  * @file
  * @brief 使用OutputDebugString的log_impl实现
  */

#include <ppl/diag/trace.h>
#include <string>

inline void log_impl(int type, int level, const std::string& text)
{
	// 分开输出，避免text过长，导致TRACEOUT中缓冲区不够
	TRACEOUT("log: %04d %04d ", type, level);
	::OutputDebugString(text.c_str());
	::OutputDebugString("\n");
}

#endif