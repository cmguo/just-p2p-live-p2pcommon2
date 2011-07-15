
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_UTIL_INI_FILE_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_UTIL_INI_FILE_H_

#include <ppl/config.h>

#if defined(_PPL_PLATFORM_MSWIN)

#include <ppl/mswin/ini_file.h>

#else

#include <ppl/util/detail/IniFile.h>

#include <ppl/data/strings.h>
#include <ppl/data/numeric.h>
#include <ppl/os/file_system.h>
#include <boost/noncopyable.hpp>

class ini_file : private boost::noncopyable
{
public:
	ini_file()
	{
	}
	~ini_file()
	{
	}


	void set_filename(const string& filename)
	{
		m_filename = filename;
		if ( false == ppl::os::file_system::file_exists( m_filename.c_str() ) )
		{
			ppl::util::detail::CIniFile::Create(m_filename.c_str());
		}
	}

	void set_section(const string& section)
	{
		m_section = section;
	}

	const tstring& get_filename() const { return m_filename; }
	const tstring& get_section() const { return m_section; }

	string get_string(const string& key, LPCTSTR defaultValue)
	{
		check_state();
		string s = ppl::util::detail::CIniFile::GetValue(key, m_section, defaultValue, m_filename);
		return s;
	}
	bool set_string(const string& key, const tstring& value)
	{
		check_state();
		return ppl::util::detail::CIniFile::SetValue(key, value, m_section, m_filename);
	}





	/************************************************************************/
	/* integer                                                              */
	/************************************************************************/
	int get_int(const string& key, int defaultValue)
	{
		string s = this->get_string(key, "");
		if ( s.empty() )
			return defaultValue;
		return numeric<int>::parse(s, defaultValue);
	}
	bool set_int(const string& key, int value)
	{
		string s = strings::format("%d", value);
		return this->set_string(key, s);
	}
	unsigned int get_uint(const string& key, int defaultValue)
	{
		string s = this->get_string(key, "");
		if ( s.empty() )
			return defaultValue;
		return numeric<unsigned int>::parse(s, defaultValue);
	}
	bool set_uint(const string& key, unsigned int value)
	{
		string s = strings::format("%u", value);
		return this->set_string(key, s);
	}


	/************************************************************************/
	/* bool                                                                 */
	/************************************************************************/
	bool get_bool(const string& key, bool defaultValue)
	{
		return 0 != this->get_int(key, defaultValue);
	}
	bool set_bool(const string& key, bool value)
	{
		return this->set_int(key, value ? 1 : 0);
	}


protected:
	void check_state()
	{
		assert(!m_filename.empty());
		assert(!m_section.empty());
	}


private:
	string m_filename;
	string m_section;
};

#endif

#endif