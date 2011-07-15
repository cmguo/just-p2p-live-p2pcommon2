
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_TEXT_ENCODING_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_TEXT_ENCODING_H_

#include <ppl/data/tstring.h>
#include <boost/noncopyable.hpp>
#include <stdlib.h>
#include <locale.h>
#include <locale>
#include <assert.h>
#include <string>


class encoding
{
public:
	class scoped_locale : private boost::noncopyable
	{
	public:
		int category;
		const char* old_name;

		explicit scoped_locale(int _category, const char* _name) : category(_category)
		{
			old_name = ::setlocale(_category, NULL);
			const char* newName = ::setlocale(_category, _name);
			if (NULL == newName)
			{
				// if setlocale failed, reset old_name, no need to restore it
				old_name = NULL;
			}
		}
		~scoped_locale()
		{
			if (old_name)
			{
				::setlocale(category, old_name);
			}
		}
	};

	class default_scoped_ctype_locale : public scoped_locale
	{
	public:
		default_scoped_ctype_locale() : scoped_locale(LC_CTYPE, ".ACP")
		{
		}
	};

	static std::locale& default_locale()
	{
		static std::locale loc(".ACP");
		return loc;
	}

	enum { max_string_length = 64 * 1024 * 1024 };

#ifdef _GLIBCXX_USE_WCHAR_T
	/// ansi - wide conversion
	static std::wstring a2w(const std::string& s)
	{
		if (s.empty())
			return std::wstring();
		default_scoped_ctype_locale useDefaultLocale;
		size_t len = mbstowcs(0,  s.c_str(), 0);
		if (0 == len)
			return std::wstring();
		if ((size_t)-1 == len)
			return std::wstring();
		if (len > max_string_length)
		{
			assert(false);
			return std::wstring();
		}
		std::wstring ws;
		ws.resize(len);
		size_t len2 = mbstowcs(&ws[0], s.c_str(), len);
		assert(len2 == len);
		if ((size_t)-1 == len2)
			return std::wstring();
		return ws;
	}
	/// wide - ansi conversion
	static std::string w2a(const std::wstring& ws)
	{
		if (ws.empty())
			return std::string();
		default_scoped_ctype_locale useDefaultLocale;
		size_t len = wcstombs(0,  ws.c_str(), 0);
		if (0 == len)
			return std::string();
		if ((size_t)-1 == len)
			return std::string();
		if (len > max_string_length)
		{
			assert(false);
			return std::string();
		}
		std::string s;
		s.resize(len);
		size_t len2 = wcstombs(&s[0], ws.c_str(), len);
		assert(len2 == len);
		if ((size_t)-1 == len2)
			return std::string();
		return s;
	}
#endif

#if defined(_UNICODE) || defined(UNICODE)
	/// tstring to ansi conversion
	static std::string t2a(const tstring& ts)
	{
		return w2a(ts);
	}
	static tstring a2t(const std::string& s)
	{
		return a2w(s);
	}
	/// tstring-wide conversion
	static std::wstring t2w(const tstring& ts)
	{
		return ts;
	}
	static tstring w2t(const std::wstring& s)
	{
		return s;
	}
#else
	static std::string t2a(const tstring& ts)
	{
		return ts;
	}
	static tstring a2t(const std::string& s)
	{
		return s;
	}
#ifdef _GLIBCXX_USE_WCHAR_T
	/// tstring-wide conversion
	static std::wstring t2w(const tstring& ts)
	{
		return a2w(ts);
	}
	static tstring w2t(const std::wstring& s)
	{
		return w2a(s);
	}
#endif
#endif
};

#endif



