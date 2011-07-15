
#include "StdAfx.h"
#include <ostream>
#include "BitMap.h"
#include "util/HuffmanCoding.h"


void Bitfield::SetBit(size_t index, bool val)
{
	if (!IsInRange(index))
	{
		assert(!"Bitfield::SetBit: index is out of range.");
		return;
	}
	size_t pos = GetByteIndex(index);
	assert(pos < m_bits->size());
	BYTE& bit = reinterpret_cast<BYTE&>((*m_bits)[pos]);
	BYTE mask = (BYTE)GetByteMask(index);
	if (val)
	{
		// mask的指定位为1，其它位为0
		bit |= mask;
	}
	else
	{
		// mask的指定位为0，其它位为1
		bit &= (~mask);
	}
}

string Bitfield::ToString() const
{
	string str;
	str.reserve(GetSize() + 1);
 	for(size_t i = 0; i< GetSize(); i ++)
 	{
 		if( GetBit(i) )
			str += '1';
 		else
			str += '0';
 	}
	return str;
}

bool Bitfield::CheckIsAllBitsSet() const
{
	if (GetSize() == 0)
		return false;
	bool ifMeetBit = false;
	bool ifMeedZero = false;
	size_t count = 0;
	for (int index = (int)GetSize() - 1; index >= 0; --index, ++count)
	{
		bool isSet = GetBit(index);
		if (isSet)
		{
			// 遇到1
			if (!ifMeetBit)
			{
				ifMeetBit = true;
			}
		}
		else
		{
			// 遇到0
			if (ifMeetBit)
			{
				// 先遇到1，再遇到0
				ifMeedZero = true;
			}
			else
			{
				if (count > 8)
					return false;
			}
		}
	}
	return ifMeetBit && !ifMeedZero;
}


UINT Bitfield::FindSkipIndex() const
{
	if (GetSize() == 0)
		return 0;
	const int max_hole = 5;
	int leftHole = max_hole;
	UINT skipIndex = 0;
	for (size_t index = 0; index < GetSize(); ++index)
	{
		bool isSet = GetBit(index);
		if (isSet)
		{
			// 遇到1
			skipIndex = (UINT)index;
		}
		else
		{
			--leftHole;
			if (leftHole == 0)
				break;
		}
	}
	return skipIndex;
}



/*
void BitMap::Revise(UINT32 newMinIndex, UINT16 startPos, UINT16 resLen, const UINT8 resources[])
{
	// 资源位图不包含oldMax
	UINT32 oldMax = GetMaxIndex();
	// 资源位图不包含newMaxTemp
	UINT32 newMaxTemp = newMinIndex + startPos + resLen * 8;
	UINT32 newMax = max(oldMax, newMaxTemp);
	assert(newMax >= newMinIndex);

	size_t newSize = newMax - newMinIndex;
	BitMap newBits(newMinIndex, newSize);
	UINT32 i = 0;
	for (i = newBits.GetMinIndex(); i < newBits.GetMaxIndex(); ++i)
	{
		assert(newBits.IsInRange(i));
		newBits.SetBit(i, GetBit(i));
	}
	BitMap changedBits(newMinIndex + startPos, resLen, resources);
	for (i = changedBits.GetMinIndex(); i < changedBits.GetMaxIndex(); ++i)
	{
		assert(newBits.IsInRange(i));
		bool bit = changedBits[i];
		// bit为true，表示从无到有，否则，保持不变
		if (bit)
		{
			newBits.SetBit(i);
		}
	}

	Swap(newBits);
	assert(GetMinIndex() == newMinIndex);
	assert(GetSize() == newSize);
	assert(GetMaxIndex() == newMax);
}*/

void BitMap::EraseBit(UINT32 index)
{
	if (!IsInRange(index))
		return;
	ResetBit(index);
}


string BitMap::ToString() const
{
	char startTag[32] = { 0 };
	::_snprintf(startTag, 31, "%u", m_minIndex);
	string startTagString = startTag;
	return "(" + startTagString + ":" + m_bits.ToString() + ")";
}

UINT8 BitMap::GetHuffmanString(pool_byte_string_ptr& buf, HuffmanCoding& coding)
{
	return coding.Encode( m_bits.GetBuffer(), m_bits.GetSize(), buf );
}



