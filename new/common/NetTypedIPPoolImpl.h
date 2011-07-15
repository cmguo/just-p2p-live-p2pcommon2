
#ifndef _LIVE_P2PCOMMON2_NEW_COMMON_NET_TYPED_IPPOOL_IMPL_H_
#define _LIVE_P2PCOMMON2_NEW_COMMON_NET_TYPED_IPPOOL_IMPL_H_


#include "IpPool.h"
#include "IPPoolStatistics.h"
#include "IPInfo.h"
#include <synacast/protocol/data/NetType.h>

#include <ppl/data/tstring.h>
#include <ppl/util/time_counter.h>

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <map>
#include <set>

struct DetectIndicator;
struct ConnectIndicator;

class NetTypedIPInfo;
class IPTable;
class PeerNetInfo;


inline bool CompareIndicator(UINT x, const PEER_ADDRESS& keyX, UINT y, const PEER_ADDRESS& keyY)
{
	if (x == y)
		return keyX < keyY;
	return x < y;
}


/// �鷺��ص�������
struct HelloIndicator
{
	PEER_ADDRESS Key;
	UINT AddressDistance;
	UINT HelloTimes;

	HelloIndicator(const PEER_ADDRESS& key, UINT _addressDistance, UINT _helloTimes) : AddressDistance(_addressDistance), HelloTimes(_helloTimes)
	{
		this->Key = key;
	}
};

inline bool operator<(const HelloIndicator& x, const HelloIndicator& y)
{
	if (x.HelloTimes == y.HelloTimes)
	{
		//return x.AddressDistance < y.AddressDistance;
		return CompareIndicator(x.AddressDistance, x.Key, y.AddressDistance, y.Key);
	}
	else
	{
		return x.HelloTimes < y.HelloTimes;
	}
}

/// ̽����ص�������
struct DetectIndicator
{
	PEER_ADDRESS Key;
	UINT RTT;

	UINT AddressDistance;

	UINT LastDetectTime;

	UINT ContinuousDetectFailedTimes;

	CandidatePeerTypeEnum FromWhere;

	DetectIndicator( const PEER_ADDRESS& key, UINT _RTT, UINT _AddressDistance, UINT _LastDetectTime, UINT _ContinuousDetectFailedTimes, CandidatePeerTypeEnum _FromWhere )
	{
		this->Key = key;
		RTT = _RTT;
		AddressDistance = _AddressDistance;
		LastDetectTime = _LastDetectTime;
		ContinuousDetectFailedTimes = _ContinuousDetectFailedTimes;
		FromWhere = _FromWhere;
	}

	inline int GetRank() const
	{
//		if ( ContinuousDetectFailedTimes >= 2 )
//		{	
//			// ˵���Ѿ���������̽��ʧ�ܣ�˵�����̽���Լ���,�ʽ���������
//			return 5;
//		}
//		else if( LastDetectTime == 0 )
//		{	// ��û��̽�� ���� �� ��Tracker��õ�)
//			return 20;
//		}
//		else
//		{	// �Ѿ�̽�⡡����̽��ɹ�������ʧ��
//			return 10;
//		}
		return 10;
	}
};

/// �Ƚ�����DetectIndicator�����ڹ���map
inline bool operator<(const DetectIndicator& x, const DetectIndicator& y)
{
//	if( x.GetRank() != y.GetRank() )
//	{	
//		return x.GetRank() > y.GetRank();
//	}
//	
//	int rank = x.GetRank();
//	if( rank == 40 )
//	{	// ̽��ɹ��� RTT<200ms  ����RTT ��С��������
//		return x.RTT < y.RTT;
//	}
//	else if( rank == 30 || rank == 20 )
//	{	// ��û��̽�� ����AD*1000 + Rand(1000) �ش�С����
//		return x.AddressDistance > y.AddressDistance;
//	}
//	else if( rank == 10 || rank == 5 )
//	{	// �Ѿ�̽�⡡(����̽��ɹ�������ʧ��) ������һ�η��� Detectʱ����
//		return x.LastDetectTime < y.LastDetectTime;
//	}
//	assert(0);
//	return true;
	//return x.LastDetectTime < y.LastDetectTime;
	return CompareIndicator(x.LastDetectTime, x.Key, y.LastDetectTime, y.Key);
}


/// ������ص�������
struct ConnectIndicator
{
	PEER_ADDRESS Key;
	UINT RTT;

	UINT AddressDistance;

	UINT ConnectRank;

	UINT Speed;

	DWORD LastConnectTime;

	CandidatePeerTypeEnum FromWhere;

