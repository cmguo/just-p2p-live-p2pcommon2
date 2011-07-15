
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_DIAG_TRACE_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_DIAG_TRACE_H_

#include <ppl/config.h>

#include <ppl/data/tchar.h>
#include <ppl/data/tstring.h>

#include <stdarg.h>
#include <assert.h>


#ifdef _PPL_PLATFORM_MSWIN
#include <ppl/mswin/windows.h>
#endif


class Tracer
{
public:
#ifdef _PPL_PLATFORM_MSWIN
	static void Output(const wchar_t* str)
	{
		assert(str != NULL);
		::OutputDebugStringW(str);
	}

	static void Trace(const wchar_t* formatString, ...)
	{
		va_list args;
		va_start(args, formatString);

		const size_t max_size = 4096;
		int nBuf;
		wchar_t szBuffer[max_size + 1] = { 0 };

		nBuf = _vsnwprintf(szBuffer, max_size, formatString, args);

		// was there an error? was the expanded string too long?
		assert(nBuf >= 0);

		Output(szBuffer);

		va_end(args);
	}

	static void Output(const char* str)
	{
		assert(str != NULL);
		::OutputDebugStringA(str);
	}

#else
        static void Output(const char* str)
        {
                assert(str != NULL);
                printf("%s", str);
        }

#endif

	static void Trace(const char* formatString, ...)
	{
		va_list args;
		va_start(args, formatString);

		const size_t max_size = 4096;
		int nBuf;
		char szBuffer[max_size + 1] = { 0 };

		nBuf = vsnprintf(szBuffer, max_size, formatString, args);

		// was there an error? was the expanded string too long?
		assert(nBuf >= 0);

		Output(szBuffer);

		va_end(args);
	}
};



#ifdef _DEBUG
#define PPLTRACE		::Tracer::Trace
#define PPLTRACE0		::Tracer::Output
#else
#define PPLTRACE		1 ? (void)0 : ::Tracer::Trace
#define PPLTRACE0		1 ? (void)0 : ::Tracer::Output
#endif


#ifndef TRACE
#define TRACE PPLTRACE
#endif

#ifndef TRACE0
#define TRACE0 PPLTRACE0
#endif

#define TRACEOUT	::Tracer::Trace



class ScopedTracer
{
public:
	explicit ScopedTracer(const tstring& str) : m_str(str)
	{
		Tracer::Trace(_T("%s:  Begin\n"), m_str.c_str());
	}
	~ScopedTracer()
	{
		Tracer::Trace(_T("%s:  End\n"), m_str.c_str());
	}
private:
	tstring	m_str;
};


#define TRACE_SCOPE(str)	ScopedTracer _scopedTracer(str)

extern bool bNeedTrace;

#define TRACEOUT_T  if(bNeedTrace)::Tracer::Trace


#endif

