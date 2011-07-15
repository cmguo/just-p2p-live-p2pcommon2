
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_MONO_MEDIA_PIECE_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_MONO_MEDIA_PIECE_H_

#include "framework/memory.h"
#include <synacast/protocol/data/MediaPieceInfo.h>
#include <synacast/protocol/base.h>
#include <synacast/protocol/ReaderTypes.h>
#include <ppl/data/int.h>
#include <boost/noncopyable.hpp>

const size_t PPL_MAX_PIECE_DATA_LENGTH = 64 * 1024 - 1;



class MonoMediaPiece : public pool_object, private boost::noncopyable
{
public:
	UINT8 PieceLevel;
	UINT32 PieceIndex;
	UINT8 PieceType;

	explicit MonoMediaPiece( UINT8 pieceLevel, UINT8 pieceType, UINT32 pieceIndex )
		: PieceLevel( pieceLevel ),PieceIndex( pieceIndex ), PieceType( pieceType )
	{
	}

	/// piece尺寸，也是签名时的数据长度，包括MediaPieceInfo的3个字段
	size_t GetSize() const
	{
		return MediaPieceInfo::object_size + GetPieceLength();
	}

	/// 获取piece索引
	UINT32 GetPieceIndex() const { return PieceIndex; }

	/// 获取piece类型
	UINT8 GetPieceType() const { return PieceType; }

	UINT8 GetPieceLevel() const { return PieceLevel; }

	UINT32 GetPieceLength() const { return GetPieceBodyLength(); }

	MediaPieceInfo GetPieceInfo() const
	{
		MediaPieceInfo pieceInfo;
		pieceInfo.PieceType = PieceType;
		pieceInfo.PieceIndex = PieceIndex;
		pieceInfo.PieceLength = GetPieceLength();
		return pieceInfo;
	}

	/// piece body指MediaPieceInfo之后的部分，但不包括signature的部分
	virtual size_t GetPieceBodyLength() const = 0;
};

class MonoMediaHeaderPiece : public MonoMediaPiece
{
public:
	UINT16 MediaType;
	pool_byte_buffer InfoData;
	pool_byte_buffer HeaderData;
	pool_byte_buffer ProfileData;

	/// 构造一个空的header piece，peer端使用
	explicit MonoMediaHeaderPiece( UINT8 pieceLevel, UINT32 pieceIndex )
		: MonoMediaPiece( pieceLevel, PPDT_MEDIA_HEADER, pieceIndex )
	{
	}

	/// 构造一个完整的header piece，由source使用
	explicit MonoMediaHeaderPiece( UINT32 pieceIndex, UINT16 mediaType, 
			UINT16 infoLen, const BYTE* info, 
			UINT32 headerLen, const BYTE* header, 
			UINT16 profileLen, const BYTE* profile )
		: MonoMediaPiece(1, PPDT_MEDIA_HEADER, pieceIndex)
		, MediaType( mediaType )
	{
		InfoData.assign( info, infoLen );
		HeaderData.assign( header, headerLen );
		ProfileData.assign( profile, profileLen );
	}

	/// 获取媒体类型
	UINT16 GetMediaType() const { return MediaType; }

	/// 获取信息数据长度
	size_t GetInfoLength() const { return InfoData.size(); }

	/// 获取媒体头数据长度
	size_t GetHeaderLength() const { return HeaderData.size(); }

	/// 获取profile数据长度
	size_t GetProfileLength() const { return ProfileData.size(); }

	/// 获取信息数据缓冲区
	const BYTE* GetInfo() const { return InfoData.data(); }

	/// 获取媒体头数据缓冲区
	const BYTE* GetHeader() const { return HeaderData.data(); }

	/// 获取profile数据缓冲区
	const BYTE* GetProfile() const { return ProfileData.data(); }

	virtual size_t GetPieceBodyLength() const
	{
		return MediaHeaderPieceInfo::object_size + InfoData.size() + HeaderData.size() + ProfileData.size();
	}
};


class MonoMediaDataPiece : public MonoMediaPiece
{
public:
	/// 时间戳
	UINT64 TimeStamp;

	/// 对应的header片的索引
	UINT32 HeaderPiece;

	/// 媒体数据
	pool_byte_buffer MediaData;


	/// 构造一个空的data piece，peer端使用
	explicit MonoMediaDataPiece( UINT8 pieceLevel, UINT32 pieceIndex )
		: MonoMediaPiece( pieceLevel, PPDT_MEDIA_DATA, pieceIndex )
		, TimeStamp( 0 )
		, HeaderPiece( 0 )
	{

	}

	/// 预留空间，由source使用
	MonoMediaDataPiece(UINT32 pieceIndex, UINT64 timeStamp, UINT32 headerPiece, size_t maxSize = 16 * 1024)
		: MonoMediaPiece(1, PPDT_MEDIA_DATA, pieceIndex)
		, TimeStamp( timeStamp )
		, HeaderPiece( headerPiece )
	{
		assert(maxSize <= PPL_MAX_PIECE_DATA_LENGTH);
		//		this->InitDataPiece(timeStamp, headerPiece, 0, NULL);
		assert(GetPieceIndex() > GetHeaderPiece());
		MediaData.reserve( maxSize );
	}

	/// 直接构造完整的媒体数据报文，由source使用
	MonoMediaDataPiece(UINT32 pieceIndex, UINT64 timeStamp, UINT32 headerPiece, size_t dataLen, const void* data)
		: MonoMediaPiece(1, PPDT_MEDIA_DATA, pieceIndex)
		, TimeStamp( timeStamp )
		, HeaderPiece( headerPiece )
	{
		assert(dataLen > 0);
		assert(dataLen <= PPL_MAX_PIECE_DATA_LENGTH);
		MediaData.assign( static_cast<const BYTE*>( data ), dataLen );
		//		this->InitDataPiece(timeStamp, headerPiece, dataLen, data);
		assert(GetPieceIndex() > GetHeaderPiece());
	}


	const BYTE* GetMediaData() const { return MediaData.data(); }
	size_t GetMediaDataLength() const { return MediaData.size(); }

	/// 获取数据片的时间戳
	UINT64 GetTimeStamp() const { return TimeStamp; }

	/// 获取数据片所属的头部的片索引
	UINT32 GetHeaderPiece() const { return HeaderPiece; }


	bool AppendData(size_t dataLen, const BYTE* data)
	{
		assert(dataLen > 0 && data != NULL);
		if (MediaData.size() + dataLen > MediaData.capacity())
			return false;
		MediaData.append( data, dataLen );
		return true;
	}


	virtual size_t GetPieceBodyLength() const
	{
		assert( MediaData.size() > 0 );
		return MediaDataPieceInfo::object_size + MediaData.size();
	}

};

#endif
