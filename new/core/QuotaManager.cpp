
#include "StdAfx.h"

#include "QuotaManager.h"
#include "DownloaderImpl.h"
#include "PeerConnection.h"
#include "common/StreamBuffer.h"
#include "common/StreamBufferStatistics.h"
#include "common/MediaStorage.h"
#include "PeerTunnel.h"
#include "PeerManagerImpl.h"

#include "common/StreamIndicator.h"

#include "framework/log.h"
#include <ppl/util/random.h>


QuotaManager::QuotaManager(PeerDownloader& downloader, const PeerTunnelCollection& tunnels) 
	: m_Tunnels( tunnels ), m_downloader(downloader), m_streamBuffer(downloader.GetStreamBuffer()), m_storage(m_streamBuffer.GetStorage())
{
	m_retrySubpieceVector.resize(4);
}

void QuotaManager::CalcTunnelReceiveTimes()
{
	m_RecvTimeMap.clear();

	// 计算windowsize总数
	DOWNLOADER_STATS& stats = m_downloader.GetPeerManager().GetStatisticsData().DownloaderData;

	DOWNLOADER_SUB_STATS tempUDPStats;
	DOWNLOADER_SUB_STATS tempTCPStats;
	tempUDPStats.ResetForTemp();
	tempTCPStats.ResetForTemp();

	double totalUDPRequestSuccedRate = 0;
	double totalTCPRequestSuccedRate = 0;

	// Statistic
	STL_FOR_EACH_CONST(PeerTunnelCollection, m_Tunnels, _iter)
	{
		PeerTunnelPtr pt = *_iter;
		PeerConnection* pc = &pt->GetConnection();
		LIVE_ASSERT(pc != NULL);

		if (!m_downloader.CanDownload( pt ) || pt->IsFreezing() )
			continue;

		DOWNLOADER_SUB_STATS& subStats = pc->IsUDP() ? tempUDPStats : tempTCPStats;

		UINT reqSucceededRate = static_cast<UINT>( pt->GetRequestSuccedRate() * 10000.0 );
		UINT usedTime = pt->GetUsedTime();
		UINT windowSize = pt->GetWindowSize();
		subStats.TotalWindowSize += windowSize;
		subStats.PeerTunnelCount++;
		subStats.TotalRequestCount += pt->GetRequestCount();
		if ( pt->IsHighConnection() )
		{
			subStats.HighConnectionCount++;
		}

		if ( false == pt->IsUsed() )
		{
			subStats.UnusedTunnelCount++;
		}

		LIMIT_MIN(subStats.MaxUsedTime, usedTime);
		LIMIT_MAX(subStats.MinUsedTime, usedTime);

		LIMIT_MIN(subStats.MaxWindowSize, windowSize);
		LIMIT_MAX(subStats.MinWindowSize, windowSize);

		LIMIT_MIN(subStats.MaxRequestCount, pt->GetRequestCount());
		LIMIT_MIN(subStats.MaxTaskQueueSize, pt->GetTaskQueueSize());

		LIMIT_MIN(subStats.MaxRequestSuccedRate, reqSucceededRate);
		LIMIT_MAX(subStats.MinRequestSuccedRate, reqSucceededRate);

		if (pc->GetMaxIndex() < m_streamBuffer.GetDownloadStartIndex()) 
		{
			subStats.DepletedTunnelCount++;
			continue;
		}

		double succeededRate = pt->GetRequestSuccedRate();
		if( pc->IsUDP() )
		{
			totalUDPRequestSuccedRate += succeededRate;
		}
		else
		{
			totalTCPRequestSuccedRate += succeededRate;
		}
	}

	stats.UDP = tempUDPStats;
	stats.TCP = tempTCPStats;
	stats.UDP.TotalRequestSuccedRate = static_cast<UINT>( totalUDPRequestSuccedRate * 10000 );
	stats.TCP.TotalRequestSuccedRate = static_cast<UINT>( totalTCPRequestSuccedRate * 10000 );

	// Adjust pcs' window-size & insert them into RecvTimeMap.
	STL_FOR_EACH_CONST(PeerTunnelCollection, m_Tunnels, iter)
	{
		PeerTunnelPtr pt = *iter;
		PeerConnection* pc = &pt->GetConnection();
		LIVE_ASSERT(pc != NULL);
		
		if (!m_downloader.CanDownload( pt ))
		{	// 不能使用这个pc去下载，那么就跳过
			continue;
		}		

#if 1//!_Savage
		// 清除每个Tunnel的预请求队列
		VIEW_INFO(" TaskQueueLength Left "<<*pc<<"  "<<pt->GetTaskQueueSize()<<" "<<pt->GetUsedTime()<<"  "<<pt->GetWindowSize());
		pt->ClearTaskQueue();
#endif

		if (pt->IsFreezing() 
			|| pc->GetMaxIndex() < m_streamBuffer.GetDownloadStartIndex()) // Tady, 07182013: Need optimization. It's loop.
		{	// 该Peer没有资源去下载,那么也跳过
			continue;
		}

		UINT minWindowSize = 2;
		if( pc->IsUDP() )
		{
			// UDP 的连接
			double tunnelRequestSuccedRate = pt->GetRequestSuccedRate();
			UINT newWindowSize = (UINT)(80 * tunnelRequestSuccedRate / totalUDPRequestSuccedRate);
			if ( newWindowSize > minWindowSize )
				minWindowSize = newWindowSize;	
		}
		pt->AdjustWindowSize(minWindowSize);

		// 将tunnel插入到这个RecvTimeMap中
#if !_Savage
		m_RecvTimeMap.insert(std::make_pair(pt->GetSortValue(), pt.get()));
#endif // !_Savage
	}

	VIEW_INFO("CalcTunnelReceiveTimes Count " << m_RecvTimeMap.size());
}

