
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_IO_STDFILE_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_IO_STDFILE_H_

#include <ppl/data/int.h>
#include <ppl/data/tstring.h>
#include <ppl/config.h>
#include <ppl/util/log.h>
#include <boost/noncopyable.hpp>

#include <stdarg.h>

#if defined(_PPL_PLATFORM_MSWIN)
#include <share.h>
#else
inline FILE* _tfsopen(const char* fname, const char* mode, int shareFlag)
{
	return fopen(fname, mode);
}

inline FILE* _fsopen(const char* fname, const char* mode, int shareFlag)
{
	return fopen(fname, mode);
}

#define _SH_DENYRW      0x10    /* deny read/write mode */
#define _SH_DENYWR      0x20    /* deny write mode */
#define _SH_DENYRD      0x30    /* deny read mode */
#define _SH_DENYNO      0x40    /* deny none mode */
#define _SH_SECURE      0x80    /* secure mode */

#endif


namespace ppl { namespace io {


/// ��׼C���ļ�������װ��
class stdfile : private boost::noncopyable
{
public:
	typedef FILE* native_type;

	stdfile() : m_handle(NULL)
	{
	}
	~stdfile()
	{
		close();
	}

	/// ��ȡ�ļ����
	FILE* native_handle() const { return m_handle; }

#if defined(_PPL_PLATFORM_MSWIN)
	longlong get_position() const
	{
		fpos_t pos = 0;
		if ( 0 == fgetpos( m_handle, &pos ) )
			return pos;
		LIVE_ASSERT(false);
		return 0;
	}
#elif defined(_PPL_PLATFORM_LINUX)
	ulonglong get_position() const
	{
                long pos=0;
                pos=ftell(m_handle);
                return pos;
//		fpos_t pos = { 0 };
//		if ( 0 == fgetpos( m_handle, &pos ) )
//			return pos.__pos;
//		LIVE_ASSERT(false);
//		return 0;
	}
#else
#error "invalid platform for fgetpos"
#endif


	/// �ļ�����Ƿ���Ч
	bool is_open() const { return m_handle != NULL; }

	/// �ر��ļ�
	void close()
	{
		if (is_open())
		{
			fclose(m_handle);
			m_handle = NULL;
		}
	}

	/// �Ƿ����ļ�ĩβ
	bool is_eof() const
	{
		return 0 != feof(m_handle);
	}

	bool open_descriptor( int fd, const char* mode = "r" )
	{
		LIVE_ASSERT(!is_open());
		this->close();
		m_handle = fdopen( fd, mode );
		if (is_open())
			return true;
		UTIL_DEBUG("fdopen failed " << " with " << fd << " for mode " << mode);
		return false;
	}

	/// ���ļ�
	bool open(const char* path, const char* mode = "r", int shareFlag = _SH_DENYNO)
	{
		LIVE_ASSERT(!is_open());
		this->close();
		m_handle = _fsopen(path, mode, shareFlag);
		if (is_open())
			return true;
		UTIL_DEBUG("fopen failed " << " with " << path << " for mode " << mode);
		return false;
	}

#if defined(_PPL_PLATFORM_MSWIN)
	bool open(const wchar_t* path, const wchar_t* mode = L"r", int shareFlag = _SH_DENYNO)
	{
		LIVE_ASSERT(!is_open());
		this->close();
		m_handle = _wfsopen(path, mode, shareFlag);
		if (is_open())
			return true;
		UTIL_DEBUG("fopen failed " << " with " << path << " for mode " << mode);
		return false;
	}
#endif

	int read_byte()
	{
		return fgetc(m_handle);
	}

	int write_byte( int ch )
	{
		return fputc(ch, m_handle);
	}

	/// ��ȡ����
	size_t read(void* buf, size_t size)
	{
		LIVE_ASSERT(buf != NULL && size > 0);
		LIVE_ASSERT(is_open());
		//if (!is_open())
		//{
		//	return 0;
		//}
		return fread(buf, 1, size, m_handle);
	}

	/// ��ȡ�е�������
	bool read_line(char* buf, int size)
	{
		LIVE_ASSERT(buf != NULL && size > 0);
		LIVE_ASSERT(is_open());
		//if (!is_open())
		//	return false;
		if (fgets(buf, size, m_handle) != NULL)
			return true;
		if (failed())
		{
			UTIL_DEBUG("fgets failed ");
		}
		return false;
	}

	/// ��ȡ�е��ַ���
	bool read_line(string& line)
	{
		LIVE_ASSERT(is_open());
		//if (!is_open())
		//	return false;
		const size_t max_size = 1024 * 4;
		char str[max_size + 1] = { 0 };
		if (fgets(str, max_size, m_handle) == NULL)
		{
			if (failed())
			{
				UTIL_DEBUG("fgets failed");
			}
			return false;
		}
		line = str;
		return true;
	}


#ifdef _GLIBCXX_USE_WCHAR_T
	/// ��ȡ�е�������
	bool read_line(wchar_t* buf, int size)
	{
		LIVE_ASSERT(buf != NULL && size > 0);
		LIVE_ASSERT(is_open());
		//if (!is_open())
		//	return false;
		if (fgetws(buf, size, m_handle) != NULL)
			return true;
		if (failed())
		{
			UTIL_DEBUG("fgets failed ");
		}
		return false;
	}

