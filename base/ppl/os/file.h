
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_OS_FILE_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_OS_FILE_H_


#include <ppl/config.h>

#include <ppl/diag/trace.h>
#include <boost/noncopyable.hpp>


#if defined(_PPL_PLATFORM_MSWIN)

namespace ppl { namespace os { 


/// win32的文件操作类(使用操作系统的接口)
class file : private boost::noncopyable
{
public:
	typedef HANDLE native_type;

	explicit file(HANDLE hFile = INVALID_HANDLE_VALUE) : m_handle(hFile)
	{
	}
	~file()
	{
		close();
	}

	/// 获取文件句柄
	HANDLE native_handle() const { return m_handle; }

	/// 文件句柄是否有效
	bool is_open() const { return m_handle != INVALID_HANDLE_VALUE; }

	/// 关闭文件
	void close()
	{
		if (is_open())
		{
			BOOL success = ::CloseHandle(m_handle);
			m_handle = INVALID_HANDLE_VALUE;
			if (!success)
			{
				TRACEOUT("File::Close CloseHandle failed with error %d\n", ::GetLastError());
				LIVE_ASSERT(false);
			}
		}
	}

	/// 打开文件
	bool open(LPCTSTR path, DWORD access, DWORD shareMode, DWORD creationDisposition)
	{
		LIVE_ASSERT(!is_open());
		close();
		m_handle = ::CreateFile(path, access, shareMode, NULL, creationDisposition, FILE_ATTRIBUTE_NORMAL, NULL);
		if (is_open())
			return true;
		// since file maybe used in logger, use TRACE instead of LOG
		TRACE("file::open CreateFile %s failed with error %d\n", path, ::GetLastError());
		return false;
	}

	/// 打开文件供读取
	bool open_reading(LPCTSTR path, DWORD shareMode = FILE_SHARE_READ | FILE_SHARE_WRITE, DWORD creationDisposition = OPEN_EXISTING)
	{
		return this->open(path, GENERIC_READ, shareMode, creationDisposition);
	}

	/// 打开文件供写入
	bool open_writing(LPCTSTR path, DWORD shareMode = FILE_SHARE_READ, DWORD creationDisposition = OPEN_ALWAYS)
	{
		return this->open(path, GENERIC_WRITE, shareMode, creationDisposition);
	}

	/// 打开文件供追加数据
	bool open_appending(LPCTSTR path, DWORD shareMode = 0, DWORD creationDisposition = OPEN_ALWAYS)
	{
		return open_writing(path, shareMode, creationDisposition) && seek_from_end(0);
	}

	/// 读取数据
	size_t read(void* buffer, size_t size)
	{
		LIVE_ASSERT(buffer != NULL && size > 0);
		LIVE_ASSERT(is_open());
		DWORD bytes = 0;
		if (::ReadFile(m_handle, buffer, size, &bytes, NULL))
		{
			//LIVE_ASSERT(bytes > 0);
			return bytes;
		}
		TRACE("file::read ReadFile failed. size=%u, bytes=%u error=%d\n", size, bytes, ::GetLastError());
		return 0;
	}

	/// 读取指定长度的数据，如果读到的数据不够，则失败
	bool read_n(void* buffer, size_t size)
	{
		size_t len = read(buffer, size);
		LIVE_ASSERT(len <= size);
		return len == size;
	}

	/// 写入缓冲区数据
	size_t write(const void* data, size_t size)
	{
		DWORD count = 0;
		if (!::WriteFile(native_handle(), data, size, &count, NULL))
			return 0;
		return count;
	}

	/// 写入结构体数据
	template <typename StructT>
	bool write_struct(const StructT& buffer)
	{
		size_t size = write(&buffer, sizeof(StructT));
		LIVE_ASSERT(size <= sizeof(StructT));
		return size == sizeof(StructT);
	}

	/// 读取结构体数据
	template <typename StructT>
	bool read_struct(StructT& buffer)
	{
		return read_n(&buffer, sizeof(StructT));
	}

	unsigned char read_byte()
	{
		unsigned char ch = 0;
		size_t bytes = read(&ch, 1);
		LIVE_ASSERT(bytes == 1);
		return ch;
	}


	/// 可变参数的写数据
	bool write_variant(const TCHAR* format, va_list args)
	{
		const int max_size = 8 * 1024;
		TCHAR buffer[max_size + 1];
		buffer[max_size] = '\0';
		int count = _vsntprintf(buffer, max_size, format, args);
		if (count <= 0)
		{
			LIVE_ASSERT(false);
			return false;
		}
		return write(buffer, count) == static_cast<size_t>( count );
	}

	/// 可变参数的写数据
	bool write_format(const TCHAR* format, ...)
	{
		va_list args;
		va_start(args, format);
		bool success = write_variant(format, args);
		va_end(args);
		return success;
	}

	/// 刷新缓冲区
	bool flush()
	{
		return ::FlushFileBuffers(native_handle()) != FALSE;
	}

	bool seek(LONG distance, DWORD moveMethod)
	{
#ifndef INVALID_SET_FILE_POINTER
		const DWORD INVALID_SET_FILE_POINTER = -1;
#endif

		LIVE_ASSERT(is_open());
		DWORD result = ::SetFilePointer(m_handle, distance, NULL, moveMethod);
		if (result == INVALID_SET_FILE_POINTER)
		{
			DWORD errcode = ::GetLastError();
			TRACE("file::seek SetFilePointer failed, result=%u,error=%d,distance=%d,move=%u\n", result, errcode, distance, moveMethod);
			LIVE_ASSERT(errcode != ERROR_SUCCESS);
			return false;
		}
		return true;
	}
	bool seek_from_beginning(long distance = 0)
	{
		return seek(distance, FILE_BEGIN);
	}
	bool seek_from_end(long distance = 0)
	{
		return seek(distance, FILE_END);
	}
	bool seek_offset(long distance)
	{
		return seek(distance, FILE_CURRENT);
	}

	DWORD get_size32()
	{
		return ::GetFileSize(m_handle, NULL);
	}

	INT64 get_size()
	{
		return get_size64();
	}

	INT64 get_size64()
	{
		LARGE_INTEGER size;
		size.QuadPart = 0;
		if ( ::GetFileSizeEx( m_handle, &size ) )
		{
			LIVE_ASSERT( size.QuadPart >= 0 );
			return size.QuadPart;
		}
		LIVE_ASSERT(false);
		return 0;
	}

private:
	/// 文件句柄
	HANDLE m_handle;
};


} }

//#elif defined(_PPL_PLATFORM_POSIX)

#else

#error invalid file implementation

#endif


#endif