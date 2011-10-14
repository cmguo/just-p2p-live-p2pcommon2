
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_MSWIN_INI_FILE_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_MAWIN_INI_FILE_H_

/**
 * @file
 * @brief ini文件操作
 */


#include <ppl/mswin/windows.h>
#include <ppl/data/tstring.h>
#include <assert.h>


/// ini配置文件操作类
class ini_file
{
public:
	ini_file()
	{
	}

	/************************************************************************
	* compatibility interfaces
	************************************************************************/
	tstring GetString(LPCTSTR key, LPCTSTR defaultValue)
	{
		return get_string(key, defaultValue);
	}
	bool SetString(LPCTSTR key, LPCTSTR value)
	{
		return set_string(key, value);
	}
	bool SetString(LPCTSTR key, const tstring& value)
	{
		return set_string(key, value);
	}
	UINT GetInt(LPCTSTR key, int defaultValue)
	{
		return get_int(key, defaultValue);
	}
	bool SetInt(LPCTSTR key, int value)
	{
		return set_int(key, value);
	}
	UINT SetUnsignedInt(LPCTSTR key, unsigned int defaultValue)
	{
		return set_uint(key, defaultValue);
	}
	bool GetBool(LPCTSTR key, bool defaultValue)
	{
		return get_bool(key, defaultValue);
	}
	bool SetBool(LPCTSTR key, bool value)
	{
		return set_bool(key, value);
	}
	BOOL GetBOOL(LPCTSTR key, BOOL defaultValue)
	{
		return get_BOOL(key, defaultValue);
	}
	bool SetBOOL(LPCTSTR key, BOOL value)
	{
		return set_BOOL(key, value);
	}


	/// 设置ini文件名
	void set_filename(const tstring& filename)
	{
		m_filename = filename;
	}

	/// 设置节名称
	void set_section(const tstring& section)
	{
		m_section = section;
	}

	const tstring& get_filename() const { return m_filename; }
	const tstring& get_section() const { return m_section; }

	/************************************************************************/
	/* string                                                               */
	/************************************************************************/
	DWORD get_string(LPCTSTR key, LPTSTR buffer, DWORD size, LPCTSTR defaultValue)
	{
		check_state();
		return ::GetPrivateProfileString(get_section_str(), key, defaultValue, buffer, size, get_filename_str());
	}
	tstring get_string(LPCTSTR key, LPCTSTR defaultValue)
	{
		const int max_size = 511;
		TCHAR buf[max_size + 1];
		buf[max_size] = '\0';
		DWORD length = get_string(key, buf, max_size, defaultValue);
		LIVE_ASSERT(length <= max_size);
		return tstring(buf, length);
	}
	bool set_string(LPCTSTR key, LPCTSTR value)
	{
		check_state();
		return FALSE != ::WritePrivateProfileString(get_section_str(), key, value, get_filename_str());
	}
	bool set_string(LPCTSTR key, const tstring& value)
	{
		return set_string(key, value.c_str());
	}

	/************************************************************************/
	/* integer                                                              */
	/************************************************************************/
	UINT get_int(LPCTSTR key, int defaultValue)
	{
		check_state();
		return ::GetPrivateProfileInt(get_section_str(), key, defaultValue, get_filename_str());
	}
	bool set_int(LPCTSTR key, int value)
	{
		TCHAR str[64] = { 0 };
		::sprintf(str, _T("%d"), value);
		return set_string(key, str);
	}
	bool set_uint(LPCTSTR key, unsigned int value)
	{
		TCHAR str[64] = { 0 };
		::sprintf(str, _T("%u"), value);
		return set_string(key, str);
	}

	/************************************************************************/
	/* bool                                                                 */
	/************************************************************************/
	bool get_bool(LPCTSTR key, bool defaultValue)
	{
		return 0 != get_int(key, defaultValue);
	}
	bool set_bool(LPCTSTR key, bool value)
	{
		return set_int(key, value);
	}
	BOOL get_BOOL(LPCTSTR key, BOOL defaultValue)
	{
		UINT res = get_int(key, defaultValue);
		return (0 != res) ? TRUE : FALSE;
	}
	bool set_BOOL(LPCTSTR key, BOOL value)
	{
		return set_int(key, value);
	}

protected:
	/// 获得ini文件名字符串
	LPCTSTR get_filename_str() const
	{
		return m_filename.c_str();
	}
	LPCTSTR get_section_str() const
	{
		return m_section.c_str();
	}
	void check_state()
	{
		LIVE_ASSERT(false == m_filename.empty());
		LIVE_ASSERT(false == m_section.empty());
	}

private:
	/// ini文件名(可能包括路径)
	tstring m_filename;
	/// ini节名
	tstring m_section;
};


/*

/// 基于ini文件的配置信息
class IniConfiguration
{
public:
	/// 从指定的配置文件和节名称加载配置信息
	void Load(const TCHAR* filename, const TCHAR* appname)
	{
		IniFile ini;
		ini.SetLocalFileName(filename);
		ini.SetSection(appname);
		DoLoad(ini, appname);
	}
	/// 将配置信息以指定的节名称保存到指定的配置文件
	void Save(const TCHAR* filename, const TCHAR* appname)
	{
		IniFile ini;
		ini.SetLocalFileName(filename);
		ini.SetSection(appname);
		DoSave(ini, appname);
	}

	/// 从指定的配置文件中加载配置信息
	void Load(const TCHAR* appname)
	{
		IniFile ini;
		ini.SetSection(appname);
		DoLoad(ini);
	}
	/// 将配置信息保存到指定的配置文件中
	void Save(const TCHAR* appname)
	{
		IniFile ini;
		ini.SetSection(appname);
		DoSave(ini);
	}

	/// 从指定的配置文件中加载配置信息
	void Load(IniFile& ini) { DoLoad(ini); }
	/// 将配置信息保存到指定的配置文件中
	void Save(IniFile& ini) { DoSave(ini); }

protected:
	/// 负载具体的配置加载操作
	virtual void DoLoad(IniFile& ini, const TCHAR* appname) { }
	/// 负载具体的配置保存操作
	virtual void DoSave(IniFile& ini, const TCHAR* appname) { }
	/// 负载具体的配置加载操作
	virtual void DoLoad(IniFile& ini) { }
	/// 负载具体的配置保存操作
	virtual void DoSave(IniFile& ini) { }
};

*/

#endif