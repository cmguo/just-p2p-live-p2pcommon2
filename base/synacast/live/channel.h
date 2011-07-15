
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_LIVE_CHANNEL_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_LIVE_CHANNEL_H_

#include <ppl/data/guid.h>
#include <ppl/os/memory_mapping.h>
#include <synacast/live/url.h>
#include <synacast/live/framework.h>
#include <ppl/data/stlutils.h>
#include <boost/noncopyable.hpp>

#include <synacast/live/startupinfo.h>

typedef vector<TRACKERADDR> TrackerAddressArray;
struct LIVE_INFO;



class LiveChannelController : private boost::noncopyable
{
public:
	LiveChannelController() : m_LiveInfoTagName(TEXT("OS_SYS_META"))
	{

	}

	LiveFrameworkModule& GetFramework() { return m_Framework; }

	void SetLiveInfoTagName(const tstring& tagName)
	{
		assert(!tagName.empty());
		m_LiveInfoTagName = tagName;
	}

	struct LIVE_PARAM_EX : public LIVEPARAM
	{
		///扩展的　Tracker列表大小
		DWORD ExTrackerCount;
		///扩展的　Tracker列表
		TRACKERADDR ExTrackerList[64];
	};

	void CopyPeerAddressesParam(const vector<PEERADDR>& addresses, DWORD& size, PEERADDR dest[], size_t maxCount)
	{
		size = min(addresses.size(), maxCount);
		if (size > 0)
		{
			memcpy(dest, &addresses[0], size);
		}
	}

	void StartChannel(const LiveChannelStartupInfo& startupInfo)
	{
		m_StartupInfo = startupInfo;
		LIVE_PARAM_EX param;
		FILL_ZERO(param);
		param.nSize = sizeof(LIVE_PARAM_EX);
		param.AppType = P2P_LIVE2;
		param.ResourceGuid = startupInfo.ChannelGUID;
		param.Framework_Info.RealAppType = startupInfo.AppType;
		param.Framework_Info.TestVersion = startupInfo.TestVersion;

		const TrackerAddressArray& trackers = startupInfo.Trackers;
		if (trackers.size() > 16)
		{
			param.TrackerCount = 16;
			arrays::copy(trackers.begin(), param.TrackerCount, param.TrackerList);
			param.ExTrackerCount = min(trackers.size() - 16, 64);
			arrays::copy(trackers.begin() + 16, param.ExTrackerCount, param.ExTrackerList);
		}
		else
		{
			param.TrackerCount = static_cast<UINT16>( trackers.size() );
			arrays::copy(trackers.begin(), param.TrackerCount, param.TrackerList);
		}
		CopyPeerAddressesParam(startupInfo.MDSs, param.MDSCount, param.MDSList, 64);
		CopyPeerAddressesParam(startupInfo.VIPs, param.VIPCount, param.VIPList, 64);
		strncpy(param.Username, startupInfo.Username.c_str(), 64);
		strncpy(param.Password, startupInfo.Password.c_str(), 64);
		m_Framework.CreateAppComponent(reinterpret_cast<VARPARAM*>(&param));
	}

	void StopChannel()
	{
		m_Framework.DeleteAppComponent(m_StartupInfo.ChannelGUID);
	}

	SYSINFO* GetSysInfo()
	{
		if (!m_SysInfoMapping.is_mapped() && !this->OpenSysInfo())
			return NULL;
		assert(m_SysInfoMapping.is_mapped());
		return static_cast<SYSINFO*>(m_SysInfoMapping.get_view());
	}
	LIVE_INFO* GetLiveInfo()
	{
		if (!m_LiveInfoMapping.is_mapped() && !this->OpenLiveInfo())
			return NULL;
		assert(m_LiveInfoMapping.is_mapped());
		return static_cast<LIVE_INFO*>(m_LiveInfoMapping.get_view());
	}

protected:
	bool OpenSysInfo()
	{
		if (m_SysInfoMapping.is_mapped())
		{
			assert(false);
			return true;
		}
		tstring sysInfoName = strings::format(TEXT("%dOS_SYS_INFO"), ::GetCurrentProcessId());
		if (m_SysInfoMapping.open(sysInfoName.c_str()))
		{
			m_SysInfoMapping.map_all();
		}
		return m_SysInfoMapping.is_mapped();
	}
	bool OpenLiveInfo()
	{
		if (m_LiveInfoMapping.is_mapped())
		{
			assert(false);
			return true;
		}
		tstring liveInfoName = m_LiveInfoTagName + m_StartupInfo.ChannelGUID.ToString();
		if (m_LiveInfoMapping.open(liveInfoName.c_str()))
		{
			m_LiveInfoMapping.map_all();
		}
		return m_LiveInfoMapping.is_mapped();
	}

private:
	LiveChannelStartupInfo m_StartupInfo;
	LiveFrameworkModule m_Framework;
	ppl::os::memory_mapping m_SysInfoMapping;
	ppl::os::memory_mapping m_LiveInfoMapping;
	tstring m_LiveInfoTagName;
};

#endif


