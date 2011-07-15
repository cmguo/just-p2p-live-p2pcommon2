
#ifndef _LIVE_P2PCOMMON2_NEW_POS_TRACKER_REQUESTER_LISTENER_H_
#define _LIVE_P2PCOMMON2_NEW_POS_TRACKER_REQUESTER_LISTENER_H_


struct TRACKER_ADDRESS;
struct PEER_MINMAX;
struct CANDIDATE_PEER_INFO;
struct INNER_CANDIDATE_PEER_INFO;

class PacketBase;

class PeerInformation;
class OldUDPPacketSender;



/// �����trackerͨ��ģ���������
class TrackerRequesterListener
{
public:
	virtual ~TrackerRequesterListener() { }

	/// ��ȡ��peer
	virtual void OnPeerListed(const TRACKER_ADDRESS& trackerAddr, UINT8 count, const CANDIDATE_PEER_INFO addrs[], UINT8 lanPeerCount, const INNER_CANDIDATE_PEER_INFO lanPeers[], UINT64 sourceTimeStamp) = 0;

	/// login��tracker��ɣ����ܳɹ���ʧ��
	virtual void OnLoginComplete(const TRACKER_ADDRESS& addr, bool success) { }

	/// ����source��minmax
	virtual bool SaveSourceMinMax(const PEER_MINMAX& minmax) { return false; }

	/// ���汻̽�⵽���ⲿ��ַ
	virtual void SaveDetectedAddress(UINT32 detectedIP, UINT16 detectedUDPPort, const TRACKER_ADDRESS& trackerAddr) = 0;

};

#endif
