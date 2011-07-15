#include "StdAfx.h"

#include "PeerManagerFactory.h"
#include "common/AppModule.h"
#include "PeerConnection.h"
#include "common/StreamBuffer.h"
#include "common/MediaStorage.h"
#include "common/GloalTypes.h"

#include "common/IpPool.h"

#include "Downloader.h"
#include "PeerConnector.h"
#include "common/BaseInfo.h"
#include "pos/TrackerRequester.h"

#include "framework/socket.h"
#include "util/testlog.h"

#include <synacast/protocol/nat.h>
#include <ppl/diag/trace.h>
#include <ppl/util/ini_file.h>

#include "AppParam.h" // Added by Tady, 011911: Spark! --> LiveAppModuleCreateParam.SparkLen



const UINT8 MDS_MAX_SKIP_PERCENT = 20;
const UINT8 MDS_MIN_SKIP_PERCENT = 0;

const UINT32 MDS_MIN_BUFFER_TIME = 30;
//const UINT32 MDS_MAX_BUFFER_TIME = 60;
//const UINT32 MDS_MAX_CONNECTION_COUNT = 20;

/// 最长60s执行一次LIST
//const UINT LIST_MAX_INTERVAL = 65*1000;

/// 最短1s执行一次List
//const UINT LIST_MIN_INTERVAL = 2*1000;


/// 对于peer连接的初始保护时间
//const UINT PEER_INITIAL_PROTECTION_TIME = 15 * 1000;


ClientPeerManager::ClientPeerManager(AppModule * lpPeerModule) : CPeerManager(lpPeerModule)
{
	APP_DEBUG("New ClientPeerManager");
	m_downloader.reset(DownloaderFactory::PeerCreate(*this, m_streamBuffer, m_PeerInformation));
	m_streamBuffer.SetDownloader(m_downloader.get());
	m_DoListInterval = m_config.MinListInterval;
	m_maxPrepaDataTime = 0;
	m_bNeedConnectForDetect = true;
}

ClientPeerManager::~ClientPeerManager()
{
	this->Stop();
	APP_DEBUG("Delete ClientPeerManager");
}

bool ClientPeerManager::DoStart()
{
	CPeerManager::DoStart();
	m_LastTickCountDoList.sync();
	m_DoListInterval = m_config.MinListInterval;
	CheckMDS();
	const PeerAddressCollection& coll = m_PeerModule.GetMDSPeers();
	bool hasMDS = (!coll.empty());
	m_downloader->Start(hasMDS);
	return true;
}

int ClientPeerManager::GetMaxLocalPeerCount() const
{
	if( m_NetInfo->IsExposedIP() )
	{	// 公网IP 只用少部分的节点去连接别人  调整为 最大连接数的 1/10, 并且最少保持20个 
		return max(m_config.WANMinLocalPeerCount, m_PeerModule.GetMaxConnectionCount() * m_config.WANMinLocalPeerPercent / 100);
	}
	else
	{	// 内网IP 全力去连接别人
		return max(m_config.LANMinLocalPeerCount, (UINT)m_PeerModule.GetMaxConnectionCount() );
	}
}