	ConnectIndicator(const PEER_ADDRESS& key, UINT _RTT, UINT _AddressDistance, UINT _ConnectRank, UINT _DownloadSeped, UINT _UploadSpeed, DWORD _LastConnectTime, CandidatePeerTypeEnum _FromWhere)
	{
		this->Key = key;
		RTT = _RTT;
		AddressDistance = _AddressDistance;
		ConnectRank = _ConnectRank;
		Speed = _DownloadSeped + _UploadSpeed;
		LastConnectTime = _LastConnectTime;
		FromWhere = _FromWhere;
	}
};

/// �Ƚ�����ConnectIndicator�����ڹ���map
inline bool operator<(const ConnectIndicator& x, const ConnectIndicator& y)
{
	if( x.ConnectRank != y.ConnectRank )
	{	
		//return x.ConnectRank > y.ConnectRank;
		return CompareIndicator(y.ConnectRank, y.Key, x.ConnectRank, x.Key);
	}

	UINT ConnectRank = x.ConnectRank;
	if( ConnectRank == 90 )
	{	// �ܹ�̽��ɹ��Ľڵ�
		assert( x.RTT > 0 && y.RTT > 0);
		//return x.RTT < y.RTT;
		return CompareIndicator(x.RTT, x.Key, y.RTT, y.Key);
	}
	else if( ConnectRank == 50 || ConnectRank == 20 )
	{	// ����̽��ɹ� ���� ̽���ʱ��̫��
		//return x.AddressDistance > y.AddressDistance;
		return CompareIndicator(y.AddressDistance, y.Key, x.AddressDistance, x.Key);
	}
	else if( ConnectRank == 100 || ConnectRank == 60 || ConnectRank == 40 )
	{	// ���Ѿ��Ͽ��������з������ӵ�ʱ���ص�ο� ��һ�εĳɹ����ӵ� �ϴ��ٶ� �� �����ٶȣ������ο� RTT��AD
		//return x.Speed > y.Speed;
		return CompareIndicator(y.Speed, y.Key, x.Speed, x.Key);
	}
	else if( ConnectRank == 30 || ConnectRank == 10 || ConnectRank == 0)
	{	// iii.	���������ܱ����ӵĽڵ� CR=10 ����LastConnectTime ��С��������
		//return x.LastConnectTime < y.LastConnectTime;
		return CompareIndicator(x.LastConnectTime, x.Key, y.LastConnectTime, y.Key);
	}
	assert(0);
	//return true;
	return x.Key < y.Key;
}

/// ��Ծ��ص�������
struct ActiveIndicator
{
	PEER_ADDRESS Key;
	UINT ActiveTime;

	ActiveIndicator(const PEER_ADDRESS& key, UINT _RTT, UINT _LastActiveTime)
	{
		this->Key = key;
		if( _RTT == 0 || _RTT > 10*60*1000 )
		{
			ActiveTime = _LastActiveTime + 10*60*1000;
		}
		else
		{
			ActiveTime = _LastActiveTime + 20*60*1000 - _RTT*60;
		}
	}

};

inline bool operator<(const ActiveIndicator& x, const ActiveIndicator& y)
{
	//return x.ActiveTime < y.ActiveTime;
	return CompareIndicator(x.ActiveTime, x.Key, y.ActiveTime, y.Key);
}

/// peer��ַ��Ϣ��
class NetTypedIPInfo : public CIPInfo
{
public:
	UINT64 TotalDownloadBytes;
	UINT64 TotalUploadBytes;

	NET_TYPE NetworkType;

	/// ����ֵ
	UINT AddressDistance;

	/// ����ʱ�����ȼ���
	UINT ConnectRank;

	/// ��һ�λ�Ծʱ��
	UINT LastActiveTime;

	/// ��һ��GetForConnetct��ʱ��
	UINT LastGetForConnectTime;

	/// ����̽��ʧ�ܵĴ���
	UINT ContinuousDetectFailedTimes;

	/// ��һ�����ӹ����е������ٶ�
	UINT DownloadSpeed;

	/// ��һ�����ӹ����е��ϴ��ٶ�
	UINT UploadSpeed;

	/// �鷺�Ĵ���
	UINT HelloTimes;

	/// ��һ�κ鷺��ʱ��
	UINT LastHelloTime;

	// ��һ��IP��ʲô�ط���õģ�1-��Tracker��� 2-��UdpDetect��� 3-��TcpDetect���
	CandidatePeerTypeEnum FromWhere;

	/// �Ƿ�����̽����
	bool IsDetecting;

	/// �Ƿ�����������
	bool IsConnection;

	/// �Ƿ��Ǿ���������
	bool IsLAN;

	explicit NetTypedIPInfo(const PEER_ADDRESS& addr);
	//NetTypedIPInfo(const CANDIDATE_PEER_INFO& info);

