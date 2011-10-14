#include "StdAfx.h"

#include "UDPPeerTunnel.h"
#include "PeerConnection.h"
#include "Downloader.h"
#include "common/MediaStorage.h"
#include "PeerManagerImpl.h"

#include "common/AppModule.h"
#include "common/StreamBuffer.h"
#include <ppl/data/stlutils.h>
#include <synacast/protocol/SubMediaPiece.h>
#include <ppl/util/time_counter.h>
const static UINT csMaxUDPTimeoutVal = 5500;
const static UINT csAddUDPTimeoutVal = 1200;
using namespace ppl::util::detail;
// Added by Tady, 071608: For Test
//#include "MyOutput.h"

PeerTunnel* PeerTunnelFactory::CreateUDP( PeerConnection& connection, Downloader& downloader, int tunnelIndex )
{
	return new UDPPeerTunnel( connection, downloader, tunnelIndex );
}


//  [8/17/2007 juli]
//  UDPPeerTunnel 开始

UDPPeerTunnel::UDPPeerTunnel(PeerConnection& connection, Downloader& downloader, int tunnelIndex) 
	: PeerTunnel( connection, downloader, tunnelIndex), m_SumDeltaUsedTime(0)
{
	m_calcUsedTimeCount = 0;
	m_SumRequestSuccedCount = 0;
// 	m_nowRequestSuccedRate = 0.5;
// 	m_lastRequestSuccedRate = 0.5;
	m_nowRequestSuccedRate = 1;
	m_lastRequestSuccedRate = 1;

	const PeerTunnelConfig& config = m_Connection.GetPeerManager().GetConfig().TunnelConfig;

	m_WindowSizeMin = config.UdpWindowSizeMin;
	m_WindowSizeMax = config.UdpWindowSizeMax;
	m_WindowSize = 3; // m_WindowSizeMin;

	m_avrgRTT = connection.GetConnectionInfo().ConnectParam.RTT > 0 
		? min( config.UsedTime,connection.GetConnectionInfo().ConnectParam.RTT) : config.UsedTime;
	m_avrgUsedTime = m_avrgRTT / m_WindowSize;
	m_maxRequestTime = m_avrgRTT;
	m_UseArraySize = config.UdpUseArraySize;
	m_udpIntTimeOutMin = config.UdpIntTimeOutMin; //2500
	m_udpIntTimeOutMax = config.UdpIntTimeOutMax; //5500

	m_RequestSubPieces.clear();

	// 防止m_UseArraySize过大导致m_usedTimeArray/m_requestSuccedArray访问越界
	LIMIT_MAX( m_UseArraySize, MAXARRAYSIZE );

	for (UINT i = 0; i < MAXARRAYSIZE; i ++)
	{
		m_usedTimeArray[i] = 0;
		m_requestSuccedArray[i] = false;
	}

	static int debugID = 0;
	m_debugID = debugID++;

#ifdef _LIGHT_TIMEOUT
	m_orderlessCount = 0;
	m_orderlessRate = 0;
#endif
}

bool UDPPeerTunnel::IsRequesting() const
{
	return !m_RequestSubPieces.empty();
}

bool UDPPeerTunnel::IsRequesting(SubPieceUnit subPiece) const
{
#ifndef _LIGHT_TIMEOUT
	return m_RequestSubPieces.find(subPiece) != m_RequestSubPieces.end();
#else
	STL_FOR_EACH_CONST(RequestingSubPieceSet, m_RequestSubPieces, iter)
	{ 
		if (iter->first == subPiece)
		{
			return true;
		}
	}
	return false;
#endif
}

bool UDPPeerTunnel::RequestSubPiece(SubPieceUnit subPiece, DWORD externalTimeout)
{
	++m_RequestTimes;
//	TimeCounter counter;
	if( false == m_Connection.RequestSubPiece(subPiece) )
	{	// 只有TCP发包失败，才会返回false
		return false;
	}
//	LIVE_ASSERT(counter.GetElapsed() < 100);

// 	if (m_RequestSubPieces.size() == 0)
// 	{
// 		m_RequestTime.Sync();
// 	}
	// 在自己的数据中添加这个请求

	
	// 在PieceRequestManger 添加这个请求
	DWORD internalTimeout = m_avrgRTT + csAddUDPTimeoutVal;		// Modified by Tady, 071508.
	LIMIT_MAX(internalTimeout, csMaxUDPTimeoutVal);
	m_InternalTimeout = internalTimeout;
	
//	DWORD externalTimeout = m_Downloader.CalcExternalTimeout(subPiece.PieceIndex);	
	LIMIT_MAX(externalTimeout, internalTimeout);
	VIEW_INFO("RequestSubPiece "<<*this<<" "<<subPiece << " " << make_tuple( externalTimeout, internalTimeout ) );

#ifndef _LIGHT_TIMEOUT
	LIVE_ASSERT(m_RequestSubPieces.find(subPiece) == m_RequestSubPieces.end());
#endif

	DWORD dwNow = ::GetTickCount();	
	if (m_RequestSubPieces.size() == 0)
	{
		m_RequestTime = time_counter(dwNow);
	}

#ifndef _LIGHT_TIMEOUT
	m_RequestSubPieces.insert(make_pair(subPiece, make_pair(dwNow + internalTimeout, dwNow)));
#else
	m_RequestSubPieces.push_back(make_pair(subPiece, make_pair(dwNow + internalTimeout, dwNow)));
#endif
	m_Downloader.AddRequest( subPiece, &m_Connection, externalTimeout );

	return true;
}

