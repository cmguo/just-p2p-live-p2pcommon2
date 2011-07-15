
#ifndef _LIVE_P2PCOMMON2_NEW_CORE_PIECE_REQUEST_MANAGER_H_
#define _LIVE_P2PCOMMON2_NEW_CORE_PIECE_REQUEST_MANAGER_H_

#include <synacast/protocol/data/SubPieceUnit.h>
#include <ppl/util/time_counter.h>
#include <map>
#include <set>

class PeerConnection;
class Downloader;


/// piece请求信息
class PieceTaskInfo
{
public:
	PieceTaskInfo() : m_startTime(0), m_connection(NULL), m_ExternalTimeout(10*1000)
	{
	}
	explicit PieceTaskInfo(UINT pieceIndex, PeerConnection* connection, UINT timeout) 
		: m_connection(connection), m_ExternalTimeout(timeout)
	{
		m_SubPieceUnit.PieceIndex = pieceIndex;
	}
	explicit PieceTaskInfo(SubPieceUnit subPiece, PeerConnection* connection, UINT timeout) 
		: m_connection(connection), m_ExternalTimeout(timeout)
	{
		m_SubPieceUnit = subPiece;
	}

	UINT GetPieceIndex() const { return m_SubPieceUnit.PieceIndex; }
	const time_counter& GetStartTime() const { return m_startTime; }
	PeerConnection* GetConnection() const { return m_connection; }
	UINT GetTimeout() const { return m_ExternalTimeout; }

private:
	SubPieceUnit m_SubPieceUnit;

	/// piece索引
	//UINT m_pieceIndex;
	/// 请求的开始时间
	time_counter m_startTime;
	/// 此请求所使用的PeerConnection
	PeerConnection* m_connection;
	/// 此请求的超时时间
	UINT m_ExternalTimeout;

//	map<UINT16, SubPieceDataPacket*> m_subPieces;
};




/// 管理piece请求(即管理正在进行的下载任务)
class PieceRequestManager
{
public:
	/// piece是否已经被请求过
	bool IsRequested(UINT pieceIndex) const
	{
		return m_requests.find(SubPieceUnit(pieceIndex)) != m_requests.end();
	}

	bool IsRequested(SubPieceUnit subPiece) const
	{
		return m_requests.find(subPiece) != m_requests.end();
	}

	/// 增加请求
	bool Add(UINT pieceIndex, PeerConnection* connection, UINT timeout);
	bool Add(SubPieceUnit subPiece, PeerConnection* connection, UINT externTimeout);
	void AddCount(UINT pieceIndex, bool isUdpt);

	/// 删除请求
	bool Remove(UINT pieceIndex, const PeerConnection* connection);
	bool RemoveSubPieceRequest(SubPieceUnit subPiece, const PeerConnection* connection);

	/// 删除PeerConnection对应的任务，返回删除的任务个数
	size_t Remove(PeerConnection* conn);

	/// 检查超时
	size_t CheckTimeout(UINT currentPos);
	
	int Count() const { return (int)m_requests.size(); }

	void RemoveOld(UINT skipIndex);

	int GetRequestCount(UINT pieceIndex);

	void CheckRequestByConnection( PeerConnection* conn );

	void View() const;

protected:
	/// piece请求集合
	typedef std::map<SubPieceUnit, PieceTaskInfo> PieceTaskInfoCollection;

	/// 正在进行sub piece请求
	PieceTaskInfoCollection m_requests;

	/// piece请求次数集合,由于推进是按照Piece为单位进行的,所以按照
	typedef std::map<UINT, int> PieceCountInfoCollection;

	/// 正在请求的次数
	PieceCountInfoCollection m_request_counts;
};

#endif

