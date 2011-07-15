
#ifndef _LIVE_P2PCOMMON2_NEW_CORE_PEER_MANAGER_FACTORY_H_
#define _LIVE_P2PCOMMON2_NEW_CORE_PEER_MANAGER_FACTORY_H_

#include "PeerManagerImpl.h"
#include <synacast/protocol/data/PeerAddress.h>
#include <map>
#include <set>

class Downloader;
class PeerConnector;





/// �ͻ����õ�peer������
class ClientPeerManager : public CPeerManager
{
public:
	explicit ClientPeerManager(AppModule* lpPeerModule);
	virtual ~ClientPeerManager();

	virtual bool ConnectForDetect(const PeerItem& peer);

	virtual void OnPlayerBufferring( bool isBufferOK );

protected:
	virtual bool DoStart();

	virtual void InitiateConnections(UINT times, UINT maxTCPPendingCount, UINT maxConnectionCount);
	virtual void ListPeers();
	virtual void KickConnections(UINT seconds);
			void KickConnections2(UINT seconds);
	virtual void CheckMDS();

	/// peer����״̬�Ƿ���Ч
	bool IsStatusOK() const;

	/// ���Ӿ������ڵ�peer
	void DialLAN();


	/// �Ƿ���Ҫ����MDS
	bool NeedMDSSupport() const;

	virtual int GetMaxLocalPeerCount() const;


	virtual void CalcMaxPFPSBandWidth(UINT seconds);


	virtual void DoAddPeer(PeerConnection* conn);

private:

	/// �ϴ�List������ʱ��
	time_counter m_LastTickCountDoList;
	time_counter m_startTime;

	/// List�����ļ��ʱ��
	UINT m_DoListInterval;
	
	// max data time. Added by Tady, 091108
	UINT m_maxPrepaDataTime;
	time_counter m_lastTickDoConnect;
	bool m_bNeedConnectForDetect;
};






/// ����Source�˵�PeerManager
class SourcePeerManager : public CPeerManager
{
public:
	SourcePeerManager(AppModule* owner) : CPeerManager(owner)
	{
	}

	/// source����Ҫ�ӱ�ĵط���������
	virtual bool NeedData() const { return false; }

	/// sourceҲ����Ҫ����peer��minmax��Ϣ
	virtual bool NeedResource(const PeerConnection* pc) const { return false; }

	virtual bool ConnectForDetect(const PeerItem& peer)
	{
		return false;
	}

protected:
	virtual bool CanDownload(const PeerConnection* conn) const;

	virtual bool CanAcceptConnection( UINT ip ) const;

	/// source��Ҫ�������announce
	virtual UINT GetAnnounceInterval() const { return 1; }

	/// source��Ҫ��ͬ�������ӵĲ���
	virtual void KickConnections(UINT seconds);

	/// ����ʵ�ʵ�������
	void DoKickConnections(UINT seconds);

	virtual int GetMaxRepeatedConnectionCount(bool isVip) const;

	/// ��ȡ����peer����
	size_t GetExternalPeerCount() const;

	virtual UINT32 CheckAcceptHandshake(bool isVIP, UINT32 externalIP, const PeerHandshakeInfo& handshakeInfo) const;

	virtual void LoadConfiguration();

	virtual void CheckMDS()
	{
		// source���������������ӵĹ��ܣ���ʵ�ִ��������������Ĺ���
		DialMDS();
	}

private:
	/// ������peer
	void KickInternalPeers();

	/// �߳����쳣�����peer
	void KickBadPeers();

};








/// ����mds������������Ϣ
class MdsConfiguration
{
public:
	MdsConfiguration();


	void Load(ini_file& ini);
	void Save(ini_file& ini);

public:
	/// ��С����ʱ��
	UINT	MinServeTime;
	/// ������ʱ��
	UINT	MaxServeTime;
	/// ���������ӱ���
	UINT	ReservedDegreeRatio;
	/// �Ƿ���Ҫ����������ʱ��
	bool	NeedLimitMaxServeTime;
};