bool UDPPeerTunnel::RequestSubPiece2(SubPieceUnit subPiece, DWORD externalTimeout)
{
#if MRP_SUPPORTED
	++m_RequestTimes;
	if (m_Connection.IsMRPSuported())
	{
		SubPieceTask task(subPiece, externalTimeout);
		m_TaskQueue.push_back(task);

// 		MyOutputDebugString("Tady[%d]: Add one Task~~~ ---- TaskqueueSize = [%d]", m_debugID, m_TaskQueue.size());
#if LIGHT_MRP
		if (m_TaskQueue.size() >= 15)
#else // !LIGHT_MRP

		if (m_TaskQueue.size() == 1)
		{
			m_lastTaskTimer.sync();
			//			MyOutputDebugString("Tady[%d]: Timercouter sync!!", m_debugID);
		}
		// 		if (m_TaskQueue.size() > 2 // At least 3 subpieces.
		// 			&& (m_TaskQueue.size() + m_RequestSubPieces.size() >= m_WindowSize || m_TaskQueue.size() >= 15))

		if (m_TaskQueue.size() > m_WindowSize 
			|| /*(m_TaskQueue.size() + m_RequestSubPieces.size() >= m_WindowSize ||*/ m_TaskQueue.size() >= 15)
#endif // !LIGHT_MRP
		{
			RequestMRP();
		}	
	}
	else
#endif // MRP_SUPPORTED
	{
		 return RequestSubPiece(subPiece, externalTimeout);
	}
#if MRP_SUPPORTED
	return true;
#endif // MRP_SUPPORTED
}

void UDPPeerTunnel::RequestMRP()
{
	if (m_TaskQueue.size() == 0)// || m_Connection.IsLastRequestFinished() == false)
		return;

// 	static DWORD sMaxDuration = 0;
 //	DWORD curDuration = m_lastTaskTimer.GetElapsed();
// 	if (sMaxDuration < curDuration)
// 	{
// 		sMaxDuration = curDuration;
// 	} 

//	MyOutputDebugString("Tady[%d]: Send MRP curDuration = <%d>, TaskQueue = {%d} ", m_debugID, curDuration, m_TaskQueue.size());
	for (SubPieceTaskList::iterator iter = m_TaskQueue.begin(); iter != m_TaskQueue.end(); ++iter)
	{
		if (m_Connection.AddSubPieceIntoMRP((*iter).subPiece) == false)
		{	
			m_Connection.ClearMRP();
			return;
		}
	}

	if (m_Connection.FlushOutMRP())
	{
		DWORD internalTimeout = m_avrgRTT + csAddUDPTimeoutVal;		// Modified by Tady, 071508.
		LIMIT_MAX(internalTimeout, csMaxUDPTimeoutVal);
		m_InternalTimeout = internalTimeout;
		DWORD dwNow = ::GetTickCount();

		SubPieceTask curTask;
		while (m_TaskQueue.size() > 0)
		{
			curTask = m_TaskQueue.front();
			m_TaskQueue.pop_front();

			LIMIT_MAX(curTask.externalTimeOut, internalTimeout);
			VIEW_INFO("RequestSubPiece "<<*this<<" "<<curTask.subPiece << " " << make_tuple( curTask.externalTimeOut, internalTimeout ) );
#ifndef _LIGHT_TIMEOUT
			LIVE_ASSERT(m_RequestSubPieces.find(curTask.subPiece) == m_RequestSubPieces.end());
#endif
			if (m_RequestSubPieces.size() == 0)
			{
				m_RequestTime = time_counter(dwNow);
			}
//？？？			m_RequestTime.sync(); // Removed by Tady, 110609: Should Not reset RequestTime.
#ifndef _LIGHT_TIMEOUT
			m_RequestSubPieces.insert(make_pair(curTask.subPiece, make_pair(dwNow + internalTimeout, dwNow)));
#else
			m_RequestSubPieces.push_back(make_pair(curTask.subPiece, make_pair(dwNow + internalTimeout, dwNow)));
#endif
			m_Downloader.AddRequest( curTask.subPiece, &m_Connection, curTask.externalTimeOut );
		}
	}
	else
	{
		m_Connection.ClearMRP();
	}
}