void ClientPeerManager::InitiateConnections(UINT times, UINT maxTCPPendingCount, UINT maxConnectionCount)
{
	// 开始几秒内不发起连接，目的是为了等待IpPool有更多的地址。默认是 0
	if (times < m_config.ConnectorConfig.DelayTimes)
		return;

	// 控制发起的间隔。默认是4次，也就是每隔1s发起一轮连接
	if (times % m_config.ConnectorConfig.Interval != 0)
		return;

	int prepaDataTime = m_streamBuffer.GetPrepaDataTime();
	bool newGoodQuality = (prepaDataTime > 10 * 1000);
	if ( newGoodQuality != m_statistics.GoodQuality )
	{
		// 播放质量变化
		{
			if ( newGoodQuality )
			{
				TEST_LOG_OUT("buffer ok: " << m_statistics.InternalLastTimeQualityChanged.elapsed32());
			}
			else
			{
				TEST_LOG_OUT("bufferring: " << m_statistics.InternalLastTimeQualityChanged.elapsed32());
			}
			TEST_LOG_FLUSH();
		}
		m_statistics.RecordQualityGoBad(newGoodQuality, prepaDataTime, m_Connections.size(), m_flow.Download.GetRate(), m_flow.Upload.GetRate(), m_PeerInformation->GetStartedSeconds(), true);
	}

	int seconds = times / APP_TIMER_TIMES_PER_SECOND;
	// 正常情况，默认连接超时为10s
	m_ConnectTimeout = m_config.ConnectorConfig.LongConnectionTimeout;
	if (seconds <= 20)
	{
		// 开始10秒钟使用很短的超时。默认3s
		m_ConnectTimeout = m_config.ConnectorConfig.ShortConnectionTimeout;
	}
	else if (seconds <= 60)
	{
		// 开始50秒钟使用较短的超时。默认5s
		m_ConnectTimeout = m_config.ConnectorConfig.NormalConnectionTimeout;
	}

//	int degreeLeft = GetDegreeLeft();

	VIEW_INFO("Connection "
		<<GetPeerCount()<<" "
		<<m_connector->GetHandshakingPeerCount()<<" "
		<<m_connector->GetConnectingPeerCount()<<" End");
	//2.接着检查是否需要主动连接
//	//对于外网用户，如果播放状况比较好且连接个数有一定数目的时候可以停止ConnectToRemote
//	if ( IsStatusOK() )
//	{	// 对于外网用户 如果其播放情况非常的好 则不用再发起连接了
//		MANAGER_INFO("I'm  have enough PeerCount and Skip = 0 , no need ConnectToRemote.");
//	}
//	else
	{
//  	int maxLocalPeerCount = GetMaxLocalPeerCount();		// 本地最大连接数（连出）
// 		int localPeerCount = GetLocalPeerCount();			// 本地已经连上的连接数（连出）

// 		if ( GetDegreeLeft() < 0 &&							// 当 剩余连接数 不足
// 			localPeerCount > ( maxLocalPeerCount / 2 )		// 希望 实际连出的连接 占到 连出最大值 的一半
// 		)
// 		{
// 			return;
// 		}
 
		if (prepaDataTime > (int)m_maxPrepaDataTime)
		{
			m_maxPrepaDataTime = prepaDataTime;
//			LIMIT_MAX(m_maxPrepaDataTime, 65000);
		}
		UINT maxConnections = (UINT)max(25, (int)(m_PeerModule.GetMinConnectionCount() * 2.1));
		if (m_PeerModule.GetInfo().IntelligentConnectionControlDisabled == true)
			LIMIT_MAX(maxConnections,(UINT) m_PeerModule.GetSysInfo().MaxAppPeerCount + 5	);
		LIMIT_MAX(maxConnections, 36);

		TRACE("Tady -> Succeeded Rate = [%.4f] \n", m_statistics.DownloaderData.CalcSubPieceSucceededRate());
		TRACE("Tady -> PrepaDataTime0 = [%d] , \n", prepaDataTime );
		TRACE("Tady ->                          MaxConnectionCount = [%d] \n", maxConnections);
//	maxConnections = 20;
		if (/*m_PeerInformation->NetInfo->GetNATType() >= STUN_TYPE_FULLCONENAT || */m_PeerInformation->NetInfo->IsExposedIP())
		{
			UINT uploadspeed = m_PeerModule.GetMaxConnectionCount2(); // Actually, uploadspeed is the max Upload speed(in history) / 2048;
			UINT slipRange = 0; // in ms.
			if (uploadspeed < 30) // 512kB
			{
				slipRange = 20000;
			}
			else if (uploadspeed < 60) // 1mB
			{
				slipRange = 15000;
			}
			else if (uploadspeed < 120) // 2mB
			{
				slipRange = 10000;
			}
			else
			{
				slipRange = 8000;
			}
			if (((prepaDataTime > 10000 && (UINT)(m_maxPrepaDataTime - prepaDataTime) < slipRange) 
				&& m_lastTickDoConnect.elapsed() < 60000)
				|| m_Connections.size() > maxConnections)
			{
				return;
			}
		}
		else
		{
			if ((prepaDataTime > 10000
				&& m_lastTickDoConnect.elapsed() < 60000)
				|| m_Connections.size() > maxConnections)
			{
				return;
			}
		}

//		assert(localPeerCount == m_statistics.Degrees.All.Out);
		// 计算应该发起的连接数
// 		int connectCount = (int) (
// 			maxLocalPeerCount
// 			- localPeerCount 
// 			- m_connector->GetHandshakingPeerCount()
// 			- m_connector->GetConnectingPeerCount() * m_config.ConnectingPeerPercent / 100 );
		int connectCount = (int) (
			maxConnections
			- m_Connections.size() 
			- m_connector->GetHandshakingPeerCount()
			- m_connector->GetConnectingPeerCount() * m_config.ConnectingPeerPercent / 100  
			- (m_connector->GetTotalPendingPeerCount() - m_connector->GetConnectingPeerCount()));
		
// 		if (seconds <= 60)
// 		{
// 			// 开始1分钟多连一些 默认多 3 个
// 			connectCount += m_config.ConnectorConfig.StartConnectIncrement;
// 		}
		// 每轮发起连接数不能超过 MaxConnectionInitiateEveryTime 默认5
		LIMIT_MAX(connectCount, m_config.ConnectorConfig.MaxConnectionInitiateEveryTime);

		bool bIsBusy = false;
		if (m_PeerModule.GetInfo().TransferMethod == TRANSFER_TCP)
		{// 只有在“纯TCP模式”下需要在 这里 做并发连接数控制。在默认模式下（udp+tcp），并发连接数控制在udp->tcp过程中。
			bIsBusy = m_connector->IsBusy();
		}
		int i = 0;
		for (; (i < connectCount) && (!bIsBusy) && (m_Connections.size() < maxConnections); i++)
		{
			PeerItem peer;
			if (m_ipPool.GetForConnect(peer))
			{
				PeerConnectParam param(peer, false, m_ConnectTimeout, false);
				VIEW_INFO("Now Pending is:"<<tcp_socket::pending_connection_count()<<"  Do Conenct!" << peer.Info.Address);
				//ConnectToRemote(addr, false, times / APP_TIMER_TIMES_PER_SECOND);
				ConnectToRemote(peer.Info.Address, param);
			}
			else
			{
				//MANAGER_INFO("Already connect "<<i<<" peers, but GetForConnect = false , so break connect to remote!");
				m_DoListInterval = m_config.MinListInterval;
				break;
			}
		}
		MANAGER_INFO("try to connect "<<i<<" peers!");
	}
}

void ClientPeerManager::DoAddPeer(PeerConnection* conn)
{
	CPeerManager::DoAddPeer(conn);
	m_lastTickDoConnect.sync();
}

void ClientPeerManager::ListPeers()
{
	//1.首先检查是否需要进行DoList
	//对于List的情况，如果IPPool->NeedList()，那么就要定期开始List，List间隔与当前的连接个数有关。
	//	NeedDoList 默认情况下是 IPPool的总Peer数小于300的时候
	//	IsStatusOK 是指 (公网节点 连接数超过8个 且 缓冲满了） （内网节点 永远都 不会IsStatusOK）
	if (m_ipPool.NeedDoList() && !IsStatusOK())
	{
		MANAGER_DEBUG("CPeerManager::AdjustPeerConnection: NeedDoList " << make_tuple(m_LastTickCountDoList.elapsed(), m_DoListInterval));
		if (m_LastTickCountDoList.elapsed() > m_DoListInterval * 1000)
		{
			m_LastTickCountDoList.sync();
			MANAGER_DEBUG("CPeerManager::AdjustPeerConnection: ListPeers");
			m_PeerModule.GetTracker().ListPeers();
			// 默认 MinListInterval=2  MaxListInterval=65
			if (m_DoListInterval < m_config.MinListInterval)
				m_DoListInterval = m_config.MinListInterval;
			if (m_DoListInterval < m_config.MaxListInterval)
				m_DoListInterval *=2;			// 这个一个List间隔时间的 指数退避算法
												// 这个指数退避不论List成功还是失败都会执行
		}

		// 当 程序启动后5s 并且 没有从UDPTracker上List到任何数据，就启用 TCP Tracker
		if (m_startTime.elapsed() > 5 * 1000 && m_PeerModule.GetIPPool().GetSize() == 0)
		{	
			m_PeerModule.GetTracker().EnabledTCPTracker();
		}
	}
}

