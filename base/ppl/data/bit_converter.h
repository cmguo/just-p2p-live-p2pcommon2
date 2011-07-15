
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_DATA_BIT_CONVERTER_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_DATA_BIT_CONVERTER_H_

#include <ppl/data/int.h>


/// 数据的解析和组装类
class bit_converter
{
public:
	static UINT16 make_word( BYTE high, BYTE low )
	{
		return ( static_cast<UINT16>( high ) << 8 ) | static_cast<UINT16>( low );
	}
	static BYTE high_byte( UINT16 x )
	{
		return static_cast<UINT8>( x >> 8 );
	}
	static BYTE low_byte( UINT16 x )
	{
		return static_cast<UINT8>( x );
	}

	static UINT32 make_dword( UINT16 high, UINT16 low )
	{
		return ( static_cast<UINT32>( high ) << 16 ) | static_cast<UINT32>( low );
	}
	static UINT16 high_word( UINT32 x )
	{
		return static_cast<UINT16>( x >> 16 );
	}
	static UINT16 low_word( UINT32 x )
	{
		return static_cast<UINT16>( x );
	}

	static UINT64 make_qword( UINT32 high, UINT32 low )
	{
		return ( static_cast<UINT64>( high ) << 32 ) | static_cast<UINT64>( low );
	}
	static UINT32 high_dword( UINT64 x )
	{
		return static_cast<UINT32>( x >> 32 );
	}
	static UINT32 low_dword( UINT64 x )
	{
		return static_cast<UINT32>( x );
	}

	/// 大头的转换器
	static bit_converter& big_endian()
	{
		static bit_converter converter( true );
		return converter;
	}
	/// 小头的转换器
	static bit_converter& little_endian()
	{
		static bit_converter converter( false );
		return converter;
	}

	bit_converter( bool isBigEndian ) : m_big_endian( isBigEndian )
	{
	}

	void set_big_endian( bool isBigEndian )
	{
		m_big_endian = isBigEndian;
	}
	bool is_big_endian() const
	{
		return m_big_endian;
	}

	UINT16 to_word( const char* data )
	{
		return to_word( reinterpret_cast<const BYTE*>( data ) );
	}
	UINT32 to_dword( const char* data )
	{
		return to_dword( reinterpret_cast<const BYTE*>( data ) );
	}
	UINT64 to_qword( const char* data )
	{
		return to_qword( reinterpret_cast<const BYTE*>( data ) );
	}

	UINT16 to_word( const BYTE* data )
	{
		assert( data );
		if ( m_big_endian )
		{
			return  ( static_cast<UINT16>( data[0] ) << 8 ) | 
					  static_cast<UINT16>( data[1] );
		}
		return ( static_cast<UINT16>( data[1] ) << 8 ) | 
				 static_cast<UINT16>( data[0] );
	}

	UINT32 to_dword( const BYTE* data )
	{
		assert( data );
		if ( m_big_endian )
		{
			return  ( static_cast<UINT32>( data[0] ) << 24 ) | 
					( static_cast<UINT32>( data[1] ) << 16 ) | 
					( static_cast<UINT32>( data[2] ) << 8 ) | 
					  static_cast<UINT32>( data[3] );
		}
		return  ( static_cast<UINT32>( data[3] ) << 24 ) | 
				( static_cast<UINT32>( data[2] ) << 16 ) | 
				( static_cast<UINT32>( data[1] ) << 8 ) | 
				  static_cast<UINT32>( data[0] );
	}

	UINT64 to_qword( const BYTE* data )
	{
		assert( data );
		if ( m_big_endian )
		{
			return  ( static_cast<UINT64>( data[0] ) << 56 ) | 
					( static_cast<UINT64>( data[1] ) << 48 ) | 
					( static_cast<UINT64>( data[2] ) << 40 ) | 
					( static_cast<UINT64>( data[3] ) << 32 ) | 
					( static_cast<UINT64>( data[4] ) << 24 ) | 
					( static_cast<UINT64>( data[5] ) << 16 ) | 
					( static_cast<UINT64>( data[6] ) << 8 ) | 
					  static_cast<UINT64>( data[7] );
		}
		return  ( static_cast<UINT64>( data[7] ) << 56 ) | 
				( static_cast<UINT64>( data[6] ) << 48 ) | 
				( static_cast<UINT64>( data[5] ) << 40 ) | 
				( static_cast<UINT64>( data[4] ) << 32 ) | 
				( static_cast<UINT64>( data[3] ) << 24 ) | 
				( static_cast<UINT64>( data[2] ) << 16 ) | 
				( static_cast<UINT64>( data[1] ) << 8 ) | 
				  static_cast<UINT64>( data[0] );
	}