void UDPPeerTunnel::OnReceiveSubPiece(SubMediaPiecePtr subPiecePtr)
{
	// 首先检查是否是我请求的
	m_Downloader.OnSubPieceReceived(subPiecePtr, &m_Connection);

	SubPieceUnit subPiece = subPiecePtr->GetSubPieceUnit();
	DWORD rtt(0xffffffff), usedTime;
	DWORD dwNow = ::GetTickCount();

#ifndef _LIGHT_TIMEOUT

	RequestingSubPieceSet::iterator iter = m_RequestSubPieces.find(subPiece);
	if ( iter == m_RequestSubPieces.end() )
	{	// 这个请求不是我要的
		VIEW_INFO(" recieve a ExternalSubPiece!");
		return;
	}
// 	DWORD dwNow = ::GetTickCount();
// 	DWORD requestTick = iter->second.second;
	rtt = dwNow - iter->second.second;
	usedTime = m_RequestTime.elapsed();
	//LIVE_ASSERT( subPiece == *(m_RequestSubPieces.begin()) );

	// 在自己的请求的 m_RequestSubPieces 中删除该SubPiece
	m_RequestSubPieces.erase(iter);

#else
	//////////////////////////////////////////////////////////////////////////
	// Add by Tady, 051310: LightTimeout Method.
	while(m_Requested.size() > 128)
	{
		m_orderlessCount -= m_Requested.begin()->second;
		m_Requested.erase(m_Requested.begin());
	}
	if (m_Requested.size() > 10 )
	{
		LIMIT_MIN(m_orderlessCount, 0);
		m_orderlessRate = m_orderlessCount * 100 / m_Requested.size();	
	}

	DWORD requestTick(0);
	STL_FOR_EACH(RequestingSubPieceSet, m_RequestSubPieces, iter)
	{
		if (iter->first == subPiece)
		{
			requestTick = iter->second.second;
			rtt = dwNow - requestTick;
			usedTime = m_RequestTime.elapsed();

			if (m_orderlessRate <= 10)
			{
				for (RequestingSubPieceSet::iterator iter2 = m_RequestSubPieces.begin(); iter2 != iter; ++iter2)
				{
					m_Downloader.OnSubPieceReceivedFailed(iter2->first, &m_Connection);
					m_Requested.insert(make_pair(iter2->first, 0));
				}
				++iter;
				m_RequestSubPieces.erase(m_RequestSubPieces.begin(), iter);

			}
			else
			{
				RequestedSubpieceSet::iterator iter1 = m_Requested.find(subPiece);
				if (iter1 != m_Requested.end())
				{
					++(iter1->second);
					++m_orderlessCount;
				}
				for (RequestingSubPieceSet::iterator iter2 = m_RequestSubPieces.begin(); iter2 != iter; ++iter2)
				{
					m_Requested.insert(make_pair(iter2->first, 0));
				}

				m_RequestSubPieces.erase(iter);
			}
			break;
		}
	}
	if (rtt == 0xffffffff)
	{
		VIEW_INFO(" recieve a ExternalSubPiece!");
		RequestedSubpieceSet::iterator iter1 = m_Requested.find(subPiece);
		if (iter1 != m_Requested.end())
		{
			++(iter1->second);
			++m_orderlessCount;
		}
		return;
	}

	STL_FOR_EACH(RequestingSubPieceSet, m_RequestSubPieces, iter)
	{
		if (iter->second.second <= requestTick && iter->second.first > dwNow + 500)
		{
			iter->second.first = dwNow + 500;
		}
		else
			break;
	}

#endif

	VIEW_INFO("subPiece recieve time(UDP): "<< usedTime);
	if (m_maxRequestTime < usedTime)
	{
		m_maxRequestTime = usedTime;
		VIEW_INFO("UDPPeerTunnel::OnReceiveSubPiece maxRequestTime: "<<m_Connection << " " << m_maxRequestTime);
	}

//	LIVE_ASSERT(usedTime < 10000);
//	DWORD firstTick = m_RequestSubPieces.begin()->second.second;
//	LIVE_ASSERT(!(rtt < usedTime && ((dwNow - firstTick) != usedTime)));
	UpdataWindowSize(usedTime, true);
	UpdateRTT(rtt);

// 	static UINT sMaxUsedtime = 0;
// 	if (sMaxUsedtime < usedTime)
// 	{
// 		sMaxUsedtime = usedTime;
// 	}
// 	static UINT sMaxRTT = 0;
// 	if (sMaxRTT < rtt)
// 	{
// 		sMaxRTT = rtt;
// 	}
//	LIVE_ASSERT(usedTime <= rtt);
	m_RequestTime.sync();
	
	VIEW_INFO(" UDPPeerTunnel::RequestFromTaskQueue " << m_WindowSize);
//	MyOutputDebugString("Tady[%d]: curUsedtime = <%d>", m_debugID, usedTime);

#if !_Savage
	RequestTillFullWindow();
#else
	RequestTillFullWindow2();
#endif
}