void ClientPeerManager::KickConnections(UINT seconds)
{
	return KickConnections2(seconds);
#if 0
	int degreeLeft = GetDegreeLeft();

	//3.最后检查已经连上的连接是否太多，需要删除
	int maxReservedCount = GetMaxReservedDegree();
	if (degreeLeft < maxReservedCount)//degreeLeft可能<0，默认情况，当剩余连接数小于最大连接数的1/10的时候,开启踢连接的策略
	{
//		assert(false == m_Connections.empty());//DegreeLeft的初始值是不是太小了？或者计算DegreeLeft有问题？
		typedef std::multimap<UINT,PeerConnection*> SpeedBasedPeerConnectionIndex;
		SpeedBasedPeerConnectionIndex DelPeerMapBySpeed;
		PeerConnection* pc = NULL;
		//根据传输速度排序
		for(PeerConnectionCollection::iterator itrOfPCSet = m_Connections.begin(); itrOfPCSet != m_Connections.end(); itrOfPCSet++)
		{
			pc = *itrOfPCSet;
			assert(pc != NULL);
			// 只踢自己主动发起的连接
			// 2006-09-08 cmp 尝试不管是否是主动发起的连接，都可以踢除
			if (CanKick(pc) && pc->GetConnectionTime() > m_config.InitialPeerProtectionTime * 1000)
			{
				// peer必须满足一般踢除的条件，并且握手时间必须超过15秒
				const FlowMeasure& flow = pc->GetFlow();

				// 计算是 历史总上传和总下载之后和 除以 总时间 得出的，也就是历史平均传输速度
#if 0                 
				UINT speed = flow.GetAverageRate();// + flow.GetRate();
#else
#pragma message("------ use LongTimeFlow to select slow peer connections")
				UINT speed = pc->GetLongTimeFlow().GetRateMax();  // It's avrg-speed in 60s.
#endif
				DelPeerMapBySpeed.insert(make_pair(speed, pc));
			}
		}
		//删除掉一定数量状态不好的Peer，一次适当多删除一些
		int maxDeleteCount = maxReservedCount;// + m_PeerModule.GetMaxAppPeerCountIncrement();
		int deletedCount = maxDeleteCount - degreeLeft;
		deletedCount = 5;
		MANAGER_WARN("Peer Count is enough , so del some peer, current PeerConnectionCount = " << make_tuple( GetPeerCount(), degreeLeft, maxDeleteCount, deletedCount ));

		LIMIT_MAX(deletedCount, (int)m_config.MaxOnceKickPeerCount); 
		int count = 0;
		int realCount = 0;
		for (SpeedBasedPeerConnectionIndex::iterator itr = DelPeerMapBySpeed.begin();
			 (count < deletedCount) && (itr != DelPeerMapBySpeed.end()); 
			 itr++,count++)
		{ 
			if (itr->first > 1500)
				break;
			pc = itr->second;

			this->KickPeer( pc, PP_KICK_DOWNLOAD_SLOW );

			realCount++;
		}
		MANAGER_WARN("actually kick peers  " << make_tuple(count, realCount));
	}
#endif
}

void ClientPeerManager::KickConnections2(UINT seconds)
{
	// Tady, 092708: Actually, curMaxconnections = curMinConnections * 2;
//	UINT curMinConnections = m_PeerModule.GetMinConnectionCount();
	size_t curMaxConnections = max(GetAvrgDownloadSpeed(), GetAvrgUploadSpeed()) / 2048;
	curMaxConnections = min(curMaxConnections, m_PeerModule.GetMaxConnectionCount2());

	if (m_Connections.size() > 0)
	{
		TRACE("Tady ->   avrgSpeed = [%d],           MaxConnectionCount2 = [%d] \n", this->GetAvrgDownloadSpeed()/* / m_Connections.size()*/,curMaxConnections);
	}

	if (m_Connections.size() >(size_t) curMaxConnections)
	{
		// assert(false == m_Connections.empty());//DegreeLeft的初始值是不是太小了？或者计算DegreeLeft有问题？
		typedef std::multimap<UINT,PeerConnection*> SpeedBasedPeerConnectionIndex;
		SpeedBasedPeerConnectionIndex DelPeerMapBySpeed;
		PeerConnection* pc = NULL;
		//根据传输速度排序
		for(PeerConnectionCollection::iterator itrOfPCSet = m_Connections.begin(); itrOfPCSet != m_Connections.end(); itrOfPCSet++)
		{ 
			pc = *itrOfPCSet;
			assert(pc != NULL);
			// 只踢自己主动发起的连接
			// 2006-09-08 cmp 尝试不管是否是主动发起的连接，都可以踢除
			if (CanKick(pc) && pc->GetConnectionTime() > m_config.InitialPeerProtectionTime * 1000
				&& pc->GetConnectionInfo().ConnectParam.IsSpark == false) // Added by Tady, 011611: Spark!
			{
				// peer必须满足一般踢除的条件，并且握手时间必须超过15秒
			//	const FlowMeasure& flow = pc->GetFlow();

				// 计算是 历史总上传和总下载之后和 除以 总时间 得出的，也就是历史平均传输速度
#if 0                 
				UINT speed = flow.GetAverageRate();// + flow.GetRate();
#else
//#pragma message("------ use LongTimeFlow to select slow peer connections")
				UINT speed = pc->GetLongTimeFlow().GetRateMax();  // It's avrg-speed in 30s.
#endif
				DelPeerMapBySpeed.insert(make_pair(speed, pc));
			}
		}
		//删除掉一定数量状态不好的Peer，一次适当多删除一些
// 		int maxDeleteCount = maxReservedCount;// + m_PeerModule.GetMaxAppPeerCountIncrement();
// 		int deletedCount = maxDeleteCount - degreeLeft;
		int deletedCount = m_Connections.size() - curMaxConnections;
//		MANAGER_WARN("Peer Count is enough , so del some peer, current PeerConnectionCount = " << make_tuple( GetPeerCount(), degreeLeft, maxDeleteCount, deletedCount ));

		LIMIT_MAX(deletedCount, (int)m_config.MaxOnceKickPeerCount); 
//		int count = 0;
		int realCount = 0;
		for (SpeedBasedPeerConnectionIndex::iterator itr = DelPeerMapBySpeed.begin();
			(realCount < deletedCount) && (itr != DelPeerMapBySpeed.end()); 
			itr++/*,count++*/)
		{ 
//			if (itr->first >= (this->GetAvrgDownloadSpeed() / m_Connections.size()))
// 			if (itr->first >= 1500)
// 				break;
			pc = itr->second;
// 			if (pc->IsInitFromRemote() == true)
// 				continue;

			this->KickPeer( pc, PP_KICK_DOWNLOAD_SLOW );

			realCount++;
		}
//		MANAGER_WARN("actually kick peers  " << make_tuple(count, realCount));
//		TRACE("actually kick peers  %d/%d", realCount, m_Connections.size());
	}

	// Added by Tady, 011611: Spark!
	if (!m_sparkConnections.empty())
	{
		for (std::vector<PeerConnection*>::iterator iter = m_sparkConnections.begin();
			iter != m_sparkConnections.end(); )
		{
			if ((*iter)->GetFlow().Download.GetTotalBytes() > 1024 * m_PeerModule.GetAppCreateParam().SparkLen)
			{
				KickPeer(*iter, PP_LEAVE_QUIT); // Enough data, bye bye.
				
				iter = m_sparkConnections.begin(); // Hehe, vector...
				continue;
			}
			++iter;
		}
	}
}

