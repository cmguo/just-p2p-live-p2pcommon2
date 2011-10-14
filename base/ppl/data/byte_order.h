
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_DATA_BYTE_ORDER_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_DATA_BYTE_ORDER_H_

#include <ppl/data/int.h>
#include <ppl/data/bit_converter.h>

#include <boost/detail/endian.hpp>
#include <assert.h>

#if defined(BOOST_BIG_ENDIAN)
const bool ppl_host_byte_order_is_big_endian = true;
#elif defined(BOOST_LITTLE_ENDIAN)
const bool ppl_host_byte_order_is_big_endian = false;
#else
#error invalid platform for boost endian!!!!!!
#endif



/// 字节序转换类
class byte_order
{
public:
	/// 反转字节序
	static UINT16 reverse_word( UINT16 val )
	{
		//UINT16 result;
		//BYTE* dest = reinterpret_cast<BYTE*>( &result );
		//const BYTE* src = reinterpret_cast<const BYTE*>( &val );
		//dest[0] = src[1];
		//dest[1] = src[0];
		//return result;
		return ((val & 0xff) << 8) | ((val & 0xff00) >> 8);
	}

	/// 反转字节序
	static UINT32 reverse_dword( UINT32 n )
	{
		//UINT32 result;
		//BYTE* dest = reinterpret_cast<BYTE*>( &result );
		//const BYTE* src = reinterpret_cast<const BYTE*>( &val );
		//dest[0] = src[3];
		//dest[1] = src[2];
		//dest[2] = src[1];
		//dest[3] = src[0];
		//return result;
		return ((n & 0xff) << 24) 
			 | ((n & 0xff00) << 8) 
			 | ((n & 0xff0000UL) >> 8) 
			 | ((n & 0xff000000UL) >> 24);
	}

	/// 反转字节序
	static UINT64 reverse_qword( UINT64 x )
	{
		//UINT64 result;
		//BYTE* dest = reinterpret_cast<BYTE*>( &result );
		//const BYTE* src = reinterpret_cast<const BYTE*>( &val );
		//dest[0] = src[7];
		//dest[1] = src[6];
		//dest[2] = src[5];
		//dest[3] = src[4];
		//dest[4] = src[3];
		//dest[5] = src[2];
		//dest[6] = src[1];
		//dest[7] = src[0];
		//return result;
		// 高低部分转换
		UINT32 low = reverse_dword( bit_converter::high_dword( x ) );
		UINT32 high = reverse_dword( bit_converter::low_dword( x ) );
		return bit_converter::make_qword( high, low );
	}


	byte_order( bool isBigEndian ) : m_big_endian( isBigEndian )
	{
		static host_byte_order_checker _checker;
	}

	void set_big_endian( bool isBigEndian )
	{
		m_big_endian = isBigEndian;
	}
	bool is_big_endian() const
	{
		return m_big_endian;
	}

	/// 根据需要转换单字（16位整数）字节序
	UINT16 convert_word( UINT16 val )
	{
		if ( ppl_host_byte_order_is_big_endian == m_big_endian )
			return val;
		return byte_order::reverse_word( val );
	}

	/// 根据需要转换双字（32位整数）字节序
	UINT32 convert_dword( UINT32 val )
	{
		if ( ppl_host_byte_order_is_big_endian == m_big_endian )
			return val;
		return byte_order::reverse_dword( val );
	}

	/// 根据需要转换4字（64位整数）字节序
	UINT64 convert_qword( UINT64 val )
	{
		if ( ppl_host_byte_order_is_big_endian == m_big_endian )
			return val;
		return byte_order::reverse_qword( val );
	}

private:
	class host_byte_order_checker
	{
	public:
		host_byte_order_checker()
		{
			LIVE_ASSERT( ppl_host_byte_order_is_big_endian == big_endian() );
		}
		static bool big_endian()
		{
			short tester = 0x0201;
			return  *(char*)&tester==2;
		}
	};

	bool m_big_endian;
};


#ifdef _PPL_RUN_TEST

#include <ppl/util/test_case.h>

class byte_order_test_case : public ppl::util::test_case
{
protected:
	virtual void do_run()
	{
		test_reverse();
		test_convert();
	}
	void test_reverse()
	{
		LIVE_ASSERT(byte_order::reverse_word(0x1234) == 0x3412);
		LIVE_ASSERT(byte_order::reverse_dword(0x12345678UL) == 0x78563412UL);
#if defined(_MSC_VER) && _MSC_VER < 1300
		LIVE_ASSERT(byte_order::reverse_qword(0x1234567890abcdefUI64) == 0xefcdab9078563412UI64);
#else
		LIVE_ASSERT(byte_order::reverse_qword(0x1234567890abcdefULL) == 0xefcdab9078563412ULL);
#endif
	}

	void test_convert()
	{
		if ( ppl_host_byte_order_is_big_endian )
		{
			{
				byte_order hbc(true);
				LIVE_ASSERT(hbc.convert_word(0x1234) == 0x1234);
				LIVE_ASSERT(hbc.convert_dword(0x12345678UL) == 0x12345678UL);
#if defined(_MSC_VER) && _MSC_VER < 1300
				LIVE_ASSERT(hbc.convert_qword(0x1234567890abcdefUI64) == 0x1234567890abcdefUI64);
#else
				LIVE_ASSERT(hbc.convert_qword(0x1234567890abcdefULL) == 0x1234567890abcdefULL);
#endif
			}
			{
				byte_order hbc(false);
				LIVE_ASSERT(hbc.convert_word(0x1234) == 0x3412);
				LIVE_ASSERT(hbc.convert_dword(0x12345678UL) == 0x78563412UL);
#if defined(_MSC_VER) && _MSC_VER < 1300
				LIVE_ASSERT(hbc.convert_qword(0x1234567890abcdefUI64) == 0xefcdab9078563412UI64);
#else
				LIVE_ASSERT(hbc.convert_qword(0x1234567890abcdefULL) == 0xefcdab9078563412ULL);
#endif
			}
		}
		else
		{
			{
				byte_order hbc(false);
				LIVE_ASSERT(hbc.convert_word(0x1234) == 0x1234);
				LIVE_ASSERT(hbc.convert_dword(0x12345678UL) == 0x12345678UL);
#if defined(_MSC_VER) && _MSC_VER < 1300
				LIVE_ASSERT(hbc.convert_qword(0x1234567890abcdefUI64) == 0x1234567890abcdefUI64);
#else
				LIVE_ASSERT(hbc.convert_qword(0x1234567890abcdefULL) == 0x1234567890abcdefULL);
#endif
			}
			{
				byte_order hbc(true);
				LIVE_ASSERT(hbc.convert_word(0x1234) == 0x3412);
				LIVE_ASSERT(hbc.convert_dword(0x12345678UL) == 0x78563412UL);
#if defined(_MSC_VER) && _MSC_VER < 1300
				LIVE_ASSERT(hbc.convert_qword(0x1234567890abcdefUI64) == 0xefcdab9078563412UI64);
#else
				LIVE_ASSERT(hbc.convert_qword(0x1234567890abcdefULL) == 0xefcdab9078563412ULL);
#endif
			}
		}
	}
};

CPPUNIT_TEST_SUITE_REGISTRATION(byte_order_test_case);

#endif

#endif