//void UDPPeerTunnel::OnReceivePiece(PPMediaDataPacketPtr piecePtr)
//{
//	LIVE_ASSERT(false);
//}

void UDPPeerTunnel::OnPieceNotFound(UINT pieceIndex)
{
	m_Downloader.OnPieceReceiveFailed(pieceIndex, &m_Connection);
	//根本没有进来过	
	for( RequestingSubPieceSet::iterator itr = m_RequestSubPieces.begin();
		itr != m_RequestSubPieces.end(); )
	{	
		SubPieceUnit subPiece = itr->first;
		if( subPiece.PieceIndex == pieceIndex)
		{
			m_RequestSubPieces.erase(itr++);
			// 由于没有实际收到SubPiece报文，所以 m_ReceiveSubPieceCountOnce 不加
		}
		else
		{
			itr ++;
		}
	}

	m_RequestTime.sync(); // Added by Tady, 071508

#if !_Savage
	RequestTillFullWindow();
#else
	RequestTillFullWindow2();
#endif

}

void UDPPeerTunnel::OnSubPieceNotFound(SubPieceUnit subPiece)
{
	m_Downloader.OnSubPieceReceivedFailed(subPiece, &m_Connection);
	//根本没有进来过
	DWORD rtt(0xffffffff), usedTime;
	DWORD dwNow = ::GetTickCount();
#ifndef _LIGHT_TIMEOUT
	RequestingSubPieceSet::iterator iter = m_RequestSubPieces.find(subPiece);
	if( iter == m_RequestSubPieces.end() )
	{	// 这个请求不是我要的
		return;
	}
	rtt = dwNow - iter->second.second;
	usedTime = m_RequestTime.elapsed();
	//LIVE_ASSERT( subPiece == *(m_RequestSubPieces.begin()) );
	// 在自己的请求的 m_RequestSubPieces 中删除该SubPiece
	m_RequestSubPieces.erase(subPiece);
#else
	//////////////////////////////////////////////////////////////////////////
	// Add by Tady, 051310: LightTimeout Method.

	while(m_Requested.size() > 128)
	{
		m_orderlessCount -= m_Requested.begin()->second;
		m_Requested.erase(m_Requested.begin());
	}
	if (m_Requested.size() > 10 )
	{
		LIMIT_MIN(m_orderlessCount, 0);
		m_orderlessRate = m_orderlessCount * 100 / m_Requested.size();	
	}

	DWORD requestTick(0);
	STL_FOR_EACH(RequestingSubPieceSet, m_RequestSubPieces, iter)
	{
		if (iter->first == subPiece)
		{
			requestTick = iter->second.second;
			rtt = dwNow - requestTick;
			usedTime = m_RequestTime.elapsed();

			if (m_orderlessRate <= 10)
			{
				for (RequestingSubPieceSet::iterator iter2 = m_RequestSubPieces.begin(); iter2 != iter; ++iter2)
				{
					m_Downloader.OnSubPieceReceivedFailed(iter2->first, &m_Connection);
					m_Requested.insert(make_pair(iter2->first, 0));
				}
				++iter;
				m_RequestSubPieces.erase(m_RequestSubPieces.begin(), iter);

			}
			else
			{
				RequestedSubpieceSet::iterator iter1 = m_Requested.find(subPiece);
				if (iter1 != m_Requested.end())
				{
					++(iter1->second);
					++m_orderlessCount;
				}
				for (RequestingSubPieceSet::iterator iter2 = m_RequestSubPieces.begin(); iter2 != iter; ++iter2)
				{
					m_Requested.insert(make_pair(iter2->first, 0));
				}

				m_RequestSubPieces.erase(iter);
			}
			break;
		}
	}
	if (rtt == 0xffffffff)
	{
		VIEW_INFO(" recieve a ExternalSubPiece!");
		RequestedSubpieceSet::iterator iter1 = m_Requested.find(subPiece);
		if (iter1 != m_Requested.end())
		{
			++(iter1->second);
			++m_orderlessCount;
		}
		return;
	}

	STL_FOR_EACH(RequestingSubPieceSet, m_RequestSubPieces, iter)
	{
		if (iter->second.second <= requestTick && iter->second.first > dwNow + 500)
		{
			iter->second.first = dwNow + 500;
		}
		else
			break;
	}

#endif // _LIGHT_TIMEOUT

	// 由于没有实际收到SubPiece报文，所以 m_ReceiveSubPieceCountOnce 不加
	UpdataWindowSize(usedTime, true);
	UpdateRTT(rtt);
	m_RequestTime.sync(); // Added by Tady, 071508

#if !_Savage
	RequestTillFullWindow();
#else
	RequestTillFullWindow2();
#endif
}