bool ClientPeerManager::IsStatusOK() const
{
	if ( m_NetInfo->IsExposedIP() )
	{
		// 公网节点
		// 如果无跳帧，并且peer数超过8个，则状态理想
		return (0 == m_StatusInfo->Status.SkipPercent) && (GetPeerCount() > (int)m_config.MinPeerCountForGoodStatus);
	}

	// 内网节点
	// 检查TransferMethod，如果是“无探测”模式，则认为状态不理想，需要一直做list
	if ( TRANSFER_NO_DETECT == m_PeerModule.GetInfo().TransferMethod )
		return false;

	// 如果无跳帧，并且peer数超过20个，则状态理想
	return (0 == m_StatusInfo->Status.SkipPercent) && (GetPeerCount() > (int)m_config.LANMinPeerCountForGoodStatus);
}

void ClientPeerManager::CheckMDS()
{
	DialPeers( m_config.ResearchConfig.PreferredPeers );
	DialLAN();
	if (NeedMDSSupport())
	{
		DialMDS();
	}
}

void ClientPeerManager::DialLAN()
{
	DialPeers(m_PeerModule.GetLANPeers());
}

bool ClientPeerManager::NeedMDSSupport() const
{
	return m_StatusInfo->BufferTime <= MDS_MIN_BUFFER_TIME || 
		m_StatusInfo->Status.SkipPercent >= MDS_MAX_SKIP_PERCENT || 
		GetPeerCount() <= 0;
}


void ClientPeerManager::CalcMaxPFPSBandWidth(UINT seconds)
{
	UINT maxPFPSBandWidth = 0;
	UINT minIndex = m_storage.GetMinIndex();
	if ( minIndex > 0 ) 
	{
		UINT maxExternalDownloadSpeed = GetMaxExternalDownloadSpeed();
		UINT referredMaxSpeed = maxExternalDownloadSpeed * 2 / 3;
		//UINT maxIndex = m_storage.GetMaxIndex();
		UINT sourceLength = m_StatusInfo->GetSourceResource()->GetLength();
		UINT prelocateIndex = m_streamBuffer.GetPrelocationIndex();
		UINT referredMinIndex = max(minIndex,prelocateIndex) + sourceLength * 3 / 4;
		UINT resourceMaxIndex = m_downloader->GetResourceMaxIndex();
		UINT nowSpeed = m_externalDownloadRate.Rate.GetRate();

		if( referredMinIndex < resourceMaxIndex && nowSpeed < referredMaxSpeed )
		{
			UINT rangeCount = m_streamBuffer.GetRangeCount(referredMinIndex,resourceMaxIndex);
			UINT rangeSize = resourceMaxIndex - referredMinIndex + 1;
			LIMIT_MIN( rangeSize, sourceLength*1/12);
			assert(rangeCount <= rangeSize );
			maxPFPSBandWidth = (referredMaxSpeed - nowSpeed) * rangeCount / rangeSize;
			VIEW_INFO("RangeInfo "<<referredMinIndex<<" "<<resourceMaxIndex<<" "<<rangeCount<<" "<<rangeSize<<" "<<referredMaxSpeed<<" "<<maxPFPSBandWidth<<" End");
		}
	}
	m_PeerModule.GetInfo().MaxPFPSBandWidth = maxPFPSBandWidth;
}

