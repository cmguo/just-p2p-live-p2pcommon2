#ifndef _LIVE_P2PCOMMON2_NEW_COMMON_IPPool_H_
#define _LIVE_P2PCOMMON2_NEW_COMMON_IPPool_H_


struct PEER_ADDRESS;
struct CANDIDATE_PEER_INFO;
struct PACKET_PEER_INFO;
class SimpleSocketAddress;

//typedef set<PEER_ADDRESS> PeerAddressCollection;

class PeerItem;

#include "IPInfo.h"
#include <boost/shared_ptr.hpp>
#include <set>

#define BLOCKED_TIMEOUT 100*1000



class IPPoolStatistics;
class IPPoolConfig;

class PeerNetInfo;
class JsonWriter;
class PeerExchangeItem;


/// peer��ַ��
class CIPPool
{
public:
	CIPPool() { }

	virtual ~CIPPool() { }

	virtual void SetConfig(const IPPoolConfig& config) = 0;

	virtual void OnTimer(UINT seconds) = 0;

	/// ���һ����ַ��IPPool�Թ�̽��
	virtual bool AddCandidate(const PEER_ADDRESS& addr, const PEER_CORE_INFO& coreInfo, CandidatePeerTypeEnum FromWhere) = 0;

	/// ���һ����ַ��IPPool�Թ�̽��
	virtual bool AddCandidate(const CANDIDATE_PEER_INFO& addr, CandidatePeerTypeEnum FromWhere) { return false; }


	virtual bool AddInnerCandidate(const INNER_CANDIDATE_PEER_INFO& addr, CandidatePeerTypeEnum FromWhere) = 0;

	/// ���һ����ַ��IPPool�Թ�̽��
	virtual void AddCandidate(size_t count, const CANDIDATE_PEER_INFO addr[], CandidatePeerTypeEnum FromWhere) = 0;

	/// ���ͨ��peer-exchange��ȡ����peers
	virtual void AddExchangedPeers(size_t count, const PeerExchangeItem* peers, CandidatePeerTypeEnum FromWhere) = 0;

	/// ���һ��̽�����
	virtual bool AddDetected(const CANDIDATE_PEER_INFO& pInfo, UINT DetectedRTT, bool isUDP) = 0;

	virtual bool AddEchoDetected(const CANDIDATE_PEER_INFO& pInfo, UINT DetectedRTT, bool isUDP) { return this->AddDetected(pInfo, DetectedRTT, isUDP); }

		
	/// ���һ���������ӵ�
	virtual bool AddConnecting(const PEER_ADDRESS& addr) = 0;

	/// ���һ�����ӳɹ��ĵ�ַ
	virtual bool AddConnected(const PEER_ADDRESS& addr, UINT16 realPort, UINT8 connectFlags, bool isLAN) = 0;

	/// ���һ�����ӳɹ��ĵ�ַ
	virtual bool AddConnectFailed(const PEER_ADDRESS& addr, bool isTCP) = 0;

	/// ���һ�����ӶϿ��˵ĵ�ַ
	virtual bool AddDisconnected(const PEER_ADDRESS& addr, UINT connectionTime, long errcode, UINT downloadSpeed, UINT uploadSpeed, UINT64 downloadBytes, UINT64 uploadBytes) = 0;


	/// ��ȡ��ַ�Թ�̽��
	virtual bool GetForHello(PeerItem& addr) = 0;

	/// ��ȡ��ַ�Թ�̽��
	virtual bool GetForDetect(PeerItem& addr) = 0;

	/// ��ȡ��ַ�Թ�����
	virtual bool GetForConnect(PeerItem& addr) = 0;

	/// ��ȡIPPool�ܵĴ�С
	virtual size_t GetSize() const { return 0; }



	/// ����ⲿIP��ʱ�򣬸����ⲿIP��������IPPool�ڵ�ĵ�����Ϣ
	virtual void SetExternalIP(UINT externalIP) = 0;
	
	/// ���ó�Ϊ ���������ӵ� �ڵ�
	virtual void AddCannotConnectNode( UINT ip, USHORT port ) = 0;

	/// ����IPPool�д治���������Ϣ
	virtual bool FindAddress(const PEER_ADDRESS& addr) = 0;

	/// �Ƿ���Ҫ̽��
	//virtual bool NeedDetect() = 0;

	/// �Ƿ���Ҫ��tracker�ϻ�ȡpeer
	virtual bool NeedDoList() = 0;

//	virtual bool AddConnectedIP(const PEER_ADDRESS& addr, UINT rtt) = 0;

	/// ��ȡͳ������
	virtual const IPPoolStatistics* GetStatistics() const = 0;

	virtual bool DeleteDuplicatedNAT(const SimpleSocketAddress& sockAddr, const PEER_ADDRESS& keyAddr, const PACKET_PEER_INFO& packetPeerInfo) = 0;

