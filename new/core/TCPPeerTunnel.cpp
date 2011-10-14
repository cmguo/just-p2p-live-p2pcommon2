#include "StdAfx.h"

#include "TCPPeerTunnel.h"
#include "PeerConnection.h"
#include "Downloader.h"
#include "common/MediaStorage.h"
#include "PeerManagerImpl.h"

//#if _Savage
#include "common/AppModule.h"
#include "common/StreamBuffer.h"
//#endif

#include <synacast/protocol/SubMediaPiece.h>

#include "ppl/util/time_counter.h"
using namespace ppl::util::detail;

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

PeerTunnel* PeerTunnelFactory::CreateTCP( PeerConnection& connection, Downloader& downloader, int tunnelIndex )
{
	return new TCPPeerTunnel( connection, downloader, tunnelIndex );
}


TCPPeerTunnel::TCPPeerTunnel(PeerConnection& connection, Downloader& downloader, int tunnelIndex) 
: PeerTunnel( connection, downloader, tunnelIndex)
{
	const PeerTunnelConfig& config = m_Connection.GetPeerManager().GetConfig().TunnelConfig;
	m_WindowSize = config.TcpWindowSizeMin;
	m_WindowSizeMin = config.TcpWindowSizeMin;
	m_WindowSizeMax = config.TcpWindowSizeMax;
	m_TcpWindowSizeCal = config.TcpWindowSizeCal;

	m_avrgRTT = connection.GetConnectionInfo().ConnectParam.RTT > 0 
		? min( config.UsedTime,connection.GetConnectionInfo().ConnectParam.RTT) : config.UsedTime;
	m_avrgUsedTime = m_avrgRTT / m_WindowSize;
	m_maxRequestTime = m_avrgRTT;

	LIMIT_MIN(m_avrgUsedTime, 10);
}

bool TCPPeerTunnel::IsRequesting() const
{
	return !m_RequestSubPieces.empty();
}

bool TCPPeerTunnel::IsRequesting(SubPieceUnit subPiece) const
{
	STL_FOR_EACH_CONST(RequestingSubPieceSet, m_RequestSubPieces, iter)
	{
		if (iter->first == subPiece)
		{
			return true;
		}
	}
	return false;
}

bool TCPPeerTunnel::RequestSubPiece(SubPieceUnit subPiece, DWORD externalTimeout)
{
	++m_RequestTimes;

	//if( false == m_Connection.IsSupportSubPiece() )
	//{	// ���TCPͨ�� ��֧�� SubPiece
	//	return false;	
	//}

	if( false == m_Connection.RequestSubPiece(subPiece) )
	{	// ֻ��TCP����ʧ�ܣ��Ż᷵��false
		return false;
	}

	DWORD dwNow = ::GetTickCount();
	if (m_RequestSubPieces.size() == 0)
		m_RequestTime = time_counter(dwNow);

	// ���Լ�������������������
	m_RequestSubPieces.push_back(std::make_pair(subPiece, dwNow));
	
	VIEW_INFO("TCPPeerTunnel::RequestSubPiece - RequestSubPiece "<<*this<<" "<<subPiece );
	
	// ��PieceRequestManger ����������
	if (externalTimeout >= 5500 && externalTimeout < m_maxRequestTime)
	{ // Not Urgent
		externalTimeout = min(m_maxRequestTime + 500, 10000);
	}
	m_Downloader.AddRequest( subPiece, &m_Connection, externalTimeout );

	return true;
}

