
#ifndef _LIVE_P2PCOMMON2_NEW_UTIL_HUFFMAN_CODING_H_
#define _LIVE_P2PCOMMON2_NEW_UTIL_HUFFMAN_CODING_H_


/**
 * @file
 * @brief ���ڶ���Դλͼ����ѹ����huffman���빤����
 */

#include "framework/memory.h"
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

typedef boost::shared_ptr<pool_byte_string> pool_byte_string_ptr;


/// huffman���빤����
class HuffmanCoding : private boost::noncopyable
{
public:
	HuffmanCoding();

	/// ���룬bytes��bitCountΪԴλͼ���ֽڻ������ͳ��ȣ�bufΪ�������Ļ�������huffmanTimesΪʵ�ʱ���Ĵ���
	UINT8 Encode(pool_byte_string_ptr bytes, size_t bitCount, pool_byte_string_ptr& buf);

	/// ���룬���ؽ������Ļ�������bytesΪԴλͼ��huffmanTimesΪ�������
	pool_byte_string_ptr Decode(pool_byte_string_ptr bytes, UINT8 huffmanTimes);

private:
	/// ���û�����������ÿ�α�����������뻺����
	pool_byte_buffer m_encodeBuffer1;
	pool_byte_buffer m_encodeBuffer2;
	pool_byte_buffer m_decodeBuffer1;
	pool_byte_buffer m_decodeBuffer2;
	pool_string m_tempBitString;
};

#endif
