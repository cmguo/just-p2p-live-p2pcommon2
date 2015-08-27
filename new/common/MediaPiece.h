
#ifndef _LIVE_P2PCOMMON2_NEW_COMMON_MEDIA_PIECE_H_
#define _LIVE_P2PCOMMON2_NEW_COMMON_MEDIA_PIECE_H_


#include "piecefwd.h"
#include "framework/memory.h"
#include <synacast/protocol/data/MediaPieceInfo.h>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <ppl/util/time_counter.h>
#include <vector>
#include <map>

class DataSigner;
typedef boost::shared_ptr<DataSigner> DataSignerPtr;

/// 最大sub-piece的数据部分大小
const size_t MAX_SUB_PIECE_DATA_SIZE = 1344;



//class SubPieceDataPacket;
//class PPMediaDataPacket;

typedef std::map<UINT8, SubMediaPiecePtr> SubPieceCollection;



class UnfinishedMediaPiece : public pool_object, private boost::noncopyable
{
public:
	explicit UnfinishedMediaPiece(SubMediaPiecePtr subPiece);
	virtual ~UnfinishedMediaPiece();

	SubMediaPiecePtr GetSubPiece(UINT8 subPieceIndex) const;
	//MediaPiecePacketPtr MakePiece() const;
	bool AddSubPiece(SubMediaPiecePtr subPiece);
	const SubPieceCollection& GetSubPieces() const { return m_subPieces; }

	/// 是否完整
	bool IsFinished() const;

	UINT8 GetTotalCount() const { return m_totalCount; }
	UINT8 GetReceivedCount() const { return static_cast<UINT8>( m_subPieces.size() ); }

	bool Contains(UINT8 subPieceIndex) const;

	const MediaPieceInfo& GetPieceInfo() const { return m_pieceInfo; }
	UINT32 GetPieceIndex() const { return m_pieceInfo.PieceIndex; }
	UINT8 GetPieceType() const { return m_pieceInfo.PieceType; }

private:
	UINT8 m_totalCount;
	SubPieceCollection m_subPieces;
	MediaPieceInfo m_pieceInfo;
	UINT8 m_pieceLevel;
};


/// 切片存储的media piece类
class MediaPiece : public pool_object, private boost::noncopyable
{
protected:
	explicit MediaPiece( UnfinishedMediaPiecePtr subPieces );
	explicit MediaPiece( MonoMediaPiecePtr monoPiece, const pool_byte_buffer& pieceData );
public:
	virtual ~MediaPiece();

	const MediaPieceInfo& GetPieceInfo() const { return m_pieceInfo; }

	UINT32 GetPieceIndex() const { return m_pieceInfo.PieceIndex; }
	UINT8 GetPieceType() const { return m_pieceInfo.PieceType; }
	UINT32 GetPieceLength() const { return m_pieceInfo.PieceLength; }

	UINT8 GetPieceLevel() const { return m_pieceLevel; }

	size_t GetSubPieceCount() const
	{
		return m_subPieces.size();
	}
	SubMediaPiecePtr GetSubPiece(size_t index) const
	{
		if (index >= m_subPieces.size())
			return SubMediaPiecePtr();
		LIVE_ASSERT(m_subPieces[index]);
		return m_subPieces[index];
	}
	const std::vector<SubMediaPiecePtr>& GetSubPieces() const { return m_subPieces; }

protected:
	bool InitBase();
	virtual bool Init() = 0;

protected:
	std::vector<SubMediaPiecePtr> m_subPieces;
	MediaPieceInfo m_pieceInfo;
	UINT8 m_pieceLevel;
};



/// 切片存储的data piece类
class MediaDataPiece : public MediaPiece
{
protected:
	explicit MediaDataPiece( UnfinishedMediaPiecePtr subPieces );
	explicit MediaDataPiece( MonoMediaDataPiecePtr monoPiece, const pool_byte_buffer& pieceData );
public:
	virtual ~MediaDataPiece() { }

	const MediaDataPieceInfo& GetDataPieceInfo() const { return m_dataPieceInfo; }

	size_t GetMediaDataLength() const { return m_dataPieceInfo.MediaDataLength; }

	UINT64 GetTimeStamp() const { return m_dataPieceInfo.TimeStamp; }
	UINT32 GetHeaderPiece() const { return m_dataPieceInfo.HeaderPiece; }

	/// 构造一个完整的data piece packet
	MonoMediaDataPiecePtr ToMonoPiece() const;
	static MediaDataPiecePtr FromMonoPiece( MonoMediaDataPiecePtr monoPiece, DataSignerPtr signer );
	static MediaDataPiecePtr FromSubPieces( UnfinishedMediaPiecePtr subPieces, DataSignerPtr signer );

protected:
	virtual bool Init();

protected:
	MediaDataPieceInfo m_dataPieceInfo;
	//UINT32 m_pieceIndex;
	//UINT8 m_pieceType;
	//size_t m_dataLength;
};

/// 切片存储的header piece类
class MediaHeaderPiece : public MediaPiece
{
protected:
	explicit MediaHeaderPiece( UnfinishedMediaPiecePtr subPieces );
	explicit MediaHeaderPiece( MonoMediaHeaderPiecePtr monoPiece, const pool_byte_buffer& pieceData );
public:
	virtual ~MediaHeaderPiece() { }

	const MediaHeaderPieceInfo& GetHeaderPieceInfo() const { return m_headerPieceInfo; }
	UINT16 GetMediaType() const { return m_headerPieceInfo.MediaType; }

	/// 构造一个完整的header piece packet
	MonoMediaHeaderPiecePtr ToMonoPiece() const;
	static MediaHeaderPiecePtr FromMonoPiece( MonoMediaHeaderPiecePtr piece, DataSignerPtr signer );
	static MediaHeaderPiecePtr FromSubPieces( UnfinishedMediaPiecePtr piece, DataSignerPtr signer );

protected:
	virtual bool Init();

protected:
	MediaHeaderPieceInfo m_headerPieceInfo;
};




/// piece信息
class PieceInfo : public pool_object
{
public:
	PieceInfo() { }
	explicit PieceInfo( MediaDataPiecePtr piece ) : m_subPiece( piece )
	{
	}

	bool IsValid() const { return !!m_subPiece; }
	MediaDataPiecePtr GetPiece() const { return m_subPiece; }
	UINT64 GetReceiveTime() const { return m_receiveTime.get_count(); }

	size_t GetMediaDataLength() const
	{
		return m_subPiece->GetMediaDataLength();
	}

	UINT8 GetSubPieceCount() const
	{
		LIVE_ASSERT( m_subPiece->GetSubPieceCount() < 255 && m_subPiece->GetSubPieceCount() > 0 );
		return static_cast<UINT8>( m_subPiece->GetSubPieceCount() );
	}
	SubMediaPiecePtr GetSubPiece(UINT8 subPieceIndex) const
	{
		return m_subPiece->GetSubPiece(subPieceIndex);
	}


protected:
	/// piece数据包
	//PPMediaDataPacketPtr m_piece;

	/// piece接收的时间
	ppl::util::time_counter m_receiveTime;

	MediaDataPiecePtr m_subPiece;
};

#endif

