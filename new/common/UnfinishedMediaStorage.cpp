
#include "StdAfx.h"

#include "UnfinishedMediaStorage.h"
#include "MediaPiece.h"
#include "framework/log.h"

#include <synacast/protocol/SubMediaPiece.h>
#include <ppl/data/stlutils.h>
#include <ppl/util/macro.h>



UnfinishedMediaPiecePtr UnfinishedSubPieceStorage::GetSubPiece(UINT pieceIndex) const
{
	return ptr_maps::find(m_subPieces, pieceIndex);
}
void UnfinishedSubPieceStorage::Clear()
{
	m_subPieces.clear();
}

size_t UnfinishedSubPieceStorage::RemoveOld(UINT minIndex)
{
	size_t deletedCount = 0;
	UnfinishedMediaDataPieceCollection::iterator subPieceIter = m_subPieces.begin();
	while (subPieceIter != m_subPieces.end())
	{
		if (subPieceIter->first < minIndex)
		{
			m_subPieces.erase(subPieceIter++);
			++deletedCount;
		}
		else
		{
			break;
		}
	}
	return deletedCount;
}

SubMediaPiecePtr UnfinishedSubPieceStorage::GetSubPiece(UINT pieceIndex, UINT8 subPieceIndex) const
{
	UnfinishedMediaDataPieceCollection::const_iterator iter = m_subPieces.find(pieceIndex);
	if (iter == m_subPieces.end())
		return SubMediaPiecePtr();
	return iter->second->GetSubPiece(subPieceIndex);
}

size_t UnfinishedSubPieceStorage::GetReceivedSubPieceCount(UINT pieceIndex) const
{
	UnfinishedMediaDataPieceCollection::const_iterator iter = m_subPieces.find(pieceIndex);
	if (iter == m_subPieces.end())
		return 0;
	return iter->second->GetReceivedCount();
}

size_t UnfinishedSubPieceStorage::GetInterested(UINT pieceIndex, std::vector<UINT8>& interestedSubPieceIndexes) const
{
	interestedSubPieceIndexes.clear();
	UnfinishedMediaDataPieceCollection::const_iterator iter = m_subPieces.find(pieceIndex);
	if (iter == m_subPieces.end())
	{
		return 0;
	}
	UnfinishedMediaPiecePtr subPieceInfo = iter->second;
	for (UINT8 i = 0; i < subPieceInfo->GetTotalCount(); ++i)
	{
		if (subPieceInfo->Contains(i) == false)
		{
			interestedSubPieceIndexes.push_back(i);
		}
	}
	LIVE_ASSERT(!interestedSubPieceIndexes.empty());
	LIVE_ASSERT(interestedSubPieceIndexes.size() + subPieceInfo->GetReceivedCount() == subPieceInfo->GetTotalCount());
	LIVE_ASSERT(subPieceInfo->GetTotalCount() > 0);
	return subPieceInfo->GetTotalCount();
}

bool UnfinishedSubPieceStorage::HasSubPiece(UINT pieceIndex) const
{
	return containers::contains(m_subPieces, pieceIndex);
}

bool UnfinishedSubPieceStorage::HasSubPiece(UINT pieceIndex, UINT8 subPieceIndex) const
{
	UnfinishedMediaDataPieceCollection::const_iterator iter = m_subPieces.find(pieceIndex);
	if (iter == m_subPieces.end())
		return false;
	return !!iter->second->GetSubPiece(subPieceIndex);
}