	virtual bool SaveLog(JsonWriter& writer, UINT32 detectedIP, UINT32& totalPeercount, UINT32& savedPeercount) const = 0;
};



/// ʲô���鶼������peer��ַ�أ���Ҫ����source��mds��ģ��
class TrivialIPPool : public CIPPool
{
public:
	virtual void SetConfig(const IPPoolConfig& config) { }
	virtual void OnTimer(UINT seconds) { }

	/// ���һ����ַ��IPPool�Թ�̽��
	virtual bool AddCandidate(const PEER_ADDRESS& addr, const PEER_CORE_INFO& coreInfo, CandidatePeerTypeEnum FromWhere) { return false; }

	virtual bool AddInnerCandidate(const INNER_CANDIDATE_PEER_INFO& addr, CandidatePeerTypeEnum FromWhere) { return false; }

	/// ���һ����ַ��IPPool�Թ�̽��
	virtual bool AddCandidate(const CANDIDATE_PEER_INFO& addr, CandidatePeerTypeEnum FromWhere) { return false; }

	/// ���һ����ַ��IPPool�Թ�̽��
	virtual void AddCandidate(size_t count, const CANDIDATE_PEER_INFO addr[], CandidatePeerTypeEnum FromWhere) { }

	virtual void AddExchangedPeers(size_t count, const PeerExchangeItem* peers, CandidatePeerTypeEnum FromWhere) { }

	/// ���һ��̽�����
	virtual bool AddDetected(const CANDIDATE_PEER_INFO& pInfo, UINT DetectedRTT, bool isUDP) { return false; }

	/// ���һ���������ӵ�
	virtual bool AddConnecting(const PEER_ADDRESS& addr) { return false; }

	/// ���һ�����ӳɹ��ĵ�ַ
	virtual bool AddConnected(const PEER_ADDRESS& addr, UINT16 realPort, UINT8 connectFlags, bool isLAN) { return false; }

	/// ���һ�����ӳɹ��ĵ�ַ
	virtual bool AddConnectFailed(const PEER_ADDRESS& addr, bool isTCP) { return false; }

	/// ���һ�����ӶϿ��˵ĵ�ַ
	virtual bool AddDisconnected(const PEER_ADDRESS& addr, UINT connectionTime, long errcode, UINT downloadSpeed, UINT uploadSpeed, UINT64 downloadBytes, UINT64 uploadBytes) { return false; }



	/// ��ȡ��ַ�Թ�̽��
	virtual bool GetForHello(PeerItem& addr) { return false; }

	/// ��ȡ��ַ�Թ�̽��
	virtual bool GetForDetect(PeerItem& addr) { return false; }

	/// ��ȡ��ַ�Թ�����
	virtual bool GetForConnect(PeerItem& addr) { return false; }

	/// ��ȡIPPool�ܵĴ�С
	virtual size_t GetSize() const { return 0; }



	/// ����ⲿIP��ʱ�򣬸����ⲿIP��������IPPool�ڵ�ĵ�����Ϣ
	virtual void SetExternalIP(UINT externalIP) { }
	
	/// ���ó�Ϊ ���������ӵ� �ڵ�
	virtual void AddCannotConnectNode( UINT ip, USHORT port ) { }

	/// ����IPPool�д治���������Ϣ
	virtual bool FindAddress(const PEER_ADDRESS& addr) { return false; }

	/// �Ƿ���Ҫ̽��
	//virtual bool NeedDetect() { return false; }

	/// �Ƿ���Ҫ��tracker�ϻ�ȡpeer
	virtual bool NeedDoList() { return false; }

//	virtual bool AddConnectedIP(const PEER_ADDRESS& addr, UINT rtt) { return false; }

	/// ��ȡͳ������
	virtual const IPPoolStatistics* GetStatistics() const { return NULL; }

	virtual bool DeleteDuplicatedNAT(const SimpleSocketAddress& sockAddr, const PEER_ADDRESS& keyAddr, const PACKET_PEER_INFO& packetPeerInfo) { return false; }

	virtual bool SaveLog(JsonWriter& writer, UINT32 detectedIP, UINT32& totalPeercount, UINT32& savedPeercount) const { return false; }
};



class IPTable;


class IPPoolFactory
{
public:
	/// ����ʲô���鶼������IPPool
	static CIPPool* CreateTrivial()
	{
		return new TrivialIPPool();
	}

//	static CIPPool* PeerCreate(const PEER_ADDRESS& localAddress);
	static CIPPool* PeerCreateNetTyped(boost::shared_ptr<PeerNetInfo> netInfo, const tstring& baseDir, const std::set<PEER_ADDRESS>& mdsPool, IPTable& iptable);
//	static CIPPool* PeerCreate(AppModule* module);
};


#endif