void TCPPeerTunnel::OnReceiveSubPiece(SubMediaPiecePtr subPiecePtr)
{
	// ���ȼ���Ƿ����������
	SubPieceUnit subPiece = subPiecePtr->GetSubPieceUnit();

	// ͨ��downloader��Add��Storage����
	m_Downloader.OnSubPieceReceived(subPiecePtr, &m_Connection);

	STL_FOR_EACH(RequestingSubPieceSet, m_RequestSubPieces, iter)
	{
		if (iter->first == subPiece)
		{
			LIVE_ASSERT(iter == m_RequestSubPieces.begin());

			VIEW_INFO("TCPPeerTunnel::OnReceiveSubPiece - subPiece recieve time(TCP): "<< m_RequestTime.elapsed() << " " << subPiece );
			UpdateAvrgUsedTime(m_RequestTime.elapsed()); 
			m_RequestTime.sync();
			UpdateRTT(m_RequestTime.get_count() - iter->second);

			// ���Լ�������� m_RequestSubPieces ��ɾ����SubPiece
			m_RequestSubPieces.erase(iter);

			// Modified by Tady, 011511: Spark! Moved into Func. AdjustWindowSize().
			//  	if (m_TcpWindowSizeCal / m_avrgUsedTime > m_WindowSize) 
			// 		++m_WindowSize;
			//  	else if (m_TcpWindowSizeCal / m_avrgUsedTime < m_WindowSize - 1)
			//  		--m_WindowSize;
			// 	LIMIT_MIN_MAX(m_WindowSize, m_WindowSizeMin, 50);
			// 	VIEW_INFO(" TCPPeerTunnel::RequestFromTaskQueue " << m_WindowSize);
			
#if !_Savage
			RequestTillFullWindow();
#else
			RequestTillFullWindow2();
#endif

			break;
		}
	}
}

void TCPPeerTunnel::OnPieceNotFound(UINT pieceIndex)
{
	//����û�н�����
	m_Downloader.OnPieceReceiveFailed(pieceIndex, &m_Connection);
	
// 	Modified by Tady, 011511: Each request must have A response.
	STL_FOR_EACH(RequestingSubPieceSet, m_RequestSubPieces, iter)
	{
		if (iter->first.PieceIndex == pieceIndex)
		{
			LIVE_ASSERT(iter == m_RequestSubPieces.begin());

			UpdateAvrgUsedTime(m_RequestTime.elapsed()); 
			m_RequestTime.sync();
			UpdateRTT(m_RequestTime.get_count() - iter->second);

			// ���Լ�������� m_RequestSubPieces ��ɾ����SubPiece
			m_RequestSubPieces.erase(iter);

#if !_Savage
			RequestTillFullWindow();
#else
			RequestTillFullWindow2();
#endif

			break;
		}
	}
}

void TCPPeerTunnel::OnSubPieceNotFound(SubPieceUnit subPiece)
{
	m_Downloader.OnSubPieceReceivedFailed(subPiece, &m_Connection);
	//����û�н�����

	STL_FOR_EACH(RequestingSubPieceSet, m_RequestSubPieces, iter)
	{
		if (iter->first == subPiece)
		{
			LIVE_ASSERT(iter == m_RequestSubPieces.begin());

			UpdateAvrgUsedTime(m_RequestTime.elapsed()); 
			m_RequestTime.sync();
			UpdateRTT(m_RequestTime.get_count() - iter->second);

			// ���Լ�������� m_RequestSubPieces ��ɾ����SubPiece
			m_RequestSubPieces.erase(iter);

#if !_Savage
			RequestTillFullWindow();
#else
			RequestTillFullWindow2();
#endif

			break;
		}
	}
}

bool TCPPeerTunnel::CheckRequestPieceTimeout()
{
	if( false == IsRequesting() )
	{	// û������SubPiece,�����ڳ�ʱ
		return false;
	}

	// TCP, no packet drop.
	return true;
}

DWORD TCPPeerTunnel::GetSortValue()
{
	DWORD result = 0;
	
	DWORD usedTime = GetUsedTime();
	VIEW_INFO("TCPPeerTunnel::GetSortValue() "<< *this << " Usedtime=" << usedTime);


	if ( false == IsRequesting() )
	{	// �������û�����󣬶��ù���һƬ��ʱ�����
		result = ::GetTickCount() + usedTime;
	}
	else
	{	// ����������󣬶��á����ϴ�����ʱ�俪ʼ��Ƭ��ʱ������
		result = m_RequestTime.get_count() + m_RequestSubPieces.size() * usedTime;
	}
	return result;
}