bool UDPPeerTunnel::CheckRequestPieceTimeout()
{ 
	VIEW_INFO(" 0 UDPPeerTunnel::RequestFromTaskQueue " << *this << " " << m_WindowSize); 

//	MyOutputDebugString("Tady[%d]: ----------------------------------- CheckRequestPieceTimeout()!!", m_debugID);

	if( false == IsRequesting() )
	{
#if _Savage
#if (MRP_SUPPORTED && !LIGHT_MRP)
		// Added by Tady, 082008: Actually we just borrow this timer's runtime.
		// CheckRequestPieceTimeout() 所在的时钟runtime刚好在两个RequestFromTaskQueue()之间，与RequestFromTaskQueue()间隔500毫秒。
		CheckTimeoutMRP();
#endif // !LIGHT_MRP
#endif // _Savage
		return false;
	}

	DWORD now = GetTickCount();
//	int iCounterOfTimeout = 0;
	for ( RequestingSubPieceSet::iterator iter = m_RequestSubPieces.begin(); iter != m_RequestSubPieces.end(); )
	{
		SubPieceUnit subPiece = iter->first;
		if( now > iter->second.first )
		{	// 请求超时
			VIEW_INFO( "InternalTimeout "<<*this<<" "<<subPiece << " " << make_tuple( now, iter->second.first ) );

			LIVE_ASSERT(iter->second.first - iter->second.second > 0);
			LIVE_ASSERT(now - iter->second.first > 0);
			UINT rtt = now - iter->second.second;
#ifdef _LIGHT_TIMEOUT
			//if (m_orderlessRate < 50)
#endif
			{
				m_Downloader.OnSubPieceReceivedFailed(iter->first, &m_Connection); 
			}
			m_RequestSubPieces.erase(iter++);

			UpdateRTT(rtt);
			if (m_RequestSubPieces.empty())
			{
				UpdataWindowSize(m_RequestTime.elapsed(), false);
			}
//			MyOutputDebugString("Tady[%d]: curUsedtime = 《%d》", m_debugID, m_RequestTime.GetElapsed() + 500);
//			LIVE_ASSERT(rtt >= m_RequestTime.GetElapsed());
			// Removed by Tady, 110609: Should not reset the requestTime, right? :)
//			m_RequestTime.sync(); 
//			iCounterOfTimeout++;
		}
		else
		{
#ifndef _LIGHT_TIMEOUT
			iter++;
#else
			break;
#endif
		}
	}

//	VIEW_INFO(" UDPPeerTunnel::RequestFromTaskQueue " << *this << " " << m_WindowSize); 

//	if (iCounterOfTimeout > 0)
	{
#if !_Savage
		RequestTillFullWindow();
#else
		RequestTillFullWindow2();
#endif
	}

#if _Savage
#if (MRP_SUPPORTED && !LIGHT_MRP)
	// Same as above one :)
	CheckTimeoutMRP();
#endif // !LIGHT_MRP
#endif // _Savage

	return true;
}

DWORD UDPPeerTunnel::GetSortValue()
{
	DWORD result = 0;
	
	DWORD usedTime = GetUsedTime();
	VIEW_INFO("UDPPeerTunnel::GetSortValue() "<< *this << " Usedtime=" << usedTime);


// 	if ( false == IsRequesting() )
// 	{	// 如果现在没有请求，而用估计一片的时间计算
// 		result = ::GetTickCount() + usedTime;
// 	}
// 	else
// 	{	// 如果现在请求，而用　从上次请求时间开始两片的时间来算
// 		result = m_RequestTime.get_count() + 2 * usedTime;
// 	}
	result = ::GetTickCount() + usedTime;
	return result;
}