bool ClientPeerManager::ConnectForDetect( const PeerItem& peer )
{
//	static bool sbNeedQuickConnect = true;
	if ( m_config.ConnectorConfig.DisableDirectConnection || m_bNeedConnectForDetect == false )
		return false;
	if ( TRANSFER_TCP == m_PeerModule.GetInfo().TransferMethod )
	{
		// 如果是tcp传输模式，则不能用udp handshake代替udp detect
		return false;
	}

// 	int degreeLeft = GetDegreeLeft();
// 	int localPeerCount = m_statistics.Degrees.All.Out;
// 	LIMIT_MAX( localPeerCount, m_Connections.size() );
// 	int maxLocalPeerCount = GetMaxLocalPeerCount();		// 本地最大连接数
// 	int handshakingPeerCount = m_connector->GetHandshakingPeerCount();
// 	int maxConn = m_PeerModule.GetSysInfo().MaxConnectPendingCount;
// 	LIMIT_MAX( maxConn, 50 );
// 	if ( m_connector->GetTotalPendingPeerCount() > maxConn + 10 )
// 		return false;
// 
// 	// 当 剩余连接数 不足
// 	// 或者 实际连出的连接 占到 连出最大值 的一半
// 	if ( (degreeLeft < handshakingPeerCount / 2) || (localPeerCount > maxLocalPeerCount / 2) )
// 	{
// 		return false;
// 	}

	size_t maxConn = m_PeerModule.GetSysInfo().MaxConnectPendingCount;
	LIMIT_MAX( maxConn, 50 );
	if ( m_connector->GetTotalPendingPeerCount() >maxConn + 10 )
		return false;

	size_t minConnections = max((size_t)20, m_PeerModule.GetMinConnectionCount());
	if (m_PeerModule.GetInfo().IntelligentConnectionControlDisabled == true)
		LIMIT_MAX(minConnections, m_PeerModule.GetSysInfo().MaxAppPeerCount);

	if (m_Connections.size() + m_connector->GetHandshakingPeerCount() > minConnections)
	{
		m_bNeedConnectForDetect = false;
		return false;
	}
// 	static UINT detectCount =0; 
// 	detectCount++;
	PeerConnectParam param(peer, true, this->GetConnectTimeout(), false);
	return this->ConnectToRemote(peer.Info.Address, param);
} 

void ClientPeerManager::OnPlayerBufferring( bool isBufferOK )
{
	m_statistics.RecordQualityGoBad(isBufferOK, m_streamBuffer.GetPrepaDataTime(), m_Connections.size(), m_flow.Download.GetRate(), m_flow.Upload.GetRate(), m_PeerInformation->GetStartedSeconds(), false);
}



//bool ClientPeerManager::DelPeer(PeerConnection* conn, long reason)
//{
//	m_downloader->CheckRequestByConnection(conn);
//	return CPeerManager::DelPeer(conn, reason);
//}


void SourcePeerManager::KickConnections(UINT seconds)
{
	DoKickConnections(seconds);
	size_t externalPeerCount = GetExternalPeerCount();
        (void)externalPeerCount;
	VIEW_INFO("SourceInfo " << m_Connections.size() << " " << m_Connections.size() - externalPeerCount << " " << m_flow.Upload.GetRate() << " End");
}

UINT32 SourcePeerManager::CheckAcceptHandshake(bool isVIP, UINT32 externalIP, const PeerHandshakeInfo& handshakeInfo) const
{
	if (isVIP)
		return 0;
	// 进行内网ip检查
	bool isExternalIP = (handshakeInfo.Address.IP == externalIP);
	if (isExternalIP)
		return 0;
	size_t maxPeerCount = m_PeerModule.GetMaxConnectionCount();
	assert(maxPeerCount > 3);
	int maxInternalPeerCount = (int) ( maxPeerCount / 3 );
	LIMIT_MIN_MAX(maxInternalPeerCount, 10, 20);
	size_t externalPeerCount = GetExternalPeerCount();
	int internalPeerCount = (int) ( m_Connections.size() - externalPeerCount );
	assert(internalPeerCount >= 0);
	if (externalPeerCount >= maxPeerCount / 2 || internalPeerCount >= maxInternalPeerCount)
	{
		// 如果外网peer已经足够多，则拒绝内网peer
		return PP_REFUSE_INNER_PEER;
	}
	return 0;
}

void SourcePeerManager::DoKickConnections(UINT seconds)
{
	KickInternalPeers();
	KickBadPeers();

	int maxDegreeLeft = m_PeerModule.GetMaxConnectionCount() / 10;
	LIMIT_MIN(maxDegreeLeft, 1);

	if (GetDegreeLeft() >= maxDegreeLeft)
		return;

	// 按照qos排序，踢最慢的若干个
	typedef std::multimap<UINT, PeerConnection*> QOSIndexedPeerSet;
	QOSIndexedPeerSet peers;
	STL_FOR_EACH_CONST(PeerConnectionCollection, m_Connections, itr)
	{
		PeerConnection* pc = *itr;
		assert(pc != NULL);
		if (CanKick(pc))
		{
			//VIEW_INFO("PeerSpeedOneMinutes "<<*pc<<pc->GetAverageUploadSpeed()<<" End");
#if 0
#pragma message("------ used long time flow for SourcePeerManager::DoKickConnections")
			peers.insert(make_pair(pc->GetLongTimeFlow().Upload.GetRate(), pc));
#else
			peers.insert(make_pair(pc->GetAverageUploadSpeed(), pc));
#endif
		}
	}
	
	int maxDeleteCount = maxDegreeLeft - GetDegreeLeft();
	int deleteCount = 0;
	STL_FOR_EACH_CONST(QOSIndexedPeerSet, peers, iterQos)
	{
		if (deleteCount >= maxDeleteCount)
			break;
		PeerConnection* pc = iterQos->second;
		if (pc->GetConnectionTime() > 10 * 1000)
		{
			// 超过了保护时间才踢
			KickPeer(pc, PP_KICK_LOW_QOS);
		}
		deleteCount++;
	}
	return;
}

bool SourcePeerManager::CanDownload(const PeerConnection* conn) const
{
	return false;
}

int SourcePeerManager::GetMaxRepeatedConnectionCount(bool isVip) const
{
	// source不需要主动连接，也不会主动断开，所以设成任何值都没有什么影响
	// 但如果在PeerConnection::HandleHello中检测到重复连接时，由接收连接方断开连接，则此值就有意义了
	// 2006-01-12 cmp server端也需要提供一定程序的防范
	return isVip ? 2 : 1;
}

