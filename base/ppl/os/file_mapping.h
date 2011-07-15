#ifndef _LIVE_P2PCOMMON2_BASE_PPL_OS_FILE_MAPPING_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_OS_FILE_MAPPING_H_

/**
 * @file
 * @brief 内存文件映射相关的类
 */

#include <ppl/config.h>

#include <ppl/util/log.h>
#include <boost/noncopyable.hpp>
#include <assert.h>



#if defined(_PPL_PLATFORM_MSWIN)

#include <ppl/mswin/windows.h>

namespace ppl { namespace os { 


// 封装文件映射操作的工具类
class file_mapping : private boost::noncopyable
{
public:
	file_mapping() : m_handle(NULL), m_view(NULL)
	{
	}
	~file_mapping()
	{
		close();
	}

	typedef HANDLE native_type;

	HANDLE native_handle() const	{ return m_handle; }
	void* get_view() const			{ return m_view; }
	bool is_open() const			{ return native_handle() != NULL; }
	bool is_mapped() const			{ return m_view != NULL; }

	bool create(HANDLE hFile, LPCTSTR name, DWORD size, DWORD protectFlag = PAGE_READWRITE)
	{
		assert(!is_open());
		this->close();
		//APP_DEBUG("Create File Mapping: size=" << size << ", name=" << (NULL == name ? "(null)" : name));
		m_handle = ::CreateFileMapping(hFile, NULL, protectFlag, 0, size, name);
		DWORD errcode = ::GetLastError();
		if ( false == is_open() )
		{
			APP_ERROR("CreateFileMapping failed " << make_tuple(errcode, size) << " " << (NULL == name ? "(null)" : name));
			return false;
		}
		assert( NO_ERROR == errcode || ERROR_ALREADY_EXISTS == errcode );
		if ( ERROR_ALREADY_EXISTS == errcode )
		{
			APP_ERROR( "CreateFileMapping failed already exists " << (NULL == name ? "(null)" : name) );
			this->close();
			return false;
		}
		return true;
	}

#if !defined(_WIN32_WCE)
	bool open(LPCTSTR name, DWORD access = FILE_MAP_READ | FILE_MAP_WRITE)
	{
		assert(name != NULL);
		assert(!is_open());
		this->close();
		APP_DEBUG("Open File Mapping: name=" << name);
		m_handle = ::OpenFileMapping(access, FALSE, name);
		if ( false == is_open())
		{
			DWORD errcode = ::GetLastError();
			APP_ERROR("OpenFileMapping failed " << make_tuple(errcode, access) << " " << name);
			return false;
		}
		return true;
	}
#endif

	void close()
	{
		unmap();
		if (is_open())
		{
			::CloseHandle(m_handle);
			m_handle = NULL;
		}
	}

	bool map_all(DWORD access = FILE_MAP_READ | FILE_MAP_WRITE)
	{
		return map(0, 0, access);
	}

	bool map(DWORD pos, DWORD size, DWORD access = FILE_MAP_READ | FILE_MAP_WRITE)
	{
		assert(is_open() && false == is_mapped());
		unmap();
		m_view = ::MapViewOfFile(native_handle(), access, 0, pos, size);
		if (is_mapped())
			return true;
		DWORD errcode = ::GetLastError();
		APP_ERROR("MapViewOfFile failed " << native_handle() << " " << errcode << " " << make_tuple(pos, size));
		return false;
	}
	void unmap()
	{
		if (is_mapped())
		{
			BOOL success = ::UnmapViewOfFile(m_view);
			if (!success)
			{
				int errcode = ::GetLastError();
				APP_ERROR("failed to unmap view, error code is " << errcode);
			}
			m_view = NULL;
		}
	}

private:
	HANDLE	m_handle;
	LPVOID	m_view;
};

} }

//#elif defined(_PPL_PLATFORM_LINUX)


#else

//#error file_mapping is not currently supported at your platform

#endif


#endif
