
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_OS_MODULE_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_OS_MODULE_H_

#include <ppl/config.h>

#if defined(_PPL_PLATFORM_MSWIN)

#include <ppl/os/paths.h>
#include <ppl/diag/trace.h>
#include <boost/noncopyable.hpp>

#include <ppl/mswin/windows.h>

namespace ppl { namespace os {


/// ��װwin32ģ������Ĺ�����
class module
{
public:
	explicit module(HMODULE hModule = NULL) : m_handle(hModule)
	{
	}
	~module()
	{
	}

	static module self()
	{
		module thisModule;
		thisModule.attach_to_self();
		return thisModule;
	}

	/// ���ݵ�ַ����ѯ�˵�ַ������ģ����������Ϊ������̬�����ĵ�ַ
	static HMODULE query_module_handle(PVOID address)
	{
		MEMORY_BASIC_INFORMATION meminfo = { 0 };
		const size_t sizeof_meminfo = sizeof(MEMORY_BASIC_INFORMATION);
		if (sizeof_meminfo != ::VirtualQuery(address, &meminfo, sizeof_meminfo))
		{
			TRACE(TEXT("VirtualQuery failed.\n"));
			return NULL;
		}
		return static_cast<HMODULE>(meminfo.AllocationBase);
	}

	bool attach_to_self()
	{
		return attach_address(&module::query_module_handle);
	}

	/// ��������Ӧ�ĵ�ַ������ģ��
	bool attach_address(PVOID address)
	{
		m_handle = query_module_handle(address);
		if (m_handle != NULL)
		{
			assert( false == get_file_path().empty() );
		}
		return this->is_open();
	}

	/// ���ģ����
	HMODULE native_handle() const { return m_handle; }

	/// ģ���Ƿ��
	bool is_open() const { return m_handle != NULL; }

	/// ��ȡdllģ��ĵ�����
	FARPROC get_export_item(LPCSTR name)
	{
		if (!is_open())
			return NULL;
		return ::GetProcAddress(m_handle, name);
	}


	tstring get_file_path() const
	{
		const size_t max_path_size = 4095;
		TCHAR pathstr[max_path_size + 1];
		DWORD len = ::GetModuleFileName( m_handle, pathstr, max_path_size );
		if ( 0 == len || len > max_path_size )
		{
			TRACE(_T("GetModuleFileName failed %p %lu %d\n"), m_handle, len, ::GetLastError());
			return tstring();
		}
		assert( '\0' == pathstr[len] );
		return tstring(pathstr, len);
	}

	/// ��ȡģ����ļ���(������·��)
	tstring get_file_name()
	{
		return ppl::os::paths::get_filename(get_file_path());
	}
	tstring get_file_directory()
	{
		return ppl::os::paths::get_directory(get_file_path());
	}
	tstring get_file_base_name()
	{
		return ppl::os::paths::get_basename(get_file_path());
	}

	tstring build_local_file_path(const tstring& filename)
	{
		return ppl::os::paths::replace_filename( get_file_path(), filename );
	}

protected:
	/// ģ����
	HMODULE m_handle;
};


/// ��װ�ص�ģ��
class loadable_module : public module, private boost::noncopyable
{
public:
	loadable_module()
	{
	}

	explicit loadable_module(LPCTSTR filename)
	{
		m_handle = ::LoadLibrary(filename);
	}

	~loadable_module()
	{
		close();
	}

	bool load(LPCTSTR filename)
	{
		assert(!is_open());
		close();
		m_handle = ::LoadLibrary(filename);
		return is_open();
	}

	/// �ر�(�ͷ�)ģ��
	void close()
	{
		if (is_open())
		{
			::FreeLibrary(m_handle);
			m_handle = NULL;
		}
	}
};


} }


#elif defined(_PPL_PLATFORM_POSIX)

#include <dlfcn.h>
#include <boost/noncopyable.hpp>
namespace ppl { namespace os {


/// ��װwin32ģ������Ĺ�����
class module
{
public:
	typedef void* HMODULE;

	explicit module(HMODULE hModule = NULL) : m_handle(hModule)
	{
	}
	~module()
	{
	}

	/// ���ģ����
	HMODULE native_handle() const { return m_handle; }

	/// ģ���Ƿ���Ч
	bool is_open() const { return m_handle != NULL; }

	/// ��ȡdllģ��ĵ�����
	void* get_export_item(LPCSTR name)
	{
		if (!is_open())
			return NULL;
		return ::dlsym(m_handle, name);
	}



protected:
	/// ģ����
	HMODULE m_handle;
};


/// ��װ�ص�ģ��
class loadable_module : public module, private boost::noncopyable
{
public:
	loadable_module()
	{
	}

	explicit loadable_module(LPCTSTR filename)
	{
		m_handle = ::dlopen(filename, RTLD_NOW);
	}

	~loadable_module()
	{
		close();
	}

	bool load(LPCTSTR filename)
	{
		assert(!is_open());
		close();
		m_handle = ::dlopen(filename, RTLD_NOW);
		return is_open();
	}

	/// �ر�(�ͷ�)ģ��
	void close()
	{
		if (is_open())
		{
			int err = ::dlclose(m_handle);
			m_handle = NULL;
			assert(0 == err);
		}
	}
};


} }


#else

#error module is not currently supported on your platform

#endif


#endif