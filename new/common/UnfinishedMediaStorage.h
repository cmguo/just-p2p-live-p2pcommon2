
#ifndef _LIVE_P2PCOMMON2_NEW_COMMON_UNFINISHED_MEDIA_STORAGE_H_
#define _LIVE_P2PCOMMON2_NEW_COMMON_UNFINISHED_MEDIA_STORAGE_H_

#include "piecefwd.h"
#include <synacast/protocol/data/SubPieceUnit.h>
#include <boost/noncopyable.hpp>
#include <map>
#include <vector>

class UnfinishedMediaPiece;

typedef std::map<UINT, UnfinishedMediaPiecePtr> UnfinishedMediaDataPieceCollection;

class UnfinishedSubPieceStorage : private boost::noncopyable
{
public:
	UnfinishedMediaPiecePtr GetSubPiece(UINT pieceIndex) const;
	SubMediaPiecePtr GetSubPiece(UINT pieceIndex, UINT8 subPieceIndex) const;

	/// �Ѿ��յ���sub piece����
	size_t GetReceivedSubPieceCount(UINT pieceIndex) const;

	/// ��ȡ����Ҫ���ص�sub piece������
	size_t GetInterested(UINT pieceIndex, std::vector<UINT8>& interestedSubPieceIndexes) const;

	bool HasSubPiece(UINT pieceIndex) const;
	bool HasSubPiece(UINT pieceIndex, UINT8 subPieceIndex) const;
	bool HasSubPiece(SubPieceUnit subPieceUnit) const { return this->HasSubPiece(subPieceUnit.PieceIndex, subPieceUnit.SubPieceIndex); }

	/// ���sub piece������nullָ���ʾ���ʧ�ܣ�����Ϊ�ɹ���������ص�subpiece��IsFinishedΪtrue�����ʾ���е�sub piece�����������
	UnfinishedMediaPiecePtr AddSubPiece(SubMediaPiecePtr subPiece);

	size_t RemoveOld(UINT minIndex);

	/// ɾ�����������Ƭ
	size_t LimitCount(size_t maxCount);

	void Clear();
	void Remove(UINT pieceIndex)
	{
		m_subPieces.erase(pieceIndex);
	}

	void View() const;

	size_t GetTotalSubPieceCount() const;

	size_t GetSize() const { return m_subPieces.size(); }

private:
	UnfinishedMediaDataPieceCollection m_subPieces;
};

#endif

