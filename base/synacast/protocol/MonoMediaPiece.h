
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

	/// piece�ߴ磬Ҳ��ǩ��ʱ�����ݳ��ȣ�����MediaPieceInfo��3���ֶ�
	size_t GetSize() const
	{
		return MediaPieceInfo::object_size + GetPieceLength();
	}

	/// ��ȡpiece����
	UINT32 GetPieceIndex() const { return PieceIndex; }

	/// ��ȡpiece����
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

	/// piece bodyָMediaPieceInfo֮��Ĳ��֣���������signature�Ĳ���
	virtual size_t GetPieceBodyLength() const = 0;
};

class MonoMediaHeaderPiece : public MonoMediaPiece
{
public:
	UINT16 MediaType;
	pool_byte_buffer InfoData;
	pool_byte_buffer HeaderData;
	pool_byte_buffer ProfileData;

	/// ����һ���յ�header piece��peer��ʹ��
	explicit MonoMediaHeaderPiece( UINT8 pieceLevel, UINT32 pieceIndex )
		: MonoMediaPiece( pieceLevel, PPDT_MEDIA_HEADER, pieceIndex )
	{
	}

	/// ����һ��������header piece����sourceʹ��
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

	/// ��ȡý������
	UINT16 GetMediaType() const { return MediaType; }

	/// ��ȡ��Ϣ���ݳ���
	size_t GetInfoLength() const { return InfoData.size(); }

	/// ��ȡý��ͷ���ݳ���
	size_t GetHeaderLength() const { return HeaderData.size(); }

	/// ��ȡprofile���ݳ���
	size_t GetProfileLength() const { return ProfileData.size(); }

	/// ��ȡ��Ϣ���ݻ�����
	const BYTE* GetInfo() const { return InfoData.data(); }

	/// ��ȡý��ͷ���ݻ�����
	const BYTE* GetHeader() const { return HeaderData.data(); }

	/// ��ȡprofile���ݻ�����
	const BYTE* GetProfile() const { return ProfileData.data(); }

	virtual size_t GetPieceBodyLength() const
	{
		return MediaHeaderPieceInfo::object_size + InfoData.size() + HeaderData.size() + ProfileData.size();
	}
};


class MonoMediaDataPiece : public MonoMediaPiece
{
public:
	/// ʱ���
	UINT64 TimeStamp;

	/// ��Ӧ��headerƬ������
	UINT32 HeaderPiece;

	/// ý������
	pool_byte_buffer MediaData;


	/// ����һ���յ�data piece��peer��ʹ��
	explicit MonoMediaDataPiece( UINT8 pieceLevel, UINT32 pieceIndex )
		: MonoMediaPiece( pieceLevel, PPDT_MEDIA_DATA, pieceIndex )
		, TimeStamp( 0 )
		, HeaderPiece( 0 )
	{

	}

	/// Ԥ���ռ䣬��sourceʹ��
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

	/// ֱ�ӹ���������ý�����ݱ��ģ���sourceʹ��
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

	/// ��ȡ����Ƭ��ʱ���
	UINT64 GetTimeStamp() const { return TimeStamp; }

	/// ��ȡ����Ƭ������ͷ����Ƭ����
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
