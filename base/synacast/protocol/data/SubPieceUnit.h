
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_SUBPIECE_UNIT_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_SUBPIECE_UNIT_H_


/**
* @file
* @brief SubPieceUnit的定义
*/

#include <ppl/data/int.h>
#include <iosfwd>


/// subpiece索引
class SubPieceUnit
{
public:
	UINT32 PieceIndex;
	UINT8 SubPieceIndex;

	enum { object_size = sizeof(UINT32) + sizeof(UINT8) } ;

	static const SubPieceUnit ZERO;

	SubPieceUnit()
	{
		this->PieceIndex = 0;
		this->SubPieceIndex = 0xFF;
	}

	explicit SubPieceUnit(UINT32 pieceIndex)
	{
		this->PieceIndex = pieceIndex;
		this->SubPieceIndex = 0xFF;
	}

	SubPieceUnit(UINT32 pieceIndex, UINT8 subPieceIndex)
	{
		this->PieceIndex = pieceIndex;
		this->SubPieceIndex = subPieceIndex;
	}

	SubPieceUnit& operator=(const SubPieceUnit& right)
	{
		this->PieceIndex = right.PieceIndex;
		this->SubPieceIndex = right.SubPieceIndex;
		return *this;
	}

	// 	UINT64 ToInteger() const
	// 	{
	// 		UINT64 val = PieceIndex;
	// 		return (val << 16) + SubPieceIndex;
	// 	}
};

inline bool operator<(const SubPieceUnit& x, const SubPieceUnit& y)
{
	if (x.PieceIndex < y.PieceIndex)
		return true;
	if (x.PieceIndex > y.PieceIndex)
		return false;
	return x.SubPieceIndex < y.SubPieceIndex;
}


inline bool operator==(const SubPieceUnit& x, const SubPieceUnit& y)
{
	return (x.PieceIndex == y.PieceIndex) && (x.SubPieceIndex == y.SubPieceIndex);
}

inline bool operator!=(const SubPieceUnit& x, const SubPieceUnit& y)
{
	return !(x == y);
}


std::ostream& operator<<(std::ostream& os, const SubPieceUnit& val);


#endif