void QuotaManager::RequestFromTaskQueue()
{
	STL_FOR_EACH_CONST(PeerTunnelCollection, m_Tunnels, iter)
	{
		PeerTunnelPtr pt = *iter;
		if (!m_downloader.CanDownload( pt ) || pt->IsFreezing())
		{	// 不能使用这个pc去下载，那么就跳过
			continue;
		}
		
		pt->RequestFromTaskQueue();
	}
}

bool QuotaManager::RequestFromTaskQueue2()
{
	bool bHasAnyBodySendRequest = false;
	STL_FOR_EACH_CONST(PeerTunnelCollection, m_Tunnels, iter)
	{
		PeerTunnelPtr pt = *iter;
		if (!m_downloader.CanDownload( pt ) || pt->IsFreezing())
		{	// 不能使用这个pc去下载，那么就跳过
			continue;
		}

		if (pt->RequestFromTaskQueue())
		{
			bHasAnyBodySendRequest = true;
		}
	}
	return bHasAnyBodySendRequest;
}

bool QuotaManager::HasRecvTimeMap() const
{
	return !m_RecvTimeMap.empty();
}

void QuotaManager::UpdateTunnelRecvTimeMap(PeerTunnelRecvTimeMap::iterator itr, UINT newRecvTime)
{
	PeerTunnel* pt = itr->second;
	m_RecvTimeMap.erase(itr);
	m_RecvTimeMap.insert(std::make_pair(newRecvTime,pt));
}

// [8/16/2007 juli]
// 函数作用：把指定的SubPiece分配给Tunnel
//		参数1: subPiece 要分配给Tunnel的SubPieceUint
//		参数2: ???
//		返回值：
//			pair->first:  分配成功
//			pair->secode: 把subPiece分给的Tunnel
std::pair<bool, PeerTunnel*> QuotaManager::TryToDownload(SubPieceUnit subPiece, int& tryCount, DWORD externalTimeOut)
{
	PeerTunnel* pt = NULL;
// 	if (!m_downloader.NeedDownload(subPiece))
// 	{	// 不需要分配subPiece
// 		MANAGER_INFO("Downloader::TryToDownload no need download piece "<<subPiece);
// 		return std::make_pair(false, pt);
// 	}
	tryCount ++;

	// 对 UsedTime 的最小值限制,避免把过多的SubPiece分配到同一个Tunnel上
	//UINT minUsedTime = 150;

	int iReference = 0;
	for( PeerTunnelRecvTimeMap::iterator itr = m_RecvTimeMap.begin();
		itr != m_RecvTimeMap.end(); )
	{
		pt= itr->second;
		UINT receiveTime = itr->first;
		PeerConnection* pc = &pt->GetConnection();

		if ( !pc->HasPiece(subPiece.PieceIndex))
		{	// 如果该PeerConnection 没有这个资源 或者 正在请求这个资源，那么就找下一个Tunnel
			itr ++;
			continue;
		}

		if (pt->IsRequesting(subPiece) )	//OPT: Downloader中已经检查一次了，可以在正式下载的时候再检查
		{
			itr++;
			continue;
		}

		// 每一个tunnel最多分配70个 SubPieces ! 一个tunnel分配多于100个SubPiece根本处理不完浪费
		if (pt->GetTaskQueueSize() >= pt->GetTaskQueueMaxSize()) 
		{
			m_RecvTimeMap.erase(itr ++); 
			continue;
		}

		// 防止靠前的subpiece分给不信任的peer(降低tcp的冗余率）
// 		if (!(externalTimeOut >= 10*1000 || pt->GetUsedTime() < externalTimeOut))
// 		{
// 			itr++;
// 			continue;
// 		}

		if(/*(m_RecvTimeMap.size() == 1 && pt->GetUsedTime() >= 3000 && externalTimeOut <= 3500 && iReference == 0) ||*/
			false == pt->TryToDownload(subPiece, externalTimeOut) )
		{	// 只有 发包失败，才会返回false, 这种情况下说明这个通道已经挂了,那么这个通道就没有必要出现在m_RecvTimeMap
			m_RecvTimeMap.erase(itr++);
			// 既然 发包失败，该是否断开连接
			
			continue;
		}

		if (!(pt->GetUsedTime() >= 3000 && externalTimeOut <= 3500))
 		{
//			receiveTime += max( minUsedTime, pt->GetUsedTime() );
			receiveTime += pt->GetUsedTime();
			// receiveTime改变，重建 m_RecvTimeMap 的索引
			UpdateTunnelRecvTimeMap(itr ++, receiveTime);		//OPT: 一次计算整个Tunnel-Time序列，不需要每次更新
		}
		else
		{
			//////////////////////////////////////////////////////////////////////////
			if (/*externalTimeOut <= 3500 && pt->GetUsedTime() >= 3000 && */iReference < 1)
			{
				iReference++;
				m_RecvTimeMap.erase(itr);
				itr =  m_RecvTimeMap.begin();
				continue;
			}
			m_RecvTimeMap.erase(itr);
		}
		//////////////////////////////////////////////////////////////////////////

		//m_StartRequestTickCount = ::GetTickCount();	//PP_1.0.6.6_C06
		return std::make_pair(true, pt);				// 分配成功
	}

	return std::make_pair(false, pt);
}