bool SourcePeerManager::CanAcceptConnection( UINT ip ) const
{
	// mcc需要需要限制同一IP(或同一内网)的peer的个数
	if (ip == 0)
		return false;
	bool isVip = this->IsVIP(ip);
	if (isVip)
	{
		// 不对vip进行限制
		return true;
	}
	int degreeLeft = GetDegreeLeft();
	assert(degreeLeft > 0);
	int maxCount = 1 + degreeLeft / 2;
	int count = 0;
	STL_FOR_EACH_CONST(PeerConnectionCollection, m_Connections, itr)
	{
		PeerConnection *pc = *itr;
		assert(pc != NULL);
		//未连接的也要检查，需要根据SocketAddress，而不是握手时汇报过来的地址来进行检查
		SimpleSocketAddress thisAddr = pc->GetSocketAddress();
		MANAGER_EVENT("CPeerManager::CanAcceptConnection " << thisAddr << " " << count);
		//const PEER_ADDRESS& thisAddr = pc->GetPeerInfo()->NetInfo.Address;
		if (thisAddr.GetIP() == ip)
		{
			++count;
			if (count >= maxCount)
			{
				MANAGER_EVENT("CPeerManager::CanAcceptConnection false " << make_tuple(GetPeerCount(), IPAddress(ip)) << "  " << make_tuple(count, maxCount));
				return false;
			}
		}
	}
	MANAGER_EVENT("CPeerManager::CanAcceptConnection " << make_tuple(GetPeerCount(), IPAddress(ip)) << "  " << make_tuple(count, maxCount));
	return true;
}

size_t SourcePeerManager::GetExternalPeerCount() const
{
	size_t count = 0;
	STL_FOR_EACH_CONST(PeerConnectionCollection, m_Connections, iter)
	{
		PeerConnection* pc = *iter;
		if (!pc->IsInnerIP())
		{
			++count;
		}
	}
	return count;
}

void SourcePeerManager::KickInternalPeers()
{
	// 踢内网
	int maxPeerCount = m_PeerModule.GetMaxConnectionCount();
	int externalPeerCount = (int)GetExternalPeerCount();
	if (externalPeerCount < maxPeerCount / 2)
		return;
	// 如果公网peer个数大于最大连接数的一半，踢掉所有的内网peer
	for (PeerConnectionCollection::iterator itr = m_Connections.begin(); itr != m_Connections.end(); )
	{
		PeerConnection* pc = *itr;
		itr++;
		assert(pc != NULL);
		if (CanKick(pc) && pc->IsInnerIP())
		{
			KickPeer(pc, PP_KICK_LAN);
		}
	}
}

void SourcePeerManager::KickBadPeers()
{	// 此函数是强踢,即使连接数不是满的

	// 踢速度过低的peer
	UINT lowestSpeed = 1000;
	UINT lowestQos = 1000;
	// 通过平均的PieceSize和计算的平均PieceRate，可以估算码流率（B/s）
	UINT bufferTime = m_storage.GetBufferTime();
	double localSpeed = 1000; // 缺省的最小值
	if ( bufferTime > 10 * 1000 )
	{
		// 缓冲时间大于10秒
		 localSpeed = 1.0 * m_storage.GetPieceSize() * m_storage.GetPieceCount() / bufferTime;
	}
	assert( localSpeed == 1000 || (localSpeed > 30 * 1024 && localSpeed < 80 * 1024) );
	// 最低速度限制为平均码流率的1/50
	if (localSpeed > 50)
	{
		lowestSpeed = static_cast<UINT>(localSpeed / 50);
	}
	else
	{
		lowestSpeed = 1;
	}
	if (localSpeed > 5 * 1024)
	{
		lowestQos = static_cast<UINT>(localSpeed / (5 * 1024));
	}
	else
	{
		lowestQos = 1;
	}
	for (PeerConnectionCollection::iterator itr = m_Connections.begin(); itr != m_Connections.end(); )
	{
		PeerConnection* pc = *itr;
		itr++;
		assert(pc != NULL);

		// 检查保护时间
		if (!CanKick(pc) || pc->GetConnectionTime() <= 10 * 1000)
			continue;

		// 检查最近一次收到报文的时间
		if (pc->GetLastReceivePacketTime() > 20 * 1000)
		{
			KickPeer(pc, PP_KICK_IDLE);
			continue;
		}

		// 检查速度是否低于预期的最低速度
		//if (pc->GetFlow().Upload.GetAverageRate() <= lowestSpeed)
		//{
		//	KickPeer(pc, PP_LEAVE_SOURCE_KICKED_TOOL_SLOW_PEER);
		//	continue;
		//}

		// 检查qos是否低于预期的最低值
/*		if (pc->GetConnectionTime() > 60 * 1000 && pc->GetMaxQOS() <= lowestQos)
		{
			KickPeer(pc, PP_LEAVE_SOURCE_KICKED_LOW_QOS_PEER);
			continue;
		}*/

		// 检查资源是否重叠
		UINT minIndex = pc->GetRealMinIndex();
		UINT maxIndex = pc->GetMaxIndex();

		if (minIndex > 0 && maxIndex > 1 && m_StatusInfo->MinMax.MaxIndex < minIndex) 
		{	// 当Source的 Max 小于 Peer 的 Min 的 时候,各个Peer要重定位的话估计很难，故直接踢掉
			KickPeer(pc, PP_KICK_NOT_OVERLAPPED);
			continue;
		}

		if (minIndex > 0 && maxIndex > 1 && maxIndex + 100 < m_StatusInfo->MinMax.MinIndex
			&& pc->GetConnectionTime() > 60 * 1000) 
		{	// 当Source的 Min 大于 Peer 的 Max 的 时候, 给这个Peer一分钟的之间去重定位
			KickPeer(pc, PP_KICK_NOT_OVERLAPPED);
			continue;
		}

	}
}

void SourcePeerManager::LoadConfiguration()
{
	CPeerManager::LoadConfiguration();
	ini_file ini;
	m_PeerModule.InitIni(ini);
	ini.set_section(TEXT("acceptor"));
	m_config.ConnectorConfig.IsRefuseUDPConnectionRequest = ini.get_bool(TEXT("RefuseUDPConnectionRequest"), false);
	m_config.ConnectorConfig.IfOnlyAcceptMDS = ini.get_bool(TEXT("OnlyAcceptMDS"), false);
	VIEW_INFO("LoadConfiguration RefuseUDPConnectionRequest, OnlyAcceptMDS " 
		<< make_tuple(m_config.ConnectorConfig.IsRefuseUDPConnectionRequest, m_config.ConnectorConfig.IfOnlyAcceptMDS));
}