	/// ת��Ϊ�ַ���
	string ToString() const
	{
		char str[1025] = { 0 };
		_snprintf(str, 1024, 
			" %u %u %u %u %u %u %d %d %d", 
			this->AddressDistance, this->ConnectRank, this->LastActiveTime, this->ContinuousDetectFailedTimes, 
			this->DownloadSpeed, this->UploadSpeed, 
			this->IsDetecting, this->IsConnection, this->FromWhere);
		return CIPInfo::ToString() + str;
	}


	void CalcAddressDistance(const NET_TYPE& localNetType, IPTable& iptable);


	DetectIndicator GetDetectIndicator() const { return DetectIndicator(this->GetAddress(), RTT,AddressDistance,LastDetectTime,ContinuousDetectFailedTimes,FromWhere); }
	ConnectIndicator GetConnectIndicator() const { return ConnectIndicator(this->GetAddress(), RTT,AddressDistance,ConnectRank,DownloadSpeed,UploadSpeed,ConnectRank,FromWhere); }
	ActiveIndicator GetActiveIndicator() const { return ActiveIndicator(this->GetAddress(), RTT, LastActiveTime); }
	HelloIndicator GetHelloIndicator() const { return HelloIndicator(this->GetAddress(), this->AddressDistance, this->HelloTimes); }

	DetectIndicator GetIndicator(DetectIndicator*) const { return GetDetectIndicator(); }
	ConnectIndicator GetIndicator(ConnectIndicator*) const { return GetConnectIndicator(); }
	ActiveIndicator GetIndicator(ActiveIndicator*) const { return GetActiveIndicator(); }
	HelloIndicator GetIndicator(HelloIndicator*) const { return GetHelloIndicator(); }


private:
	///�����Ϣ
	void Clear();
};


typedef boost::shared_ptr<NetTypedIPInfo> NetTypedIPInfoPtr;

/// ��peer��ַΪkey��peer��Ϣ����
typedef std::map<PEER_ADDRESS, NetTypedIPInfoPtr> NetTypedPeerInfoCollection;

/// ��tcp������ϢΪkey��Peer��Ϣ����
typedef std::set<SimpleSocketAddress> TcpAddressCollection;

/// peer��ַ�ļ���
//typedef set<PEER_ADDRESS> PeerAddressCollection;

/// peer��ַ�ļ���
//typedef set<PEER_ADDRESS> PeerAddressCollection;


typedef std::map<DetectIndicator, NetTypedIPInfoPtr> DetectIndex;
typedef std::map<ConnectIndicator, NetTypedIPInfoPtr> ConnectIndex;
typedef std::map<ActiveIndicator, NetTypedIPInfoPtr> ActiveIndex;
typedef std::map<HelloIndicator, NetTypedIPInfoPtr> HelloIndex;



/// ������IpPool  �����������е�Pool
class NetTypedIPPoolImpl : public CIPPool, private boost::noncopyable
{
public:
	explicit NetTypedIPPoolImpl(boost::shared_ptr<const PeerNetInfo> netInfo, const tstring& baseDir, const std::set<PEER_ADDRESS>& mdsPool, IPTable& iptable);
	~NetTypedIPPoolImpl();

	virtual void SetConfig(const IPPoolConfig& config) { m_config = config; }

	virtual void SetNoUDP(const PEER_ADDRESS& addr);

	// OnNew �������CandidatePeer
	bool AddCandidate(const PEER_ADDRESS& addr, const PEER_CORE_INFO& coreInfo, CandidatePeerTypeEnum FromWhere);
	bool AddCandidate(const CANDIDATE_PEER_INFO& addr, CandidatePeerTypeEnum FromWhere);
	void AddCandidate(size_t count, const CANDIDATE_PEER_INFO addrs[], CandidatePeerTypeEnum FromWhere);

	virtual void AddExchangedPeers(size_t count, const PeerExchangeItem* peers, CandidatePeerTypeEnum FromWhere);

	virtual bool AddInnerCandidate(const INNER_CANDIDATE_PEER_INFO& addr, CandidatePeerTypeEnum FromWhere);

	// OnUdpDetect	
	bool OnUdpDetect(const NetTypedIPInfoPtr& ipInfo);		// ������̽���ʱ�� ���õ�
	bool OnUdpHello(const NetTypedIPInfoPtr& ipInfo);		// ������鷶��ʱ�� ���õ�

	//	���鷶�յ�RTT��ʱ�� ���õ� 
	virtual bool AddEchoDetected(const CANDIDATE_PEER_INFO& pInfo, UINT DetectedRTT, bool isUDP);

	// OnUdpDetectSucced  ��̽���յ�RTT��ʱ�� ���õ�
	bool AddDetected(const CANDIDATE_PEER_INFO& pInfo,UINT DetectedRTT, bool isUDP);

