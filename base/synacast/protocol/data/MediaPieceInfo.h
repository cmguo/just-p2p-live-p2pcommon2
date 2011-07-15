
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_MEDIA_PIECE_INFO_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_MEDIA_PIECE_INFO_H_

#include <ppl/data/int.h>


class MediaSubPieceInfo
{
public:
	/// 传输的层次
	UINT8 PieceLevel;
	UINT8 SubPieceCount;
	UINT8 SubPieceIndex;

	MediaSubPieceInfo()
	{
		PieceLevel = 0;
		SubPieceCount = 0;
		SubPieceIndex = 0;
	}

	enum { object_size = sizeof(UINT8) * 3 };
};

class MediaPieceInfo
{
public:
	UINT8 PieceType;
	UINT32 PieceIndex;
	/// PieceLength为PP_MEDIA结构体后面数据的长度
	UINT32 PieceLength;

	MediaPieceInfo()
	{
		PieceType = 0;
		PieceIndex = 0;
		PieceLength = 0;
	}

	enum { object_size = sizeof(UINT8) + sizeof(UINT32) + sizeof(UINT32) };
};


inline bool operator==(const MediaPieceInfo& x, const MediaPieceInfo& y)
{
	return x.PieceIndex == y.PieceIndex && x.PieceType == y.PieceType && x.PieceLength == y.PieceLength;
}

inline bool operator!=(const MediaPieceInfo& x, const MediaPieceInfo& y)
{
	return !(x == y);
}



class MediaDataPieceInfo
{
public:
	/// 时间戳
	UINT64 TimeStamp;
	/// 对应的header片的索引
	UINT32 HeaderPiece;
	/// 媒体数据的长度
	UINT32 MediaDataLength;

	MediaDataPieceInfo()
	{
		TimeStamp = 0;
		HeaderPiece = 0;
		MediaDataLength = 0;
	}

	enum { object_size = sizeof(UINT64) + sizeof(UINT32) * 2 };
};

class MediaHeaderPieceInfo
{
public:
	/// 媒体类型
	UINT16 MediaType;
	UINT16 InfoLength;
	UINT32 HeaderLength;
	UINT16 ProfileLength;

	MediaHeaderPieceInfo()
	{
		MediaType = 0;
		InfoLength = 0;
		HeaderLength = 0;
		ProfileLength = 0;
	}

	enum { object_size = sizeof(UINT16) * 3 + sizeof(UINT32) };
};



#endif