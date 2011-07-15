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

/// �60sִ��һ��LIST
//const UINT LIST_MAX_INTERVAL = 65*1000;

/// ���1sִ��һ��List
//const UINT LIST_MIN_INTERVAL = 2*1000;


/// ����peer���ӵĳ�ʼ����ʱ��
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
	{	// ����IP ֻ���ٲ��ֵĽڵ�ȥ���ӱ���  ����Ϊ ����������� 1/10, �������ٱ���20�� 
		return max(m_config.WANMinLocalPeerCount, m_PeerModule.GetMaxConnectionCount() * m_config.WANMinLocalPeerPercent / 100);
	}
	else
	{	// ����IP ȫ��ȥ���ӱ���
		return max(m_config.LANMinLocalPeerCount, (UINT)m_PeerModule.GetMaxConnectionCount() );
	}
}

void ClientPeerManager::InitiateConnections(UINT times, UINT maxTCPPendingCount, UINT maxConnectionCount)
{
	// ��ʼ�����ڲ��������ӣ�Ŀ����Ϊ�˵ȴ�IpPool�и���ĵ�ַ��Ĭ���� 0
	if (times < m_config.ConnectorConfig.DelayTimes)
		return;

	// ���Ʒ���ļ����Ĭ����4�Σ�Ҳ����ÿ��1s����һ������
	if (times % m_config.ConnectorConfig.Interval != 0)
		return;

	int prepaDataTime = m_streamBuffer.GetPrepaDataTime();
	bool newGoodQuality = (prepaDataTime > 10 * 1000);
	if ( newGoodQuality != m_statistics.GoodQuality )
	{
		// ���������仯
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
	// ���������Ĭ�����ӳ�ʱΪ10s
	m_ConnectTimeout = m_config.ConnectorConfig.LongConnectionTimeout;
	if (seconds <= 20)
	{
		// ��ʼ10����ʹ�ú̵ܶĳ�ʱ��Ĭ��3s
		m_ConnectTimeout = m_config.ConnectorConfig.ShortConnectionTimeout;
	}
	else if (seconds <= 60)
	{
		// ��ʼ50����ʹ�ý϶̵ĳ�ʱ��Ĭ��5s
		m_ConnectTimeout = m_config.ConnectorConfig.NormalConnectionTimeout;
	}

//	int degreeLeft = GetDegreeLeft();

	VIEW_INFO("Connection "
		<<GetPeerCount()<<" "
		<<m_connector->GetHandshakingPeerCount()<<" "
		<<m_connector->GetConnectingPeerCount()<<" End");
	//2.���ż���Ƿ���Ҫ��������
//	//���������û����������״���ȽϺ������Ӹ�����һ����Ŀ��ʱ�����ֹͣConnectToRemote
//	if ( IsStatusOK() )
//	{	// ���������û� ����䲥������ǳ��ĺ� �����ٷ���������
//		MANAGER_INFO("I'm  have enough PeerCount and Skip = 0 , no need ConnectToRemote.");
//	}
//	else
	{
//  	int maxLocalPeerCount = GetMaxLocalPeerCount();		// ���������������������
// 		int localPeerCount = GetLocalPeerCount();			// �����Ѿ����ϵ���������������

// 		if ( GetDegreeLeft() < 0 &&							// �� ʣ�������� ����
// 			localPeerCount > ( maxLocalPeerCount / 2 )		// ϣ�� ʵ������������ ռ�� �������ֵ ��һ��
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
		// ����Ӧ�÷����������
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
// 			// ��ʼ1���Ӷ���һЩ Ĭ�϶� 3 ��
// 			connectCount += m_config.ConnectorConfig.StartConnectIncrement;
// 		}
		// ÿ�ַ������������ܳ��� MaxConnectionInitiateEveryTime Ĭ��5
		LIMIT_MAX(connectCount, m_config.ConnectorConfig.MaxConnectionInitiateEveryTime);

		bool bIsBusy = false;
		if (m_PeerModule.GetInfo().TransferMethod == TRANSFER_TCP)
		{// ֻ���ڡ���TCPģʽ������Ҫ�� ���� ���������������ơ���Ĭ��ģʽ�£�udp+tcp��������������������udp->tcp�����С�
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
	//1.���ȼ���Ƿ���Ҫ����DoList
	//����List����������IPPool->NeedList()����ô��Ҫ���ڿ�ʼList��List����뵱ǰ�����Ӹ����йء�
	//	NeedDoList Ĭ��������� IPPool����Peer��С��300��ʱ��
	//	IsStatusOK ��ָ (�����ڵ� ����������8�� �� �������ˣ� �������ڵ� ��Զ�� ����IsStatusOK��
	if (m_ipPool.NeedDoList() && !IsStatusOK())
	{
		MANAGER_DEBUG("CPeerManager::AdjustPeerConnection: NeedDoList " << make_tuple(m_LastTickCountDoList.elapsed(), m_DoListInterval));
		if (m_LastTickCountDoList.elapsed() > m_DoListInterval * 1000)
		{
			m_LastTickCountDoList.sync();
			MANAGER_DEBUG("CPeerManager::AdjustPeerConnection: ListPeers");
			m_PeerModule.GetTracker().ListPeers();
			// Ĭ�� MinListInterval=2  MaxListInterval=65
			if (m_DoListInterval < m_config.MinListInterval)
				m_DoListInterval = m_config.MinListInterval;
			if (m_DoListInterval < m_config.MaxListInterval)
				m_DoListInterval *=2;			// ���һ��List���ʱ��� ָ���˱��㷨
												// ���ָ���˱ܲ���List�ɹ�����ʧ�ܶ���ִ��
		}

		// �� ����������5s ���� û�д�UDPTracker��List���κ����ݣ������� TCP Tracker
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

	//3.������Ѿ����ϵ������Ƿ�̫�࣬��Ҫɾ��
	int maxReservedCount = GetMaxReservedDegree();
	if (degreeLeft < maxReservedCount)//degreeLeft����<0��Ĭ���������ʣ��������С�������������1/10��ʱ��,���������ӵĲ���
	{
//		assert(false == m_Connections.empty());//DegreeLeft�ĳ�ʼֵ�ǲ���̫С�ˣ����߼���DegreeLeft�����⣿
		typedef std::multimap<UINT,PeerConnection*> SpeedBasedPeerConnectionIndex;
		SpeedBasedPeerConnectionIndex DelPeerMapBySpeed;
		PeerConnection* pc = NULL;
		//���ݴ����ٶ�����
		for(PeerConnectionCollection::iterator itrOfPCSet = m_Connections.begin(); itrOfPCSet != m_Connections.end(); itrOfPCSet++)
		{
			pc = *itrOfPCSet;
			assert(pc != NULL);
			// ֻ���Լ��������������
			// 2006-09-08 cmp ���Բ����Ƿ���������������ӣ��������߳�
			if (CanKick(pc) && pc->GetConnectionTime() > m_config.InitialPeerProtectionTime * 1000)
			{
				// peer��������һ���߳�����������������ʱ����볬��15��
				const FlowMeasure& flow = pc->GetFlow();

				// ������ ��ʷ���ϴ���������֮��� ���� ��ʱ�� �ó��ģ�Ҳ������ʷƽ�������ٶ�
#if 0                 
				UINT speed = flow.GetAverageRate();// + flow.GetRate();
#else
#pragma message("------ use LongTimeFlow to select slow peer connections")
				UINT speed = pc->GetLongTimeFlow().GetRateMax();  // It's avrg-speed in 60s.
#endif
				DelPeerMapBySpeed.insert(make_pair(speed, pc));
			}
		}
		//ɾ����һ������״̬���õ�Peer��һ���ʵ���ɾ��һЩ
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
		// assert(false == m_Connections.empty());//DegreeLeft�ĳ�ʼֵ�ǲ���̫С�ˣ����߼���DegreeLeft�����⣿
		typedef std::multimap<UINT,PeerConnection*> SpeedBasedPeerConnectionIndex;
		SpeedBasedPeerConnectionIndex DelPeerMapBySpeed;
		PeerConnection* pc = NULL;
		//���ݴ����ٶ�����
		for(PeerConnectionCollection::iterator itrOfPCSet = m_Connections.begin(); itrOfPCSet != m_Connections.end(); itrOfPCSet++)
		{ 
			pc = *itrOfPCSet;
			assert(pc != NULL);
			// ֻ���Լ��������������
			// 2006-09-08 cmp ���Բ����Ƿ���������������ӣ��������߳�
			if (CanKick(pc) && pc->GetConnectionTime() > m_config.InitialPeerProtectionTime * 1000
				&& pc->GetConnectionInfo().ConnectParam.IsSpark == false) // Added by Tady, 011611: Spark!
			{
				// peer��������һ���߳�����������������ʱ����볬��15��
			//	const FlowMeasure& flow = pc->GetFlow();

				// ������ ��ʷ���ϴ���������֮��� ���� ��ʱ�� �ó��ģ�Ҳ������ʷƽ�������ٶ�
#if 0                 
				UINT speed = flow.GetAverageRate();// + flow.GetRate();
#else
//#pragma message("------ use LongTimeFlow to select slow peer connections")
				UINT speed = pc->GetLongTimeFlow().GetRateMax();  // It's avrg-speed in 30s.
#endif
				DelPeerMapBySpeed.insert(make_pair(speed, pc));
			}
		}
		//ɾ����һ������״̬���õ�Peer��һ���ʵ���ɾ��һЩ
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
		// �����ڵ�
		// �������֡������peer������8������״̬����
		return (0 == m_StatusInfo->Status.SkipPercent) && (GetPeerCount() > (int)m_config.MinPeerCountForGoodStatus);
	}

	// �����ڵ�
	// ���TransferMethod������ǡ���̽�⡱ģʽ������Ϊ״̬�����룬��Ҫһֱ��list
	if ( TRANSFER_NO_DETECT == m_PeerModule.GetInfo().TransferMethod )
		return false;

	// �������֡������peer������20������״̬����
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
		// �����tcp����ģʽ��������udp handshake����udp detect
		return false;
	}

// 	int degreeLeft = GetDegreeLeft();
// 	int localPeerCount = m_statistics.Degrees.All.Out;
// 	LIMIT_MAX( localPeerCount, m_Connections.size() );
// 	int maxLocalPeerCount = GetMaxLocalPeerCount();		// �������������
// 	int handshakingPeerCount = m_connector->GetHandshakingPeerCount();
// 	int maxConn = m_PeerModule.GetSysInfo().MaxConnectPendingCount;
// 	LIMIT_MAX( maxConn, 50 );
// 	if ( m_connector->GetTotalPendingPeerCount() > maxConn + 10 )
// 		return false;
// 
// 	// �� ʣ�������� ����
// 	// ���� ʵ������������ ռ�� �������ֵ ��һ��
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
	// ��������ip���
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
		// �������peer�Ѿ��㹻�࣬��ܾ�����peer
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

	// ����qos���������������ɸ�
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
			// �����˱���ʱ�����
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
	// source����Ҫ�������ӣ�Ҳ���������Ͽ�����������κ�ֵ��û��ʲôӰ��
	// �������PeerConnection::HandleHello�м�⵽�ظ�����ʱ���ɽ������ӷ��Ͽ����ӣ����ֵ����������
	// 2006-01-12 cmp server��Ҳ��Ҫ�ṩһ������ķ���
	return isVip ? 2 : 1;
}

bool SourcePeerManager::CanAcceptConnection( UINT ip ) const
{
	// mcc��Ҫ��Ҫ����ͬһIP(��ͬһ����)��peer�ĸ���
	if (ip == 0)
		return false;
	bool isVip = this->IsVIP(ip);
	if (isVip)
	{
		// ����vip��������
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
		//δ���ӵ�ҲҪ��飬��Ҫ����SocketAddress������������ʱ�㱨�����ĵ�ַ�����м��
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
	// ������
	int maxPeerCount = m_PeerModule.GetMaxConnectionCount();
	int externalPeerCount = (int)GetExternalPeerCount();
	if (externalPeerCount < maxPeerCount / 2)
		return;
	// �������peer�������������������һ�룬�ߵ����е�����peer
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
{	// �˺�����ǿ��,��ʹ��������������

	// ���ٶȹ��͵�peer
	UINT lowestSpeed = 1000;
	UINT lowestQos = 1000;
	// ͨ��ƽ����PieceSize�ͼ����ƽ��PieceRate�����Թ��������ʣ�B/s��
	UINT bufferTime = m_storage.GetBufferTime();
	double localSpeed = 1000; // ȱʡ����Сֵ
	if ( bufferTime > 10 * 1000 )
	{
		// ����ʱ�����10��
		 localSpeed = 1.0 * m_storage.GetPieceSize() * m_storage.GetPieceCount() / bufferTime;
	}
	assert( localSpeed == 1000 || (localSpeed > 30 * 1024 && localSpeed < 80 * 1024) );
	// ����ٶ�����Ϊƽ�������ʵ�1/50
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

		// ��鱣��ʱ��
		if (!CanKick(pc) || pc->GetConnectionTime() <= 10 * 1000)
			continue;

		// ������һ���յ����ĵ�ʱ��
		if (pc->GetLastReceivePacketTime() > 20 * 1000)
		{
			KickPeer(pc, PP_KICK_IDLE);
			continue;
		}

		// ����ٶ��Ƿ����Ԥ�ڵ�����ٶ�
		//if (pc->GetFlow().Upload.GetAverageRate() <= lowestSpeed)
		//{
		//	KickPeer(pc, PP_LEAVE_SOURCE_KICKED_TOOL_SLOW_PEER);
		//	continue;
		//}

		// ���qos�Ƿ����Ԥ�ڵ����ֵ
/*		if (pc->GetConnectionTime() > 60 * 1000 && pc->GetMaxQOS() <= lowestQos)
		{
			KickPeer(pc, PP_LEAVE_SOURCE_KICKED_LOW_QOS_PEER);
			continue;
		}*/

		// �����Դ�Ƿ��ص�
		UINT minIndex = pc->GetRealMinIndex();
		UINT maxIndex = pc->GetMaxIndex();

		if (minIndex > 0 && maxIndex > 1 && m_StatusInfo->MinMax.MaxIndex < minIndex) 
		{	// ��Source�� Max С�� Peer �� Min �� ʱ��,����PeerҪ�ض�λ�Ļ����ƺ��ѣ���ֱ���ߵ�
			KickPeer(pc, PP_KICK_NOT_OVERLAPPED);
			continue;
		}

		if (minIndex > 0 && maxIndex > 1 && maxIndex + 100 < m_StatusInfo->MinMax.MinIndex
			&& pc->GetConnectionTime() > 60 * 1000) 
		{	// ��Source�� Min ���� Peer �� Max �� ʱ��, �����Peerһ���ӵ�֮��ȥ�ض�λ
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
	// ��λ: ��
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

	// ת��Ϊ����
	MinServeTime *= 1000;
	MaxServeTime *= 1000;
}

void MdsConfiguration::Save(ini_file& ini)
{
	// ��λ: ��
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
	// ���Saveֻ��Ϊ�˴���һ����ʼ��ini�ļ�
	//m_mdsConfig.Save(ini);
}

bool SourceAgentPeerManager::HasServedTooLongTime(DWORD serveTime) const
{
	return m_mdsConfig.NeedLimitMaxServeTime && serveTime > m_mdsConfig.MaxServeTime; // ����������ʱ��
}

bool SourceAgentPeerManager::NeedContinueServing(DWORD serveTime) const
{
	return serveTime <= m_mdsConfig.MinServeTime; // ��������С����ʱ��
}

int SourceAgentPeerManager::CalcExtraCount() const
{
//	assert(m_ReservedConnectionPercent > 0 && m_ReservedConnectionPercent <= 100);
	int degreeLeft = GetDegreeLeft();

	int maxCount = max(m_PeerModule.GetSysInfo().MaxAppPeerCount, static_cast<WORD>(1)); // Ԥ��MaxAppPeerCountΪ0�����
	int degreeLeftRatio = degreeLeft * 100 / maxCount;

	// Ҫɾ����peer�������˸�����������������ʱ�����ɾ����peer
	int extraCount = maxCount * m_mdsConfig.ReservedDegreeRatio / 100 - degreeLeft;
	(void)degreeLeftRatio;
        MANAGER_INFO("PeerManager:AdjustPeerConnection ExtraCount=" << extraCount 
		<< " (DegreeLeft,DegreeLeftRatio)=" << make_tuple(degreeLeft, degreeLeftRatio));

	return extraCount;
}

//���SourceAgent�ĵ��ڷ���������10%��DegreeLeft�������Ĳ����ߵ�QOSС�ģ�����������
void SourceAgentPeerManager::KickConnections(UINT seconds)
{
	if (seconds % 5 != 0)
		return;

	PeerConnectionQOSCollection qosColl; // ��qos��С�ļ�������qosColl��Ȼ��ɾ����
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
	// ������С����ʱ�䣬��Ҫ���
	UINT qos = pc->GetQOS();
	if (qosColl.size() < maxCount)
	{
		qosColl.insert(make_pair(qos, pc));
		MANAGER_INFO("insert deleting peer " << qos);
		return;
	}

	// maxCount > 0������qosColl.size() > 0
	assert(!qosColl.empty());
	PeerConnectionQOSCollection::iterator iterMinQos = qosColl.end();
	--iterMinQos;
//	PeerConnection* tempConn = iterMinQos->second;
	if (qos < iterMinQos->first)
	{
		// ���pc��qosС��qosColl������qos����ɾ��ԭ��qos���Ľڵ㣬�����ӽ���
		qosColl.erase(iterMinQos);
		qosColl.insert(make_pair(qos, pc));
		MANAGER_INFO("insert deleting peer " << make_tuple(iterMinQos->first, qos));
	}
	// ���pc��qos��С��qosColl������qos�������֮
}

size_t SourceAgentPeerManager::CheckExtraPeers(PeerConnectionQOSCollection& qosColl, size_t extraCount)
{
	PeerConnectionCollection::const_iterator iter = m_Connections.begin();
	while (iter != m_Connections.end())
	{
		PeerConnection* pc = *iter;
		++iter; // ���ƽ�����������Ϊ�������ɾ����pc

		if (!CanKick(pc))
			continue;
		UINT64 serveTime = pc->GetStartTime().elapsed(); // ����ʱ�䣺������(������)�ɹ�������Ϊֹ��ʱ��
		if (HasServedTooLongTime(serveTime))
		{
			// ����������ʱ�䣬ɾ��֮
			DelPeer(pc, PP_LEAVE_KICKED);
			--extraCount; // extraCount��Ҫ�ų�ֱ���ߵ���peer����
			continue;
		}
		if (extraCount <= 0 || NeedContinueServing(serveTime))
			continue; // ���û�ж����peer�����߻�û�г�����С����ʱ�䣬��Ҫ��������������

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
	// Source Agent ���ܴ�Input Source�������ݣ���Input Source��������
	const PEER_ADDRESS & innerAddress = conn->GetHandshakeInfo().Address;
	UINT outterIP = conn->GetSocketAddress().GetIP();
	if ( IsInputSource( outterIP, innerAddress ) )
	{
		return false;
	}

	// mdsֻ��vip peer����������
	return conn->IsVIP();
}

int SourceAgentPeerManager::GetMaxRepeatedConnectionCount(bool isVip) const
{
	// mds��mcc������mds�����ӿ�����2��������ͨpeerֻ����һ������
	return isVip ? 2 : 1;
}

bool SourceAgentPeerManager::NeedResource(const PeerConnection* pc) const
{
	// mdsֻ��Ҫ����vip peer����Դλͼ
	return pc->IsVIP();
}

void SourceAgentPeerManager::LoadInputSources( const PeerAddressArray &addresses )
{
	m_InputSources.clear();
	for( PeerAddressArray::const_iterator pointer = addresses.begin(); pointer != addresses.end(); ++pointer )
	{
		PEER_ADDRESS innerAddress = *pointer;

		PeerAddressArray::const_iterator outterPointer = ++pointer;
		if ( outterPointer == addresses.end() )		// ��������ַ��û�꣬������������ַ���϶����д���
		{
			break;
		}

		PEER_ADDRESS outterAddress = *outterPointer;
		assert( outterAddress.IP );					// �����ַ
		if ( innerAddress.IP == 0 || outterAddress.IP == 0 )
		{
			break;
		}

		m_InputSources.insert( PairedPeerAddress( innerAddress, outterAddress ) );
	}
}

bool SourceAgentPeerManager::ConnectToRemote( const PEER_ADDRESS& addr, const PeerConnectParam& param )
{
	if ( IsInputSource( addr ) )	// ������Դ�б��У�������������
	{
		return false;
	}

	return m_connector->Connect( addr, param );
}

bool SourceAgentPeerManager::IsInputSourceIP( UINT ip ) const
{
	for ( InputSourceCollection::const_iterator it = m_InputSources.begin(); it != m_InputSources.end(); ++it )
	{
		if( it->OutterAddress.IP == ip )	// ���ﲻ����SS��Publisherʹ��ͬһ��������ַ
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




