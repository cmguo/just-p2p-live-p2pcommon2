
#ifndef _LIVE_P2PCOMMON2_NEW_UTIL_TESTLOG_H_
#define _LIVE_P2PCOMMON2_NEW_UTIL_TESTLOG_H_


//#define _PPL_TEST_LOG



#include <ppl/io/stdfile.h>
#include <ppl/data/time.h>
#include <boost/noncopyable.hpp>
#include <sstream>


class SimpleLogger : private boost::noncopyable
{
public:
	static SimpleLogger& Instance()
	{
		static SimpleLogger logger;
		return logger;
	}

	~SimpleLogger()
	{
	}

	bool Open( const TCHAR* filename )
	{
		return m_fout.open( filename, _T("a"), _SH_DENYWR );
	}

	void Flush()
	{
		m_fout.flush();
	}

	void Log(const char* text)
	{
		m_fout.write_format("%s\n", text);
	}

	void Close()
	{
		this->Log( "end." );
		this->Flush();
		m_fout.close();
	}

private:
	ppl::io::stdfile_writer m_fout;
};



#ifdef _PPL_TEST_LOG

#define TEST_LOG_OUT(message) do{\
	std::ostringstream temposs; \
	temposs << date_time::now().str() << " " << message; \
	SimpleLogger::Instance().Log( temposs.str().c_str() ); } while (false)

#define TEST_LOG_OUT_ONCE(message) do{\
	std::ostringstream temposs; \
	temposs << date_time::now().str() << " " << message; \
	SimpleLogger::Instance().Log( temposs.str().c_str() ); \
	SimpleLogger::Instance().Flush(); } while (false)

#define TEST_LOG_OPEN(filename) SimpleLogger::Instance().Open(filename)
#define TEST_LOG_FLUSH() SimpleLogger::Instance().Flush()



#else

#define TEST_LOG_OUT(message) ((void)0)
#define TEST_LOG_OPEN(filename) ((void)0)
#define TEST_LOG_FLUSH() ((void)0)

#define TEST_LOG_OUT_ONCE(message) ((void)0)

#endif


#endif