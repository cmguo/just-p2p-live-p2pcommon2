
#include "StdAfx.h"

#include "PieceRequestManager.h"
#include "PeerConnection.h"
#include "framework/log.h"


bool PieceRequestManager::Add(UINT pieceIndex, PeerConnection* connection, UINT timeout)
{
	assert(connection != NULL);
	PieceTaskInfo request(pieceIndex, connection, timeout); 
	std::pair<PieceTaskInfoCollection::iterator, bool> result = m_requests.insert(std::make_pair(pieceIndex, request));
	if (result.second)
	{
		//AddCount(pieceIndex, connection->IsUDPT() );
		return true;
	}
	// 插入失败，请求已经存在
	assert(!"Piece has already been requested.");
	return false;
}

bool PieceRequestManager::Add(SubPieceUnit subPiece, PeerConnection* connection, UINT externTimeout)
{
	assert(connection != NULL);
	PieceTaskInfo subPieceRequest(subPiece, connection, externTimeout);
	std::pair<PieceTaskInfoCollection::iterator, bool> result = m_requests.insert(std::make_pair(subPiece, subPieceRequest));
	if (result.second)
	{
		//AddCount(subPiece.PieceIndex, connection->IsUDPT());
		return true;
	}
	// 插入失败，请求已经存在
//	assert(!"SubPiece has already been requested.");
	return false;
}

void PieceRequestManager::AddCount(UINT pieceIndex, bool isUdpt)
{
	if( m_request_counts.find(pieceIndex) == m_request_counts.end() )
	{
		m_request_counts[pieceIndex] = 0;
	}
	
	// TCP的请求次数+1
	m_request_counts[pieceIndex] += 1;
	
	if( m_request_counts[pieceIndex] > 1 )
	{
		VIEW_INFO("RequestCountMore "<<pieceIndex<<" "<<m_request_counts[pieceIndex]<<" End");
	}
}

bool PieceRequestManager::Remove(UINT pieceIndex, const PeerConnection* connection)
{
	PieceTaskInfoCollection::iterator pos = m_requests.find(SubPieceUnit(pieceIndex));
	if (pos == m_requests.end())
		return false;
	if (connection != NULL && pos->second.GetConnection() != connection)
		return false;
	m_requests.erase(pos);
	return true;
}

bool PieceRequestManager::RemoveSubPieceRequest(SubPieceUnit subPiece, const PeerConnection* connection)
{
	PieceTaskInfoCollection::iterator pos = m_requests.find(subPiece);
	if (pos == m_requests.end()) 
		return false;
	if (connection != NULL && pos->second.GetConnection() != connection)
		return false;
	m_requests.erase(pos);
	return true;
}

size_t PieceRequestManager::Remove(PeerConnection* conn)
{
	size_t count = 0;
	PieceTaskInfoCollection::iterator iter = m_requests.begin();
	while (iter != m_requests.end())
	{
		if (iter->second.GetConnection() == conn)
		{
			m_requests.erase(iter++);
			++count;
		}
		else
		{
			++iter;
		}
	}
	return count;
}

size_t PieceRequestManager::CheckTimeout(UINT currentPos)
{
//#pragma message("暂时不检查piece下载的超时，即不进行重复的下载")
//	return;

	size_t timeoutCount = 0;
	PieceTaskInfoCollection::iterator iter = m_requests.begin();
	//VIEW_INFO("Size: "<< m_requests.size());
	while (iter != m_requests.end())
	{
		SubPieceUnit subPiece = iter->first;
		const PieceTaskInfo& request = iter->second;

		UINT timeout = request.GetTimeout();
		UINT requestTime = request.GetStartTime().elapsed32();
		if ( requestTime >= timeout)
		{
			MANAGER_DEBUG("PieceRequestManager::CheckTimeout " << make_tuple(subPiece, currentPos, subPiece.PieceIndex-currentPos));
			PeerConnection* connection = request.GetConnection();
			VIEW_INFO( "ExternalTimeout is UDP? " << connection->IsUDP()<<" "<<connection->GetKeyAddress() << " " <<subPiece << " " << requestTime << " > " << timeout << " End" );

			AddCount(subPiece.PieceIndex, connection->IsUDP());
//			assert(requestTime < 5000);
			++timeoutCount;
			m_requests.erase(iter++);
		}
		else
		{
			++iter;
		}
	}
	return timeoutCount;
}

void PieceRequestManager::RemoveOld(UINT skipIndex)
{
	// 删除小于
	PieceCountInfoCollection::iterator endPos = m_request_counts.lower_bound(skipIndex);
	m_request_counts.erase(m_request_counts.begin(), endPos);
}

int PieceRequestManager::GetRequestCount(UINT pieceIndex)
{
	if( m_request_counts.find(pieceIndex) == m_request_counts.end() )
	{
		return 0;
	}
	else
	{
		return m_request_counts[pieceIndex];
	}
}

void PieceRequestManager::CheckRequestByConnection( PeerConnection* conn )
{
	PieceTaskInfoCollection::iterator iter = m_requests.begin();
	while (iter != m_requests.end())
	{
		UINT pieceIndex = iter->first.PieceIndex;
		const PieceTaskInfo& request = iter->second;
		(void)pieceIndex;
                if( request.GetConnection() == conn )
		{
			VIEW_INFO( "CheckRequestByConnection "<<*conn<<" "<<pieceIndex<<" "<<request.GetStartTime().elapsed()<<" "<<request.GetTimeout() );
		}
		++iter;
	}
}

void PieceRequestManager::View() const
{
#ifdef _DEBUG
	VIEW_INFO( "PieceRequestManager::View - request count: " << m_requests.size() << " requested count: " << m_request_counts.size() );
	STL_FOR_EACH_CONST(PieceTaskInfoCollection,m_requests, subPieceRequestIter )
	{
		VIEW_INFO( "PieceRequestManager::View - Sub Piece: " << subPieceRequestIter->first );
	}

	//STL_FOR_EACH_CONST(PieceCountInfoCollection, m_request_counts, )
	//{

	//}	
#endif // _DEBUG
}
#ifdef _PPL_RUN_TEST

class TestPieceRequestManager : public PieceRequestManager
{
	friend class PieceRequestManagerTestCase;
};

class PieceRequestManagerTestCase : public ppl::util::test_case
{
	virtual void DoRun()
	{
		TestPieceRequestManager mgr;
		assert(mgr.m_requests.size() == 0);

		assert(mgr.Add(1, NULL, 10*1000));
		assert(mgr.m_requests.size() == 1);
		assert(!mgr.Add(1, NULL, 10*1000));
		assert(mgr.m_requests.size() == 1);

		assert(mgr.Add(20000, NULL, 10*1000));
		assert(mgr.m_requests.size() == 2);

		assert(mgr.Add(2001, NULL, 10*1000));
		assert(mgr.m_requests.size() == 3);

		assert(!mgr.Remove(200, NULL));
		assert(mgr.m_requests.size() == 3);

		assert(mgr.Remove(2001, NULL));
		assert(mgr.m_requests.size() == 2);

		assert(mgr.Remove(1, NULL));
		assert(mgr.m_requests.size() == 1);

		assert(mgr.Remove(20000, NULL));
		assert(mgr.m_requests.size() == 0);
	}
};

#endif

