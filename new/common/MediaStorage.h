
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



/// piece����
class Storage : private boost::noncopyable, public pool_object
{
public:
	Storage();
	virtual ~Storage();

public:
	bool AddHeaderPiece(MediaHeaderPiecePtr piece);


	/// ��ȡ����������Ƭ
	MediaHeaderPiecePtr GetHeader(UINT pieceIndex) const;
	SubMediaPiecePtr GetSubPiece(UINT pieceIndex, UINT8 subPieceIndex) const;
	SubMediaPiecePtr GetSubPiece(UINT64 inTS) const;

	size_t RemoveOldHeaders(size_t maxCount, UINT upperBound);

	/// �Ƿ��и���������Ƭ
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

	/// ɾ�����ڵ�Ƭ������ʵ��ɾ����Ƭ��(�ο���׼�㣬minBufferTimeΪ��С�����Ļ�������С, upperBoundΪɾ��ʱ�����ޣ�lowerBoundΪɾ��ʱ������)
	size_t RemoveExpired(const StreamIndicator& baseIndicator, UINT minBufferTime, UINT upperBound, UINT lowerBound);

	/// ɾ�����ڵ�Ƭ������ʵ��ɾ����Ƭ��(�ο���׼�㣬minBufferTimeΪ��С�����Ļ�������С)������source��
	size_t RemoveExpired(const StreamIndicator& baseIndicator, UINT minBufferTime);

	///��õ�ǰ��������С��������DataPacket���
	UINT GetMinIndex() const;

	///��õ�ǰ����������������DataPacket���
	UINT GetMaxIndex() const;

	///�����һƬ�����DataPacket��Piece
	MediaDataPiecePtr GetFirst(UINT index) const;

	///�����һƬ���ڵ�DataPacket��Piece
	MediaDataPiecePtr GetNext(UINT index) const;

	MediaPiecePtr GetMediaPiece(UINT pieceIndex) const;

	UINT GetBufferSize() const;
	UINT GetBufferTime() const;
	/// �����(��)���䷶Χ�ڵ�ʵ��Ƭ��
	size_t CountPieces(UINT rangeMin, UINT rangeMax) const;

	BitMap BuildBitmap() const;
	BitMap BuildBitmap(UINT maxLength) const;

	/// ��ȡpiece����
	size_t GetPieceCount() const { return m_dataPieces.size(); }

	size_t CountRange(UINT refIndex) const;

	void Serialize() const;


	void ViewStreamBuffer() const;

	UINT GetPieceSize() const { return m_pieceSize; }

private:
	/// ɾ��ĳ��Piece������ֵ�е�bool��ʾ�ýڵ��Ƿ���DataPacket��UINT��ʾ��packet������(DoDelete�����iter����Ч��)
	std::pair<bool, UINT> DoDelete(PieceInfoCollection::iterator iter);

	//ɾ��ĳ��Piece������ֵ�е�bool��ʾ�ýڵ��Ƿ���DataPacket��UINT��ʾ��packet������
	std::pair<bool, UINT> Delete(PieceInfoCollection::iterator iter);

private:
	MediaHeaderPieceCollection m_headers;
	PieceInfoCollection	m_dataPieces;
	UINT m_pieceSize;

};

#endif