void QuotaManager::CalcHealthy(HealthyDegreeCollection& HealthyMap, UINT ResourceMaxIndex, const PEER_MINMAX& sourceMinMax) const
{
	//LIVE_ASSERT(HasQuota());
	time_counter counter;

	/// 如果两个Piece间隔5000就会ResetBuffer
	//const UINT RESET_BUFFER_COUNT = 5000;

	//获得计算健康度最远的范围
	HealthyMap.clear();


//	UINT tryCount = 0;
	//获得健康度（如果endIndex非常大，如僵尸节点，则此处会造成死循环!）
	UINT startIndex = m_downloader.GetStartIndex(); // It's about max(skip, ResourceMinIndex).

	UINT sourceLength = sourceMinMax.GetLength();	// 正常情况时120s
	if (sourceLength <= 12) return;
	UINT sourceForUrgent = sourceLength * 3 / 4;	// 离min 90s 的长度做为urgent下载的依据
	UINT sourceFor50s = sourceLength * 5 / 12;
//	UINT sourceFor40s = sourceLength * 4 / 12;
//	UINT sourceFor30s = sourceLength * 3 / 12;
	UINT sourceFor20s = sourceLength * 2 / 12;

//	UINT dataLenFor10s = sourceLength * 1 / 12;
//	UINT dataLenFor5s = sourceLength * 5 / 120;
//	UINT dataLenFor3s = sourceLength * 3 / 120;
//	UINT dataLenFor1s = sourceLength / 120;

	VIEW_DEBUG("SourceMinMax "<<sourceMinMax.MinIndex<<" "<<sourceMinMax.MaxIndex<<" "<<sourceLength<<" End");
	UINT minIndex = m_storage.GetMinIndex();
	UINT preloacteIndex = m_streamBuffer.GetPrelocationIndex();
	LIMIT_MIN( minIndex, preloacteIndex );
	bool isGood = false;
	UINT bufferTime = m_streamBuffer.GetStatistics().BufferTime;

	DWORD ExTimeOutLevel = m_downloader.GetPeerManager().GetConfig().TunnelConfig.ExTimeOutLevel;
//	UINT64 locateTimeCounter = m_downloader.GetLocateElapsed();

	UINT urgentCount = 0;
	// 当Skip与Min之间的差距拉开了80s的距离,则启用完全健康度下载策略
 	if( 
 		sourceLength > 100 &&	// source长度小于100表示，source可能是刚刚启动。避免source长度为0，算法无效
 		minIndex > 0 && 
		startIndex > 0 && 
 		(startIndex - minIndex) >= sourceForUrgent )
 	{
 		isGood = true;
 	}
 	else if( 
 		sourceLength > 100 &&
 		minIndex > 0 && 
 		startIndex > 0 )
 	{
 		LIVE_ASSERT(sourceForUrgent > (startIndex-minIndex));
 		urgentCount = sourceForUrgent - (startIndex-minIndex);
 		if( urgentCount < 100 ) urgentCount = 100;	//至少计算100个Piece的健康度
 	}
 	else
 	{
 		urgentCount = max(static_cast<UINT>(100), sourceForUrgent);	//至少计算100个Piece的健康度
 	}
// 
// 	if( locateTimeCounter < 60*1000 )
// 	{	// Locate 后的前 60s 钟
// 		if( bufferTime < 25*1000  )
// 	 	{
// 	 		urgentCount = min( urgentCount, max(100,  sourceLength*(bufferTime+20*1000)/(120*1000) ) );
// 	 	}		
// 	 	else if( bufferTime < 50*1000  )
// 	 	{
// 	 		urgentCount = min( urgentCount, max(100,  sourceLength*(bufferTime+30*1000)/(120*1000) ) );
// 	 	}
// 	}

	VIEW_INFO("LocalPeerGood "<<isGood<<" End");
	(void)bufferTime;
        VIEW_INFO("UrgentCount "<<bufferTime<<" "<<urgentCount<<" End");
	VIEW_DEBUG("PeerMinSkip "<<m_storage.GetMinIndex()<<" "<<startIndex<<" End");

	m_downloader.GetPeerManager().GetStatisticsData().DownloaderData.DownloadRange.MinIndex = startIndex;
	RandomGenerator rnd;

	UINT index = 0;
	UINT maxIndex = min(ResourceMaxIndex, startIndex + max(static_cast<UINT>(2000), sourceLength*4));
	for (index = startIndex; index <= maxIndex; index++)
	{
		if (m_downloader.NeedDownload(index))//仅统计需要下载的Piece
		{
			if( isGood == false && index < startIndex + urgentCount )
			{	// 本地资源状况不好,采用完全紧迫度策略
				UINT key = index - startIndex;
				LIVE_ASSERT( key >= 0 );
				LIVE_ASSERT( key < urgentCount );
//				DWORD externalTimeout = 2000 + key * 3000 / urgentCount;
				DWORD externalTimeout;

				if (index - minIndex < sourceFor50s)
				{
					if (index - minIndex <= sourceFor20s)
						externalTimeout = ExTimeOutLevel;
					else
						externalTimeout = ExTimeOutLevel + (index - minIndex - sourceFor20s) * (10000 - ExTimeOutLevel) / (sourceForUrgent - sourceFor20s);
				} else
				{
					//! 可能写错了    应该是  ...   4000 * 30 / 70;
					if (index - minIndex <= sourceFor50s + sourceFor20s)
						externalTimeout = ExTimeOutLevel;
					else
						externalTimeout = ExTimeOutLevel + (index - minIndex - sourceFor20s - sourceFor50s) * (10000-ExTimeOutLevel) / (sourceForUrgent - sourceFor20s - sourceFor50s);
				}

				//externalTimeout = 1000;
				
				PieceTask pieceTask(index, externalTimeout );
				//VIEW_INFO( "Urgent timeOut index=" << index << " Index - min=" << index - minIndex  << " externalTimeout=" << externalTimeout);
				HealthyMap.insert(std::make_pair(key,pieceTask));//index对应的健康度是healthyCount
			}
			else
			{	// 本地资源状况很好,采用完全健康度策略
// 				if( locateTimeCounter < 90*1000)
// 				{
//  					if( bufferTime < 70*1000 )
//  					{	// 如果缓冲时间小于 50*1000, 则锁定下载视野.不下载太远的Piece
// 	 					break;
//  					}
//  				}
				UINT healthyCount = 0;
				STL_FOR_EACH_CONST(PeerTunnelCollection, m_Tunnels, iter)
				{
					PeerTunnelPtr pt = *iter;
					PeerConnection* pc = &pt->GetConnection();
					if (pc->HasPiece(index))
						healthyCount++;
				}
				if( healthyCount > 0)
				{
					// 计算紧迫度参数
					// 采用随机数的原因是 在相同健康度的资源中 随机制定优先级
					UINT key = urgentCount + healthyCount*1000 + rnd.Next(999) + 2000;
					LIVE_ASSERT(key >= urgentCount);

					//PieceTask pieceTask(index, 2000);

					PieceTask pieceTask(index, 10000);
					HealthyMap.insert(std::make_pair(key,pieceTask));	
				}
			}
		}
	}
	m_downloader.GetPeerManager().GetStatisticsData().DownloaderData.DownloadRange.MaxIndex = index;

	VIEW_INFO("Healthy map length" << HealthyMap.size()<<" MinIndex "<<minIndex<<" StartINdex "<<startIndex<<" ResourceMaxIndex-SI "<<ResourceMaxIndex - startIndex<<" urgentCount "<<urgentCount);
	//VIEW_DEBUG("CalcHealthy To Index " << minIndex << " " << startIndex << "(" << (startIndex-minIndex) << ")" << " " << index << "(" << (index-minIndex) << ")" );
	//VIEW_DEBUG("CalcHealthy " << make_tuple(startIndex, ResourceMaxIndex, ResourceMaxIndex - startIndex) << make_tuple(m_Tunnels.size(), counter.GetElapsed()));
}

