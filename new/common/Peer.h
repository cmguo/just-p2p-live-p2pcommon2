
#ifndef _LIVE_P2PCOMMON2_NEW_COMMON_PEER_H_
#define _LIVE_P2PCOMMON2_NEW_COMMON_PEER_H_

#include "ClientAppModule.h"
#include "PeerManager.h"
#include "StreamBuffer.h"
#include "common/BaseInfo.h"
#include "pos/TrackerRequester.h"
#include "stdbase.h"
#include "GloalTypes.h"



//#pragma comment(lib, PPL_LIB_FILE("../lib/p2pcore"))
//#pragma comment(lib, PPL_LIB_FILE("../lib/p2pcommon"))



class PeerModule : public ClientLiveAppModule
{
public:
	explicit PeerModule(const LiveAppModuleCreateParam& param) : ClientLiveAppModule(param, TEXT("OS_SYS_META"), false)
	{
	}

protected:
	void DoCreateComponents()
	{
		LIVE_ASSERT(!m_tracker);
		LIVE_ASSERT(!m_streamBuffer);
		LIVE_ASSERT(!m_Manager);
		m_tracker.reset(DoCreateTracker());
		m_streamBuffer.reset(StreamBufferFactory::PeerCreate());
		m_Manager.reset(PeerManagerFactory::PeerCreate(this));
	}
	virtual TrackerRequester* DoCreateTracker()
	{
		if (m_trackerAddressList.size() > 0)
			return TrackerRequesterFactory::PeerCreate();
		return TrackerRequesterFactory::SimpleMDSCreate();
	}
	
	virtual void OnAppTimer()
	{
		ClientLiveAppModule::OnAppTimer();
		AdjustMaxPeerCount2();
	}

