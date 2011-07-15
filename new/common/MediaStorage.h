
#ifndef _LIVE_P2PCOMMON2_NEW_COMMON_MEDIA_STORAGE_H_
#define _LIVE_P2PCOMMON2_NEW_COMMON_MEDIA_STORAGE_H_

#include "piecefwd.h"
#include "framework/memory.h"
#include <boost/noncopyable.hpp>
#include <map>

class StreamIndicator;
class BitMap;
class PieceInfo;

typedef std::map<UINT, PieceInfo> PieceInfoCollection;
//typedef PieceCollection PieceInfoCollection;
typedef std::map<UINT, MediaHeaderPiecePtr> MediaHeaderPieceCollection;



/// piece集合
class Storage : private boost::noncopyable, public pool_object
{
public:
	Storage();
	virtual ~Storage();

public:
	bool AddHeaderPiece(MediaHeaderPiecePtr piece);


	/// 获取给定索引的片
	MediaHeaderPiecePtr GetHeader(UINT pieceIndex) const;
	SubMediaPiecePtr GetSubPiece(UINT pieceIndex, UINT8 subPieceIndex) const;
	SubMediaPiecePtr GetSubPiece(UINT64 inTS) const;

	size_t RemoveOldHeaders(size_t maxCount, UINT upperBound);

	/// 是否有给定索引的片
	bool HasPiece(UINT pieceIndex) const { return HasDataPiece(pieceIndex) || HasHeader(pieceIndex); }
	bool HasHeader(UINT pieceIndex) const;
	bool HasDataPiece(UINT pieceIndex) const;

	PieceInfo GetFirstPiece() const;
	MediaDataPiecePtr GetLastPiece() const;


	bool IsEmpty() const { return m_dataPieces.empty(); }

	void Clear();
	bool AddDataPiece(MediaDataPiecePtr packet);
	MediaDataPiecePtr GetDataPiece(UINT index) const;
	PieceInfo GetPieceInfo(UINT pieceIndex) const;

	/// 删除过期的片，返回实际删除的片数(参考基准点，minBufferTime为最小保留的缓冲区大小, upperBound为删除时的上限，lowerBound为删除时的下限)
	size_t RemoveExpired(const StreamIndicator& baseIndicator, UINT minBufferTime, UINT upperBound, UINT lowerBound);

	/// 删除过期的片，返回实际删除的片数(参考基准点，minBufferTime为最小保留的缓冲区大小)，用于source端
	size_t RemoveExpired(const StreamIndicator& baseIndicator, UINT minBufferTime);

	///获得当前缓冲区最小的已下载DataPacket编号
	UINT GetMinIndex() const;

	///获得当前缓冲区最大的已下载DataPacket编号
	UINT GetMaxIndex() const;

	///获得这一片最靠近的DataPacket的Piece
	MediaDataPiecePtr GetFirst(UINT index) const;

	///获得下一片存在的DataPacket的Piece
	MediaDataPiecePtr GetNext(UINT index) const;

	MediaPiecePtr GetMediaPiece(UINT pieceIndex) const;

	UINT GetBufferSize() const;
	UINT GetBufferTime() const;
	/// 计算从(闭)区间范围内的实际片数
	size_t CountPieces(UINT rangeMin, UINT rangeMax) const;

	BitMap BuildBitmap() const;
	BitMap BuildBitmap(UINT maxLength) const;

	/// 获取piece个数
	size_t GetPieceCount() const { return m_dataPieces.size(); }

	size_t CountRange(UINT refIndex) const;

	void Serialize() const;


	void ViewStreamBuffer() const;

	UINT GetPieceSize() const { return m_pieceSize; }

private:
	/// 删除某个Piece，返回值中的bool表示该节点是否有DataPacket，UINT表示该packet的索引(DoDelete不检查iter的有效性)
	std::pair<bool, UINT> DoDelete(PieceInfoCollection::iterator iter);

	//删除某个Piece，返回值中的bool表示该节点是否有DataPacket，UINT表示该packet的索引
	std::pair<bool, UINT> Delete(PieceInfoCollection::iterator iter);

private:
	MediaHeaderPieceCollection m_headers;
	PieceInfoCollection	m_dataPieces;
	UINT m_pieceSize;

};

#endif



