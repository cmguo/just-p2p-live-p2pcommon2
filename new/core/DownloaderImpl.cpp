
#include "StdAfx.h"

#include "DownloaderImpl.h"
#include "PeerManagerImpl.h"
#include "PeerConnection.h"
#include "common/StreamBuffer.h"
#include "common/StreamBufferStatistics.h"
#include "common/MediaStorage.h"
#include "QuotaManager.h"
#include "PeerTunnel.h"
#include "common/MediaPiece.h"
#include "common/BaseInfo.h"
#include "framework/log.h"
#include "util/testlog.h"

#include <synacast/protocol/SubMediaPiece.h>

#include <boost/static_assert.hpp>
#include "ppl/util/time_counter.h"

#include <ppl/diag/trace.h>

using namespace ppl::util::detail;
//BOOST_STATIC_ASSERT(LIGHT_MRP == 1 && MRP_SUPPORTED != 0);

///Request���
const UINT REQUESET_TIMER_INTERVAL		= 250;
///���60sû�з�����Request����ô����ҪResetBuffer
const UINT REQUEST_STOPPED_TIMEOUT      = 1000 * 60;

///��ʼҪ�ȴ�4S���ܹ�RequestNextPiece
//const UINT START_REQUEST_NEXT_PIECE		= 4*1000;

///��Ҫ��������Piece����
//const UINT URGENT_DOWNLOAD_PIECE_COUNT	= 10;

///����ResetBuffer���ٱ�֤�ļ��
//const UINT MIN_RESET_BUFFER_INTERVAL	= 10*1000;

///ÿ���м��һ��RequestPieceTimeout�Ϳ�����
const UINT CHECK_REQUEST_TIMEOUT_INTERVAL = 800;



Downloader* DownloaderFactory::PeerCreate(CPeerManager& owner, StreamBuffer& streamBuffer, boost::shared_ptr<PeerInformation> peerInformation)
{
	return new PeerDownloader(owner, streamBuffer, peerInformation);
}

Downloader* DownloaderFactory::CreateTrivial()
{
	return new TrivialDownloader();
}

void TrivialDownloader::AddTunnel( PeerConnection* pc )
{
	PeerTunnelPtr tunnel;
	if ( pc->IsUDP() )
		tunnel.reset( PeerTunnelFactory::CreateUDP( *pc, *this, 0 ) );
	else
		tunnel.reset( PeerTunnelFactory::CreateTCP( *pc, *this, 0 ) );
	pc->SetTunnel( tunnel );
}


PeerDownloader::PeerDownloader(CPeerManager& owner, StreamBuffer& streamBuffer, boost::shared_ptr<PeerInformation> peerInformation) : 
		m_PeerManager(owner), 
		m_streamBuffer(streamBuffer), 
		m_storage(streamBuffer.GetStorage()), 
		m_StatusInfo(peerInformation->StatusInfo), 
		m_Connections(m_PeerManager.GetConnections()), 
		m_Statistics( owner.GetStatisticsData().DownloaderData ), 
		m_MaxSubPieceCountPerPiece( 0 ),
		quotaManager(*this, m_Tunnels)
{
	APP_DEBUG("New PeerDownloader");

	m_IsStartRequest = false;
	m_TotalExtraDown = 0;
	m_SourceResource = m_StatusInfo->GetSourceResource();

	m_RequestTimer.set_callback(boost::bind(&PeerDownloader::OnTimer, this));
}

PeerDownloader::~PeerDownloader()
{
	Stop();
	APP_DEBUG("Delete PeerDownloader");
}

bool PeerDownloader::Start(bool hasMDS)
{
	if (m_RequestTimer.is_started())
	{
		MANAGER_WARN("PeerDownloader already start!");
		return false;
	}
	m_StartRequestTickCount = ::GetTickCount() + m_PeerManager.GetConfig().StartupReferTime * 1000;	//�ȴ�������󣬿�ʼRequestNextPiece
	m_IsStartRequest = false;

	MANAGER_EVENT("PeerDownloader::Start()");

	m_RequestTimer.start(REQUESET_TIMER_INTERVAL);
	if (hasMDS)
	{
		// ���mds pool��Ϊ�գ�������Ŀ�ʼrequest����ʵ�ֿ���
		m_StartRequestTickCount = ::GetTickCount();
		m_LastPrelocateTickCount.sync();
		m_LastCalcsPrelocateTickCount.sync();
	}
	return true;
}

void PeerDownloader::Stop()
{
	if (m_RequestTimer.is_started())
	{
		MANAGER_EVENT("PeerDownloader::Stop() " << m_RequestTimer.is_started());
		m_RequestTimer.stop();
	}
}