const bool MDS_DEFAULT_NEED_LIMIT_MAX_SERVE_TIME	= true;
const UINT MDS_DEFAULT_MIN_SERVE_TIME				= 1 * 60;
const UINT MDS_DEFAULT_MAX_SERVE_TIME				= 10 * 60;
const UINT MDS_DEFAULT_RESERVED_DEGREE_RATIO		= 10;


MdsConfiguration::MdsConfiguration()
{
	MinServeTime = MDS_DEFAULT_MIN_SERVE_TIME;
	MaxServeTime = MDS_DEFAULT_MAX_SERVE_TIME;
	ReservedDegreeRatio = MDS_DEFAULT_RESERVED_DEGREE_RATIO;
	NeedLimitMaxServeTime = MDS_DEFAULT_NEED_LIMIT_MAX_SERVE_TIME;
}

void MdsConfiguration::Load(ini_file& ini)
{
	// 单位: 秒
	MinServeTime = ini.get_int(TEXT("MinServeTime"), 0);
	MaxServeTime = ini.get_int(TEXT("MaxServeTime"), 0);
	ReservedDegreeRatio = ini.get_int(TEXT("ReservedDegreeRatio"), 0);
	NeedLimitMaxServeTime = ini.get_bool(TEXT("NeedLimitMaxServeTime"), false);
	if (MinServeTime == 0)
		MinServeTime = MDS_DEFAULT_MIN_SERVE_TIME;
	if (MaxServeTime == 0)
		MaxServeTime = MDS_DEFAULT_MAX_SERVE_TIME;
	if (ReservedDegreeRatio == 0)
		ReservedDegreeRatio = MDS_DEFAULT_RESERVED_DEGREE_RATIO;
	if (ReservedDegreeRatio > 90)
		ReservedDegreeRatio = 90;
	MANAGER_INFO("MDS Configuration: ServeTime=" << make_tuple(MinServeTime, MaxServeTime) 
		<< ", ReservedDegreeRatio=" << ReservedDegreeRatio 
		<< ", NeedLimitMaxServeTime=" << (NeedLimitMaxServeTime ? "true" : "false"));

	// 转化为毫秒
	MinServeTime *= 1000;
	MaxServeTime *= 1000;
}

void MdsConfiguration::Save(ini_file& ini)
{
	// 单位: 秒
	ini.set_int(TEXT("MinServeTime"), MinServeTime / 1000);
	ini.set_int(TEXT("MaxServeTime"), MaxServeTime / 1000);
	ini.set_int(TEXT("ReservedDegreeRatio"), ReservedDegreeRatio);
	ini.set_bool(TEXT("NeedLimitMaxServeTime"), NeedLimitMaxServeTime);
}





SourceAgentPeerManager::SourceAgentPeerManager(AppModule* owner) : ClientPeerManager(owner)
{
}

void SourceAgentPeerManager::CheckMDS()
{
	DialMDS();
}


void SourceAgentPeerManager::LoadConfiguration()
{
	ClientPeerManager::LoadConfiguration();
	//IniFile ini;
	//const TCHAR* section_name = TEXT("SourceInfo");
	//ini.SetLocalFileName(TEXT("config\\PPSRC.ini"));
	//ini.SetSection(section_name);
	//m_mdsConfig.Load(ini);
	// 这个Save只是为了创建一个初始的ini文件
	//m_mdsConfig.Save(ini);
}

bool SourceAgentPeerManager::HasServedTooLongTime(DWORD serveTime) const
{
	return m_mdsConfig.NeedLimitMaxServeTime && serveTime > m_mdsConfig.MaxServeTime; // 超过最大服务时间
}

bool SourceAgentPeerManager::NeedContinueServing(DWORD serveTime) const
{
	return serveTime <= m_mdsConfig.MinServeTime; // 不超过最小服务时间
}

int SourceAgentPeerManager::CalcExtraCount() const
{
//	assert(m_ReservedConnectionPercent > 0 && m_ReservedConnectionPercent <= 100);
	int degreeLeft = GetDegreeLeft();

	int maxCount = max(m_PeerModule.GetSysInfo().MaxAppPeerCount, static_cast<WORD>(1)); // 预防MaxAppPeerCount为0的情况
	int degreeLeftRatio = degreeLeft * 100 / maxCount;

	// 要删除的peer个数，此个数包含超过最大服务时间而被删除的peer
	int extraCount = maxCount * m_mdsConfig.ReservedDegreeRatio / 100 - degreeLeft;
	(void)degreeLeftRatio;
        MANAGER_INFO("PeerManager:AdjustPeerConnection ExtraCount=" << extraCount 
		<< " (DegreeLeft,DegreeLeftRatio)=" << make_tuple(degreeLeft, degreeLeftRatio));

	return extraCount;
}

//针对SourceAgent的调节方法：保持10%的DegreeLeft，超出的部分踢掉QOS小的－－不提内网
void SourceAgentPeerManager::KickConnections(UINT seconds)
{
	if (seconds % 5 != 0)
		return;

	PeerConnectionQOSCollection qosColl; // 将qos最小的几个填入qosColl，然后删除掉
	int extraCount = CalcExtraCount();
	int newExtraCount = (int) ( CheckExtraPeers(qosColl, extraCount) );
	int count = (int)DeleteExtraPeers(qosColl, newExtraCount);
	(void)count;
        MANAGER_INFO(count << " peers have been deleted. (ExtraCount,KickedCount)=" 
		<< make_tuple(newExtraCount, extraCount - newExtraCount));
}

