
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_STL_STREAM_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_STL_STREAM_H_

#include <ppl/config.h>

#include <iosfwd>
#include <ostream>

//using std::ostream;
#ifdef _PPL_PLATFORM_MSWIN
#pragma warning(disable: 4127)
#endif
#include <ppl/data/tchar.h>
#include <ppl/data/int.h>
//#include <ppl/text/conv.h>
//#include <ppl/text/encoding.h>
#include <utility>

//#include <boost/tuple/tuple.hpp>
//#include <boost/tuple/tuple_io.hpp>

#include <ppl/data/tuple.h>

using ppl::make_tuple;


/*
namespace std
{
#if _MSC_VER >= 1400
	inline std::ostream& operator<<(std::ostream& os, unsigned char val)
	{
		os << static_cast<size_t>(val);
		return os;
	}

#if (defined(_WIN32_WCE) || _MSC_VER >= 1400)
#pragma message("------output wstring to ostream")
	inline std::ostream& operator<<(std::ostream& os, const wstring& s)
	{
		return os << encoding::w2a(s);
	}
#endif

#endif
}
*/

#ifdef NEED_LOG

#if !defined(_PPL_PLATFORM_POSIX)

#if defined(_MSC_VER) && (_MSC_VER < 1400)

inline void put_longlong(std::ostream& os, longlong val, bool isSigned)
{
	const char* fmt_signed = "%lld";
	const char* fmt_unsigned = "%llu";
	const char* fmt = isSigned ? fmt_signed : fmt_unsigned;
	const size_t max_size = 128;
	char buf[max_size + 1]; // ×ã¹»´óµÄ»º³åÇø
	snprintf(buf, max_size, fmt, val);
	os << buf;
}

#pragma message("------add support to ostream output for 64 bit integer")

namespace std {

	inline std::ostream& operator<<(std::ostream& os, longlong val)
	{
		put_longlong(os, val, true);
		return os;
	}

	inline std::ostream& operator<<(std::ostream& os, ulonglong val)
	{
		put_longlong(os, val, false);
		return os;
	}

}

#endif

#endif

#endif





inline std::pair<const void*, size_t> make_buffer_pair(const void* data, size_t size)
{
	return std::make_pair(data, size);
}


#endif