std::ostream& operator<<(std::ostream& os, const Bitfield& bf)
{
	return os << bf.ToString();
}

std::ostream& operator<<(std::ostream& os, const BitMap& bitmap)
{
	return os << "(" << bitmap.GetMinIndex() << ":" << bitmap.GetData() << ")";
}










#ifdef _RUN_TEST

#include <ppl/util/random.h>
#include <ppl/util/test_case.h>


class BitfieldTestCase : public ppl::util::test_case
{
	virtual void DoRun()
	{
		assert(CalcRound8(0) == 0);
		assert(CalcRound8(1) == 8);
		assert(CalcRound8(2) == 8);
		assert(CalcRound8(3) == 8);
		assert(CalcRound8(4) == 8);
		assert(CalcRound8(5) == 8);
		assert(CalcRound8(6) == 8);
		assert(CalcRound8(7) == 8);
		assert(CalcRound8(8) == 8);
		assert(CalcRound8(9) == 16);

		assert(CalcRoundQuotient8(0) == 0);
		assert(CalcRoundQuotient8(1) == 1);
		assert(CalcRoundQuotient8(2) == 1);
		assert(CalcRoundQuotient8(3) == 1);
		assert(CalcRoundQuotient8(4) == 1);
		assert(CalcRoundQuotient8(5) == 1);
		assert(CalcRoundQuotient8(6) == 1);
		assert(CalcRoundQuotient8(7) == 1);
		assert(CalcRoundQuotient8(8) == 1);
		assert(CalcRoundQuotient8(9) == 2);

		Bitfield bf;
		assert(bf.GetSize() == 0);
		assert(bf.GetByteCount() == 0);

		BYTE bytes[1] = { 0x0 };
		bf = Bitfield(1, bytes);
		assert(bf.GetSize() == 8);
		assert(bf.GetByteCount() == 1);
		CheckBits(bf, 0, bf.GetSize(), false);
		assert(bf.ToString() == "00000000");
		assert(!bf.CheckIsAllBitsSet());

		size_t i = 0;
		for (i = 0; i < 8; ++i)
		{
			TestByte(bf, i);
		}

		bool status;
		bf.GetData()[0] = 0xf0;
		CheckBits(bf, 0, 4, true);
		CheckBits(bf, 4, bf.GetSize(), false);
		assert(bf.ToString() == "11110000");
		assert(bf.CheckIsAllBitsSet());

		bf.GetData()[0] = 0x0f;
		CheckBits(bf, 0, 4, false);
		CheckBits(bf, 4, bf.GetSize(), true);
		assert(bf.ToString() == "00001111");
		status = bf.CheckIsAllBitsSet();
		assert(!status);
		assert(bf.FindSkipIndex() == 7);

		bf.ResetBit(5);
		assert(!bf[5]);
		assert(bf.ToString() == "00001011");
		assert(!bf.CheckIsAllBitsSet());
		assert(bf.FindSkipIndex() == 4);

		bf.SetBit(5);
		assert(bf[5]);
		assert(bf.ToString() == "00001111");
		assert(!bf.CheckIsAllBitsSet());

		bf.ResetBit(5);
		assert(!bf[5]);
		assert(bf.ToString() == "00001011");
		assert(!bf.CheckIsAllBitsSet());

		bf.SetBit(2);
		assert(bf.ToString() == "00101011");
		assert(!bf.CheckIsAllBitsSet());
		bf.ResetBit(3);
		assert(bf.ToString() == "00101011");
		assert(!bf.CheckIsAllBitsSet());
		assert(!bf[3]);
		assert(bf[2]);
		assert(bf.ToString() == "00101011");
		assert(!bf.CheckIsAllBitsSet());

		bf.SetBit(3);
		assert(bf[3]);
		assert(bf[2]);
		assert(bf.ToString() == "00111011");
		assert(!bf.CheckIsAllBitsSet());
		bf.ResetBit(3);
		assert(!bf[3]);
		assert(bf[2]);
		assert(bf.ToString() == "00101011");
		assert(!bf.CheckIsAllBitsSet());

		TestZeroBitfield();

		for (i = 0; i < bf.GetSize(); ++i)
		{
			bf.SetBit(i, true);
		}
		assert(bf.ToString() == "11111111");
		assert(bf.CheckIsAllBitsSet());


		{
			BYTE bytes1[2] = { 0xFF, 0xE0 };
			Bitfield bf1(2, bytes1);
			assert(bf1.ToString() == "1111111111100000");
			assert(bf1.CheckIsAllBitsSet());
			assert(bf1.FindSkipIndex() == 10);
		}
		{
			BYTE bytes1[2] = { 0xFF, 0xE8 };
			Bitfield bf1(2, bytes1);
			assert(bf1.ToString() == "1111111111101000");
			assert(!bf1.CheckIsAllBitsSet());
			assert(bf1.FindSkipIndex() == 12);
		}
		{
			BYTE bytes1[3] = { 0xFF, 0xEA, 0xA8 };
			Bitfield bf1(3, bytes1);
			assert(bf1.ToString() == "111111111110101010101000");
			assert(!bf1.CheckIsAllBitsSet());
			assert(bf1.FindSkipIndex() == 18);
		}
	}

