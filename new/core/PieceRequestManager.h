
#ifndef _LIVE_P2PCOMMON2_NEW_CORE_PIECE_REQUEST_MANAGER_H_
#define _LIVE_P2PCOMMON2_NEW_CORE_PIECE_REQUEST_MANAGER_H_

#include <synacast/protocol/data/SubPieceUnit.h>
#include <ppl/util/time_counter.h>
#include <map>
#include <set>

class PeerConnection;
class Downloader;


/// piece������Ϣ
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

	/// piece����
	//UINT m_pieceIndex;
	/// ����Ŀ�ʼʱ��
	time_counter m_startTime;
	/// ��������ʹ�õ�PeerConnection
	PeerConnection* m_connection;
	/// ������ĳ�ʱʱ��
	UINT m_ExternalTimeout;

//	map<UINT16, SubPieceDataPacket*> m_subPieces;
};




/// ����piece����(���������ڽ��е���������)
class PieceRequestManager
{
public:
	/// piece�Ƿ��Ѿ��������
	bool IsRequested(UINT pieceIndex) const
	{
		return m_requests.find(SubPieceUnit(pieceIndex)) != m_requests.end();
	}

	bool IsRequested(SubPieceUnit subPiece) const
	{
		return m_requests.find(subPiece) != m_requests.end();
	}

	/// ��������
	bool Add(UINT pieceIndex, PeerConnection* connection, UINT timeout);
	bool Add(SubPieceUnit subPiece, PeerConnection* connection, UINT externTimeout);
	void AddCount(UINT pieceIndex, bool isUdpt);

	/// ɾ������
	bool Remove(UINT pieceIndex, const PeerConnection* connection);
	bool RemoveSubPieceRequest(SubPieceUnit subPiece, const PeerConnection* connection);

	/// ɾ��PeerConnection��Ӧ�����񣬷���ɾ�����������
	size_t Remove(PeerConnection* conn);

	/// ��鳬ʱ
	size_t CheckTimeout(UINT currentPos);
	
	int Count() const { return (int)m_requests.size(); }

	void RemoveOld(UINT skipIndex);

	int GetRequestCount(UINT pieceIndex);

	void CheckRequestByConnection( PeerConnection* conn );

	void View() const;

protected:
	/// piece���󼯺�
	typedef std::map<SubPieceUnit, PieceTaskInfo> PieceTaskInfoCollection;

	/// ���ڽ���sub piece����
	PieceTaskInfoCollection m_requests;

	/// piece�����������,�����ƽ��ǰ���PieceΪ��λ���е�,���԰���
	typedef std::map<UINT, int> PieceCountInfoCollection;

	/// ��������Ĵ���
	PieceCountInfoCollection m_request_counts;
};

#endif