DWORD UDPPeerTunnel::GetUsedTime()
{
// 
// 	DWORD result = m_avrgUsedTime;
// 	if(m_RequestSubPieces.size() == 0)
// 	{
// 		// 未发起请求，故按照前面平均时间计算
// 		return result;
// 	}
// 	else
	if (m_RequestSubPieces.size() > 0)
	{
		DWORD dwDuration = m_RequestTime.elapsed();
// 		if( requestingTime > result * 9 / 10)
// 		{	
// 			return requestingTime + result;
// 		}
// 		else
// 		{
// 			return result;
// 		}

		if (dwDuration > m_avrgUsedTime)
		{
//			LIVE_ASSERT(dwDuration < 10000);
			
			// Modified by Tady, 071708: m_avrgUsedTime may be sinked.
//			m_avrgUsedTime = m_avrgUsedTime * 0.8 + requestingTime * 0.2;
			return m_avrgUsedTime * 0.8 + dwDuration * 0.2;
		}
	}

//	LIVE_ASSERT(m_avrgUsedTime < 10000);
	return m_avrgUsedTime;
}


UINT UDPPeerTunnel::GetRequestCount() const
{
	return (UINT)m_RequestSubPieces.size();
}

bool UDPPeerTunnel::RequestFromTaskQueue()
{

#if !_Savage
	return RequestTillFullWindow();
#else
	return RequestTillFullWindow2();
#endif
}

// Actually, it's update used-time.
void UDPPeerTunnel::UpdataWindowSize(UINT usedTime, bool requestSucced)
{
	m_SumDeltaUsedTime = m_SumDeltaUsedTime + usedTime - m_usedTimeArray[m_calcUsedTimeCount%m_UseArraySize];
	
	m_SumRequestSuccedCount = m_SumRequestSuccedCount + requestSucced - m_requestSuccedArray[m_calcUsedTimeCount%m_UseArraySize];

	m_usedTimeArray[m_calcUsedTimeCount%m_UseArraySize] = usedTime;		
	m_requestSuccedArray[m_calcUsedTimeCount%m_UseArraySize] = requestSucced;
	++m_calcUsedTimeCount;

	UINT calcUsedTimeCount = min((UINT)m_UseArraySize, m_calcUsedTimeCount);

//	VIEW_INFO(m_Connection<<"m_SumDeltaUsedTime: "<<m_SumDeltaUsedTime<<" calcUsedTimeCount: "<<calcUsedTimeCount);
	m_avrgUsedTime = m_SumDeltaUsedTime / calcUsedTimeCount;
// 	LIVE_ASSERT(m_avrgUsedTime > 0);
// 	LIVE_ASSERT(m_avrgUsedTime < 10000);
//	LIVE_ASSERT(m_avrgUsedTime < 10000);
	LIMIT_MIN_MAX(m_avrgUsedTime, 1, 10000);
	m_nowRequestSuccedRate = (double)m_SumRequestSuccedCount / (double)calcUsedTimeCount;

//	MyOutputDebugString("Tady[%d]UpdateWindowSize: curUsedtime: [%d], m_avrgUsedtime [%d] -----------curSucced [%d], SuccedRate [%f]",
//		m_debugID, usedTime, m_avrgUsedTime, requestSucced, m_nowRequestSuccedRate);

// 	if (m_nowRequestSuccedRate > 0.85
// 		&& m_TaskQueue.size() > (m_WindowSize - m_RequestSubPieces.size()))
// 	{
// 		m_WindowSize ++;
// 	}
// 	if (m_nowRequestSuccedRate < 0.80) 
// 		m_WindowSize --;

// 	if (m_nowRequestSuccedRate > 0.95)
// 	{
// 		m_WindowSize *= 4;
// 	}
// 	else if (m_nowRequestSuccedRate > 0.9)
// 	{
// 		m_WindowSize *= 2;
// 	}
// 	else if (m_nowRequestSuccedRate > 0.85)
// 	{
// 		m_WindowSize++;
// 	}
// 	else if (m_nowRequestSuccedRate < 0.65)
// 	{
// 		m_WindowSize /= 3;
// 	}
// 	else if (m_nowRequestSuccedRate < 0.75)
// 	{
// 		m_WindowSize /= 1.5;
// 	}
// 	else if (m_nowRequestSuccedRate < 0.8)
// 	{
// 		m_WindowSize--;
// 	}
/*
	if (m_nowRequestSuccedRate > m_lastRequestSuccedRate + 0.04)
	{
		m_WindowSize ++;
	}
	else
	{
		if (m_nowRequestSuccedRate + 0.04 < m_lastRequestSuccedRate)
			m_WindowSize --;
	}

	if (m_nowRequestSuccedRate - m_lastRequestSuccedRate < 0.02 && m_nowRequestSuccedRate - m_lastRequestSuccedRate > -0.02)
	{
		if (m_nowRequestSuccedRate > 0.85)
		{
			m_WindowSize++;
		}
		else if (m_nowRequestSuccedRate < 0.8)
		{
			m_WindowSize--;
		}
	}
*/
	m_lastRequestSuccedRate = m_nowRequestSuccedRate;

//	LIMIT_MIN_MAX(m_WindowSize, 1, m_TaskQueue.size() + 10);

	VIEW_INFO("Used Time :" << usedTime << " request succed "<<requestSucced);
	VIEW_INFO(m_Connection<<"m_SumDeltaUsedTime: "<<m_SumDeltaUsedTime<<" calcUsedTimeCount: "<<calcUsedTimeCount<<" m_avrgUsedTime: "<<m_avrgUsedTime);
}

