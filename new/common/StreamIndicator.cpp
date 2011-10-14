
#include "StdAfx.h"

#include "StreamIndicator.h"
#include "MediaPiece.h"
#include "framework/log.h"

#include <synacast/protocol/MonoMediaPiece.h>


StreamIndicator::StreamIndicator() : m_receiveTime(0)
{
	Clear();
}


void StreamIndicator::Clear()
{
	m_pieceIndex = 0;
	m_timeStamp = 0;
	m_receiveTime = time_counter(0);
}

void StreamIndicator::Reset(MediaDataPiecePtr piece)
{
	CheckDataPiece(piece);
	m_pieceIndex = piece->GetPieceIndex();
	m_timeStamp = piece->GetTimeStamp();
	m_receiveTime.sync();
}

void StreamIndicator::Update(MediaDataPiecePtr piece, UINT64 receiveTime, bool useNewPiece)
{
	CheckDataPiece(piece);
	if (m_pieceIndex == 0 && m_timeStamp == 0)
	{
		LIVE_ASSERT(false);
		Reset(piece);
		return;
	}
	UINT newPieceIndex = piece->GetPieceIndex();
	UINT64 newTimeStamp = piece->GetTimeStamp();
	LIVE_ASSERT(newPieceIndex > m_pieceIndex);
	// 按时间戳来推算正常的接收时间
	UINT64 newReceiveTime = m_receiveTime + static_cast<DWORD>(newTimeStamp - m_timeStamp);
	LIVE_ASSERT(newTimeStamp - m_timeStamp < INT_MAX);
	STREAMBUFFER_DEBUG("StreamIndicator::Update " << make_tuple(m_pieceIndex, m_timeStamp, m_receiveTime) 
		<< " " << make_tuple(newPieceIndex, newTimeStamp, newReceiveTime) 
		<< " " << make_tuple(newPieceIndex - m_pieceIndex, newTimeStamp - m_timeStamp, newReceiveTime - m_receiveTime));
	LIVE_ASSERT(m_receiveTime.get_realtime_count() - newReceiveTime < INT_MAX);
	m_pieceIndex = newPieceIndex;
	m_timeStamp = newTimeStamp;
	if (useNewPiece)
	{
		m_receiveTime = time_counter(receiveTime);
	}
	else
	{
		m_receiveTime = time_counter(newReceiveTime);
	}
}






StreamFeedback::StreamFeedback()
{
	Clear();
}

void StreamFeedback::Clear()
{
	m_pushedPieceCount = 0;
	m_totalPushedPieceCount = 0;
	m_downloadedPieceCount = 0;
	m_totalDownloadedPieceCount = 0;
	m_totalSkippedPieceCount = 0;
}

void StreamFeedback::Record(bool pushed)
{
	++m_totalDownloadedPieceCount;
	++m_downloadedPieceCount;
	if (pushed)
	{
		++m_pushedPieceCount;
		++m_totalPushedPieceCount;
		STREAMBUFFER_DEBUG("StreamFeedback::Record " << m_pushedPieceCount);
	}
}

void StreamFeedback::Push(size_t count)
{
	STREAMBUFFER_DEBUG("StreamFeedback::Push " << make_tuple(m_pushedPieceCount, count));
	m_pushedPieceCount += (int)count;
	m_totalPushedPieceCount += count;
}

void StreamFeedback::Skip(size_t count)
{
	m_totalSkippedPieceCount += count;
}

void StreamFeedback::Discard(size_t count)
{
	if (count == 0)
		return;
	STREAMBUFFER_DEBUG("StreamFeedback::Discard " << make_tuple(m_pushedPieceCount, count));
	if (m_pushedPieceCount >= (int)count)
	{
		m_pushedPieceCount -= (int)count;
	}
	else
	{
		LIVE_ASSERT(false);
		m_pushedPieceCount = 0;
	}
	if (m_downloadedPieceCount >= (int)count)
	{
		m_downloadedPieceCount -= (int)count;
	}
	else
	{
		LIVE_ASSERT(false);
		m_downloadedPieceCount = 0;
	}
}



