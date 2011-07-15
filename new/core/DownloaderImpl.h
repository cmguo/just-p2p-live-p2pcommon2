
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


/// 负责数据的下载
class TrivialDownloader : public Downloader
{
public:
	TrivialDownloader() { }
	virtual ~TrivialDownloader() { }

	virtual void AddTunnel( PeerConnection* pc );
	virtual void RemoveTunnel( PeerConnection* pc ) { }

	/// 启动
	virtual bool Start(bool hasMDS) { return true; }
	/// 停止
	virtual void Stop() {}

	/// PeerConnection在收到一个header piece时调用
	//virtual void OnHeaderPieceReceived(PPMediaHeaderPacketPtr packet, PeerConnection* connection) { assert(false); }

	/// PeerConnection在收到一个data piece时调用
	//virtual void OnDataPieceReceived(PPMediaDataPacketPtr packet, PeerConnection* connection) { assert(false); }
	virtual void OnSubPieceReceived(SubMediaPiecePtr packet, PeerConnection* connection) { assert(false); }

	/// PeerConnection在接收一个piece失败时调用
	virtual void OnPieceReceiveFailed(UINT pieceIndex, PeerConnection* connection) { assert(false); }
	virtual void OnSubPieceReceivedFailed(SubPieceUnit subPiece, PeerConnection* connection) { assert(false); }

	/// 请求下一个Pieice
	virtual void RequestNextPiece() { }

	/// 判断一个PeerConnection是否可以用来下载数据
	virtual bool CanDownload( PeerTunnelPtr tunnel ) const { return false; }

	/// 检查piece是否需要下载
	virtual bool NeedDownload(UINT pieceIndex) const { return false; }
	virtual bool NeedDownload(SubPieceUnit subPiece) const { return false; }

	/// 是否已经请求过
	virtual bool IsRequested(UINT piece) const { return false; }

	/// 是有拥有指定的piece资源
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

	/// 获得资源的最大值
	virtual UINT GetResourceMaxIndex() const { return 0; }

	virtual bool NeedSubPiece( UINT32 pieceIndex, UINT8 subPieceIndex ) const { return false; }

	
};



/// 负责数据的下载
class PeerDownloader : public Downloader
{
public:
	explicit PeerDownloader(CPeerManager& owner, CStreamBuffer& streamBuffer, boost::shared_ptr<PeerInformation> peerInformation);
	virtual ~PeerDownloader();

	virtual void AddTunnel( PeerConnection* pc );
	virtual void RemoveTunnel( PeerConnection* pc );

	/// 启动
	bool Start(bool hasMDS);
	/// 停止
	void Stop();

	/// PeerConnection在收到一个header piece时调用
	void OnHeaderPieceReceived(UnfinishedMediaPiecePtr packet, PeerConnection* connection);

	/// PeerConnection在收到一个data piece时调用
	void OnDataPieceReceived(UnfinishedMediaPiecePtr packet, PeerConnection* connection);
	void OnSubPieceReceived(SubMediaPiecePtr packet, PeerConnection* connection);

	/// PeerConnection在接收一个piece失败时调用
	void OnPieceReceiveFailed(UINT pieceIndex, PeerConnection* connection);
	void OnSubPieceReceivedFailed(SubPieceUnit subPiece, PeerConnection* connection);

	/// 请求下一个Pieice
	void RequestNextPiece();

protected:
	/// 将 RequestNextPiece 分成3个小函数, 以免 RequestNextPiece 太大
	UINT DoLocate(UINT ResourceMinIndex,UINT ResourceMaxIndex);
	void DoRequestNextPiece();
	void DoPreloacate(UINT ResourceMinIndex,UINT ResourceMaxIndex);

public:
	/// 判断一个PeerConnection是否可以用来下载数据
	bool CanDownload( PeerTunnelPtr tunnel ) const;

