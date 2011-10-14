
#include "StdAfx.h"
#include <ostream>
#include "BitMap.h"
#include "util/HuffmanCoding.h"


void Bitfield::SetBit(size_t index, bool val)
{
	if (!IsInRange(index))
	{
		LIVE_ASSERT(!"Bitfield::SetBit: index is out of range.");
		return;
	}
	size_t pos = GetByteIndex(index);
	LIVE_ASSERT(pos < m_bits->size());
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
	LIVE_ASSERT(newMax >= newMinIndex);

	size_t newSize = newMax - newMinIndex;
	BitMap newBits(newMinIndex, newSize);
	UINT32 i = 0;
	for (i = newBits.GetMinIndex(); i < newBits.GetMaxIndex(); ++i)
	{
		LIVE_ASSERT(newBits.IsInRange(i));
		newBits.SetBit(i, GetBit(i));
	}
	BitMap changedBits(newMinIndex + startPos, resLen, resources);
	for (i = changedBits.GetMinIndex(); i < changedBits.GetMaxIndex(); ++i)
	{
		LIVE_ASSERT(newBits.IsInRange(i));
		bool bit = changedBits[i];
		// bit为true，表示从无到有，否则，保持不变
		if (bit)
		{
			newBits.SetBit(i);
		}
	}

	Swap(newBits);
	LIVE_ASSERT(GetMinIndex() == newMinIndex);
	LIVE_ASSERT(GetSize() == newSize);
	LIVE_ASSERT(GetMaxIndex() == newMax);
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
		LIVE_ASSERT(CalcRound8(0) == 0);
		LIVE_ASSERT(CalcRound8(1) == 8);
		LIVE_ASSERT(CalcRound8(2) == 8);
		LIVE_ASSERT(CalcRound8(3) == 8);
		LIVE_ASSERT(CalcRound8(4) == 8);
		LIVE_ASSERT(CalcRound8(5) == 8);
		LIVE_ASSERT(CalcRound8(6) == 8);
		LIVE_ASSERT(CalcRound8(7) == 8);
		LIVE_ASSERT(CalcRound8(8) == 8);
		LIVE_ASSERT(CalcRound8(9) == 16);

		LIVE_ASSERT(CalcRoundQuotient8(0) == 0);
		LIVE_ASSERT(CalcRoundQuotient8(1) == 1);
		LIVE_ASSERT(CalcRoundQuotient8(2) == 1);
		LIVE_ASSERT(CalcRoundQuotient8(3) == 1);
		LIVE_ASSERT(CalcRoundQuotient8(4) == 1);
		LIVE_ASSERT(CalcRoundQuotient8(5) == 1);
		LIVE_ASSERT(CalcRoundQuotient8(6) == 1);
		LIVE_ASSERT(CalcRoundQuotient8(7) == 1);
		LIVE_ASSERT(CalcRoundQuotient8(8) == 1);
		LIVE_ASSERT(CalcRoundQuotient8(9) == 2);

		Bitfield bf;
		LIVE_ASSERT(bf.GetSize() == 0);
		LIVE_ASSERT(bf.GetByteCount() == 0);

		BYTE bytes[1] = { 0x0 };
		bf = Bitfield(1, bytes);
		LIVE_ASSERT(bf.GetSize() == 8);
		LIVE_ASSERT(bf.GetByteCount() == 1);
		CheckBits(bf, 0, bf.GetSize(), false);
		LIVE_ASSERT(bf.ToString() == "00000000");
		LIVE_ASSERT(!bf.CheckIsAllBitsSet());

		size_t i = 0;
		for (i = 0; i < 8; ++i)
		{
			TestByte(bf, i);
		}

		bool status;
		bf.GetData()[0] = 0xf0;
		CheckBits(bf, 0, 4, true);
		CheckBits(bf, 4, bf.GetSize(), false);
		LIVE_ASSERT(bf.ToString() == "11110000");
		LIVE_ASSERT(bf.CheckIsAllBitsSet());

		bf.GetData()[0] = 0x0f;
		CheckBits(bf, 0, 4, false);
		CheckBits(bf, 4, bf.GetSize(), true);
		LIVE_ASSERT(bf.ToString() == "00001111");
		status = bf.CheckIsAllBitsSet();
		LIVE_ASSERT(!status);
		LIVE_ASSERT(bf.FindSkipIndex() == 7);

		bf.ResetBit(5);
		LIVE_ASSERT(!bf[5]);
		LIVE_ASSERT(bf.ToString() == "00001011");
		LIVE_ASSERT(!bf.CheckIsAllBitsSet());
		LIVE_ASSERT(bf.FindSkipIndex() == 4);

		bf.SetBit(5);
		LIVE_ASSERT(bf[5]);
		LIVE_ASSERT(bf.ToString() == "00001111");
		LIVE_ASSERT(!bf.CheckIsAllBitsSet());

		bf.ResetBit(5);
		LIVE_ASSERT(!bf[5]);
		LIVE_ASSERT(bf.ToString() == "00001011");
		LIVE_ASSERT(!bf.CheckIsAllBitsSet());

		bf.SetBit(2);
		LIVE_ASSERT(bf.ToString() == "00101011");
		LIVE_ASSERT(!bf.CheckIsAllBitsSet());
		bf.ResetBit(3);
		LIVE_ASSERT(bf.ToString() == "00101011");
		LIVE_ASSERT(!bf.CheckIsAllBitsSet());
		LIVE_ASSERT(!bf[3]);
		LIVE_ASSERT(bf[2]);
		LIVE_ASSERT(bf.ToString() == "00101011");
		LIVE_ASSERT(!bf.CheckIsAllBitsSet());

		bf.SetBit(3);
		LIVE_ASSERT(bf[3]);
		LIVE_ASSERT(bf[2]);
		LIVE_ASSERT(bf.ToString() == "00111011");
		LIVE_ASSERT(!bf.CheckIsAllBitsSet());
		bf.ResetBit(3);
		LIVE_ASSERT(!bf[3]);
		LIVE_ASSERT(bf[2]);
		LIVE_ASSERT(bf.ToString() == "00101011");
		LIVE_ASSERT(!bf.CheckIsAllBitsSet());

		TestZeroBitfield();

		for (i = 0; i < bf.GetSize(); ++i)
		{
			bf.SetBit(i, true);
		}
		LIVE_ASSERT(bf.ToString() == "11111111");
		LIVE_ASSERT(bf.CheckIsAllBitsSet());


		{
			BYTE bytes1[2] = { 0xFF, 0xE0 };
			Bitfield bf1(2, bytes1);
			LIVE_ASSERT(bf1.ToString() == "1111111111100000");
			LIVE_ASSERT(bf1.CheckIsAllBitsSet());
			LIVE_ASSERT(bf1.FindSkipIndex() == 10);
		}
		{
			BYTE bytes1[2] = { 0xFF, 0xE8 };
			Bitfield bf1(2, bytes1);
			LIVE_ASSERT(bf1.ToString() == "1111111111101000");
			LIVE_ASSERT(!bf1.CheckIsAllBitsSet());
			LIVE_ASSERT(bf1.FindSkipIndex() == 12);
		}
		{
			BYTE bytes1[3] = { 0xFF, 0xEA, 0xA8 };
			Bitfield bf1(3, bytes1);
			LIVE_ASSERT(bf1.ToString() == "111111111110101010101000");
			LIVE_ASSERT(!bf1.CheckIsAllBitsSet());
			LIVE_ASSERT(bf1.FindSkipIndex() == 18);
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
		LIVE_ASSERT(bf.ToString() == string(bf.GetSize(), '0'));
	}

	void CheckBits(const Bitfield& bf, size_t begin, size_t end, bool val)
	{
		for (size_t i = begin; i < end; ++i)
		{
			LIVE_ASSERT(bf[i] == val);
			LIVE_ASSERT(bf[i] == bf.GetBit(i));
		}
	}
	void TestByte(Bitfield& bf, size_t pos)
	{
		LIVE_ASSERT(bf.GetSize() == 8);
		LIVE_ASSERT(bf.GetByteCount() == 1);
		LIVE_ASSERT(pos < 8);
		bf.GetData()[0] = static_cast<BYTE>(0x80 >> pos);
		LIVE_ASSERT(bf[pos]);
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
		LIVE_ASSERT(bf.GetSize() == 0);
		LIVE_ASSERT(bf.GetByteCount() == 0);

		BYTE bytes[1] = { 0x0 };
		bf = BitMap(0, 1, bytes);
		LIVE_ASSERT(bf.GetSize() == 8);
		LIVE_ASSERT(bf.GetByteCount() == 1);
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
		LIVE_ASSERT(!bf[5]);
		bf.SetBit(5);
		LIVE_ASSERT(bf[5]);
		bf.ResetBit(5);
		LIVE_ASSERT(!bf[5]);


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
			LIVE_ASSERT(bf[i] == val);
			LIVE_ASSERT(bf[i] == bf.GetBit(i));
		}
	}

	void TestByte(BitMap& bf, size_t pos)
	{
		LIVE_ASSERT(bf.GetSize() == 8);
		LIVE_ASSERT(bf.GetByteCount() == 1);
		LIVE_ASSERT(pos < 8);
		bf.GetData().GetData()[0] = static_cast<BYTE>(0x80 >> pos);
		LIVE_ASSERT(bf[pos]);
		CheckBits(bf, 0, pos, false);
		CheckBits(bf, pos + 1, bf.GetSize(), false);
	}



	void TestBasic()
	{
		src[0] = 0xFF;
		src[1] = 0x0F;
		BitMap bmp(100, 2, src);
		LIVE_ASSERT(bmp.GetMinIndex() == 100);
		LIVE_ASSERT(bmp.GetMaxIndex() == 116);
		size_t i = 0;
		for (i = 100; i < 108; ++i)
		{
			LIVE_ASSERT(bmp[i]);
		}
		for (i = 108; i < 112; ++i)
		{
			LIVE_ASSERT(!bmp[i]);
		}
		for (i = 112; i < 116; ++i)
		{
			LIVE_ASSERT(bmp[i]);
		}
		LIVE_ASSERT(!bmp[98]);
		LIVE_ASSERT(!bmp[99]);
		LIVE_ASSERT(!bmp[116]);
		LIVE_ASSERT(!bmp[117]);
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
		LIVE_ASSERT(bmp.GetMinIndex() == 100);
		LIVE_ASSERT(bmp.GetMaxIndex() == 116);
		size_t i = 0;
		for (i = 100; i < 108; ++i)
		{
			LIVE_ASSERT(bmp[i]);
		}
		for (i = 108; i < 112; ++i)
		{
			LIVE_ASSERT(!bmp[i]);
		}
		for (i = 112; i < 116; ++i)
		{
			LIVE_ASSERT(bmp[i]);
		}
		LIVE_ASSERT(!bmp[98]);
		LIVE_ASSERT(!bmp[99]);
		LIVE_ASSERT(!bmp[116]);
		LIVE_ASSERT(!bmp[117]);
	}
/*
	void TestDecode()
	{
		src[0] = 1;
		size_t size = BitMap::DecodeBitMap(dest, src, 1);
		LIVE_ASSERT(size = 1);
		src[0] = 3;
		size = BitMap::DecodeBitMap(dest, src, 1);
		LIVE_ASSERT(size = 1);
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
				LIVE_ASSERT(huffmap.GetBytes()[i] == (BYTE)b[i]);
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
