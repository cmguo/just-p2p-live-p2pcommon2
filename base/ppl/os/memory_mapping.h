#ifndef _LIVE_P2PCOMMON2_BASE_PPL_OS_MEMORY_MAPPING_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_OS_MEMORY_MAPPING_H_

/**
 * @file
 * @brief 内存文件映射相关的类
 */


#include <ppl/config.h>
#include <ppl/os/file_mapping.h>
#include <boost/noncopyable.hpp>



#if defined(_PPL_PLATFORM_MSWIN)

namespace ppl { namespace os { 


/// 内存映射
class memory_mapping : private boost::noncopyable
{
public:
	memory_mapping()
	{
	}
	~memory_mapping()
	{
	}

	HANDLE native_handle() const	{ return m_impl.native_handle(); }
	void* get_view() const			{ return m_impl.get_view(); }
	bool is_open() const			{ return m_impl.is_open(); }
	bool is_mapped() const			{ return m_impl.is_mapped(); }

	bool create(LPCTSTR name, DWORD size, DWORD protectFlag = PAGE_READWRITE)
	{
		return m_impl.create(INVALID_HANDLE_VALUE, name, size, protectFlag);
	}

#if !defined(_WIN32_WCE)
	bool open(LPCTSTR name, DWORD access = FILE_MAP_READ | FILE_MAP_WRITE)
	{
		return m_impl.open(name, access);
	}
#endif

	void close()
	{
		m_impl.close();
	}

	bool map_all(DWORD access = FILE_MAP_READ | FILE_MAP_WRITE)
	{
		return m_impl.map_all(access);
	}

	bool map(DWORD pos, DWORD size, DWORD access = FILE_MAP_READ | FILE_MAP_WRITE)
	{
		return m_impl.map(pos, size, access);
	}
	void unmap()
	{
		m_impl.unmap();
	}

	static bool mapping_exists(const TCHAR* name)
	{
		memory_mapping mapping;
		if (mapping.open(name, FILE_MAP_READ))
			return true;
		return false;
	}

private:
	file_mapping m_impl;
};

} }


#else

#include <ppl/data/buffer.h>

namespace ppl { namespace os { 

/// 伪内存映射，使用内存实现，仅用于部分情况下模拟内存映射的接口
class fake_memory_mapping : private boost::noncopyable
{
public:
	fake_memory_mapping()
	{
	}

	bool create(LPCTSTR name, DWORD size)
	{
		LIVE_ASSERT( size < 1 * 1024 * 1024 );
		m_buffer.resize(size);
		return true;
	}
    bool map_all()
	{
		return true;
	}

	void close()
	{
	}

	void* get_view() { return m_buffer.data(); }
	bool is_open() const { return false == m_buffer.empty(); }
	bool is_mapped() const { return this->is_open(); }


private:
	byte_buffer m_buffer;
};

class memory_mapping : public fake_memory_mapping
{
};

} }

#endif

#endif