	void AdjustMaxPeerCount()
	{

#ifdef _WIN32_WCE
#pragma message("------wince下不启用智能连接")
		m_AutoMaxAppPeerCount = GetSysInfo().MaxAppPeerCount;
		GetInfo().ChannelInfo.AutoMaxAppPeerCount = m_AutoMaxAppPeerCount;
		return;
#endif

		bool intelligentConnectionControlDisabled = GetInfo().IntelligentConnectionControlDisabled;
		if( intelligentConnectionControlDisabled == true )
		{	// 不启用智能连接
			m_AutoMaxAppPeerCount = 0;
			GetInfo().ChannelInfo.AutoMaxAppPeerCount = (WORD)m_AutoMaxAppPeerCount;
			return;
		}

		if( m_AutoMaxAppPeerCount < 30 )
		{	// 智能连接数最小保证为 30, 这是P2P能够正常的根本条件之一
			m_AutoMaxAppPeerCount = 30;
		}


		UINT downloadSpeed = m_Manager->GetMaxExternalDownloadSpeed();
		UINT uploadSpeed = m_Manager->GetMaxExternalUploadSpeed();

		// 调整连接数的几个原则
		// 1. 内网用户连接数少一些，外网用户连接数多一些，不能让内网用户的出度把外网用户的入度占光了
		// 2. ADSL 512K外网用户不能连接数太多，否则会占用太多下载带宽，导致下载反而变差，连接数不应该超过40个
		// 3. 连接数上调主要是为了保证用户启动的时候能够尽快融入网络，不是为了下载数据，下载数据在连接建立很多之后逐步调优
		// 4. 连接数调整的主要依据是上传能力

		if( m_PeerInformation->NetInfo->IsExposedIP() )	// 当 公网 或者 UPnP开启的时候 才启用智能连接
		{
			if ( downloadSpeed < 60 * 1024 )							// 512K ADSL
			{
				if ( uploadSpeed > 15 * 1024 )
				{
					m_AutoMaxAppPeerCount = 40;			// + 1 * 10
				}
				else if ( uploadSpeed > 10 * 1024 )
				{
					m_AutoMaxAppPeerCount = 35;			// + 0.5 * 10
				}
			}
			else if ( downloadSpeed < 100 * 1024 )	// 60KB~100KB 		// 1M ADSL
			{
				// 1M ADSL 最大上传64KB，最多允许80个连接
				if ( uploadSpeed > 65 * 1024 )
				{
					m_AutoMaxAppPeerCount = 80;			// + 5 * 10
				}
				else if ( uploadSpeed > 40 * 1024 )
				{
					m_AutoMaxAppPeerCount = 60;			// + 3 * 10
				}
				else if ( uploadSpeed > 25 * 1024 )
				{
					m_AutoMaxAppPeerCount = 50;			// + 2 * 10
				}
				else if ( uploadSpeed > 15 * 1024 )
				{
					m_AutoMaxAppPeerCount = 40;			// + 1 * 10
				}
				else if ( uploadSpeed > 10 * 1024 )
				{
					m_AutoMaxAppPeerCount = 35;			// + 0.5 * 10
				}
			}
			else														// 2M ADSL or Ethernet
			{
				if( uploadSpeed > 170 * 1024 )
				{
					m_AutoMaxAppPeerCount = 240;		// + 21 * 10
				}
				else
				if ( uploadSpeed > 105 * 1024 )
				{
					m_AutoMaxAppPeerCount = 160;		// + 13 * 10
				}
				else if ( uploadSpeed > 65 * 1024 )
				{
					m_AutoMaxAppPeerCount = 110;		// + 8 * 10
				}
				else if ( uploadSpeed > 40 * 1024 )
				{
					m_AutoMaxAppPeerCount = 80;			// + 5 * 10
				}
				else if ( uploadSpeed > 25 * 1024 )
				{
					m_AutoMaxAppPeerCount = 60;			// + 3 * 10
				}
				else if ( uploadSpeed > 15 * 1024 )
				{
					m_AutoMaxAppPeerCount = 50;			// + 2 * 10
				}
				else if ( uploadSpeed > 10 * 1024 )
				{
					m_AutoMaxAppPeerCount = 40;			// + 1 * 10
				}
				else if ( uploadSpeed > 5 * 1024 )
				{
					m_AutoMaxAppPeerCount = 35;			// + 0.5 * 10
				}
			}
		}
		else	// 当 是内网 用户 的时候
		{
			if (downloadSpeed < 60 * 1024)
			{
				m_AutoMaxAppPeerCount = 30;
			}
			if( uploadSpeed > 170 * 1024 )
			{
				m_AutoMaxAppPeerCount = 100;		// + 7 * 10
			}
			else if ( uploadSpeed > 105 * 1024 )
			{
				m_AutoMaxAppPeerCount = 80;			// + 5 * 10
			}
			else if ( uploadSpeed > 65 * 1024 )
			{
				m_AutoMaxAppPeerCount = 60;			// + 3 * 10
			}
			else if ( uploadSpeed > 40 * 1024 )
			{
				m_AutoMaxAppPeerCount = 50;			// + 2 * 10
			}
			else if ( uploadSpeed > 25 * 1024 )
			{
				m_AutoMaxAppPeerCount = 40;			// + 1 * 10
			}
			else if ( uploadSpeed > 15 * 1024 )
			{
				m_AutoMaxAppPeerCount = 35;			// + 0.5 * 10
			}
		}

		GetInfo().ChannelInfo.AutoMaxAppPeerCount = (WORD)m_AutoMaxAppPeerCount;
	}

