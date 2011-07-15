
#ifndef _LIVE_P2PCOMMON2_NEW_UTIL_HUFFMAN_CODING_H_
#define _LIVE_P2PCOMMON2_NEW_UTIL_HUFFMAN_CODING_H_


/**
 * @file
 * @brief 用于对资源位图进行压缩的huffman编码工具类
 */

#include "framework/memory.h"
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

typedef boost::shared_ptr<pool_byte_string> pool_byte_string_ptr;


/// huffman编码工具类
class HuffmanCoding : private boost::noncopyable
{
public:
	HuffmanCoding();

	/// 编码，bytes和bitCount为源位图的字节缓冲区和长度，buf为编码结果的缓冲区，huffmanTimes为实际编码的次数
	UINT8 Encode(pool_byte_string_ptr bytes, size_t bitCount, pool_byte_string_ptr& buf);

	/// 解码，返回解码结果的缓冲区，bytes为源位图，huffmanTimes为编码次数
	pool_byte_string_ptr Decode(pool_byte_string_ptr bytes, UINT8 huffmanTimes);

private:
	/// 共用缓冲区，避免每次编解码重新申请缓冲区
	pool_byte_buffer m_encodeBuffer1;
	pool_byte_buffer m_encodeBuffer2;
	pool_byte_buffer m_decodeBuffer1;
	pool_byte_buffer m_decodeBuffer2;
	pool_string m_tempBitString;
};

#endif