	// OnConnect ���������ӵ�ʱ�� ���õ�
	bool AddConnecting(const PEER_ADDRESS& addr);

	// OnConnectSucced	�����ӳɹ���ʱ�� ���õ�
	bool AddConnected(const PEER_ADDRESS& addr, UINT16 realPort, UINT8 connectFlags, bool isLAN);

	// OnConnectFailed	������ʧ�ܵ�ʱ�� ���õ�
	bool AddConnectFailed(const PEER_ADDRESS& addr, bool isTCP);
	
	// OnDisConnect		���Ͽ����ӵ�ʱ�� ���õ�
	bool AddDisconnected(const PEER_ADDRESS& addr, UINT connectionTime, long errcode, UINT downloadSpeed, UINT uploadSpeed, UINT64 downloadBytes, UINT64 uploadBytes);

	// �����Ҫ����̽��� CandidatePeer 
	bool GetForDetect(PeerItem& addr);

	// �����Ҫ�����鷶�� CandidatePeer
	virtual bool GetForHello(PeerItem& addr);

	// �����Ҫ�������ӵ� CandidatePeer
	bool GetForConnect(PeerItem& addr);

	bool NeedDoList();

	virtual void SetExternalIP(UINT externalIP);

	size_t GetSize() const;

	/// ���ó�Ϊ ���������ӵ� �ڵ�
	virtual void AddCannotConnectNode( UINT ip, USHORT port );

	/// ����IPPool�д治���������Ϣ
	virtual bool FindAddress(const PEER_ADDRESS& addr);

	/// ���IPPool
	void Clear();

	void AddIndex(NetTypedIPInfoPtr ipInfo);
	void DeleteIndex(NetTypedIPInfoPtr ipInfo);

	virtual bool DeleteDuplicatedNAT(const SimpleSocketAddress& sockAddr, const PEER_ADDRESS& keyAddr, const PACKET_PEER_INFO& packetPeerInfo);

	virtual bool SaveLog(JsonWriter& writer, UINT32 detectedIP, UINT32& totalPeercount, UINT32& savedPeercount) const;

protected:
	/// ���ݵ�ַ����ip��Ϣ��
	NetTypedIPInfoPtr Find(const PEER_ADDRESS& PeerAddr);

	/// ��ͳ�����ݸ��µ������ڴ�
	void UpdateIPPoolInfo();

	/// ����ַ�Ƿ���Ч
	bool IsValidAddress(const PEER_ADDRESS& addr) const;

	/// ��ȡһ��Peer��Ϣ������Ӧ��ַ������ڣ��򴴽�һ���µ�
	NetTypedIPInfoPtr GetPeerInfo(const PEER_ADDRESS& addr);

	/// ��ȡ�ӿ�ʼ�����ڵ�ʱ��(NetTypedIPInfo�е�ʱ�������ʹ������ʱ�������GetTickCount���Լ�����ΪTickCount�������������)
	DWORD GetTimeCount() const;

	/// ����Ӹ���ʱ�䵽���ڵ�ʱ���
	DWORD CalcTimeSpan(DWORD startingTime) const;

	/// ����̽��Ĵ��¼��ʱ��
	UINT CalcDetectInterval() const;

	/// ɾ����ַ
	void DeleteItem(NetTypedIPInfoPtr ipInfo);

	virtual const IPPoolStatistics* GetStatistics() const { return &m_statistics; }

	virtual void OnTimer(UINT seconds);

	void OutputDebugData(UINT seconds);

private:
	/// Detect ����
	DetectIndex m_DetectIndex;				// ����̽�������
	ConnectIndex m_ConnectIndex;			// �������ӵ�����
	ActiveIndex m_ActiveIndex;				// ����ɾ��������
	HelloIndex m_HelloIndex;				// ���ں鷶������

	/// ͳ������
	IPPoolStatistics m_statistics;

	/// �ܵ�peer��Ϣ���
	NetTypedPeerInfoCollection m_peers;

	/// ���Ӻ�����,��������Ľڵ�,���������ٱ�����
	TcpAddressCollection m_CannotConnectPeers;

	/// mds��peer��ַ����
	const std::set<PEER_ADDRESS>& m_mdsPool;


	boost::shared_ptr<const PeerNetInfo> m_NetInfo;
	/// ����peer��ַ
	const PEER_ADDRESS& m_localAddress;


	UINT32 m_externalIP;

	tstring m_BaseDirectory;

	/// ������ʱ�䣬��NetTypedIPInfo�м�¼�ļ���ʱ��ֵ�Ļ�׼��
	time_counter m_StartTime;

	NET_TYPE m_localNetType;

	IPTable& m_iptable;

	IPPoolConfig m_config;

};

#endif