	void AdjustMaxPeerCount2()
	{

#ifdef _WIN32_WCE
#pragma message("------wince下不启用智能连接")
		m_AutoMaxAppPeerCount = GetSysInfo().MaxAppPeerCount;
		GetInfo().ChannelInfo.AutoMaxAppPeerCount = m_AutoMaxAppPeerCount;
		return;
#endif

		//UINT maxDownloadSpeed = m_Manager->GetMaxExternalDownloadSpeed();
		UINT maxUploadSpeed = m_Manager->GetMaxExternalUploadSpeed();

		UINT avrgDownloadSpeed = m_Manager->GetAvrgDownloadSpeed();
		UINT avrgUploadSpeed = m_Manager->GetAvrgUploadSpeed();

		m_AutoMaxAppPeerCount = max(maxUploadSpeed, avrgDownloadSpeed) / 2048;

		// 调整连接数的几个原则
		// 1. 内网用户连接数少一些，外网用户连接数多一些，不能让内网用户的出度把外网用户的入度占光了
		// 2. ADSL 512K外网用户不能连接数太多，否则会占用太多下载带宽，导致下载反而变差，连接数不应该超过40个
		// 3. 连接数上调主要是为了保证用户启动的时候能够尽快融入网络，不是为了下载数据，下载数据在连接建立很多之后逐步调优
		// 4. 连接数调整的主要依据是上传能力
// 		UINT minCountFromDownSpeed = avrgDownloadSpeed / 2 / 2048;
// 		UINT minCountFromUpSpeed = avrgUploadSpeed / 2 / 2048;
//  	UINT minCountFromDownSpeed = avrgDownloadSpeed / 4096;
// 		UINT minCountFromUpSpeed = avrgUploadSpeed / 4096;
// 		m_AutoMinAppPeerCount = max(minCountFromDownSpeed, minCountFromUpSpeed);		
		m_AutoMinAppPeerCount = max(avrgDownloadSpeed, avrgUploadSpeed) / 4096;

		GetInfo().ChannelInfo.AutoMaxAppPeerCount = (WORD)m_AutoMaxAppPeerCount;
	}
};

class SuperNodeModule : public ClientLiveAppModule
{
public:
	explicit SuperNodeModule(const LiveAppModuleCreateParam& param) : ClientLiveAppModule(param)
	{
		m_PeerInformation->NetInfo->CoreInfo.PeerType = MAS_PEER;
		LoadMDS(param.MDSs);
	}
	~SuperNodeModule()
	{

	}

	virtual void DoCreateComponents()
	{
		LIVE_ASSERT(!m_tracker);
		LIVE_ASSERT(!m_streamBuffer);
		LIVE_ASSERT(!m_Manager);
		m_tracker.reset(DoCreateTracker());
		m_streamBuffer.reset(StreamBufferFactory::PeerCreate());
		m_Manager.reset(PeerManagerFactory::SimpleMDSCreate(this));
	}

	virtual TrackerRequester* DoCreateTracker()
	{
		return TrackerRequesterFactory::SimpleMDSCreate();
	}

	virtual void OnAppTimer()
	{
		ClientLiveAppModule::OnAppTimer();
		AdjustMaxPeerCount();
	}

	void AdjustMaxPeerCount()
	{

#ifdef _WIN32_WCE
#pragma message("------wince下不启用智能连接")
		m_AutoMaxAppPeerCount = GetSysInfo().MaxAppPeerCount;
		GetInfo().ChannelInfo.AutoMaxAppPeerCount = m_AutoMaxAppPeerCount;
		return;
#endif

		//UINT maxDownloadSpeed = m_Manager->GetMaxExternalDownloadSpeed();
		UINT maxUploadSpeed = m_Manager->GetMaxExternalUploadSpeed();

		UINT avrgDownloadSpeed = m_Manager->GetAvrgDownloadSpeed();
		UINT avrgUploadSpeed = m_Manager->GetAvrgUploadSpeed();

		m_AutoMaxAppPeerCount = max(maxUploadSpeed, avrgDownloadSpeed) / 2048;	
		m_AutoMinAppPeerCount = max(avrgDownloadSpeed, avrgUploadSpeed) / 4096;

		GetInfo().ChannelInfo.AutoMaxAppPeerCount = (WORD)m_AutoMaxAppPeerCount;
	}
};


class SparkSNModule : public SuperNodeModule
{
public:
	explicit SparkSNModule(const LiveAppModuleCreateParam& param) : SuperNodeModule(param)
	{
	}

	virtual TrackerRequester* DoCreateTracker()
	{
		return TrackerRequesterFactory::SparkMDSCreate();
	}

};

#endif
