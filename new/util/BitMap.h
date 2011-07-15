/********************************************************************
Copyright(c) 2004-2005 PPLive.com All rights reserved.                                                 
filename: 	BitMap.h
created:	2005-4-22   9:40
author:		ElNino
purpose:	
*********************************************************************/

#ifndef _LIVE_P2PCOMMON2_NEW_UTIL_BITMAP_H_
#define _LIVE_P2PCOMMON2_NEW_UTIL_BITMAP_H_


#include <synacast/protocol/data/PeerMinMax.h>
#include <boost/shared_ptr.hpp>
#include "framework/memory.h"
#include <string>
#include <iosfwd>

using std::string;


class HuffmanCoding;
class Bitfield;
class BitMap;
typedef boost::shared_ptr<pool_byte_string> pool_byte_string_ptr;

std::ostream& operator<<(std::ostream& os, const Bitfield& bf);
std::ostream& operator<<(std::ostream& os, const BitMap& bitmap);


inline size_t CalcRound8(size_t val)
{
	return val + 7 - ((val + 7) % 8);
}

inline size_t CalcRoundQuotient8(size_t val)
{
	return (val + 7) / 8;
}



/// λ��������BitTorrent
class Bitfield
{
public:
	/// ����һ���յĶ���
	Bitfield() : m_bits( new pool_byte_string ), m_size(0) { CheckSize(); }
	/// ��ָ���Ļ���������
	Bitfield(size_t size, const BYTE* data) : m_bits(new pool_byte_string(data, size)), m_size(size * 8)
	{
		assert(size < USHRT_MAX);
		assert(size == 0 || data != NULL);
		CheckSize();
	}
	/// ����ָ�����ȵ�λ��
	explicit Bitfield(size_t size, unsigned char initialValue = 0) : m_bits(new pool_byte_string(CalcRoundQuotient8(size), initialValue)), m_size(size)
	{
		assert(size < USHRT_MAX);
		CheckSize();
	}
	explicit Bitfield(pool_byte_string_ptr data) : m_bits(data), m_size(data->size() * 8)
	{
		CheckSize();
	}

	/// ��ȡ����
	size_t GetSize() const
	{
		CheckSize();
		return m_size;
	}

	/// �����ַ���
	string ToString() const;

	/// ��ȡָ��������λ
	bool operator[](size_t index) const { return GetBit(index); }

	/// ��ȡָ��������λ
	bool GetBit(size_t index) const;

	/// ����ָ��������λ
	void SetBit(size_t index, bool val = true);

	/// ����ָ��������λ
	void ResetBit(size_t index) { SetBit(index, false); }

	/// ��ȡԭʼ���ݵ��ֽ���
	size_t GetByteCount() const { return m_bits->size(); }
	/// ��ȡԭʼ���ֽ�����
	const BYTE* GetBytes() const { return m_bits->data(); }

	/// ��ȡԭʼ����
	const pool_byte_string& GetData() const { return *m_bits; }
	/// ��ȡ��д��ԭʼ����
	pool_byte_string& GetData() { return *m_bits; }

	pool_byte_string_ptr GetBuffer() { return m_bits; }

	/// ��������Ƿ�Խ��
	bool IsInRange(size_t index) const
	{
		return index < GetSize();
	}

	/// ��������λ�������
	void Swap(Bitfield& bf)
	{
		m_bits.swap(bf.m_bits);
		std::swap(m_size, bf.m_size);
	}

	/// ������С
	void Resize(size_t size)
	{
		m_bits->resize(CalcRoundQuotient8(size));
		m_size = size;
	}

	/// ����Ƿ����е�λ���ѱ���(������飬��΢��ʱ)��ͬʱ�������ӿ��ĩβ
	bool CheckIsAllBitsSet() const;

	/// ��ȡ�������ĩβ
	UINT FindSkipIndex() const;

protected:
	/// ����λ���������ֽ�����
	static size_t GetByteIndex(size_t index)
	{
		return index / 8;
	}
	/// ����λ���������ֽ�����(ָ��λΪ1)
	static size_t GetByteMask(size_t index)
	{
		return 128 >> (index % 8);
	}

	void CheckSize() const
	{
		assert(m_bits->size() < USHRT_MAX);
		assert(m_size < USHRT_MAX * 8);
		size_t capacity = (m_bits->size() * 8);
		assert(m_size <= capacity);
		assert(CalcRound8(m_size) == capacity);
	}

private:
	/// ԭʼ��λ����(��ѹ����ʽ�洢���ֽ�������)
	pool_byte_string_ptr m_bits;