	/// 检查piece是否需要下载
	bool NeedDownload(UINT pieceIndex) const;
	virtual bool NeedDownload(SubPieceUnit subPiece) const;

	/// 是否已经请求过
	bool IsRequested(UINT piece) const;
	bool IsRequested(UINT32 piece, UINT8 subPieceIndex) const;

	/// 是有拥有指定的piece资源
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


	/// 供QuotaManager等相关模块调用
public:
	UINT64 GetLocateElapsed() const { return m_LocateTimeCounter.elapsed(); }

	/// 获取数据缓冲区
	CStreamBuffer& GetStreamBuffer() const { return m_streamBuffer; }

	/// 获取起始的下载位置
	UINT GetStartIndex() const;
	
	CPeerManager& GetPeerManager() const { return m_PeerManager;}

protected:
	/// 检查piece索引是否有效
//	bool CheckPieceIndexValid(UINT pieceIndex) const;

	/// 根据当前的PeerConnection连接，找到合适的起始下载位置
	UINT FindStartPosition();
	UINT FindGoodStartPosition();
	
	/// 在运行过程中，根据当前的PeerConnection连接，找到合适的资源定位位置
	UINT FindPreLocationPosition();
	UINT FindGoodPreLocationPosition();

	/// 检查正在进行的下载任务是否超时
	bool CheckRquestPieceTimeout();

	/// 尝试下载指定的 SubPiece
	bool TryToDownload(SubPieceUnit subPiece, QuotaManager& quotaManager, int& tryCount, DWORD );

	/// 是否资源正常的节点有拥有指定的piece资源
	bool HasResourceInGoodPeer(UINT piece) const;


	void GetConnectionStatistics(int& highConnectionCount, int& requestingConnectionCount, int requestTimeoutConnectionCount) const;

	/// 获得总资源的MinIndex (所有高速节点的MinIndex的最小值)
	UINT GetResourceMinIndex() const;

	/// 获得总资源的MaxIndex (所有高速节点的MinIndex的最小值)
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
	/// 所属的PeerManager
	CPeerManager& m_PeerManager;

	CStreamBuffer& m_streamBuffer;
	Storage& m_storage;

	boost::shared_ptr<const PeerStatusInfo> m_StatusInfo;

	/// 用于请求数据的定时器
	periodic_timer m_RequestTimer;

	/// Peer连接的集合
	const PeerConnectionCollection& m_Connections;

	PeerTunnelCollection m_Tunnels;
	// Added by Tady, 011611: Spark! 
	PeerTunnelPtr m_sparkTunnelPtr;
	std::vector<PeerTunnelPtr> m_sparkTunnels;
	UINT m_sparkTick;

	/// 按照配额索引的PeerConnection集合
	//CPeerConnectionQuotaMap m_PeerConnectionQuotaMap;

	/// 可以启动RequestNextPiece的时间
	UINT m_StartRequestTickCount;

	/// 上一次预定位的时间
	time_counter m_LastPrelocateTickCount;
	time_counter m_LastCalcsPrelocateTickCount;

	/// 是否开始了RequestNextPiece
	bool  m_IsStartRequest;

	/// 上次检查超时请求的时间
	time_counter m_LastRequestCheckTime;

	/// 使用剩余配额下载到的piece个数
	UINT m_TotalExtraDown;

	time_counter m_LocateTimeCounter;

	HealthyDegreeCollection m_healthyMap;//健康度和编号
	HealthyDegreeCollection2 m_healthyMap2;

	/// 活动的请求
	PieceRequestManager m_requests;

	DownloaderStatistics& m_Statistics;

	UnfinishedSubPieceStorage m_unfinishedDataPieces;
	/// piece的最大sub-piece个数
	size_t m_MaxSubPieceCountPerPiece;


	UnfinishedSubPieceStorage m_unfinishedHeaderPieces;

	boost::shared_ptr<const SourceResource> m_SourceResource;
	QuotaManager quotaManager;
};

#endif