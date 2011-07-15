
#ifndef _LIVE_P2PCOMMON2_NEW_CORE_DOWNLOADER_IMPL_H_
#define _LIVE_P2PCOMMON2_NEW_CORE_DOWNLOADER_IMPL_H_

//#include "PeerManager.h"
#include "Downloader.h"
#include "PieceRequestManager.h"
#include "common/UnfinishedMediaStorage.h"

#include "SubPieceTask.h"
#include "framework/timer.h"
#include <ppl/util/time_counter.h>
#include <boost/shared_ptr.hpp>
#include <set>

typedef std::set<PeerTunnelPtr> PeerTunnelCollection;


class QuotaManager;
class CPeerManager;
class PeerConnection;
class CStreamBuffer;
class Storage;
class PeerStatusInfo;
class SourceResource;

class DownloaderStatistics;

typedef std::set<PeerConnection*> PeerConnectionCollection;


/// �������ݵ�����
class TrivialDownloader : public Downloader
{
public:
	TrivialDownloader() { }
	virtual ~TrivialDownloader() { }

	virtual void AddTunnel( PeerConnection* pc );
	virtual void RemoveTunnel( PeerConnection* pc ) { }

	/// ����
	virtual bool Start(bool hasMDS) { return true; }
	/// ֹͣ
	virtual void Stop() {}

	/// PeerConnection���յ�һ��header pieceʱ����
	//virtual void OnHeaderPieceReceived(PPMediaHeaderPacketPtr packet, PeerConnection* connection) { assert(false); }

	/// PeerConnection���յ�һ��data pieceʱ����
	//virtual void OnDataPieceReceived(PPMediaDataPacketPtr packet, PeerConnection* connection) { assert(false); }
	virtual void OnSubPieceReceived(SubMediaPiecePtr packet, PeerConnection* connection) { assert(false); }

	/// PeerConnection�ڽ���һ��pieceʧ��ʱ����
	virtual void OnPieceReceiveFailed(UINT pieceIndex, PeerConnection* connection) { assert(false); }
	virtual void OnSubPieceReceivedFailed(SubPieceUnit subPiece, PeerConnection* connection) { assert(false); }

	/// ������һ��Pieice
	virtual void RequestNextPiece() { }

	/// �ж�һ��PeerConnection�Ƿ����������������
	virtual bool CanDownload( PeerTunnelPtr tunnel ) const { return false; }

	/// ���piece�Ƿ���Ҫ����
	virtual bool NeedDownload(UINT pieceIndex) const { return false; }
	virtual bool NeedDownload(SubPieceUnit subPiece) const { return false; }

	/// �Ƿ��Ѿ������
	virtual bool IsRequested(UINT piece) const { return false; }

	/// ����ӵ��ָ����piece��Դ
	virtual bool HasResource(UINT piece) const { return false; }

	virtual void AddRequest(UINT pieceIndex, PeerConnection* pc, UINT timeout) { }
	virtual void AddRequest(SubPieceUnit subPiece, PeerConnection* connection, UINT externalTimeout) { }
	virtual size_t RemoveRequest(UINT pieceIndex, PeerConnection* pc) { return 0; }
	virtual size_t RemoveRequests(PeerConnection* pc) { return 0; }
	virtual size_t RemoveSubPieceRequest(SubPieceUnit subPiece, const PeerConnection* connection) { return 0; }

	virtual UINT CalcExternalTimeout(UINT index) const { return 0; }

	virtual int GetRequestCount(UINT pieceIndex) { return 0; }

	virtual void CheckRequestByConnection( PeerConnection* conn ) { }

	virtual SubMediaPiecePtr GetSubPiece(UINT index, USHORT subPieceIndex) const { return SubMediaPiecePtr(); }
	virtual std::pair<bool, MonoMediaDataPiecePtr> AddSubPiece(SubMediaPiecePtr subPiece) { return std::make_pair(false, MonoMediaDataPiecePtr()); }

	//virtual bool HasSubPiece(UINT32 pieceIndex, UINT8 subPieceIndex) const { return false; }

	/// �����Դ�����ֵ
	virtual UINT GetResourceMaxIndex() const { return 0; }