	/// λ��ĳ���
	size_t m_size;
};


inline bool Bitfield::GetBit(size_t index) const
{
	if (!IsInRange(index))
	{
		//assert(!"Bitfield::GetBit: index is out of range.");
		return false;
	}
	size_t pos = GetByteIndex(index);
	assert(pos < m_bits->size());
	BYTE mask = (BYTE)GetByteMask(index);
	BYTE bit = (*m_bits)[pos];
	return 0 != (bit & mask);
}


class BitMap
{
public:
	/// ����һ���յĶ���
	BitMap() : m_minIndex(0) { }
	/// ��ָ���Ļ���������
	BitMap(UINT32 minIndex, size_t size, const BYTE* data) : m_bits(size, data), m_minIndex(minIndex) { }
	/// ��pool_byte_string����
	BitMap(UINT32 minIndex, pool_byte_string_ptr data) : m_bits(data), m_minIndex(minIndex) { }
	/// ����ָ�����ȵĶ���
	BitMap(UINT32 minIndex, size_t size, unsigned char initialValue = 0) : m_bits(size, initialValue), m_minIndex(minIndex) { }

	/// encode
	UINT8 GetHuffmanString(pool_byte_string_ptr& buf, HuffmanCoding& coding);

	/// ��ȡ����
	size_t GetSize() const { return m_bits.GetSize(); }

	/// ��ȡָ��������λ
	bool operator[](UINT32 index) const { return GetBit(index); }

	/// ��ȡָ��������λ
	bool GetBit(UINT32 index) const
	{
		if (!IsInRange(index))
		{
			//assert(false);
			return false;
		}
		return m_bits.GetBit(index - m_minIndex);
	}

	/// ����ָ��������λ
	void SetBit(UINT32 index, bool val = true)
	{
		if (!IsInRange(index))
		{
			assert(false);
			return;
		}
		m_bits.SetBit(index - m_minIndex, val);
	}

	/// ����ָ��������λ
	void ResetBit(UINT32 index) { SetBit(index, false); }
	void EraseBit(UINT32 index);

	/// ��ȡԭʼ���ݵ��ֽ���
	size_t GetByteCount() const { return m_bits.GetByteCount(); }
	
	const pool_byte_string& GetBytesData() const { return m_bits.GetData(); }
	pool_byte_string_ptr GetBytesBuffer() { return m_bits.GetBuffer(); }

	/// ��ȡԭʼ���ֽ�����
	const BYTE* GetBytes() const { return m_bits.GetBytes(); }

	/// ��ȡԭʼ����
	const Bitfield& GetData() const { return m_bits; }

	/// ��ȡ��д��ԭʼ����
	Bitfield& GetData() { return m_bits; }

	/// ��ȡ��С����ԴƬ����
	UINT32 GetMinIndex() const { return m_minIndex; }
	/// ��ȡ������ԴƬ����(ע����λͼ���յ�����£���������������ʵ�������������һλ��)
	UINT32 GetMaxIndex() const { return m_minIndex + GetSize(); }

	PEER_MINMAX GetMinMax() const
	{
		PEER_MINMAX minmax = { GetMinIndex(), GetMaxIndex() };
		return minmax;
	}

	/// ����Ƿ��ٷ�Χ��
	bool IsInRange(UINT32 index) const
	{
		return index >= GetMinIndex() && index < GetMaxIndex();
	}
  
	/// ���ݱ仯��Դ��������
	//void Revise(UINT32 minIndex, UINT16 startPos, UINT16 resLen, const UINT8 resources[]);

	/// ��������
	void Swap(BitMap& bf)
	{
		m_bits.Swap(bf.m_bits);
		std::swap(m_minIndex, bf.m_minIndex);
	}

	/// �����ַ���
	string ToString() const;

	/// ��ȡ��Դ�ַ���
	string GetResourceString() const { return m_bits.ToString(); }


	/// ����Ƿ����е�λ��������
	bool CheckIfAllBitsSet() const { return m_bits.CheckIsAllBitsSet(); }

	/// �����������ĩβλ��
	UINT FindSkipIndex() const { return m_bits.FindSkipIndex(); }

private:
	/// ��������Դλͼ
	Bitfield m_bits;

	/// ��ʼλ��
	UINT32 m_minIndex;

};




#endif