DWORD TCPPeerTunnel::GetRealUsedTime()
{
	if (m_avrgUsedTime > 0)
	{
		return m_avrgUsedTime;
	}

	return m_Connection.GetPeerManager().GetConfig().TunnelConfig.UsedTime;

}
DWORD TCPPeerTunnel::GetUsedTime()
{
	if (m_avrgUsedTime == 0)
	{
		return m_Connection.GetPeerManager().GetConfig().TunnelConfig.UsedTime;
	}
	else
	{
		if (m_RequestSubPieces.size() > 0)
		{
			UINT usedTime = m_RequestTime.elapsed();
			if (usedTime > m_avrgUsedTime)
			{
				return m_avrgUsedTime * 0.8 + m_avrgUsedTime * 0.2;
			}

		}
		return m_avrgUsedTime;
	}
}

UINT TCPPeerTunnel::GetRequestCount() const
{
	return (UINT)m_RequestSubPieces.size();
}

bool TCPPeerTunnel::RequestFromTaskQueue()
{
	VIEW_INFO( "TCPPeerTunnel::RequestFromTaskQueue - Requesting Count: " << GetRequestCount() );

#if !_Savage
	return RequestTillFullWindow();
#else
	return RequestTillFullWindow2();
#endif
}

bool TCPPeerTunnel::RequestTillFullWindow()
{		
	while( m_RequestSubPieces.size() < m_WindowSize && ! m_TaskQueue.empty())
	{
		SubPieceTask nextTask;
		if (! m_TaskQueue.empty()) 
		{
			nextTask = m_TaskQueue.front();
			m_TaskQueue.pop_front();
		}


		if ( !m_Downloader.NeedDownload(nextTask.subPiece) )
		{	// ����Ѿ�������Ƭ������ ���� ������������� ��ô�Ͳ�������
			continue;
		}

		if ( false == RequestSubPiece(nextTask.subPiece, nextTask.externalTimeOut) )
		{
			// �������ʧ��˵�������������⣬��ô�������Ӧ�ñ��ϵ�
			return false;
		}
	}

	return true;
}

bool TCPPeerTunnel::RequestTillFullWindow2()
{		
	SubPieceTask* pTask;
	UINT uStartIndex = m_Connection.GetAppModule().GetStreamBuffer().GetDownloadStartIndex();

	if (!IsFreezing() // Modified by Tady, 011511: Spark!
		&& m_taskCollectionPtr != NULL && !m_taskCollectionPtr->empty()
		&& uStartIndex <= m_Connection.GetMaxIndex() )
	{
		for (HealthyDegreeCollection2::iterator iter = m_taskCollectionPtr->begin(); iter != m_taskCollectionPtr->end();)
		{	
#if NO_HEALTHCOUNTCALC
			pTask = &(*iter);
			if (m_RequestSubPieces.size() >= m_WindowSize 
				|| pTask->subPiece.PieceIndex > m_Connection.GetMaxIndex())
#else  // !NO_HEALTHCOUNTCALC
			pTask = &(iter->second);
			if (m_RequestSubPieces.size() >= m_WindowSize) 
#endif // !NO_HEALTHCOUNTCALC
			{
				break;
			}

			if (!m_Connection.HasPiece(pTask->subPiece.PieceIndex))
			{
				++iter;
				continue;
			}

			if ( false == RequestSubPiece(pTask->subPiece, pTask->externalTimeOut) )
			{
				// �������ʧ��˵�������������⣬��ô�������Ӧ�ñ��ϵ�
				return false;
			}

			m_taskCollectionPtr->erase(iter++);
		}

	}

	// 		if ( !m_Downloader.NeedDownload(nextTask.subPiece) )
	// 		{	// ����Ѿ�������Ƭ������ ���� ������������� ��ô�Ͳ�������
	// 			continue;
	// 		}

	return true;
}
