
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_DATA_BUFFER_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_DATA_BUFFER_H_

#include <ppl/config.h>
#include <ppl/data/alloc.h>

#include <boost/noncopyable.hpp>
#include <algorithm>


template <typename T, typename AllocT>
class basic_buffer : private boost::noncopyable
{
public:
	typedef T element_type;
	typedef AllocT alloc_type;
	typedef basic_buffer<T, AllocT> this_type;

	typedef element_type* iterator;
	typedef element_type* const_iterator;

	enum { max_buffer_size = 32 * 1024 * 1024 + 1 };

	basic_buffer() : m_data(NULL), m_capacity(0), m_size(0)
	{
	}
	explicit basic_buffer(const T* data, size_t size) : m_capacity(size), m_size(size)
	{
		if ( size > 0 )
		{
			m_data = static_cast<element_type*>(AllocT::allocate(size * sizeof(element_type)));
			memcpy( m_data, data, size );
		}
		else
		{
			m_data = NULL;
		}
	}
	explicit basic_buffer(size_t size) : m_capacity(size), m_size(size)
	{
		if ( size > 0 )
		{
			m_data = static_cast<element_type*>(AllocT::allocate(size * sizeof(element_type)));
		}
		else
		{
			m_data = NULL;
		}
	}
	explicit basic_buffer(size_t size, const T& initialVal) : m_capacity(size), m_size(size)
	{
		if ( m_size > 0 )
		{
			m_data = static_cast<element_type*>(AllocT::allocate(m_size * sizeof(element_type)));
			std::fill_n( m_data, m_size, initialVal );
		}
		else
		{
			m_data = NULL;
		}
	}
	~basic_buffer()
	{
		if (m_data != NULL)
		{
			AllocT::deallocate(m_data);
			m_data = NULL;
			m_capacity = 0;
			m_size = 0;
		}
		else
		{
			assert(m_capacity == 0 && m_size == 0);
		}
	}

	/// 预留空间(原来的数据保留)
	void reserve(size_t size)
	{
		assert(size < max_buffer_size);
		assert(m_size <= m_capacity);
		if (m_capacity >= size)
			return;
		this_type newBuffer(size);
		newBuffer.resize(m_size);
		if (m_size > 0)
		{
			memcpy(newBuffer.m_data, m_data, m_size);
		}
		this->swap(newBuffer);
	}

	/// 调整大小，原有数据保留，新数据内容未知
	void resize(size_t size)
	{
		reserve(size);
		assert(m_capacity >= size);
		m_size = size;
	}
	/// 保证大小为指定长度，不保证数据有效(原来的数据可能丢失)
	void ensure_size(size_t size)
	{
		resize(0);
		resize(size);
	}

	void assign( const element_type& elem )
	{
		if ( m_data && m_size > 0 )
		{
			std::fill_n( m_data, m_size, elem );
		}
	}

	void assign( const this_type& src )
	{
		this->assign( src.data(), src.size() );
	}

	/// 设置数据
	void assign(const element_type* src, size_t size)
	{
		if (size == 0)
		{
			this->resize(0);
			return;
		}
		assert(src != NULL && size > 0);
		assert(!::IsBadReadPtr(src, size));
		resize(size);
		memcpy(m_data, src, size);
		assert(m_size == size);
		assert(m_capacity >= size);
	}

	void append(const element_type* src, size_t size)
	{
		size_t oldSize = m_size;
		resize(oldSize + size);
		memcpy(m_data + oldSize, src, size * sizeof(element_type));
	}

	void append(size_t size, const element_type& elem)
	{
		size_t oldSize = m_size;
		resize(oldSize + size);
		std::fill_n(m_data + oldSize, elem, size);
	}

	void append(const this_type& src)
	{
		this->append(src.data(), src.size());
	}

	/// 释放资源
	void clear()
	{
		this_type buf;
		this->swap(buf);
	}

	void swap(this_type& b)
	{
		std::swap(m_data, b.m_data);
		std::swap(m_capacity, b.m_capacity);
		std::swap(m_size, b.m_size);
	}

	/// 获取数据大小
	size_t size() const { return m_size; }

	/// 获取容量大小
	size_t capacity() const { return m_capacity; }

	/// 是否为空
	bool empty() const { return m_data == 0; }

	/// 获取可写的缓冲区
	element_type* data() { return m_data; }

	/// 获取只读的缓冲区
	const element_type* data() const { return m_data; }


	element_type* begin() { return m_data; }
	const element_type* begin() const { return m_data; }
	element_type* end() { return m_data + m_size; }
	const element_type* end() const { return m_data + m_size; }

	/// 索引操作符
	element_type operator[](size_t index) const { assert(!empty()); assert(index < m_size); return m_data[index]; }
	/// 索引操作符
	element_type& operator[](size_t index) { assert(!empty()); assert(index < m_size); return m_data[index]; }


private:
	/// 缓冲区
	element_type* m_data;

	/// 容量大小
	size_t m_capacity;

	/// 有效数据大小
	size_t m_size;
};


template<typename T, typename AllocT> inline void swap(basic_buffer<T, AllocT> & a, basic_buffer<T, AllocT> & b) // never throws
{
    a.swap(b);
}

//typedef basic_buffer<char, pool_alloc> dynamic_buffer;
typedef basic_buffer<char, malloc_alloc> char_buffer;
typedef basic_buffer<unsigned char, malloc_alloc> byte_buffer;





/*
#ifdef _PPL_RUN_TEST

class DynamicBufferTestCase : public TestCase
{
public:
	virtual void DoRun()
	{
		dynamic_buffer buf;
		assert(buf.empty());
		CheckBuffer(buf, 0, 0, true);

		buf.reserve(1);
		CheckBuffer(buf, 1, 0);

		buf.resize(1);
		CheckBuffer(buf, 1, 1);

		buf.reserve(2);
		CheckBuffer(buf, 2, 1);

		buf.resize(2);
		CheckBuffer(buf, 2, 2);

		buf.resize(3);
		CheckBuffer(buf, 3, 3);

		buf.resize(19);
		CheckBuffer(buf, 19, 19);

		string str("Hello");
		buf.assign(str.data(), str.size());
		CheckBuffer(buf, 19, str.size());
		assert(0 == memcmp(buf.data(), str.data(), str.size()));

		str = "111222333444555666777888999abcdefghijklmnopq";
		buf.assign(str.data(), str.size());
		CheckBuffer(buf, str.size(), str.size());
		assert(0 == memcmp(buf.data(), str.data(), str.size()));

		buf.resize(64 * 1000);
		CheckBuffer(buf, 64 * 1000, 64 * 1000);
		
		dynamic_buffer buf2(110);
		assert(!buf2.empty());
		assert(buf2.data() != NULL);
		assert(buf2.capacity() == 110);
		assert(buf2.size() == 110);
	}

	void CheckBuffer(const dynamic_buffer& buf, size_t capacity, size_t size, bool isNull = false)
	{
		assert(buf.capacity() == capacity);
		if (buf.size() != size)
		{
			assert(false);
		}
		assert(buf.size() == size);
		if (isNull)
		{
			assert(buf.data() == NULL);
		}
		else
		{
			assert(buf.data() != NULL);
		}
	}
};

CPPUNIT_TEST_SUITE_REGISTRATION(DynamicBufferTestCase);

#endif
*/

#endif
