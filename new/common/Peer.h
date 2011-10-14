
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
#pragma message("------wince�²�������������")
		m_AutoMaxAppPeerCount = GetSysInfo().MaxAppPeerCount;
		GetInfo().ChannelInfo.AutoMaxAppPeerCount = m_AutoMaxAppPeerCount;
		return;
#endif

		bool intelligentConnectionControlDisabled = GetInfo().IntelligentConnectionControlDisabled;
		if( intelligentConnectionControlDisabled == true )
		{	// ��������������
			m_AutoMaxAppPeerCount = 0;
			GetInfo().ChannelInfo.AutoMaxAppPeerCount = (WORD)m_AutoMaxAppPeerCount;
			return;
		}

		if( m_AutoMaxAppPeerCount < 30 )
		{	// ������������С��֤Ϊ 30, ����P2P�ܹ������ĸ�������֮һ
			m_AutoMaxAppPeerCount = 30;
		}


		UINT downloadSpeed = m_Manager->GetMaxExternalDownloadSpeed();
		UINT uploadSpeed = m_Manager->GetMaxExternalUploadSpeed();

		// �����������ļ���ԭ��
		// 1. �����û���������һЩ�������û���������һЩ�������������û��ĳ��Ȱ������û������ռ����
		// 2. ADSL 512K�����û�����������̫�࣬�����ռ��̫�����ش����������ط�������������Ӧ�ó���40��
		// 3. �������ϵ���Ҫ��Ϊ�˱�֤�û�������ʱ���ܹ������������磬����Ϊ���������ݣ��������������ӽ����ܶ�֮���𲽵���
		// 4. ��������������Ҫ�������ϴ�����

		if( m_PeerInformation->NetInfo->IsExposedIP() )	// �� ���� ���� UPnP������ʱ�� ��������������
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
				// 1M ADSL ����ϴ�64KB���������80������
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
		else	// �� ������ �û� ��ʱ��
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
#pragma message("------wince�²�������������")
		m_AutoMaxAppPeerCount = GetSysInfo().MaxAppPeerCount;
		GetInfo().ChannelInfo.AutoMaxAppPeerCount = m_AutoMaxAppPeerCount;
		return;
#endif

		//UINT maxDownloadSpeed = m_Manager->GetMaxExternalDownloadSpeed();
		UINT maxUploadSpeed = m_Manager->GetMaxExternalUploadSpeed();

		UINT avrgDownloadSpeed = m_Manager->GetAvrgDownloadSpeed();
		UINT avrgUploadSpeed = m_Manager->GetAvrgUploadSpeed();

		m_AutoMaxAppPeerCount = max(maxUploadSpeed, avrgDownloadSpeed) / 2048;

		// �����������ļ���ԭ��
		// 1. �����û���������һЩ�������û���������һЩ�������������û��ĳ��Ȱ������û������ռ����
		// 2. ADSL 512K�����û�����������̫�࣬�����ռ��̫�����ش����������ط�������������Ӧ�ó���40��
		// 3. �������ϵ���Ҫ��Ϊ�˱�֤�û�������ʱ���ܹ������������磬����Ϊ���������ݣ��������������ӽ����ܶ�֮���𲽵���
		// 4. ��������������Ҫ�������ϴ�����
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
#pragma message("------wince�²�������������")
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