typedef std::multimap<UINT, PeerConnection*> PeerConnectionQOSCollection;

///! ��������ַ��Ϣ
struct PairedPeerAddress
{
	PairedPeerAddress(PEER_ADDRESS innerAddress, PEER_ADDRESS outterAddress) :
		InnerAddress( innerAddress ), OutterAddress( outterAddress )
		{
		}

	PEER_ADDRESS InnerAddress;
	PEER_ADDRESS OutterAddress;
};

inline bool operator<( const PairedPeerAddress & address1, const PairedPeerAddress address2 )
{
	if ( address1.OutterAddress != address2.OutterAddress )
	{
		return address1.OutterAddress < address2.OutterAddress;
	}

	return address1.InnerAddress < address2.InnerAddress;
}


typedef std::set<PairedPeerAddress> InputSourceCollection;

/// ����SourceAgent�˵�PeerManager
class SourceAgentPeerManager : public ClientPeerManager
{
public:
	SourceAgentPeerManager(AppModule* owner);

	/// mds��Ҫ�ӱ�ĵط���������
	virtual bool NeedData() const { return true; }
	virtual bool NeedResource(const PeerConnection* pc) const;

	virtual void LoadConfiguration();

	/// ��������Դ��ַ��Ϣ��MDS�����У�0��ǰ�ľ���Publisher��ַ����ַ�ɶԣ�������ǰ�������ں�
	virtual void LoadInputSources( const PeerAddressArray &addresses );

	/// ���˵�����Դ������
	virtual bool ConnectToRemote(const PEER_ADDRESS& addr, const PeerConnectParam& param);
	/// ������Kick����Դ
	virtual bool CanKick(const PeerConnection* conn) const;

	virtual bool IsInputSourceIP( UINT ip ) const;

	virtual bool ConnectForDetect(const PeerItem& peer)
	{
		return false;
	}

protected:
	virtual void KickConnections(UINT seconds);
	virtual UINT GetAnnounceInterval() const { return 1; }
	virtual void CheckMDS();

	virtual void InitiateConnections(UINT times) { }
	virtual void ListPeers() { }

	virtual bool CanDownload(const PeerConnection* conn) const;
	virtual int GetMaxRepeatedConnectionCount(bool isVip) const;

	/// ������е�peer���ߵ�����������ʱ���peer����������peer����qos������
	size_t CheckExtraPeers(PeerConnectionQOSCollection& qosColl, size_t extraCount);
	/// ���PeerConnectionQOSCollection�����maxCount��peer
	size_t DeleteExtraPeers(const PeerConnectionQOSCollection& qosColl, size_t maxCount);
	/// ���һ�������peer��PeerConnectionQOSCollection��
	void AddExtraPeer(PeerConnectionQOSCollection& qosColl, PeerConnection* pc, size_t maxCount);

	/// �Ƿ��Ѿ��ṩ�˹���ʱ��ķ���(����������ʱ��)
	bool HasServedTooLongTime(DWORD serveTime) const;
	/// �Ƿ���Ҫ��������(С����С����ʱ��)
	bool NeedContinueServing(DWORD serveTime) const;

	/// �Ƿ���Ե�������
	bool CanAdjustConnection();

	/// ������Ҫɾ����peer����
	int CalcExtraCount() const;

	virtual bool CanAcceptConnection( UINT ip ) const { return true; }

	/// mds��ص�����
	MdsConfiguration m_mdsConfig;

	/// ����Դ�������������ӣ����������������󣬵���Ӧ�÷���Announce�����Է���Error Leave
	InputSourceCollection m_InputSources;

private:
	bool IsInputSource( const PEER_ADDRESS & outterAddress ) const { return this->IsInputSourceIP(outterAddress.IP); };
	bool IsInputSource( UINT outIP, const PEER_ADDRESS & innerAddress ) const;

};

#endif