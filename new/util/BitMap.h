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



/// 位域，类似于BitTorrent
class Bitfield
{
public:
	/// 构造一个空的对象
	Bitfield() : m_bits( new pool_byte_string ), m_size(0) { CheckSize(); }
	/// 从指定的缓冲区构造
	Bitfield(size_t size, const BYTE* data) : m_bits(new pool_byte_string(data, size)), m_size(size * 8)
	{
		assert(size < USHRT_MAX);
		assert(size == 0 || data != NULL);
		CheckSize();
	}
	/// 构造指定长度的位域
	explicit Bitfield(size_t size, unsigned char initialValue = 0) : m_bits(new pool_byte_string(CalcRoundQuotient8(size), initialValue)), m_size(size)
	{
		assert(size < USHRT_MAX);
		CheckSize();
	}
	explicit Bitfield(pool_byte_string_ptr data) : m_bits(data), m_size(data->size() * 8)
	{
		CheckSize();
	}

	/// 获取长度
	size_t GetSize() const
	{
		CheckSize();
		return m_size;
	}

	/// 生成字符串
	string ToString() const;

	/// 获取指定索引的位
	bool operator[](size_t index) const { return GetBit(index); }

	/// 获取指定索引的位
	bool GetBit(size_t index) const;

	/// 设置指定索引的位
	void SetBit(size_t index, bool val = true);

	/// 重置指定索引的位
	void ResetBit(size_t index) { SetBit(index, false); }

	/// 获取原始数据的字节数
	size_t GetByteCount() const { return m_bits->size(); }
	/// 获取原始的字节数据
	const BYTE* GetBytes() const { return m_bits->data(); }

	/// 获取原始数据
	const pool_byte_string& GetData() const { return *m_bits; }
	/// 获取可写的原始数据
	pool_byte_string& GetData() { return *m_bits; }

	pool_byte_string_ptr GetBuffer() { return m_bits; }

	/// 检查索引是否越界
	bool IsInRange(size_t index) const
	{
		return index < GetSize();
	}

	/// 交换两个位域的数据
	void Swap(Bitfield& bf)
	{
		m_bits.swap(bf.m_bits);
		std::swap(m_size, bf.m_size);
	}

	/// 调整大小
	void Resize(size_t size)
	{
		m_bits->resize(CalcRoundQuotient8(size));
		m_size = size;
	}

	/// 检查是否所有的位都已被置(遍历检查，略微耗时)，同时返回连接块的末尾
	bool CheckIsAllBitsSet() const;

	/// 获取连续块的末尾
	UINT FindSkipIndex() const;

protected:
	/// 根据位索引计算字节索引
	static size_t GetByteIndex(size_t index)
	{
		return index / 8;
	}
	/// 根据位索引计算字节掩码(指定位为1)
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
	/// 原始的位数据(以压缩形式存储于字节数组中)
	pool_byte_string_ptr m_bits;

	/// 位域的长度
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
	/// 构造一个空的对象
	BitMap() : m_minIndex(0) { }
	/// 从指定的缓冲区构造
	BitMap(UINT32 minIndex, size_t size, const BYTE* data) : m_bits(size, data), m_minIndex(minIndex) { }
	/// 从pool_byte_string构造
	BitMap(UINT32 minIndex, pool_byte_string_ptr data) : m_bits(data), m_minIndex(minIndex) { }
	/// 构造指定长度的对象
	BitMap(UINT32 minIndex, size_t size, unsigned char initialValue = 0) : m_bits(size, initialValue), m_minIndex(minIndex) { }

	/// encode
	UINT8 GetHuffmanString(pool_byte_string_ptr& buf, HuffmanCoding& coding);

	/// 获取长度
	size_t GetSize() const { return m_bits.GetSize(); }

	/// 获取指定索引的位
	bool operator[](UINT32 index) const { return GetBit(index); }

	/// 获取指定索引的位
	bool GetBit(UINT32 index) const
	{
		if (!IsInRange(index))
		{
			//assert(false);
			return false;
		}
		return m_bits.GetBit(index - m_minIndex);
	}

	/// 设置指定索引的位
	void SetBit(UINT32 index, bool val = true)
	{
		if (!IsInRange(index))
		{
			assert(false);
			return;
		}
		m_bits.SetBit(index - m_minIndex, val);
	}

	/// 重置指定索引的位
	void ResetBit(UINT32 index) { SetBit(index, false); }
	void EraseBit(UINT32 index);

	/// 获取原始数据的字节数
	size_t GetByteCount() const { return m_bits.GetByteCount(); }
	
	const pool_byte_string& GetBytesData() const { return m_bits.GetData(); }
	pool_byte_string_ptr GetBytesBuffer() { return m_bits.GetBuffer(); }

	/// 获取原始的字节数据
	const BYTE* GetBytes() const { return m_bits.GetBytes(); }

	/// 获取原始数据
	const Bitfield& GetData() const { return m_bits; }

	/// 获取可写的原始数据
	Bitfield& GetData() { return m_bits; }

	/// 获取最小的资源片索引
	UINT32 GetMinIndex() const { return m_minIndex; }
	/// 获取最大的资源片索引(注意在位图不空的情况下，这里的最大索引是实际最大索引的下一位置)
	UINT32 GetMaxIndex() const { return m_minIndex + GetSize(); }

	PEER_MINMAX GetMinMax() const
	{
		PEER_MINMAX minmax = { GetMinIndex(), GetMaxIndex() };
		return minmax;
	}

	/// 检查是否再范围内
	bool IsInRange(UINT32 index) const
	{
		return index >= GetMinIndex() && index < GetMaxIndex();
	}
  
	/// 根据变化资源进行修正
	//void Revise(UINT32 minIndex, UINT16 startPos, UINT16 resLen, const UINT8 resources[]);

	/// 交换数据
	void Swap(BitMap& bf)
	{
		m_bits.Swap(bf.m_bits);
		std::swap(m_minIndex, bf.m_minIndex);
	}

	/// 生成字符串
	string ToString() const;

	/// 获取资源字符串
	string GetResourceString() const { return m_bits.ToString(); }


	/// 检查是否所有的位都有数据
	bool CheckIfAllBitsSet() const { return m_bits.CheckIsAllBitsSet(); }

	/// 查找连续块的末尾位置
	UINT FindSkipIndex() const { return m_bits.FindSkipIndex(); }

private:
	/// 基本的资源位图
	Bitfield m_bits;

	/// 起始位置
	UINT32 m_minIndex;

};




#endif