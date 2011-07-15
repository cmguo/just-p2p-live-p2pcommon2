
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_LIVE_STARTUPINFO_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_LIVE_STARTUPINFO_H_


#include <ppl/data/guid.h>
#include <vector>
using std::vector;
#include <ppl/data/tstring.h>


class LiveChannelStartupInfo
{
public:
	Guid ChannelGUID;
	vector<TRACKERADDR> Trackers;
	vector<PEERADDR> MDSs;
	vector<PEERADDR> VIPs;
	string Username;
	string Password;

	int AppType;
	int TestVersion;

	LiveChannelStartupInfo() : AppType( 0 ), TestVersion( 0 )
	{

	}
};


#endif