	void from_word( UINT16 x, char* buf )
	{
		from_word( x, reinterpret_cast<BYTE*>( buf ) );
	}
	void from_dword( UINT32 x, char* buf )
	{
		from_dword( x, reinterpret_cast<BYTE*>( buf ) );
	}
	void from_qword( UINT64 x, char* buf )
	{
		from_qword( x, reinterpret_cast<BYTE*>( buf ) );
	}

	void from_word( UINT16 x, BYTE* buf )
	{
		assert(buf);
		if ( m_big_endian )
		{
			buf[0] = high_byte( x );
			buf[1] = low_byte( x );
		}
		else
		{
			buf[1] = high_byte( x );
			buf[0] = low_byte( x );
		}
	}
	void from_dword( UINT32 x, BYTE* buf )
	{
		assert(buf);
		if ( m_big_endian )
		{
			buf[0] = static_cast<BYTE>( ( x & 0xFF000000UL ) >> 24 );
			buf[1] = static_cast<BYTE>( ( x & 0xFF0000UL ) >> 16 );
			buf[2] = static_cast<BYTE>( ( x & 0xFF00UL ) >> 8 );
			buf[3] = static_cast<BYTE>( x & 0xFFUL );
		}
		else
		{
			buf[3] = static_cast<BYTE>( ( x & 0xFF000000 ) >> 24 );
			buf[2] = static_cast<BYTE>( ( x & 0xFF0000 ) >> 16 );
			buf[1] = static_cast<BYTE>( ( x & 0xFF00 ) >> 8 );
			buf[0] = static_cast<BYTE>( x & 0xFF );
		}
	}
	void from_qword( UINT64 x, BYTE* buf )
	{
		assert(buf);
		UINT32 h = high_dword( x );
		UINT32 l = low_dword( x );
		if ( m_big_endian )
		{
			buf[0] = static_cast<BYTE>( ( h & 0xFF000000UL ) >> 24 );
			buf[1] = static_cast<BYTE>( ( h & 0xFF0000UL ) >> 16 );
			buf[2] = static_cast<BYTE>( ( h & 0xFF00UL ) >> 8 );
			buf[3] = static_cast<BYTE>( h & 0xFFUL );
			buf[4] = static_cast<BYTE>( ( l & 0xFF000000UL ) >> 24 );
			buf[5] = static_cast<BYTE>( ( l & 0xFF0000UL ) >> 16 );
			buf[6] = static_cast<BYTE>( ( l & 0xFF00UL ) >> 8 );
			buf[7] = static_cast<BYTE>( l & 0xFFUL );
		}
		else
		{
			buf[7] = static_cast<BYTE>( ( h & 0xFF000000UL ) >> 24 );
			buf[6] = static_cast<BYTE>( ( h & 0xFF0000UL ) >> 16 );
			buf[5] = static_cast<BYTE>( ( h & 0xFF00UL ) >> 8 );
			buf[4] = static_cast<BYTE>( h & 0xFFUL );
			buf[3] = static_cast<BYTE>( ( l & 0xFF000000UL ) >> 24 );
			buf[2] = static_cast<BYTE>( ( l & 0xFF0000UL ) >> 16 );
			buf[1] = static_cast<BYTE>( ( l & 0xFF00UL ) >> 8 );
			buf[0] = static_cast<BYTE>( l & 0xFFUL );
		}
	}

private:
	bool m_big_endian;
};


#ifdef _PPL_RUN_TEST

#include <ppl/util/test_case.h>