	virtual bool NeedSubPiece( UINT32 pieceIndex, UINT8 subPieceIndex ) const { return false; }

	
};



/// �������ݵ�����
class PeerDownloader : public Downloader
{
public:
	explicit PeerDownloader(CPeerManager& owner, CStreamBuffer& streamBuffer, boost::shared_ptr<PeerInformation> peerInformation);
	virtual ~PeerDownloader();

	virtual void AddTunnel( PeerConnection* pc );
	virtual void RemoveTunnel( PeerConnection* pc );

	/// ����
	bool Start(bool hasMDS);
	/// ֹͣ
	void Stop();

	/// PeerConnection���յ�һ��header pieceʱ����
	void OnHeaderPieceReceived(UnfinishedMediaPiecePtr packet, PeerConnection* connection);

	/// PeerConnection���յ�һ��data pieceʱ����
	void OnDataPieceReceived(UnfinishedMediaPiecePtr packet, PeerConnection* connection);
	void OnSubPieceReceived(SubMediaPiecePtr packet, PeerConnection* connection);

	/// PeerConnection�ڽ���һ��pieceʧ��ʱ����
	void OnPieceReceiveFailed(UINT pieceIndex, PeerConnection* connection);
	void OnSubPieceReceivedFailed(SubPieceUnit subPiece, PeerConnection* connection);

	/// ������һ��Pieice
	void RequestNextPiece();

protected:
	/// �� RequestNextPiece �ֳ�3��С����, ���� RequestNextPiece ̫��
	UINT DoLocate(UINT ResourceMinIndex,UINT ResourceMaxIndex);
	void DoRequestNextPiece();
	void DoPreloacate(UINT ResourceMinIndex,UINT ResourceMaxIndex);

public:
	/// �ж�һ��PeerConnection�Ƿ����������������
	bool CanDownload( PeerTunnelPtr tunnel ) const;

	/// ���piece�Ƿ���Ҫ����
	bool NeedDownload(UINT pieceIndex) const;
	virtual bool NeedDownload(SubPieceUnit subPiece) const;

	/// �Ƿ��Ѿ������
	bool IsRequested(UINT piece) const;
	bool IsRequested(UINT32 piece, UINT8 subPieceIndex) const;

	/// ����ӵ��ָ����piece��Դ
	bool HasResource(UINT piece) const;

	void AddRequest(UINT index, PeerConnection* pc, UINT timeout);
	void AddRequest(SubPieceUnit subPiece, PeerConnection* connection, UINT externalTimeout);

	UINT CalcExternalTimeout(UINT index) const;

	int GetRequestCount(UINT pieceIndex) { return m_requests.GetRequestCount(pieceIndex); }

	virtual size_t RemoveRequest(UINT pieceIndex, PeerConnection* pc)
	{
		return m_requests.Remove(pieceIndex, pc);
	}

	virtual size_t RemoveRequests(PeerConnection* pc)
	{
		return m_requests.Remove(pc);
	}
	virtual size_t RemoveSubPieceRequest(SubPieceUnit subPiece, const PeerConnection* connection)
	{
		return m_requests.RemoveSubPieceRequest(subPiece, connection);
	}

	virtual void CheckRequestByConnection( PeerConnection* conn )
	{
		m_requests.CheckRequestByConnection(conn);
	}
	UnfinishedMediaPiecePtr AddDataSubPiece(SubMediaPiecePtr subPiece);

	virtual bool HasSubPiece(UINT32 pieceIndex, UINT8 subPieceIndex) const;
	UnfinishedMediaPiecePtr AddHeaderSubPiece(SubMediaPiecePtr subPiece);
	//	pair<bool, PPMediaDataPacketPtr> AddDataSubPiece(SubMediaPiecePtr subPiece);

	size_t GetMaxSubPieceCountPerPiece() const;
	size_t GetPossibleSubPieceCount(UINT pieceIndex, UINT skipIndex) const;
	size_t GetSubPieceCount(UINT pieceIndex) const;


	/// ��QuotaManager�����ģ�����
public:
	UINT64 GetLocateElapsed() const { return m_LocateTimeCounter.elapsed(); }