void QuotaManager::CalcHealthy2(HealthyDegreeCollection& HealthyMap, UINT ResourceMaxIndex, const PEER_MINMAX& sourceMinMax) const
{
	//LIVE_ASSERT(HasQuota());
	time_counter counter;

	/// 如果两个Piece间隔5000就会ResetBuffer
	//const UINT RESET_BUFFER_COUNT = 5000;

	//获得计算健康度最远的范围
	HealthyMap.clear();


	//	UINT tryCount = 0;
	//获得健康度（如果endIndex非常大，如僵尸节点，则此处会造成死循环!）
	UINT startIndex = m_downloader.GetStartIndex(); // It's about max(skip, ResourceMinIndex).

	UINT sourceLength = sourceMinMax.GetLength();	// 正常情况时120s
	if (sourceLength <= 12) return;
	// 	UINT sourceForUrgent = sourceLength * 3 / 4;	// 离min 90s 的长度做为urgent下载的依据
	// 	UINT sourceFor50s = sourceLength * 5 / 12;
	// 	UINT sourceFor40s = sourceLength * 4 / 12;
	// 	UINT dataLenFor30s = sourceLength * 3 / 12;
	// 	UINT dataLenFor20s = sourceLength * 2 / 12;
	// 
	// 	UINT dataLenFor10s = sourceLength * 1 / 12;
	// 	UINT dataLenFor5s = sourceLength * 5 / 120;
	// 	UINT dataLenFor3s = sourceLength * 3 / 120;
	double dataLenFor1s = (double)sourceLength / 120;

	// 	VIEW_DEBUG("SourceMinMax "<<sourceMinMax.MinIndex<<" "<<sourceMinMax.MaxIndex<<" "<<sourceLength<<" End");
	//  	UINT minIndex = m_storage.GetMinIndex();
	// 	UINT preloacteIndex = m_streamBuffer.GetPrelocationIndex();
	// 	LIMIT_MIN( minIndex, preloacteIndex );
	bool isGood = false;
	// 	UINT bufferTime = m_streamBuffer.GetStatistics().BufferTime;
	// 
	// 	DWORD ExTimeOutLevel = m_downloader.GetPeerManager().GetConfig().TunnelConfig.ExTimeOutLevel;
	// 	DWORD locateTimeCounter = m_downloader.GetLocateElapsed();

	UINT urgentCount = 0;
	// 	UINT curIndex = 0;
	// 	const StreamIndicator& pBase = m_streamBuffer.GetBaseIndicator();
	// 	if (pBase.GetTimeStamp() != 0)
	// 	{
	// 		curIndex = pBase.GetElapsedTime() * dataLenFor1s / 1000 + pBase.GetPieceIndex();
	// 	}
	// 
	// 	curIndex = max(minIndex,curIndex);

	//	UINT curdatalen1(0), curdatalen2(0);
	// 	curdatalen1 = startIndex - curIndex;
	// 	curdatalen2 = m_streamBuffer.GetPrepaDataLen();
	//  UINT timeLen1(0), timeLen2(0), timeLen3(0);
	//	timeLen1 = curdatalen1 / dataLenFor1s;
	// 	timeLen2 = curdatalen2 / dataLenFor1s;
	// 	timeLen3 = m_streamBuffer.GetPrepaDataTime();

//	UINT prepaDataLen = m_streamBuffer.GetPrepaDataLen();
	int	 iPrepaDataTime = m_streamBuffer.GetPrepaDataTime();
//	UINT dataLenFor15s = sourceLength / 8; // sourceLength * 15 /120;

	if (sourceLength < 100 )
		//		|| !(startIndex > 0 && minIndex > 0))
	{
		urgentCount = 100;
	}
	else 
	{
		if (iPrepaDataTime > 15000)
		{
			isGood = true;
		}
		else 
		{
			urgentCount = (UINT)((15000 - iPrepaDataTime) * dataLenFor1s / 1000);
		}
	}


	VIEW_INFO("LocalPeerGood "<<isGood<<" End");
//	VIEW_INFO("UrgentCount "<<bufferTime<<" "<<urgentCount<<" End");
	VIEW_DEBUG("PeerMinSkip "<<m_storage.GetMinIndex()<<" "<<startIndex<<" End");

	m_downloader.GetPeerManager().GetStatisticsData().DownloaderData.DownloadRange.MinIndex = startIndex;
	RandomGenerator rnd;

	UINT index = 0;
	UINT maxIndex = min(ResourceMaxIndex, startIndex + max(static_cast<UINT>(2000), sourceLength*4));
	for (index = startIndex; index <= maxIndex; index++)
	{
		if (m_downloader.NeedDownload(index))//仅统计需要下载的Piece
		{
			if( isGood == false && index < startIndex + urgentCount )
			{	// 本地资源状况不好,采用完全紧迫度策略
				UINT key = index - startIndex;
				LIVE_ASSERT( key >= 0 );
				LIVE_ASSERT( key < urgentCount );
				//				DWORD externalTimeout = 2000 + key * 3000 / urgentCount;
				DWORD externalTimeout;

//				externalTimeout =  (index - minIndex) * 5000 / dataLenFor20s;
				externalTimeout = 3500;
// 				LIMIT_MIN(externalTimeout, 500);
// 				LIMIT_MAX(externalTimeout, 5000);

				PieceTask pieceTask(index, externalTimeout );
				//VIEW_INFO( "Urgent timeOut index=" << index << " Index - min=" << index - minIndex  << " externalTimeout=" << externalTimeout);
				HealthyMap.insert(std::make_pair(key, pieceTask));//index对应的健康度是healthyCount
			}
			else
			{	// 本地资源状况很好,采用完全健康度策略
				// 				if( locateTimeCounter < 90*1000)
				// 				{
				//  					if( bufferTime < 70*1000 )
				//  					{	// 如果缓冲时间小于 50*1000, 则锁定下载视野.不下载太远的Piece
				// 	 					break;
				//  					}
				//  				}
				UINT healthyCount = 0;
				STL_FOR_EACH_CONST(PeerTunnelCollection, m_Tunnels, iter)
				{
					PeerTunnelPtr pt = *iter;
					PeerConnection* pc = &pt->GetConnection();
					if (pc->HasPiece(index))
						healthyCount++;
				}
				if( healthyCount > 0)
				{
					// 计算紧迫度参数
					// 采用随机数的原因是 在相同健康度的资源中 随机制定优先级
					UINT key = urgentCount + healthyCount*1000 + rnd.Next(999) + 2000;
//					UINT key = index - startIndex;
					LIVE_ASSERT(key >= urgentCount);

					//PieceTask pieceTask(index, 2000);

					PieceTask pieceTask(index, 5000);
					HealthyMap.insert(std::make_pair(key,pieceTask));	
				}
			}
		}
	}
	m_downloader.GetPeerManager().GetStatisticsData().DownloaderData.DownloadRange.MaxIndex = index;

//	VIEW_INFO("Healthy map length" << HealthyMap.size()<<" MinIndex "<<minIndex<<" StartINdex "<<startIndex<<" ResourceMaxIndex-SI "<<ResourceMaxIndex - startIndex<<" urgentCount "<<urgentCount);
	//VIEW_DEBUG("CalcHealthy To Index " << minIndex << " " << startIndex << "(" << (startIndex-minIndex) << ")" << " " << index << "(" << (index-minIndex) << ")" );
	//VIEW_DEBUG("CalcHealthy " << make_tuple(startIndex, ResourceMaxIndex, ResourceMaxIndex - startIndex) << make_tuple(m_Tunnels.size(), counter.GetElapsed()));
}

