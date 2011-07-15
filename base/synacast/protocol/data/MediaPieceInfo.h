
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_MEDIA_PIECE_INFO_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_MEDIA_PIECE_INFO_H_

#include <ppl/data/int.h>


class MediaSubPieceInfo
{
public:
	/// ����Ĳ��
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
	/// PieceLengthΪPP_MEDIA�ṹ��������ݵĳ���
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
	/// ʱ���
	UINT64 TimeStamp;
	/// ��Ӧ��headerƬ������
	UINT32 HeaderPiece;
	/// ý�����ݵĳ���
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
	/// ý������
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