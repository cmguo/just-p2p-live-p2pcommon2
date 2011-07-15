
#ifndef _LIVE_P2PCOMMON2_NEW_COMMON_LOG_CLIENT_H_
#define _LIVE_P2PCOMMON2_NEW_COMMON_LOG_CLIENT_H_

#include <synacast/live/logwrapper.h>
#include <ppl/data/tstring.h>
#include <ppl/util/time_counter.h>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

class PeerInformation;
class StreamBufferStatistics;
class PeerManager;
class CIPPool;

class LogClient : private boost::noncopyable
{
public:
	explicit LogClient(const tstring& baseDir, const tstring& configDir, boost::shared_ptr<PeerInformation> peerInfo);
	~LogClient();


	void SaveLog(const StreamBufferStatistics& streamStats, PeerManager& peerManager, const CIPPool& ipPool);
	void DoSaveLog(const StreamBufferStatistics& streamStats, PeerManager& peerManager, const CIPPool& ipPool);
	void SendLog();

	bool IsEnabled() const;
private:
	time_counter m_LogDataSaveTime;

	tstring m_ConfigDirectory;
	boost::shared_ptr<PeerInformation> m_PeerInformation;


	LogMarkerWrapper m_Marker;
	bool m_BufferLogSaved;
	bool m_FlowLogSaved;
};

#endif



