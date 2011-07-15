#ifndef _LIVE_P2PCOMMON2_BASE_PPL_UTIL_LOGTRACE_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_UTIL_LOGTRACE_H_

/**
  * @file
  * @brief ʹ��OutputDebugString��log_implʵ��
  */

#include <ppl/diag/trace.h>
#include <string>

inline void log_impl(int type, int level, const std::string& text)
{
	// �ֿ����������text����������TRACEOUT�л���������
	TRACEOUT("log: %04d %04d ", type, level);
	::OutputDebugString(text.c_str());
	::OutputDebugString("\n");
}

#endif