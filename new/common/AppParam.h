
#ifndef _LIVE_P2PCOMMON2_NEW_COMMON_APP_PARAM_H_
#define _LIVE_P2PCOMMON2_NEW_COMMON_APP_PARAM_H_

//#include "GloalTypes.h"
#include <synacast/protocol/base.h>
#include <synacast/protocol/data/PeerAddress.h>

#include <boost/shared_ptr.hpp>
#include <set>
#include <vector>


class UDPSender;
struct TRACKER_LOGIN_ADDRESS;

typedef boost::shared_ptr<UDPSender> UDPSenderPtr;


class LiveAppBootConfig
{
public:
	bool IntelligentConnectionControlDisabled;

	LiveAppBootConfig()
	{
		this->IntelligentConnectionControlDisabled = false;
	}
};

typedef std::set<PEER_ADDRESS> PeerAddressCollection;
typedef std::vector<PEER_ADDRESS> PeerAddressArray;

class LiveAppModuleCreateParam
{
public:
	LiveAppModuleCreateParam()
	{
		this->SysInfo = NULL;
		this->AppType = 0;
		FILL_ZERO(this->ChannelGUID);
		this->CookieType = COOKIE_NONE;
		this->CookieValue = 0;
		this->PeerType = NORMAL_PEER;

		this->AppVersion = 0;
		this->AppVersionNumber16 = 0;
	}

	const SYSINFO* SysInfo;
	GUID ChannelGUID;
	int AppType;

	UDPSenderPtr NormalUDPSender;
	UDPSenderPtr TCPProxyUDPSender;
	UDPSenderPtr HTTPProxyUDPSender;

	std::vector<TRACKER_LOGIN_ADDRESS> Trackers;
	PeerAddressCollection VIPs;
	PeerAddressArray MDSs;

	UINT CookieType;
	UINT CookieValue;

	string Username;
	string Password;

	PeerTypeEnum PeerType;

	tstring BaseDirectory;
	tstring ConfigDirectory;

	UINT32 AppVersion;
	UINT16 AppVersionNumber16;

	LiveAppBootConfig BootConfig;

	// Added by Tady, 011811: Spark!
	PeerAddressArray	Sparks;
	tstring				SparkTranstype;
	UINT				SparkLen;
};


class ExtraLiveParam
{
public:
	HWND DHTMessageWindow;

	ExtraLiveParam()
	{
		this->DHTMessageWindow = NULL;
	}
};
#endif