	/// ��ȡ�е��ַ���
	bool read_line(wstring& line)
	{
		LIVE_ASSERT(is_open());
		//if (!is_open())
		//	return false;
		const size_t max_size = 1024;
		wchar_t str[max_size + 1] = { 0 };
		if (fgetws(str, max_size, m_handle) == NULL)
		{
			if (failed())
			{
				UTIL_DEBUG("fgets failed");
			}
			return false;
		}
		line = str;
		return true;
	}
#endif

	/// д�뻺��������
	size_t write(const void* data, size_t size)
	{
		LIVE_ASSERT(data != NULL && size > 0);
		LIVE_ASSERT(is_open());
		//if (!is_open())
		//{
		//	return 0;
		//}
		return fwrite(data, 1, size, m_handle);
	}

	/// д���ַ���
	bool write(const char* str)
	{
		LIVE_ASSERT(str != NULL);
		LIVE_ASSERT(is_open());
		//if (!is_open())
		//{
		//	return false;
		//}
		return EOF != fputs(str, m_handle);
	}

	/// ���������ĸ�ʽ��д��
	int write_variant(const char* format, va_list argptr)
	{
		LIVE_ASSERT(format != NULL);
		LIVE_ASSERT(is_open());
		//if (!is_open())
		//{
		//	return 0;
		//}
		return vfprintf(m_handle, format, argptr);
	}

	/// ��ʽ����д��
	int write_format(const char* format, ...)
	{
		LIVE_ASSERT(format != NULL);
		va_list(args);
		va_start(args, format);
		int count = write_variant(format, args);
		va_end(args);
		return count;
	}

	/// д��ṹ������
	template <typename StructT>
	bool write_struct(const StructT& buffer)
	{
		size_t size = write(&buffer, sizeof(StructT));
		return size == sizeof(StructT);
	}

	/// ��ȡ�ṹ������
	template <typename StructT>
	bool read_struct(StructT& buffer)
	{
		size_t size = read(&buffer, sizeof(StructT));
		return size == sizeof(StructT);
	}

	bool seek(long offset, int origin = SEEK_SET)
	{
		return 0 == fseek(m_handle, offset, origin);
	}

	/// ˢ�»�����
	bool flush()
	{
		if (!is_open())
		{
			return false;
		}
		return EOF != fflush(m_handle);
	}

	/// �Ƿ�ʧ��
	bool failed()
	{
		LIVE_ASSERT(is_open());
		return ferror(m_handle) != 0;
	}

private:
	/// �ļ����
	FILE* m_handle;
};


/// ������ļ��ı�׼C�ļ�������װ��
class stdfile_reader : public stdfile
{
public:
	/// �Զ����Ʒ�ʽ��
	bool open_binary(const char* path, int shareFlag = _SH_DENYNO)
	{
		return this->open(path, "rb", shareFlag);
	}

	/// ���ı���ʽ��
	bool open_text(const char* path, int shareFlag = _SH_DENYNO)
	{
		return this->open(path, "w", shareFlag);
	}

#if defined(_PPL_PLATFORM_MSWIN)
	bool open_binary(const wchar_t* path, int shareFlag = _SH_DENYNO)
	{
		return this->open(path, L"rb", shareFlag);
	}
	bool open_text(const wchar_t* path, int shareFlag = _SH_DENYNO)
	{
		return this->open(path, L"r", shareFlag);
	}
#endif
};


/// ����д�ļ��ı�׼C�ļ�������װ��
class stdfile_writer : public stdfile
{
public:
	stdfile_writer() { }
	~stdfile_writer()
	{
		close();
	}

	void close()
	{
		if (is_open())
		{
			flush();
			stdfile::close();
		}
	}

	/// �Զ����Ʒ�ʽ��
	bool open_binary(const char* path, int shareFlag = _SH_DENYWR)
	{
		return this->open(path, "wb", shareFlag);
	}

	/// ���ı���ʽ��
	bool open_text(const char* path, int shareFlag = _SH_DENYWR)
	{
		return this->open(path, "w", shareFlag);
	}

	/// д���ļ�
	static bool write_binary(const char* path, const void* data, size_t size)
	{
		stdfile_writer file;
		if (!file.open_binary(path, _SH_DENYRW))
			return false;
		if (file.write(data, size) != size)
			return false;
		file.flush();
		return true;
	}

#if defined(_PPL_PLATFORM_MSWIN)
	bool open_binary(const wchar_t* path, int shareFlag = _SH_DENYWR)
	{
		return this->open(path, L"wb", shareFlag);
	}
	bool open_text(const wchar_t* path, int shareFlag = _SH_DENYWR)
	{
		return this->open(path, L"w", shareFlag);
	}
	static bool write_binary(const wchar_t* path, const void* data, size_t size)
	{
		stdfile_writer file;
		if (!file.open_binary(path, _SH_DENYRW))
			return false;
		if (file.write(data, size) != size)
			return false;
		file.flush();
		return true;
	}
#endif

};


} }



#endif