bool PeerDownloader::CheckRquestPieceTimeout()
{
	// The internal-timeout.
	STL_FOR_EACH_CONST( PeerTunnelCollection, m_Tunnels, itr)
	{
		PeerTunnelPtr tunnel = *itr;
		LIVE_ASSERT( tunnel );
		if ( CanDownload( tunnel ) )
		{
			tunnel->CheckRequestPieceTimeout();
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//	��RequestNextPiece��صĺ���
//////////////////////////////////////////////////////////////////////////
UINT PeerDownloader::FindStartPosition()
{
	this->Verify();
	// �������ʽڵ��ж�λ����������ʽڵ��ж�λ�ɹ��Ļ����ͷ����ˡ����û�гɹ��������нڵ��ж�λ
	int ret = FindGoodStartPosition();
	if( ret > 0 ) return ret;

	//��������PeerConnection����Դ���¾���indexPieceStart������ResetBuffer()ʹ��

	UINT countOfPeerHaveThisPiece ,maxOfPeerHaveThisPiece = 0;
	PeerConnection* pc = NULL;
	UINT index ,StartIndex = 0;

	//���������һ��Peer��IndexOfPiece���ص��Ĵ������
	STL_FOR_EACH_CONST( PeerTunnelCollection, m_Tunnels, itr1 )
	{
		PeerTunnelPtr tunnel = *itr1;
		PeerConnection* conn = &tunnel->GetConnection();
		if ( CanDownload( tunnel ) )
		{
			countOfPeerHaveThisPiece = 0;
			index = conn->GetDenseMinIndex();
			if (index == 0)
				continue; // ���indexΪ0,�Ƿ���Ҫ������

			if ( m_SourceResource->CheckPieceIndexValid( index ) )//PP_1.0.6.6_C06 ��Reset���Buffer����һ�����ʵķ�Χ��
			{
				STL_FOR_EACH_CONST( PeerTunnelCollection, m_Tunnels, itr2 )
				{
					PeerTunnelPtr tempTunnel = *itr2;
					PeerConnection* tempConn = &tempTunnel->GetConnection();
					if (CanDownload( tempTunnel ) && tempConn->HasPiece(index))
						countOfPeerHaveThisPiece++;
				}
				if (countOfPeerHaveThisPiece > maxOfPeerHaveThisPiece || 
					(countOfPeerHaveThisPiece == maxOfPeerHaveThisPiece && index > StartIndex))
				{
					pc = conn;
					StartIndex = index;
					maxOfPeerHaveThisPiece = countOfPeerHaveThisPiece;
				}
			}
		}
	}

	if ((maxOfPeerHaveThisPiece > 0)&&(pc!= NULL))
	{
		MANAGER_INFO("Relocation to Peer "<<*pc<<" and index= "<<StartIndex);
		return StartIndex;
	}
	else
		return 0;
}

UINT PeerDownloader::FindGoodStartPosition()
{
	//��������PeerConnection����Դ���¾���indexPieceStart������ResetBuffer()ʹ��

	UINT countOfPeerHaveThisPiece ,maxOfPeerHaveThisPiece = 0;
	PeerConnection* pc = NULL;
	UINT index ,StartIndex = 0;

	//���������һ��Peer��IndexOfPiece���ص��Ĵ������
	STL_FOR_EACH_CONST( PeerTunnelCollection, m_Tunnels, itr1 )
	{
		PeerTunnelPtr tunnel = *itr1;
		PeerConnection* conn = &tunnel->GetConnection();
		// ��Դ���ȴ���source��Դ����һ��Ľڵ� ������Դ״���Ϻã����ʽڵ㣩
		if (CanDownload( tunnel ) && conn->IsResourceGood())
		{
			// ֻ�ںõĽڵ��ж�λ
			countOfPeerHaveThisPiece = 0;
			index = conn->GetDenseMinIndex();
			if (index == 0)
				continue; // ���indexΪ0,�Ƿ���Ҫ������

			if ( m_SourceResource->CheckPieceIndexValid( index ) )//PP_1.0.6.6_C06 ��Reset���Buffer����һ�����ʵķ�Χ��
			{
				STL_FOR_EACH_CONST( PeerTunnelCollection, m_Tunnels, itr2 )
				{
					PeerTunnelPtr tempTunnel = *itr2;
					PeerConnection* tempConn = &tempTunnel->GetConnection();
					if (CanDownload( tempTunnel ) && tempConn->HasPiece(index) && tempConn->IsResourceGood())
					{	// Ҳֻ�ںõ���Դ״���¼�����Դ�ص���
						countOfPeerHaveThisPiece++;
					}
				}
				if ((countOfPeerHaveThisPiece > maxOfPeerHaveThisPiece) || 
					(countOfPeerHaveThisPiece == maxOfPeerHaveThisPiece && index > StartIndex))
				{
					pc = conn;
					StartIndex = index;
					maxOfPeerHaveThisPiece = countOfPeerHaveThisPiece;
				}
			}
		}
	}

	if ((maxOfPeerHaveThisPiece > 0)&&(pc!= NULL))
	{
		MANAGER_INFO("Relocation to Peer "<<*pc<<" and index= "<<StartIndex);
		return StartIndex;
	}
	else
		return 0;
}


UINT PeerDownloader::FindPreLocationPosition()
{
	return FindStartPosition();
}

bool PeerDownloader::TryToDownload(SubPieceUnit subPiece, QuotaManager& quotaManager, int& tryCount, DWORD externalTimeOut)
{
	pair<bool, PeerTunnel*> result = quotaManager.TryToDownload(subPiece, tryCount, externalTimeOut);
	if (result.first)
	{
		if ( 0 == m_Statistics.StartAssignTime )
		{
			m_Statistics.StartAssignTime = m_Statistics.StartTime.elapsed32();
		}
		m_StartRequestTickCount = ::GetTickCount();
	}
	return result.first;
}

//��������
//��������
void PeerDownloader::RequestNextPiece()
{
	VIEW_INFO( "RequestNextPiece Begin "<<GetTickCount() );
	this->Verify();

	// The external-timeout
	m_Statistics.TotalExpiredSubPieces += m_requests.CheckTimeout(m_streamBuffer.GetSkipIndex());


	if ( m_Tunnels.empty() )
	{
		MANAGER_INFO("No PeerConnection to Request Piece.");
		return;
	}

	// ��������Դ��λ�㷨

	UINT ResourceMinIndex = GetResourceMinIndex();
	UINT ResourceMaxIndex = GetResourceMaxIndex();
	VIEW_INFO("ResourceInfo "<<ResourceMinIndex<<" "<<ResourceMaxIndex<<" End");

	if ( false == m_IsStartRequest || m_streamBuffer.GetSkipIndex() == 0 )
	{
		if ( false == m_IsStartRequest && m_streamBuffer.GetSkipIndex() == 0 )
		{
			UINT64 startTS = m_SourceResource->GetSourceTimeStamp();
			if (startTS != 0 && !m_Connections.empty())
			{
				for (PeerConnectionCollection::const_iterator iter = m_Connections.begin(); iter != m_Connections.end(); ++iter)
				{
					(*iter)->RequestFirstByTS(startTS);

					PPLTRACE("Tady -> Send First request [%d] \n", m_RequestTimer.get_times());
				}
			}
		}

		// ��λĬ�������� ������ 4s ������ 4��Peer �ͻῪ����λ����
		if ((::GetTickCount() > m_StartRequestTickCount) || 
			(m_Tunnels.size() > m_PeerManager.GetConfig().StartupReferPeerCount)
			|| !m_sparkTunnels.empty()) // Added by Tady, 011711: Spark!
		{
			UINT StartIndex = FindStartPosition(); // ���StartIndexΪ0,�Ƿ���Ҫ����
			// source_minmax����Ϊ��
			//if (!minmax.IsEmpty() && (StartIndex + 1000 > minmax.MinIndex) && (StartIndex < minmax.MaxIndex + 1000))//PP_1.0.6.6_C06 ��Reset���Buffer����һ�����ʵķ�Χ��
			if ( m_SourceResource->CheckPieceIndexValid( StartIndex ) )
			{
				m_Statistics.StartLocateTime = m_Statistics.StartTime.elapsed32();
				m_Statistics.FirstStartIndex = StartIndex;
				m_unfinishedDataPieces.Clear();
				m_streamBuffer.Reset(StartIndex);
				m_LocateTimeCounter.sync();
				VIEW_INFO("Relocate "<<StartIndex<<" End");
				m_PeerManager.ResetMaxExternalSpeed();
				MANAGER_EVENT("Reset Stream Buffer to index = "<<StartIndex<<" and Start Request Next Piece." << *m_SourceResource);
				m_IsStartRequest = true;
				m_StartRequestTickCount = ::GetTickCount();	//PP_1.0.6.6_C06
				m_LastPrelocateTickCount.sync();
				m_LastCalcsPrelocateTickCount.sync();
			}
			else
			{
				MANAGER_ERROR("PeerDownloader::RequestNextPiece Relocate Starting Position Failed. " << StartIndex << ", source_minmax: " << *m_SourceResource);
				return;
			}
		}
		else
		{
			MANAGER_INFO("PeerDownloader::RequestNextPiece Wait for more Peer to connect.");
			return;
		}
	}

	if (::GetTickCount() > m_StartRequestTickCount + m_PeerManager.GetConfig().RequestStoppedTimeout * 1000)	//PP_1.0.6.6_C06
	{
		m_IsStartRequest = false;
		MANAGER_WARN("There is too long not to send Request , maybe Source Reset and I came to be a dummy peer, so ResetBuffer");
		return;
	}

	/*
	if (m_PeerManager.GetConfig().MaxDownloadSpeed > 0 && 
		m_PeerManager.GetConfig().MaxDownloadSpeed < m_PeerManager.GetFlow().Download.GetRate() )
	{	// ��� ʵ�������ٶ� ������ �趨�� ��������ٶ� �Ͳ��ٷ���request��
		VIEW_INFO("The Speed ("<<m_PeerManager.GetFlow().Download.GetRate()<<") is Reach Max ("<<m_PeerManager.GetConfig().MaxDownloadSpeed<<")");
		return;
	}
	*/

	// ��������������㷨

	//��Request֮ǰ����ͳ��ÿһ��PC�����
	/*static QuotaManager quotaManager( *this, m_Tunnels );*/


	quotaManager.CalcTunnelReceiveTimes(); // Adjust windowsize of tunnel, and tunnel map.
#if !_Savage
	//if (!quotaManager.HasQuota())
	if (!quotaManager.HasRecvTimeMap())
	{
		MANAGER_INFO("PeerDownloader::RequestNextPiece no quota");
		return;
	}
#endif
	//MakeRequesting();

	//TimeCounter profiler;
	//UINT timeout = REQUEST_MAX_TIMEOUT;
	UINT StartIndex = m_streamBuffer.GetDownloadStartIndex();
//	UINT index = StartIndex;
//	size_t downCount = 0;
//	int tryCount = 0;	// ���Դ���
	
#if _Savage

	PPLTRACE("Tady -> CalcHealthy [%d] \n", m_RequestTimer.get_times());
	quotaManager.CalcHealthy2(m_healthyMap2, ResourceMaxIndex, m_SourceResource->GetMinMax()); // Calc tasks.
	if (!m_healthyMap2.empty())
	{
		if (quotaManager.RequestFromTaskQueue2()) // Start request.
		{
			if ( 0 == m_Statistics.StartAssignTime )
			{
				m_Statistics.StartAssignTime = m_Statistics.StartTime.elapsed32();
			}
			m_StartRequestTickCount = ::GetTickCount();

			PPLTRACE("Tady -> Send some request [%d] \n", m_RequestTimer.get_times());
		}
	}

#else // _Savage

	if (quotaManager.HasRecvTimeMap())
	{
		quotaManager.CalcHealthy2(m_healthyMap, ResourceMaxIndex, m_SourceResource->GetMinMax());

		static UINT maxMapLen = 0;
		if (m_healthyMap.size() > maxMapLen)
		{
			maxMapLen = m_healthyMap.size();
		}

		UINT countForQ = 0;
		for (HealthyDegreeCollection::iterator itr = m_healthyMap.begin(); 
			(itr != m_healthyMap.end()) && quotaManager.HasRecvTimeMap(); 
			//			m_healthyMap.erase(itr++)
			itr++
			)
		{
			PieceTask pieceTask = itr->second;
			UINT pieceIndex = pieceTask.pieceIndex;

			size_t subPieceCount = this->GetPossibleSubPieceCount(pieceIndex, m_streamBuffer.GetSkipIndex());
			if( subPieceCount == 0)
			{
				LIVE_ASSERT(false);
				subPieceCount = 5;
			}

			// ��piece���ҳ�Ҫ�����SubPiece��
			//VIEW_INFO("RequestNextPiece FIRST FOR IN!" << pieceIndex);
			for( UINT8 subPieceIndex = 0; subPieceIndex < subPieceCount; subPieceIndex ++ )
			{
				if ( false == quotaManager.HasRecvTimeMap() )
				{
					break;
				}
				SubPieceUnit subPiece(pieceIndex, subPieceIndex);
				if( NeedDownload(subPiece) )
				{
					//VIEW_INFO("RequestNextPiece SECOND FOR IN!" << pieceIndex << " " << subPieceIndex);
					if (TryToDownload(subPiece, quotaManager, tryCount, pieceTask.externalTimeOut))
					{
						countForQ ++;
						//++count;
						//++m_TotalExtraDown;
						++downCount;
						MANAGER_INFO( "Download " << subPiece );
						//if (m_TotalExtraDown < 100)
						//	break;
					}
// 					if( countForQ > 400 )
// 					{
// 						break;
// 					}
				}
			}
// 			if( countForQ > 400 )
// 			{
// 				break;
// 			}
			if ( false == quotaManager.HasRecvTimeMap() )
			{
				break;
			}
			//++count2;
		}
		//UINT downloadTime = profiler.GetElapsed();
		//VIEW_INFO( "TryToDownload End " << make_tuple(profiler.GetElapsed(), downloadTime - calcHealthyTime, calcHealthyTime) );
		VIEW_INFO(" countForQ " << countForQ);
	}

	quotaManager.RequestFromTaskQueue();
#endif // !_Savage

	

	// ������Prelocate�㷨

	int QuotaMax = (int)m_Tunnels.size();
	int QuotaBegin = 0;
	int QuotaUse = 0;
	int QuotaTimeout = 0;
	GetConnectionStatistics(QuotaBegin, QuotaUse, QuotaTimeout);
	(void)QuotaMax;
        VIEW_INFO( "QuotaUse "<<QuotaBegin<<" "<<QuotaUse<<" "<<QuotaMax<<" "<<QuotaTimeout<<" End");

	if ( m_LastPrelocateTickCount.elapsed() > m_PeerManager.GetConfig().PrelocateInterval * 1000 )
	{
		// Prelocate �������30sִ��һ��
		UINT sourceLength = m_SourceResource->GetLength();	// ���120���Piece��
		LIMIT_MIN( sourceLength, 24*5 );
		// ǰհ5s��Դ���
		int ForecastResource = sourceLength / 24;	// 5 s �ľ���
		int ForecastCount = 0;
		for( int i = 0; i < ForecastResource; i ++ )
		{
			if( m_streamBuffer.NeedDownload(StartIndex + i) == false )
			{
				ForecastCount ++;
			}
			else if( HasResourceInGoodPeer(StartIndex + i) == true )	
			{
				ForecastCount ++;
			}
		}

		// ���5s�������
		int DownloadResource = sourceLength / 24;	// 5 s �ľ���
		int DownloadCount = 0;
		for( int j = 1; j < DownloadResource; j ++ )
		{
			if( m_streamBuffer.NeedDownload(StartIndex - j) == true )	
				DownloadCount ++;
		}
			
		// Prelocate ���������� ��������һ���������ɷ���Prelocate����
		if( StartIndex < (2*ResourceMinIndex + ResourceMaxIndex)/3 )
		{	// StartIndex λ�� С�� ResourceMinIndex �� ResourceMaxIndex �� ǰ1/3��λ�õ�ʱ��

			// 1. ǰհ5s����Դ�ֲ��������Դ��С��90%
			// 2. ���5s���ݣ�������������ܶ�С��90%
			// 3. ���ڷ����������� С�� Ӧ�÷���ĸ�����һ��
			// 4. ��Դ�����1000 ���� StartIndex λ�� С�� ResourceMinIndex �� ResourceMaxIndex �� ǰ1/20 ��λ�õ�ʱ��
			if( (ForecastCount * 10  <= ForecastResource * 9 && ForecastCount > 0)
				|| (DownloadCount * 10 <= DownloadResource * 9 && DownloadCount > 0)
				|| (QuotaUse < QuotaBegin / 2 && QuotaUse < 15)
				|| (ResourceMaxIndex-ResourceMinIndex > 1000 && StartIndex < (19*ResourceMinIndex+ResourceMaxIndex)/20) 
			)
			{
				UINT PrelocationIndex = FindPreLocationPosition(); // ���StartIndexΪ0,�Ƿ���Ҫ����
				if( PrelocationIndex > m_streamBuffer.GetSkipIndex() )
				{	// ���� PrelocationIndex ����
					VIEW_DEBUG( "PreLocateIndex "<<PrelocationIndex<<" End" );
					m_streamBuffer.SetPrelocationIndex(PrelocationIndex);
					m_LocateTimeCounter.sync();
					// ���ﲻ��Ҫ����
					//m_PeerManager.ResetMaxExternalSpeed();
					m_LastPrelocateTickCount.sync();
				}
			}
		}
	}
//	VIEW_INFO( "RequestNextPiece End " << profiler.GetElapsed());
}

void PeerDownloader::OnTimer()
{
	this->Verify();

	// ���ݲ�ͬ�Ļ���, �ƶ���ͬ�� ���� Ƶ��
	//	UINT bufferTime = m_streamBuffer.GetStatistics().BufferTime;

	if (m_RequestTimer.get_times() % 2 == 0) // 0.5 sec.
	{
		CheckRquestPieceTimeout();
#if !_Savage
		RequestNextPiece();
#endif
	}

	if (m_RequestTimer.get_times() % 4 == 0) // 1 sec.
	{
		// Added by Tady, 011611: Spark!
		if (!m_sparkTunnels.empty() && m_RequestTimer.get_times() - m_sparkTick >= 40)
		{ // Unfreeze all!
			STL_FOR_EACH_CONST( PeerTunnelCollection, m_Tunnels, iter)
			{
				(*iter)->Freeze(false);
			}
			m_sparkTunnels.clear();
		}

#if _Savage
		RequestNextPiece(); 
#endif

		m_requests.RemoveOld(m_streamBuffer.GetSkipIndex());
		m_unfinishedDataPieces.RemoveOld( m_storage.GetMinIndex() );
		m_unfinishedHeaderPieces.LimitCount( 50 );
		m_Statistics.UnfinishedDataPieces = m_unfinishedDataPieces.GetSize();
		m_Statistics.UnfinishedSubPieceCount = m_unfinishedDataPieces.GetTotalSubPieceCount();

		m_Statistics.TotalRequestCount = m_requests.Count();
		m_Statistics.PeerTunnelCount = m_Tunnels.size();

		
	}
}

bool PeerDownloader::CanDownload( PeerTunnelPtr tunnel ) const
{
	return m_PeerManager.CanDownload( &tunnel->GetConnection() );
}

void PeerDownloader::OnHeaderPieceReceived(UnfinishedMediaPiecePtr headerPiece, PeerConnection* connection)
{
	MediaHeaderPiecePtr piece = MediaHeaderPiece::FromSubPieces( headerPiece, m_streamBuffer.GetSigner() );
	if ( ! piece )
	{
		VIEW_ERROR("invalid header piece received " << headerPiece->GetPieceIndex());
		LIVE_ASSERT( false );
		return;
	}
	TEST_LOG_OUT_ONCE("header piece received " << piece->GetPieceIndex());
	if(m_streamBuffer.NeedDownload(piece->GetPieceIndex()) == false)
	{	// �Ѿ����������Ƭ��
		VIEW_INFO("Retran "<<*connection<<" "<<piece->GetPieceIndex()<<" End");
	}
	m_requests.Remove(piece->GetPieceIndex(), connection);
	m_unfinishedHeaderPieces.Remove( piece->GetPieceIndex() );
	m_streamBuffer.AddHeaderPiece( piece );
}

void PeerDownloader::OnDataPieceReceived(UnfinishedMediaPiecePtr dataPiece, PeerConnection* connection)
{
	MediaDataPiecePtr piece = MediaDataPiece::FromSubPieces( dataPiece, m_streamBuffer.GetSigner() );
	if ( ! piece )
	{
		VIEW_ERROR("invalid data piece received " << dataPiece->GetPieceIndex());
		LIVE_ASSERT( false );
		return;
	}

	TEST_LOG_OUT_ONCE("data piece received " << piece->GetPieceIndex());
	if(m_streamBuffer.NeedDownload(dataPiece->GetPieceIndex()) == false)
	{	// �Ѿ����������Ƭ��
		VIEW_INFO("Retran "<<*connection<<" "<<piece->GetPieceIndex()<<" End");
	}
	m_unfinishedDataPieces.Remove(piece->GetPieceIndex());

	UINT headerPiece = piece->GetHeaderPiece();
	if (m_streamBuffer.NeedHeader(headerPiece) && !IsRequested(headerPiece))
	{
		if ( connection->RequestHeaderPiece(headerPiece, 3500 ) )
			this->AddRequest(headerPiece, connection, 3500);
	}
	m_streamBuffer.AddDataPiece( piece );
}

void PeerDownloader::OnSubPieceReceived(SubMediaPiecePtr subPiecePtr, PeerConnection* connection)
{
	SubPieceUnit subPiece = subPiecePtr->GetSubPieceUnit();

	VIEW_INFO( "SubPieceRecvSucced " << subPiece << subPiecePtr->GetSubPieceCount() << " from " << *connection );

	m_requests.RemoveSubPieceRequest(subPiecePtr->GetSubPieceUnit(), connection);

	if (subPiecePtr->GetPieceType() == PPDT_MEDIA_HEADER)
	{
		// sub piece for header piece
		UnfinishedMediaPiecePtr headerPiece = this->AddHeaderSubPiece(subPiecePtr);
		if ( headerPiece && headerPiece->IsFinished() )
		{
			OnHeaderPieceReceived(headerPiece, connection);
		}
		return;
	}

	if ( 0 == m_Statistics.FirstSubPieceReceivedTime )
	{
		m_Statistics.FirstSubPieceReceivedTime = m_Statistics.StartTime.elapsed32();
	}

	UnfinishedMediaPiecePtr dataPiece = AddDataSubPiece(subPiecePtr);
	if ( dataPiece && dataPiece->IsFinished() )
	{
		// �ܶ�subPiece �ϳ���һ�� Piece
		PPL_TRACE("data piece received " << dataPiece->GetPieceIndex());
		OnDataPieceReceived(dataPiece, connection);
		UDPT_DEBUG("Peer " << *connection << " UDPT: full data piece constructed " << subPiece);
	}
}

void PeerDownloader::OnPieceReceiveFailed(UINT pieceIndex, PeerConnection* connection)
{
	m_requests.Remove(pieceIndex, connection);
}

void PeerDownloader::OnSubPieceReceivedFailed(SubPieceUnit subPiece, PeerConnection* connection)
{
	if (m_requests.RemoveSubPieceRequest(subPiece, connection))
	{
		SubPieceTask newTask;
		newTask.subPiece = subPiece;
		newTask.externalTimeOut = 3500;
#if NO_HEALTHCOUNTCALC
		m_healthyMap2.push_front(newTask);
#else
		m_healthyMap2.insert(make_pair(0, newTask));
#endif
	}
}

bool PeerDownloader::NeedDownload(UINT pieceIndex) const
{
	return m_streamBuffer.NeedDownload(pieceIndex)/* && !m_requests.IsRequested(pieceIndex)<-------No useful*/; 
}

bool PeerDownloader::NeedDownload(SubPieceUnit subPieceUnit) const
{
	if( !m_streamBuffer.NeedDownload(subPieceUnit.PieceIndex) )
		return false;
	if( this->HasSubPiece(subPieceUnit.PieceIndex, subPieceUnit.SubPieceIndex) )
		return false;
	if( m_requests.IsRequested(subPieceUnit) )
		return false;
	return true;
}

UINT PeerDownloader::GetStartIndex() const
{
	return max(m_streamBuffer.GetDownloadStartIndex(), GetResourceMinIndex());
}

bool PeerDownloader::IsRequested(UINT piece) const
{
	return m_requests.IsRequested(piece);
}

bool PeerDownloader::IsRequested(UINT32 piece, UINT8 subPieceIndex) const
{
	return m_requests.IsRequested(SubPieceUnit(piece, subPieceIndex));
}


bool PeerDownloader::HasResource(UINT piece) const
{
	STL_FOR_EACH_CONST( PeerTunnelCollection, m_Tunnels, itr)
	{
		PeerTunnelPtr tunnel = *itr;
		PeerConnection* pc = &tunnel->GetConnection();
		//! ��Ҫ���˵���������
		if (pc->HasPiece(piece) )
			return true;
	}
	return false;
}

bool PeerDownloader::HasResourceInGoodPeer(UINT piece) const
{
	if( m_Tunnels.size() < 10 )
		return HasResource(piece);

	STL_FOR_EACH_CONST( PeerTunnelCollection, m_Tunnels, itr)
	{
		PeerTunnelPtr tunnel = *itr;
		PeerConnection* pc = &tunnel->GetConnection();
		//! ��Ҫ���˵���������
		if (pc->HasPiece(piece) && pc->IsResourceGood() == true)
			return true;
	}
	return false;
}

//bool PeerDownloader::CheckPieceIndexValid(UINT pieceIndex) const
//{
//	return m_SourceResource->CheckPieceIndexValid(pieceIndex);
//}

UnfinishedMediaPiecePtr PeerDownloader::AddDataSubPiece(SubMediaPiecePtr subPiece)
{
	MediaDataPiecePtr dataPiece = m_storage.GetDataPiece(subPiece->GetPieceIndex());
	if (dataPiece)
	{
		// data piece�Ѿ����ڣ�ֱ�ӷ��أ�����ΪAddSubPiece�ɹ��������е�subpiece���յ���
		return UnfinishedMediaPiecePtr();
	}
	if (false == m_IsStartRequest && m_streamBuffer.GetSkipIndex() == 0)
	{
		m_Statistics.StartLocateTime = m_Statistics.StartTime.elapsed32();
		UINT32 startIndex = subPiece->GetPieceIndex() - 1;
		LIVE_ASSERT(startIndex > 0);
		m_Statistics.FirstStartIndex = startIndex;
		m_streamBuffer.Reset(startIndex);
		m_LocateTimeCounter.sync();
		VIEW_INFO("Relocate "<<startIndex<<" End");
		m_PeerManager.ResetMaxExternalSpeed();
		MANAGER_EVENT("Reset Stream Buffer to index = "<<startIndex<<" and Start Request Next Piece." << *m_SourceResource);
		m_IsStartRequest = true;
		m_StartRequestTickCount = ::GetTickCount();	//PP_1.0.6.6_C06
		m_LastPrelocateTickCount.sync();
		m_LastCalcsPrelocateTickCount.sync();

		RequestNextPiece();
	}
	LIMIT_MIN(m_MaxSubPieceCountPerPiece, subPiece->GetSubPieceCount());
	//return m_unfinished.AddSubPiece(subPiece);
	return m_unfinishedDataPieces.AddSubPiece(subPiece);
}

void PeerDownloader::GetConnectionStatistics(int& highConnectionCount, int& requestingConnectionCount, int requestTimeoutConnectionCount) const
{
	highConnectionCount = 0;
	requestingConnectionCount = 0;
	requestTimeoutConnectionCount = 0;
	STL_FOR_EACH_CONST( PeerTunnelCollection, m_Tunnels, itr)
	{
		PeerTunnelPtr tunnel = *itr;
		LIVE_ASSERT( tunnel );
		if( tunnel->IsHighConnection() )
		{
			highConnectionCount ++;
		}
// 		if( pc->IsRequesting() )
// 		{
// 			requestingConnectionCount ++;
// 		}
//		if( pc->IsRequestTimeout() )
//		{
//			requestTimeoutConnectionCount ++;
//		}
	}
}

UINT PeerDownloader::GetResourceMinIndex() const
{
	//const PEER_MINMAX& sourceMinMax = m_SourceResource->GetMinMax();
    //    (void)sourceMinMax;
	UINT ResourceMinIndex = 0xffffffff;
	STL_FOR_EACH_CONST( PeerTunnelCollection, m_Tunnels, itr)
	{
		PeerTunnelPtr tunnel = *itr;
		PeerConnection* pc = &tunnel->GetConnection();
		if( // �ǽ��ֽڵ��ж�
		pc->GetMinIndex() > 0 
		&& pc->GetMinIndex() < ResourceMinIndex 
		&& //(pc->GetMinIndex()+2000 > sourceMinMax.MinIndex && pc->GetMinIndex() < sourceMinMax.MaxIndex+2*60*60*10) 
			m_SourceResource->CheckPieceIndexValid(pc->GetMinIndex())
		)
		{
			ResourceMinIndex = pc->GetMinIndex();
		}
	}
	return ResourceMinIndex;
}

UINT PeerDownloader::GetResourceMaxIndex() const
{
	//const PEER_MINMAX& sourceMinMax = m_SourceResource->GetMinMax();
    //    (void)sourceMinMax;
	UINT ResourceMaxIndex = 0;
	STL_FOR_EACH_CONST( PeerTunnelCollection, m_Tunnels, itr)
	{
		PeerTunnelPtr tunnel = *itr;
		PeerConnection* pc = &tunnel->GetConnection();
		if( pc->GetMaxIndex() > 0 
		&& pc->GetMaxIndex() > ResourceMaxIndex 
		&& //(pc->GetMaxIndex()+2000 > sourceMinMax.MinIndex && pc->GetMaxIndex() < sourceMinMax.MaxIndex+2*60*60*10) 
			m_SourceResource->CheckPieceIndexValid(pc->GetMaxIndex())
		)
		{
			ResourceMaxIndex = pc->GetMaxIndex();
		}
	}
	return ResourceMaxIndex;
}

bool PeerDownloader::HasSubPiece(UINT32 pieceIndex, UINT8 subPieceIndex) const
{
	SubPieceUnit subPieceUnit( pieceIndex, subPieceIndex );
	if ( m_unfinishedDataPieces.HasSubPiece(subPieceUnit) || m_unfinishedHeaderPieces.HasSubPiece(subPieceUnit) )
		return true;
	return m_storage.HasPiece(subPieceUnit.PieceIndex);
}

void PeerDownloader::AddTunnel( PeerConnection* pc )
{
	PeerTunnelPtr tunnel;
	if ( pc->IsUDP() )
		tunnel.reset( PeerTunnelFactory::CreateUDP( *pc, *this, 0 ) );
	else
		tunnel.reset( PeerTunnelFactory::CreateTCP( *pc, *this, 0 ) );
#if _Savage
	tunnel->SetTaskMapPtr(&m_healthyMap2);
#endif
	pc->SetTunnel( tunnel );

	// Added by Tady, 011611: Spark!
	if (pc->GetConnectionInfo().ConnectParam.IsSpark == true)
	{
		STL_FOR_EACH_CONST( PeerTunnelCollection, m_Tunnels, iter)
		{
			(*iter)->Freeze(true);
		}
		STL_FOR_EACH_CONST( std::vector<PeerTunnelPtr>, m_sparkTunnels, iter)
		{
			(*iter)->Freeze(false);
		}

		m_sparkTunnels.push_back(tunnel);
		m_sparkTick = m_RequestTimer.get_times(); // The last spark's tick.
	}
	else if (!m_sparkTunnels.empty())
	{
		tunnel->Freeze(true);
	}

	m_Tunnels.insert( tunnel );
	this->Verify();

	PPLTRACE("Tady -> Added a Tunnel [%d] \n", m_RequestTimer.get_times());
	if (m_healthyMap2.empty() /*&& m_Tunnels.size() == 1*/)
	{
		RequestNextPiece();
	}
	else
	{
		tunnel->RequestFromTaskQueue();
	}
}

void PeerDownloader::RemoveTunnel( PeerConnection* pc )
{
	this->Verify();
	size_t erasedCount = m_Tunnels.erase( pc->GetTunnel()->shared_from_this() );
	LIVE_ASSERT( 1 == erasedCount );

	// Added by Tady, 011611: Spark!
	if (pc->GetConnectionInfo().ConnectParam.IsSpark == true && !m_sparkTunnels.empty())
	{ // Unfreeze all!
		STL_FOR_EACH(std::vector<PeerTunnelPtr>, m_sparkTunnels, iter)
		{
			if (*iter == pc->GetTunnel()->shared_from_this())
			{
				m_sparkTunnels.erase(iter);
				break;
			}
		}

		if (m_sparkTunnels.empty())
		{
			STL_FOR_EACH_CONST(PeerTunnelCollection, m_Tunnels, iter)
			{
				(*iter)->Freeze(false);
			}
#ifdef _Savage
			if (m_RequestTimer.is_started())
			{
				quotaManager.RequestFromTaskQueue(); // Added for Spark.
			}
#endif
		}
	}
}

void PeerDownloader::AddRequest( UINT index, PeerConnection* pc, UINT timeout )
{
	m_requests.Add(index, pc, timeout);
	m_Statistics.TotalRequsts++;
}

void PeerDownloader::AddRequest( SubPieceUnit subPiece, PeerConnection* connection, UINT externalTimeout )
{
	m_requests.Add(subPiece, connection, externalTimeout);
	m_Statistics.TotalRequsts++;
}

SubMediaPiecePtr PeerDownloader::GetSubPiece(UINT pieceIndex, UINT8 subPieceIndex) const
{
	SubMediaPiecePtr subPiece = m_unfinishedDataPieces.GetSubPiece(pieceIndex, subPieceIndex);
	if ( subPiece )
		return subPiece;
	PieceInfo pieceInfo = m_storage.GetPieceInfo(pieceIndex);
	if (pieceInfo.IsValid())
	{
		// ��piece����ȡsub piece
		subPiece = pieceInfo.GetSubPiece(subPieceIndex);
		//LIVE_ASSERT( subPiece );
	}
	else
	{
		// ���headers
		subPiece = m_unfinishedHeaderPieces.GetSubPiece(pieceIndex, subPieceIndex);
		if ( subPiece )
			return subPiece;
		MediaHeaderPiecePtr headerPiece = m_storage.GetHeader(pieceIndex);
		if ( headerPiece )
		{
			subPiece = headerPiece->GetSubPiece(subPieceIndex);
			//LIVE_ASSERT( subPiece );
		}
	}
	return subPiece;
}

UnfinishedMediaPiecePtr PeerDownloader::AddHeaderSubPiece(SubMediaPiecePtr subPiece)
{
	//LIMIT_MIN(m_MaxSubPieceCountPerPiece, subPiece->GetSubPieceCount());
	LIVE_ASSERT( PPDT_MEDIA_HEADER == subPiece->GetPieceType() );
	MediaHeaderPiecePtr piece = m_storage.GetHeader( subPiece->GetPieceIndex() );
	if ( piece )
	{
		return UnfinishedMediaPiecePtr();
	}
	return m_unfinishedHeaderPieces.AddSubPiece(subPiece);
}

size_t PeerDownloader::GetPossibleSubPieceCount(UINT pieceIndex, UINT skipIndex) const
{
	size_t subPieceCount = this->GetSubPieceCount(pieceIndex);
	if (subPieceCount == 0)
	{
		MediaDataPiecePtr dataPiece = m_storage.GetDataPiece(skipIndex);
		if (dataPiece)
		{
			subPieceCount = dataPiece->GetSubPieceCount();
		}
	}
	if (subPieceCount == 0)
	{
		subPieceCount = GetMaxSubPieceCountPerPiece();
	}
	return subPieceCount;
}

size_t PeerDownloader::GetMaxSubPieceCountPerPiece() const
{
	return (m_MaxSubPieceCountPerPiece == 0) ? 5 : m_MaxSubPieceCountPerPiece;
}

size_t PeerDownloader::GetSubPieceCount(UINT pieceIndex) const
{
	size_t subPieceCount = 0;
	UnfinishedMediaPiecePtr subPiece = m_unfinishedDataPieces.GetSubPiece(pieceIndex);
	if (subPiece)
	{
		// subpiece����
		subPieceCount = subPiece->GetTotalCount();
	}
	else
	{
		PieceInfo pieceInfo = m_storage.GetPieceInfo(pieceIndex);
		if (pieceInfo.IsValid())
		{
			// piece����
			subPieceCount = pieceInfo.GetSubPieceCount();
		}
	}
	return subPieceCount;
}

bool PeerDownloader::NeedSubPiece( UINT32 pieceIndex, UINT8 subPieceIndex ) const
{
	return m_storage.HasPiece( pieceIndex ) || pieceIndex < m_storage.GetMinIndex() || this->HasSubPiece( pieceIndex, subPieceIndex );
}