void UDPPeerTunnel::AdjustWindowSize(UINT minSize)
{ 
/*
	if (m_nowRequestSuccedRate > m_lastRequestSuccedRate + 0.02)
	{
		m_WindowSize ++;
	}
	else
	{
		if (m_nowRequestSuccedRate +0.02 < m_lastRequestSuccedRate)
			m_WindowSize --;
	}

	m_lastRequestSuccedRate = m_nowRequestSuccedRate;

	if (m_nowRequestSuccedRate > 0.85)
		m_WindowSize ++;
	if (m_nowRequestSuccedRate < 0.80) 
		m_WindowSize --;
	LIMIT_MAX(minSize, (UINT)m_WindowSizeMax);
	LIMIT_MIN_MAX(m_WindowSize, minSize, m_WindowSizeMax);
	//m_WindowSize = minSize;
*/
//	if (m_avrgUsedTime < 1000)
	if (m_avrgUsedTime > 0 && m_avrgUsedTime < 1500)
	{
		LIVE_ASSERT(m_avrgUsedTime > 0);
//		UINT magic = min(2 * 1000 /m_avrgUsedTime, 8 + 1000 / m_avrgUsedTime);
//		m_WindowSize = magic;
 		UINT match = m_avrgRTT  / m_avrgUsedTime + bool(m_avrgRTT  % m_avrgUsedTime); // == m_avrgRTT * 1.0 / m_avrgUsedTime + 0.5;
// 		match = min(2 * match, 2 + match); 
//		m_WindowSize = min(magic, match);
		m_WindowSize = match + 2;
	}
	LIMIT_MIN_MAX(m_WindowSize, m_WindowSizeMin, m_WindowSizeMax); 
//	m_WindowSize = 60;
}

bool UDPPeerTunnel::RequestTillFullWindow()
{
	while(m_RequestSubPieces.size() < m_WindowSize && ! m_TaskQueue.empty())
	{
		SubPieceTask nextTask;
		if (! m_TaskQueue.empty()) 
		{
			nextTask = m_TaskQueue.front();
			m_TaskQueue.pop_front();
		}
		
		if ( !m_Downloader.NeedDownload(nextTask.subPiece) )
		{	// 如果已经存在这片数据了 或者 正在请求队列中 那么就不在请求
			continue;
		}

		if ( false == RequestSubPiece(nextTask.subPiece, nextTask.externalTimeOut) )
		{
			// 如果请求失败说明网络层出了问题，那么这个连接应该被断掉
			return false;
		}

	}
//  MyOutputDebugString("Tady[%d]: m_WindowSize = [%d], m_avrgUsedTime = [%d], m_avrgRTT = [%d], m_nm_nowRequestSuccedRate[%f]",
//	m_debugID, m_WindowSize, m_avrgUsedTime, m_avrgRTT, m_nowRequestSuccedRate);
	return true;
}