class bit_converter_test_case : public ppl::util::test_case
{
protected:
	virtual void do_run()
	{
		{
			bit_converter bc( true );
			assert( bc.to_word("\x12\x33") != 0x1234 );
			assert( bc.to_word("\x12\x34") == 0x1234 );
			assert( bc.to_dword("\x12\x34\x56\x18") != 0x12345678 );
			assert( bc.to_dword("\x12\x34\x56\x78") == 0x12345678 );

			BYTE buf[4];
			bc.from_word( 0x1234, buf );
			assert( memcmp("\x12\x34", buf, 2 ) == 0 );
			bc.from_dword( 0x12345678, buf );
			assert( memcmp("\x12\x34\x56\x78", buf, 4 ) == 0 );
		}
		{
			bit_converter bc( false );
			assert( bc.to_word("\x12\x31") != 0x3412 );
			assert( bc.to_word("\x12\x34") == 0x3412 );
			assert( bc.to_dword("\x12\x34\x56\x71") != 0x78563412 );
			assert( bc.to_dword("\x12\x34\x56\x78") == 0x78563412 );

			BYTE buf[4];
			bc.from_word( 0x3412, buf );
			assert( memcmp("\x12\x34", buf, 2 ) == 0 );
			bc.from_dword( 0x78563412, buf );
			assert( memcmp("\x12\x34\x56\x78", buf, 4 ) == 0 );
		}
		assert(bit_converter::make_word(0x34, 0x12) == 0x3412);
		assert(bit_converter::low_byte(0x3412) == 0x12);
		assert(bit_converter::high_byte(0x3412) == 0x34);
		assert(bit_converter::make_dword(0x5678, 0x1234) == 0x56781234UL);
		assert(bit_converter::low_word(0x56781234UL) == 0x1234);
		assert(bit_converter::high_word(0x56781234UL) == 0x5678);

#if defined(_MSC_VER) && _MSC_VER < 1300
		assert(bit_converter::make_qword(0x90abcdefUL, 0x12345678UL) == 0x90abcdef12345678UI64);
		assert(bit_converter::low_dword(0x90abcdef12345678UI64) == 0x12345678UL);
		assert(bit_converter::high_dword(0x90abcdef12345678UI64) == 0x90abcdefUL);
		{
			bit_converter bc( true );
			assert( bc.to_qword("\x12\x34\x56\x78\x90\xab\xcd\xef") == 0x1234567890abcdefUI64 );
			BYTE buf[8];
			bc.from_qword( 0x1234567890abcdefUI64, buf );
			assert( memcmp("\x12\x34\x56\x78\x90\xab\xcd\xef", buf, 8 ) == 0 );
		}
		{
			bit_converter bc( false );
			assert( bc.to_qword("\x12\x34\x56\x78\x90\xab\xcd\xef") == 0xefcdab9078563412UI64 );
			BYTE buf[8];
			bc.from_qword( 0xefcdab9078563412UI64, buf );
			assert( memcmp("\x12\x34\x56\x78\x90\xab\xcd\xef", buf, 8 ) == 0 );
		}
#else
		assert(bit_converter::make_qword(0x90abcdefUL, 0x12345678UL) == 0x90abcdef12345678ULL);
		assert(bit_converter::low_dword(0x90abcdef12345678UI64) == 0x12345678UL);
		assert(bit_converter::high_dword(0x90abcdef12345678UI64) == 0x90abcdefUL);
		{
			bit_converter bc( true );
			assert( bc.to_qword("\x12\x34\x56\x78\x90\xab\xcd\xef") == 0x1234567890abcdefULL );
			BYTE buf[8];
			bc.from_qword( 0x1234567890abcdefULL, buf );
			assert( memcmp("\x12\x34\x56\x78\x90\xab\xcd\xef", buf, 8 ) == 0 );
		}
		{
			bit_converter bc( false );
			assert( bc.to_qword("\x12\x34\x56\x78\x90\xab\xcd\xef") == 0xefcdab9078563412ULL );
			BYTE buf[8];
			bc.from_qword( 0xefcdab9078563412ULL, buf );
			assert( memcmp("\x12\x34\x56\x78\x90\xab\xcd\xef", buf, 8 ) == 0 );
		}
#endif
	}
};

CPPUNIT_TEST_SUITE_REGISTRATION(bit_converter_test_case);


#endif


#endif