UnfinishedMediaPiecePtr UnfinishedSubPieceStorage::AddSubPiece(SubMediaPiecePtr subPiece)
{
	//VIEW_INFO("Storage::AddSubPiece ");
	LIVE_ASSERT(subPiece->GetSubPieceCount() < 64);
	LIVE_ASSERT(subPiece->GetSubPieceIndex() < subPiece->GetSubPieceCount());
	UnfinishedMediaPiecePtr piece;
	UINT pieceIndex = subPiece->GetPieceIndex();
	UINT8 subPieceIndex = subPiece->GetSubPieceIndex();
	//VIEW_INFO("Storage::AddSubPiece 00 " << make_tuple(pieceIndex, subPieceIndex));
	if (HasSubPiece(pieceIndex, subPieceIndex) == true)
		return UnfinishedMediaPiecePtr();
	UnfinishedMediaDataPieceCollection::iterator iter = m_subPieces.find(pieceIndex);
	if (iter == m_subPieces.end())
	{
		//VIEW_INFO("Storage::AddSubPiece 01 " << make_tuple(pieceIndex, subPieceIndex));
		piece.reset(new UnfinishedMediaPiece(subPiece));
		m_subPieces[pieceIndex] = piece;
		//VIEW_INFO("Storage::AddSubPiece 02 " << make_tuple(pieceIndex, subPieceIndex));
	}
	else
	{
		//VIEW_INFO("Storage::AddSubPiece 11 " << make_tuple(pieceIndex, subPieceIndex));
		piece = iter->second;
		if (piece->AddSubPiece(subPiece) == false)
		{
			//VIEW_INFO("Storage::AddSubPiece 12 " << make_tuple(pieceIndex, subPieceIndex));
			return UnfinishedMediaPiecePtr();
		}
		//VIEW_INFO("Storage::AddSubPiece 13 " << make_tuple(pieceIndex, subPieceIndex));
	}
	//VIEW_INFO("Storage::AddSubPiece 2 " << make_tuple(pieceIndex, subPieceIndex));
	if (piece->IsFinished())
	{
		//VIEW_INFO("Storage::AddSubPiece 21 " << make_tuple(pieceIndex, subPieceIndex));
		//VIEW_INFO("Storage::AddSubPiece 22 " << make_tuple(pieceIndex, subPieceIndex));
		// remove sub piece
		m_subPieces.erase(pieceIndex);
		//VIEW_INFO("Storage::AddSubPiece 23 " << make_tuple(pieceIndex, subPieceIndex));
	}
	//VIEW_INFO("Storage::AddSubPiece 3 " << make_tuple(pieceIndex, subPieceIndex));
	return piece;
}

void UnfinishedSubPieceStorage::View() const
{
#ifdef _DEBUG
	VIEW_INFO( "UnfinishedSubPieceStorage::View - Unfinished Count: " << m_subPieces.size() );
	STL_FOR_EACH_CONST( UnfinishedMediaDataPieceCollection, m_subPieces, iter )
	{
		std::ostringstream oss;

		UINT32 pieceIndex = iter->first;
		UnfinishedMediaPiecePtr unfinishedPiece = iter->second;

		oss << pieceIndex << "[";

		const SubPieceCollection & subPieces = unfinishedPiece->GetSubPieces();
		STL_FOR_EACH_CONST( SubPieceCollection, subPieces, subPieceIter )
		{
			SubMediaPiecePtr subPiece = subPieceIter->second;
			LIVE_ASSERT( pieceIndex == subPiece->GetPieceIndex() );
			oss << subPiece->GetSubPieceIndex() << "/" << subPiece->GetSubPieceCount() << ", ";
		}
		oss << "]";

		VIEW_INFO( "UnfinishedSubPieceStorage::View - Piece: " << oss.str() );
	}
#endif // _DEBUG
}

size_t UnfinishedSubPieceStorage::GetTotalSubPieceCount() const
{
#ifdef _DEBUG
	size_t totalCount = 0;
	STL_FOR_EACH_CONST( UnfinishedMediaDataPieceCollection, m_subPieces, iter )
	{
		const SubPieceCollection & subPieces = iter->second->GetSubPieces();
		totalCount += subPieces.size();
	}
	return totalCount;
#else
	return 0;
#endif
}

size_t UnfinishedSubPieceStorage::LimitCount( size_t maxCount )
{
	size_t erasedCount = 0;
	while ( m_subPieces.size() > maxCount )
	{
		m_subPieces.erase( m_subPieces.begin() );
		++erasedCount;
	}
	return erasedCount;
}