void SourceAgentPeerManager::AddExtraPeer(PeerConnectionQOSCollection& qosColl, PeerConnection* pc, size_t maxCount)
{
	assert(maxCount > 0);
	// 超过最小服务时间，需要检查
	UINT qos = pc->GetQOS();
	if (qosColl.size() < maxCount)
	{
		qosColl.insert(make_pair(qos, pc));
		MANAGER_INFO("insert deleting peer " << qos);
		return;
	}

	// maxCount > 0，所以qosColl.size() > 0
	assert(!qosColl.empty());
	PeerConnectionQOSCollection::iterator iterMinQos = qosColl.end();
	--iterMinQos;
//	PeerConnection* tempConn = iterMinQos->second;
	if (qos < iterMinQos->first)
	{
		// 如果pc的qos小于qosColl中最大的qos，则删除原来qos最大的节点，并将加进来
		qosColl.erase(iterMinQos);
		qosColl.insert(make_pair(qos, pc));
		MANAGER_INFO("insert deleting peer " << make_tuple(iterMinQos->first, qos));
	}
	// 如果pc的qos不小于qosColl中最大的qos，则忽略之
}

size_t SourceAgentPeerManager::CheckExtraPeers(PeerConnectionQOSCollection& qosColl, size_t extraCount)
{
	PeerConnectionCollection::const_iterator iter = m_Connections.begin();
	while (iter != m_Connections.end())
	{
		PeerConnection* pc = *iter;
		++iter; // 先推进迭代器，因为下面可能删除掉pc

		if (!CanKick(pc))
			continue;
		UINT64 serveTime = pc->GetStartTime().elapsed(); // 服务时间：从连接(即握手)成功到现在为止的时间
		if (HasServedTooLongTime(serveTime))
		{
			// 超过最大服务时间，删除之
			DelPeer(pc, PP_LEAVE_KICKED);
			--extraCount; // extraCount需要排除直接踢掉的peer个数
			continue;
		}
		if (extraCount <= 0 || NeedContinueServing(serveTime))
			continue; // 如果没有多余的peer，或者还没有超过最小服务时间，需要继续服务，则跳过

		AddExtraPeer(qosColl, pc, extraCount);
	}
	return extraCount;
}

size_t SourceAgentPeerManager::DeleteExtraPeers(const PeerConnectionQOSCollection& qosColl, size_t maxCount)
{
	size_t count = 0;
	STL_FOR_EACH_CONST(PeerConnectionQOSCollection, qosColl, iter)
	{
		if (count >= maxCount)
			break;
		++count;
		MANAGER_INFO("kick peer " << iter->first << " " << *iter->second);
		DelPeer(iter->second, PP_LEAVE_KICKED);
	}
	return count;
}

bool SourceAgentPeerManager::CanDownload(const PeerConnection* conn) const
{
	// Source Agent 不能从Input Source下载数据，由Input Source推送数据
	const PEER_ADDRESS & innerAddress = conn->GetHandshakeInfo().Address;
	UINT outterIP = conn->GetSocketAddress().GetIP();
	if ( IsInputSource( outterIP, innerAddress ) )
	{
		return false;
	}

	// mds只从vip peer处下载数据
	return conn->IsVIP();
}

int SourceAgentPeerManager::GetMaxRepeatedConnectionCount(bool isVip) const
{
	// mds到mcc和其它mds的连接可以有2个，而普通peer只允许一个连接
	return isVip ? 2 : 1;
}

bool SourceAgentPeerManager::NeedResource(const PeerConnection* pc) const
{
	// mds只需要解析vip peer的资源位图
	return pc->IsVIP();
}

void SourceAgentPeerManager::LoadInputSources( const PeerAddressArray &addresses )
{
	m_InputSources.clear();
	for( PeerAddressArray::const_iterator pointer = addresses.begin(); pointer != addresses.end(); ++pointer )
	{
		PEER_ADDRESS innerAddress = *pointer;

		PeerAddressArray::const_iterator outterPointer = ++pointer;
		if ( outterPointer == addresses.end() )		// 输入流地址还没完，遇到奇数个地址，肯定是有错误
		{
			break;
		}

		PEER_ADDRESS outterAddress = *outterPointer;
		assert( outterAddress.IP );					// 检验地址
		if ( innerAddress.IP == 0 || outterAddress.IP == 0 )
		{
			break;
		}

		m_InputSources.insert( PairedPeerAddress( innerAddress, outterAddress ) );
	}
}

bool SourceAgentPeerManager::ConnectToRemote( const PEER_ADDRESS& addr, const PeerConnectParam& param )
{
	if ( IsInputSource( addr ) )	// 在输入源列表中，不允许发起连接
	{
		return false;
	}

	return m_connector->Connect( addr, param );
}

bool SourceAgentPeerManager::IsInputSourceIP( UINT ip ) const
{
	for ( InputSourceCollection::const_iterator it = m_InputSources.begin(); it != m_InputSources.end(); ++it )
	{
		if( it->OutterAddress.IP == ip )	// 这里不允许SS跟Publisher使用同一个外网地址
		{
			return true;
		}
	}
	return false;
}

bool SourceAgentPeerManager::IsInputSource( UINT outIP, const PEER_ADDRESS & innerAddress ) const
{
	for ( InputSourceCollection::const_iterator it = m_InputSources.begin(); it != m_InputSources.end(); ++it )
	{
		if( it->OutterAddress.IP == outIP && it->InnerAddress == innerAddress )
		{
			return true;
		}
	}
	return false;
}

bool SourceAgentPeerManager::CanKick( const PeerConnection* conn ) const
{
	return !conn->IsVIP() && !conn->MaybeSource() && 
		( ! IsInputSource(conn->GetSocketAddress().GetIP(), conn->GetHandshakeInfo().Address ) );
}

PeerManager* PeerManagerFactory::PeerCreate(AppModule* owner)
{
	return new ClientPeerManager(owner);
}

PeerManager* PeerManagerFactory::MCCCreate(AppModule* owner)
{
	return new SourcePeerManager(owner);
}

PeerManager* PeerManagerFactory::MDSCreate(AppModule* owner)
{
	return new SourceAgentPeerManager(owner);
}

PeerManager* PeerManagerFactory::SimpleMDSCreate(AppModule* owner)
{
	return new ClientPeerManager(owner);
}




