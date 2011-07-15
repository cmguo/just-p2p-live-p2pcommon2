
#ifndef _LIVE_P2PCOMMON2_NEW_POS_TRACKER_REQUESTER_LISTENER_H_
#define _LIVE_P2PCOMMON2_NEW_POS_TRACKER_REQUESTER_LISTENER_H_


struct TRACKER_ADDRESS;
struct PEER_MINMAX;
struct CANDIDATE_PEER_INFO;
struct INNER_CANDIDATE_PEER_INFO;

class PacketBase;

class PeerInformation;
class OldUDPPacketSender;



/// 负责跟tracker通信模块的侦听器
class TrackerRequesterListener
{
public:
	virtual ~TrackerRequesterListener() { }

	/// 获取到peer
	virtual void OnPeerListed(const TRACKER_ADDRESS& trackerAddr, UINT8 count, const CANDIDATE_PEER_INFO addrs[], UINT8 lanPeerCount, const INNER_CANDIDATE_PEER_INFO lanPeers[], UINT64 sourceTimeStamp) = 0;

	/// login到tracker完成，可能成功或失败
	virtual void OnLoginComplete(const TRACKER_ADDRESS& addr, bool success) { }

	/// 保存source的minmax
	virtual bool SaveSourceMinMax(const PEER_MINMAX& minmax) { return false; }

	/// 保存被探测到的外部地址
	virtual void SaveDetectedAddress(UINT32 detectedIP, UINT16 detectedUDPPort, const TRACKER_ADDRESS& trackerAddr) = 0;

};

#endif