void QuotaManager::CalcHealthy2(HealthyDegreeCollection2& HealthyMap, UINT ResourceMaxIndex, const PEER_MINMAX& sourceMinMax) /*const*/
{
	//LIVE_ASSERT(HasQuota());
//	TimeCounter counter;

	/// 如果两个Piece间隔5000就会ResetBuffer
	//const UINT RESET_BUFFER_COUNT = 5000;

	//获得计算健康度最远的范围
	HealthyMap.clear();


//	UINT tryCount = 0;
	//获得健康度（如果endIndex非常大，如僵尸节点，则此处会造成死循环!）
	UINT startIndex = m_downloader.GetStartIndex(); // It's about max(skip, ResourceMinIndex).

	UINT sourceLength = sourceMinMax.GetLength();	// 正常情况时120s
	if (sourceLength <= 12) return;
// 	UINT sourceForUrgent = sourceLength * 3 / 4;	// 离min 90s 的长度做为urgent下载的依据
// 	UINT sourceFor50s = sourceLength * 5 / 12;
// 	UINT sourceFor40s = sourceLength * 4 / 12;
// 	UINT dataLenFor30s = sourceLength * 3 / 12;
// 	UINT dataLenFor20s = sourceLength * 2 / 12;
// 
// 	UINT dataLenFor10s = sourceLength * 1 / 12;
// 	UINT dataLenFor5s = sourceLength * 5 / 120;
// 	UINT dataLenFor3s = sourceLength * 3 / 120;
	double dataLenFor1s = (double)sourceLength / 120;

// 	VIEW_DEBUG("SourceMinMax "<<sourceMinMax.MinIndex<<" "<<sourceMinMax.MaxIndex<<" "<<sourceLength<<" End");
//  	UINT minIndex = m_storage.GetMinIndex();
// 	UINT preloacteIndex = m_streamBuffer.GetPrelocationIndex();
// 	LIMIT_MIN( minIndex, preloacteIndex );
	bool isGood = false;
// 	UINT bufferTime = m_streamBuffer.GetStatistics().BufferTime;
// 
// 	DWORD ExTimeOutLevel = m_downloader.GetPeerManager().GetConfig().TunnelConfig.ExTimeOutLevel;
// 	DWORD locateTimeCounter = m_downloader.GetLocateElapsed();


 	UINT urgentCount = 0;
// 	UINT curIndex = 0;
// 	const StreamIndicator& pBase = m_streamBuffer.GetBaseIndicator();
// 	if (pBase.GetTimeStamp() != 0)
// 	{
// 		curIndex = pBase.GetElapsedTime() * dataLenFor1s / 1000 + pBase.GetPieceIndex();
// 	}
// 
// 	curIndex = max(minIndex,curIndex);

//	UINT curdatalen1(0), curdatalen2(0);
// 	curdatalen1 = startIndex - curIndex;
// 	curdatalen2 = m_streamBuffer.GetPrepaDataLen();
//  UINT timeLen1(0), timeLen2(0), timeLen3(0);
//	timeLen1 = curdatalen1 / dataLenFor1s;
// 	timeLen2 = curdatalen2 / dataLenFor1s;
// 	timeLen3 = m_streamBuffer.GetPrepaDataTime();

//	UINT prepaDataLen = m_streamBuffer.GetPrepaDataLen();
	int	 iPrepaDataTime = m_streamBuffer.GetPrepaDataTime();
//	UINT dataLenFor15s = sourceLength / 8; // sourceLength * 15 /120;

	if (sourceLength < 100 )
//		|| !(startIndex > 0 && minIndex > 0))
	{
		urgentCount = 100;
	}
	else 
	{
		if (iPrepaDataTime > 15000)
		{
			isGood = true;
		}
		else 
		{
			urgentCount = (UINT)((15000 - iPrepaDataTime) * dataLenFor1s / 1000);
		}
	}


	VIEW_INFO("LocalPeerGood "<<isGood<<" End");
//	VIEW_INFO("UrgentCount "<<bufferTime<<" "<<urgentCount<<" End");
	VIEW_DEBUG("PeerMinSkip "<<m_storage.GetMinIndex()<<" "<<startIndex<<" End");

	m_downloader.GetPeerManager().GetStatisticsData().DownloaderData.DownloadRange.MinIndex = startIndex;
	DWORD externalTimeout;
	double redundantRate = m_downloader.GetPeerManager().GetStatisticsData().DownloaderData.GetRecentRedundentSubPieceRate();
	if (redundantRate < 5.0)
		externalTimeout = 1500;
	else if (redundantRate < 10.0)
		externalTimeout = 2000;
	else if (redundantRate < 15.0)
		externalTimeout = 2500;
	else
		externalTimeout = 3500;
	//PPLTRACE("Tady-> ||| redundantRate[%f] |||", redundantRate);
	RandomGenerator rnd;

	UINT uiDoubleKick(0);
	if (iPrepaDataTime < -5000)
	{
		uiDoubleKick = 3;
	}

	UINT index = 0;
	UINT maxIndex = min(ResourceMaxIndex, startIndex + max(static_cast<UINT>(2000), sourceLength*4));
	for (index = startIndex; index <= maxIndex; index++)
	{
		if (m_downloader.NeedDownload(index))//仅统计需要下载的Piece
		{
			if( isGood == false && index < startIndex + urgentCount )
			{	// 本地资源状况不好,采用完全紧迫度策略
#if !NO_HEALTHCOUNTCALC
				UINT key = index - startIndex;
				LIVE_ASSERT( key >= 0 );
				LIVE_ASSERT( key < urgentCount );
#endif // !NO_HEALTHCOUNTCALC

//				DWORD externalTimeout = 2000 + key * 3000 / urgentCount;
//				DWORD externalTimeout = 3500;

//				externalTimeout =  (index - minIndex) * 5000 / dataLenFor20s;
// 				LIMIT_MIN(externalTimeout, 500);
// 				LIMIT_MAX(externalTimeout, 5000);
				size_t subPieceCount = m_downloader.GetPossibleSubPieceCount(index, m_streamBuffer.GetSkipIndex());
				for (INT8 subI = 0;(size_t) subI < subPieceCount; subI++)
				{
					if (m_downloader.HasSubPiece(index, subI) )
					{
						continue;
					}

					SubPieceTask pieceTask(index, subI, externalTimeout );
					if (!m_downloader.IsRequested(index, subI))
					{
#if NO_HEALTHCOUNTCALC
						HealthyMap.push_back(pieceTask);//index对应的健康度是healthyCount
#else // !NO_HEALTHCOUNTCALC
						HealthyMap.insert(std::make_pair(key, pieceTask));
#endif // !NO_HEALTHCOUNTCALC
					}
					if (uiDoubleKick > 0 && redundantRate < 10.0)
					{
#if NO_HEALTHCOUNTCALC
						HealthyMap.push_back(pieceTask);//index对应的健康度是healthyCount
#else // !NO_HEALTHCOUNTCALC
						HealthyMap.insert(std::make_pair(key, pieceTask));
#endif // !NO_HEALTHCOUNTCALC
						UINT retryTimes(1);
						for (UINT i(uiDoubleKick); i > 0; --i)
						{
							if (m_retrySubpieceVector[i].subPiece == pieceTask.subPiece)
							{
								retryTimes += m_retrySubpieceVector[i].times;
							} 
						}
						m_retrySubpieceVector[uiDoubleKick].subPiece = pieceTask.subPiece;
						m_retrySubpieceVector[uiDoubleKick].times = retryTimes;

						static int siMaxRetryTimes(retryTimes);
						if (retryTimes >(size_t) siMaxRetryTimes)
						{
							siMaxRetryTimes = retryTimes;
							PPLTRACE("Tady-> Max retry time[%d][%d]", uiDoubleKick, retryTimes);
						}
						PPLTRACE("Tady-> SubPiece[%d][%d] retry time[%d][%d]", index, subI, uiDoubleKick, retryTimes);
						--uiDoubleKick;
					}

					//VIEW_INFO( "Urgent timeOut index=" << index << " Index - min=" << index - minIndex  << " externalTimeout=" << externalTimeout);

				}
			}
			else
			{	// 本地资源状况很好,采用完全健康度策略
				// 				if( locateTimeCounter < 90*1000)
				// 				{
				//  					if( bufferTime < 70*1000 )
				//  					{	// 如果缓冲时间小于 50*1000, 则锁定下载视野.不下载太远的Piece
				// 	 					break;
				//  					}
				//  				}
#if NO_HEALTHCOUNTCALC
				{
#else // !NO_HEALTHCOUNTCALC
				UINT healthyCount = 0;
				STL_FOR_EACH_CONST(PeerTunnelCollection, m_Tunnels, iter)
				{
					PeerTunnelPtr pt = *iter;
					PeerConnection* pc = &pt->GetConnection();
					if (pc->HasPiece(index))
						healthyCount++;
				}
				if( healthyCount > 0)
				{
					// 计算紧迫度参数
					// 采用随机数的原因是 在相同健康度的资源中 随机制定优先级
					UINT key = urgentCount + healthyCount*1000 + rnd.Next(999) + 2000;
//					UINT key = index - startIndex;
					LIVE_ASSERT(key >= urgentCount);
#endif // !NO_HEALTHCOUNTCALC
					size_t subPieceCount = m_downloader.GetPossibleSubPieceCount(index, m_streamBuffer.GetSkipIndex());
					for (INT8 subI = 0; (size_t)subI < subPieceCount; subI++)
					{
						SubPieceTask pieceTask(index, subI, 5500);
						//VIEW_INFO( "Urgent timeOut index=" << index << " Index - min=" << index - minIndex  << " externalTimeout=" << externalTimeout);
						if (!m_downloader.NeedDownload(pieceTask.subPiece))
						{
							continue;
						}
#if NO_HEALTHCOUNTCALC
							HealthyMap.push_back(pieceTask);//index对应的健康度是healthyCount
#else // !NO_HEALTHCOUNTCALC
							HealthyMap.insert(std::make_pair(key, pieceTask));
#endif // !NO_HEALTHCOUNTCALC
					}
				}
			}
		}
	}
	m_downloader.GetPeerManager().GetStatisticsData().DownloaderData.DownloadRange.MaxIndex = index;

//	VIEW_INFO("Healthy map length" << HealthyMap.size()<<" MinIndex "<<minIndex<<" StartINdex "<<startIndex<<" ResourceMaxIndex-SI "<<ResourceMaxIndex - startIndex<<" urgentCount "<<urgentCount);
	//VIEW_DEBUG("CalcHealthy To Index " << minIndex << " " << startIndex << "(" << (startIndex-minIndex) << ")" << " " << index << "(" << (index-minIndex) << ")" );
	//VIEW_DEBUG("CalcHealthy " << make_tuple(startIndex, ResourceMaxIndex, ResourceMaxIndex - startIndex) << make_tuple(m_Tunnels.size(), counter.GetElapsed()));

}
