
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_UTIL_FILE_LOGGER_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_UTIL_FILE_LOGGER_H_


#include <ppl/io/stdfile.h>
#include <ppl/os/module.h>


class Logger : private noncopyable
{
public:
	Logger()
	{
		bool res = Open(ppl::os::module().build_local_file_path(TEXT(".log")).data());
	}
	virtual ~Logger()
	{
	}

	bool Open(const TCHAR* filename)
	{
		return m_file.open_binary(filename, _SH_DENYWR);
	}
	void log(const char* str)
	{
		if (!m_file.is_open())
			return;
		this->LogHeader();
		m_file.write(str);
		m_file.write("\r\n");
		puts(str);
		m_file.flush();
	}
	void logf(const char* format, ...)
	{
		if (!m_file.is_open())
			return;
		this->LogHeader();
		va_list args;
		va_start(args, format);
		m_file.write_variant(format, args);
		m_file.write("\r\n");
		vprintf(format, args);
		va_end(args);
		m_file.flush();
	}

	static Logger& Instance()
	{
		static Logger logger;
		return logger;
	}

protected:
	void LogHeader()
	{
		SYSTEMTIME st;
		::GetLocalTime(&st);
		m_file.write_format("%04d-%02d-%02d %02d:%02d:%02d.%03d ", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
	}

private:
	ppl::io::stdfile_writer m_file;
};


#ifndef _PPL_DISABLE_FILE_LOG_IMPL

inline void log_impl(unsigned long type, unsigned long level, const char* cText)
{
	Logger::Instance().log(cText);
}

#endif

#endif