bool UDPPeerTunnel::RequestTillFullWindow2()
{		
	SubPieceTask* pTask;
	UINT uStartIndex = m_Connection.GetAppModule().GetStreamBuffer().GetDownloadStartIndex();

	// Modified by Tady, 011511: Spark!
	if (!IsFreezing()
		&& m_taskCollectionPtr != NULL && !m_taskCollectionPtr->empty() 
		&& uStartIndex <= m_Connection.GetMaxIndex() )
	{
		for (HealthyDegreeCollection2::iterator iter = m_taskCollectionPtr->begin(); iter != m_taskCollectionPtr->end();)
		{
#if NO_HEALTHCOUNTCALC
			pTask = &(*iter);
			if (m_RequestSubPieces.size() + m_TaskQueue.size() >= m_WindowSize 
				|| pTask->subPiece.PieceIndex > m_Connection.GetMaxIndex())
#else  // !NO_HEALTHCOUNTCALC
			pTask = &(iter->second);
			if (m_RequestSubPieces.size() + m_TaskQueue.size() >= m_WindowSize) 
#endif // !NO_HEALTHCOUNTCALC
			{
				break;
			}	
			
			// Does this peer have this subPiece?
			if (!m_Connection.HasPiece(pTask->subPiece.PieceIndex))
			{
//				iter = m_taskCollectionPtr->upper_bound(iter->first);
				++iter;
				continue;
			}

// 			if ( !m_Downloader.NeedDownload(pTask->subPiece) )
// 			{	// 如果已经存在这片数据了 或者 正在请求队列中 那么就不在请求
// 				iter++;
// 				continue;
// 			}

#ifndef _LIGHT_TIMEOUT
			if (m_RequestSubPieces.size() != 0 && m_RequestSubPieces.find(pTask->subPiece)  != m_RequestSubPieces.end())
			{
				++iter;
				continue;
			}
#else
// 			bool isRequesting(false);
// 			STL_FOR_EACH_CONST(RequestingSubPieceSet, m_RequestSubPieces, i)
// 			{
// 				if (i->first == pTask->subPiece)
// 				{
// 					isRequesting = true;
// 					break;
// 				}
// 			}
// 			if (isRequesting == true)
// 			{
// 				iter++;
// 				continue;
// 			}
#endif

			if ( false == RequestSubPiece2(pTask->subPiece, pTask->externalTimeOut) )
			{
				// 如果请求失败说明网络层出了问题，那么这个连接应该被断掉
				return false;
			}

			m_taskCollectionPtr->erase(iter++);
		}
	}

	// Flush out all data in m_taskqueue.
//	CheckTimeoutMRP();
//	LIVE_ASSERT(m_TaskQueue.size() == 0);

#if (MRP_SUPPORTED && LIGHT_MRP)
	RequestMRP();
#endif

	return true;
} 

bool UDPPeerTunnel::TryToDownload(SubPieceUnit subPiece, DWORD externalTimeout)
{
	//if ( containers::contains( m_RequestSubPieces, subPiece ) )
	//{
	//	VIEW_DEBUG( "TryToDownload reduplicative " << subPiece << " " << *this );
	//	return false;
	//}
#ifndef _LIGHT_TIMEOUT
	LIVE_ASSERT( false == containers::contains( m_RequestSubPieces, subPiece ) );
#endif

#ifdef _DEBUG
	STL_FOR_EACH_CONST( SubPieceTaskList, m_TaskQueue, iter )
	{
		if ( iter->subPiece == subPiece )
		{
			LIVE_ASSERT( false );
		}
	}
#endif
	return PeerTunnel::TryToDownload( subPiece, externalTimeout );
}

void UDPPeerTunnel::UpdateRTT(UINT inRTT)
{
	if (m_avrgRTT == 0)
		m_avrgRTT = inRTT;
	else
		m_avrgRTT = m_avrgRTT * 0.8 + inRTT * 0.2;
// 	LIVE_ASSERT(m_avrgRTT != 0);
// 	LIVE_ASSERT(m_avrgRTT < 10000);
	LIMIT_MIN_MAX(m_avrgRTT, 10, 10000);
}

UDPPeerTunnel::~UDPPeerTunnel()
{
//	LIVE_ASSERT(m_debugID != 3);
}

void UDPPeerTunnel::ClearTaskQueue() 
{ 
#if _Savage

#if MRP_SUPPORTED
#if !LIGHT_MRP
	// Added by Tady, 082008: Before calc Healthymap, we should clear our taskqueue. And before 
	// clearing task queue we should send out old MRPs.
	CheckTimeoutMRP();  
#else // LIGHT_MRP
	LIVE_ASSERT(m_TaskQueue.empty());
#endif // LIGHT_MRP
#endif // MRP_SUPPORTED

	m_TaskQueue.clear();

#else // !_Savage

	PeerTunnel::ClearTaskQueue();

#endif // _Savage

}

void UDPPeerTunnel::CheckTimeoutMRP()
{
	if (m_TaskQueue.size() > 0 && m_lastTaskTimer.elapsed() > 400)
	{
//		MyOutputDebugString("Tady[%d]: Timercouter TimeOut---<<%d>> 111 !!", m_debugID, m_lastTaskTimer.GetElapsed());
		RequestMRP();
	}
}