	/// 测试根据长度构造的位域
	void TestZeroBitfield()
	{
		TestZeroBitfield(0);
		TestZeroBitfield(1);
		TestZeroBitfield(6);
		TestZeroBitfield(16);
		TestZeroBitfield(70);
		TestZeroBitfield(180);
		TestZeroBitfield(660);
		TestZeroBitfield(1800);
	}
	void TestZeroBitfield(size_t size)
	{
		Bitfield bf(size);
		CheckBits(bf, 0, bf.GetSize(), false);
		assert(bf.ToString() == string(bf.GetSize(), '0'));
	}

	void CheckBits(const Bitfield& bf, size_t begin, size_t end, bool val)
	{
		for (size_t i = begin; i < end; ++i)
		{
			assert(bf[i] == val);
			assert(bf[i] == bf.GetBit(i));
		}
	}
	void TestByte(Bitfield& bf, size_t pos)
	{
		assert(bf.GetSize() == 8);
		assert(bf.GetByteCount() == 1);
		assert(pos < 8);
		bf.GetData()[0] = static_cast<BYTE>(0x80 >> pos);
		assert(bf[pos]);
		CheckBits(bf, 0, pos, false);
		CheckBits(bf, pos + 1, bf.GetSize(), false);
	}

};


class StreamBitfieldTestCase : public ppl::util::test_case
{
	Random m_random;

	virtual void DoRun()
	{
		TestZeroBitfield();

		BitMap bf;
		assert(bf.GetSize() == 0);
		assert(bf.GetByteCount() == 0);

		BYTE bytes[1] = { 0x0 };
		bf = BitMap(0, 1, bytes);
		assert(bf.GetSize() == 8);
		assert(bf.GetByteCount() == 1);
		CheckBits(bf, 0, bf.GetSize(), false);

		for (size_t i = 0; i < 8; ++i)
		{
			TestByte(bf, i);
		}

		bf.GetData().GetData()[0] = 0xf0;
		CheckBits(bf, 0, 4, true);
		CheckBits(bf, 4, bf.GetSize(), false);

		bf.GetData().GetData()[0] = 0x0f;
		CheckBits(bf, 0, 4, false);
		CheckBits(bf, 4, bf.GetSize(), true);
		bf.ResetBit(5);
		assert(!bf[5]);
		bf.SetBit(5);
		assert(bf[5]);
		bf.ResetBit(5);
		assert(!bf[5]);


		TestBasic();
	}
	/// 测试根据长度构造的位域
	void TestZeroBitfield()
	{
		TestZeroBitfield(0);
		TestZeroBitfield(1);
		TestZeroBitfield(6);
		TestZeroBitfield(16);
		TestZeroBitfield(70);
		TestZeroBitfield(180);
		TestZeroBitfield(660);
		TestZeroBitfield(1800);
	}
	void TestZeroBitfield(size_t size)
	{
		UINT32 minIndex = m_random.Next();
		BitMap bf(m_random.Next(), size);
		CheckBits(bf, bf.GetMinIndex(), bf.GetMaxIndex(), false);
	}

