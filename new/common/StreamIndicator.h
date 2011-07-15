
#ifndef _LIVE_P2PCOMMON2_NEW_COMMON_STREAM_INDICATOR_H_
#define _LIVE_P2PCOMMON2_NEW_COMMON_STREAM_INDICATOR_H_

#include "piecefwd.h"
#include <ppl/util/time_counter.h>



/// 流位置的指示器
class StreamIndicator
{
public:
	StreamIndicator();

	/// 清空
	void Clear();

	/// 重置到指定的piece
	void Reset(MediaDataPiecePtr piece);

	/// 从初始位置更新到指定的piece
	void Update(MediaDataPiecePtr piece, UINT64 receiveTime, bool useNewPiece);

	UINT GetPieceIndex() const { return m_pieceIndex; }
	UINT64 GetTimeStamp() const { return m_timeStamp; }
	const time_counter& GetReceiveTime() const { return m_receiveTime; }
	UINT64 GetElapsedTime() const { return m_receiveTime.elapsed(); }

private:
	/// Piece索引
	UINT m_pieceIndex;

	/// 时间戳
	UINT64 m_timeStamp;

	/// 收到的时间
	time_counter m_receiveTime;
};



/// streambuffer的统计数据收集器
class StreamFeedback
{
public:
	StreamFeedback();

	void Clear();
	void Record(bool pushed);
	void Push(size_t count);
	void Discard(size_t count);
	void Skip(size_t count);

	int GetPushed() const { return m_pushedPieceCount; }
	size_t GetTotalPushed() const { return m_totalPushedPieceCount; }
	int GetDownloaded() const { return m_downloadedPieceCount; }
	size_t GetTotalDownloaded() const { return m_totalDownloadedPieceCount; }
	size_t GetTotalSkipped() const { return m_totalSkippedPieceCount; }

private:
	int m_pushedPieceCount;
	int m_downloadedPieceCount;
	size_t m_totalPushedPieceCount;
	size_t m_totalDownloadedPieceCount;
	/// 总共跳过的piece片数
	size_t m_totalSkippedPieceCount;
};

#endif