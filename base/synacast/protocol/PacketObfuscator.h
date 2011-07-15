
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_PACKET_OBFUSCATOR_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_PACKET_OBFUSCATOR_H_

#include <ppl/data/int.h>
#include <ppl/util/macro.h>
#include <ppl/util/log.h>


/**
* @file
* @brief 报文混淆器
*/


class ChecksumCalculator
{
public:
	static std::pair<BYTE, WORD> Calc(const BYTE* buf, size_t size )
	{
		unsigned long sum = 0;

		while( size > 1 )
		{
			//*  This is the inner loop 
			//sum += READ_MEMORY( buf, WORD );
                        sum += ((WORD)buf[0] | ((WORD)buf[1] << 8));
			buf += 2;
			size -= 2;
		}

		//*  Add left-over byte, if any 
		if( size > 0 )
			//sum += READ_MEMORY( buf, BYTE );
                        sum += buf[0];

		BYTE firstByte = static_cast<BYTE>( ( sum & 0x00FF0000 ) >> 16 );
		//*  Fold 32-bit sum to 16 bits 
		while (sum>>16)
			sum = (sum & 0xffff) + (sum >> 16);

		WORD checksum = static_cast<WORD>(~sum);
		return std::make_pair( firstByte, checksum );
	}
};

class PacketObfuscator
{
public:
	/// 计算需要填补的长度
	static UINT CalcPaddingLength( BYTE checksum )
	{
		BYTE val = checksum ^ 0x49;
		val |= 0x01;
		return static_cast<UINT>( val & 0x07 );
	}
	/// 解混淆
	static int UnObfuscate( BYTE* data, size_t size )
	{
		if ( size < 3 )
			return -1;
		//WORD firstWord = READ_MEMORY( data, WORD );
		BYTE firstByte = *data;
		WORD checksumWord = READ_MEMORY( data + 1, WORD );
		DWORD len = CalcPaddingLength( firstByte );
		assert( len == 1 || len == 3 || len == 5 || len == 7 );
		len += 3;
		if ( size <= len + sizeof( UINT32 ) * 2 )
		{
			APP_ERROR( "unshuffle invalid 1 " << make_tuple( size, len) );
			//assert( false );
			return -1;
		}

		//DWORD* realData = reinterpret_cast<DWORD*>( data + len );
                DWORD realData[2];
                memcpy(realData, data + len, sizeof(realData));
		//DWORD val = READ_MEMORY( data, DWORD );
                DWORD val;
                memcpy(&val, data, sizeof(val));
		realData[0] ^= val;
		realData[1] ^= val;
                memcpy(data + len, realData, sizeof(realData));

		std::pair<BYTE, WORD> checksum = ChecksumCalculator::Calc( data + len, size - len );
		checksum.second ^= 0xE903;
		if ( checksum.first != firstByte || checksum.second != checksumWord	)
		{
			// 解混淆失败，还原被修改的数据，不是安全协议报文，可能是vod能其它模块的报文，还原后交给其它模块处理
			APP_ERROR( "unshuffle invalid 2 " << checksum << " " << make_tuple( firstByte, checksumWord) );
			realData[1] ^= val;
			realData[0] ^= val;
                        memcpy(data + len, realData, sizeof(realData));
			//assert( false );
			return -2;
		}

		return static_cast<int>( len );
	}

};

#endif