	void CheckBits(const BitMap& bf, size_t begin, size_t end, bool val)
	{
		for (size_t i = begin; i < end; ++i)
		{
			assert(bf[i] == val);
			assert(bf[i] == bf.GetBit(i));
		}
	}

	void TestByte(BitMap& bf, size_t pos)
	{
		assert(bf.GetSize() == 8);
		assert(bf.GetByteCount() == 1);
		assert(pos < 8);
		bf.GetData().GetData()[0] = static_cast<BYTE>(0x80 >> pos);
		assert(bf[pos]);
		CheckBits(bf, 0, pos, false);
		CheckBits(bf, pos + 1, bf.GetSize(), false);
	}



	void TestBasic()
	{
		src[0] = 0xFF;
		src[1] = 0x0F;
		BitMap bmp(100, 2, src);
		assert(bmp.GetMinIndex() == 100);
		assert(bmp.GetMaxIndex() == 116);
		size_t i = 0;
		for (i = 100; i < 108; ++i)
		{
			assert(bmp[i]);
		}
		for (i = 108; i < 112; ++i)
		{
			assert(!bmp[i]);
		}
		for (i = 112; i < 116; ++i)
		{
			assert(bmp[i]);
		}
		assert(!bmp[98]);
		assert(!bmp[99]);
		assert(!bmp[116]);
		assert(!bmp[117]);
	}

	UINT8 dest[1024];
	UINT8 src[1024];
};




class BitMapTestCase : public ppl::util::test_case
{
protected:
	virtual void DoRun()
	{
//		TestDecode();

		TestBasic();

//		TestHuffman();
	}

	void TestBasic()
	{
		src[0] = 0xFF;
		src[1] = 0x0F;
		BitMap bmp(100, 2, src);
		assert(bmp.GetMinIndex() == 100);
		assert(bmp.GetMaxIndex() == 116);
		size_t i = 0;
		for (i = 100; i < 108; ++i)
		{
			assert(bmp[i]);
		}
		for (i = 108; i < 112; ++i)
		{
			assert(!bmp[i]);
		}
		for (i = 112; i < 116; ++i)
		{
			assert(bmp[i]);
		}
		assert(!bmp[98]);
		assert(!bmp[99]);
		assert(!bmp[116]);
		assert(!bmp[117]);
	}
/*
	void TestDecode()
	{
		src[0] = 1;
		size_t size = BitMap::DecodeBitMap(dest, src, 1);
		assert(size = 1);
		src[0] = 3;
		size = BitMap::DecodeBitMap(dest, src, 1);
		assert(size = 1);
	}
*/
	
/*	void TestHuffman()
	{
//		BitMap bmp;
		UINT8 times = 0;
		UINT32 i, j = 0;
		return;
		freopen("D:\\announce\\pick\\pick.txt","r",stdin);
		freopen("d:\\announce\\pick\\huf.out","w",stdout);
		char b[600];
		pool_byte_string huffs, a;

		while (std::cin>>a)
		{
			if (a.length() > 4000||a.length()<=64) continue;
			memset(b,0,sizeof(b));
			for (i = 0; i < a.length(); i ++)
				b[i/8] |=(a[i]-'0')<<(i%8);
			//字节丢失
			BitMap bmp(0,(a.length()+7)/8,(const char*)b);
			huffs = bmp.GetHuffmanString(times);
			BitMap huffmap(0,times,huffs.length(),huffs.c_str());
			for (i = 0; i < huffmap.GetByteCount()-1; i ++)
				assert(huffmap.GetBytes()[i] == (BYTE)b[i]);
			//printf("%d...........\n",++j);
			UTIL_DEBUG(++j);
		}
		fclose(stdin);fclose(stdout);
	}*/
public:
	BYTE dest[1024];
	BYTE src[1024];
};




CPPUNIT_TEST_SUITE_REGISTRATION(BitfieldTestCase);
CPPUNIT_TEST_SUITE_REGISTRATION(StreamBitfieldTestCase);
CPPUNIT_TEST_SUITE_REGISTRATION(BitMapTestCase);



#endif