	/// ��ȡ���ݻ�����
	CStreamBuffer& GetStreamBuffer() const { return m_streamBuffer; }

	/// ��ȡ��ʼ������λ��
	UINT GetStartIndex() const;
	
	CPeerManager& GetPeerManager() const { return m_PeerManager;}

protected:
	/// ���piece�����Ƿ���Ч
//	bool CheckPieceIndexValid(UINT pieceIndex) const;

	/// ���ݵ�ǰ��PeerConnection���ӣ��ҵ����ʵ���ʼ����λ��
	UINT FindStartPosition();
	UINT FindGoodStartPosition();
	
	/// �����й����У����ݵ�ǰ��PeerConnection���ӣ��ҵ����ʵ���Դ��λλ��
	UINT FindPreLocationPosition();
	UINT FindGoodPreLocationPosition();

	/// ������ڽ��е����������Ƿ�ʱ
	bool CheckRquestPieceTimeout();

	/// ��������ָ���� SubPiece
	bool TryToDownload(SubPieceUnit subPiece, QuotaManager& quotaManager, int& tryCount, DWORD );

	/// �Ƿ���Դ�����Ľڵ���ӵ��ָ����piece��Դ
	bool HasResourceInGoodPeer(UINT piece) const;


	void GetConnectionStatistics(int& highConnectionCount, int& requestingConnectionCount, int requestTimeoutConnectionCount) const;

	/// �������Դ��MinIndex (���и��ٽڵ��MinIndex����Сֵ)
	UINT GetResourceMinIndex() const;

	/// �������Դ��MaxIndex (���и��ٽڵ��MinIndex����Сֵ)
	UINT GetResourceMaxIndex() const;

	SubMediaPiecePtr GetSubPiece(UINT pieceIndex, UINT8 subPieceIndex) const;

	virtual bool NeedSubPiece( UINT32 pieceIndex, UINT8 subPieceIndex ) const;

protected:
	void OnTimer();

	void Verify()
	{
		assert( m_Connections.size() == m_Tunnels.size() );
	}

protected:
	/// ������PeerManager
	CPeerManager& m_PeerManager;

	CStreamBuffer& m_streamBuffer;
	Storage& m_storage;

	boost::shared_ptr<const PeerStatusInfo> m_StatusInfo;

	/// �����������ݵĶ�ʱ��
	periodic_timer m_RequestTimer;

	/// Peer���ӵļ���
	const PeerConnectionCollection& m_Connections;

	PeerTunnelCollection m_Tunnels;
	// Added by Tady, 011611: Spark! 
	PeerTunnelPtr m_sparkTunnelPtr;
	std::vector<PeerTunnelPtr> m_sparkTunnels;
	UINT m_sparkTick;

	/// �������������PeerConnection����
	//CPeerConnectionQuotaMap m_PeerConnectionQuotaMap;

	/// ��������RequestNextPiece��ʱ��
	UINT m_StartRequestTickCount;

	/// ��һ��Ԥ��λ��ʱ��
	time_counter m_LastPrelocateTickCount;
	time_counter m_LastCalcsPrelocateTickCount;

	/// �Ƿ�ʼ��RequestNextPiece
	bool  m_IsStartRequest;

	/// �ϴμ�鳬ʱ�����ʱ��
	time_counter m_LastRequestCheckTime;

	/// ʹ��ʣ��������ص���piece����
	UINT m_TotalExtraDown;

	time_counter m_LocateTimeCounter;

	HealthyDegreeCollection m_healthyMap;//�����Ⱥͱ��
	HealthyDegreeCollection2 m_healthyMap2;

	/// �������
	PieceRequestManager m_requests;

	DownloaderStatistics& m_Statistics;

	UnfinishedSubPieceStorage m_unfinishedDataPieces;
	/// piece�����sub-piece����
	size_t m_MaxSubPieceCountPerPiece;


	UnfinishedSubPieceStorage m_unfinishedHeaderPieces;

	boost::shared_ptr<const SourceResource> m_SourceResource;
	QuotaManager quotaManager;
};

#endif