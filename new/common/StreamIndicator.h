
#ifndef _LIVE_P2PCOMMON2_NEW_COMMON_STREAM_INDICATOR_H_
#define _LIVE_P2PCOMMON2_NEW_COMMON_STREAM_INDICATOR_H_

#include "piecefwd.h"
#include <ppl/util/time_counter.h>



/// ��λ�õ�ָʾ��
class StreamIndicator
{
public:
	StreamIndicator();

	/// ���
	void Clear();

	/// ���õ�ָ����piece
	void Reset(MediaDataPiecePtr piece);

	/// �ӳ�ʼλ�ø��µ�ָ����piece
	void Update(MediaDataPiecePtr piece, UINT64 receiveTime, bool useNewPiece);

	UINT GetPieceIndex() const { return m_pieceIndex; }
	UINT64 GetTimeStamp() const { return m_timeStamp; }
	const time_counter& GetReceiveTime() const { return m_receiveTime; }
	UINT64 GetElapsedTime() const { return m_receiveTime.elapsed(); }

private:
	/// Piece����
	UINT m_pieceIndex;

	/// ʱ���
	UINT64 m_timeStamp;

	/// �յ���ʱ��
	time_counter m_receiveTime;
};



/// streambuffer��ͳ�������ռ���
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
	/// �ܹ�������pieceƬ��
	size_t m_totalSkippedPieceCount;
};

#endif