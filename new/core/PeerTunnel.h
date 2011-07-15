#ifndef _LIVE_P2PCOMMON2_NEW_CORE_PEER_TUNNEL_H_
#define _LIVE_P2PCOMMON2_NEW_CORE_PEER_TUNNEL_H_

#include "SubPieceTask.h"
#include "framework/memory.h"
//#include "Downloader.h"
#include <synacast/protocol/piecefwd.h>
#include <ppl/util/time_counter.h>
#include <boost/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <iosfwd>

class PeerConnection;
class UDPTPeerConnection;
class TCPPeerConnection;
class Storage;
class Downloader;



// #if SAVAGE_USE_TINY_TASK_QUEUE

//#if NO_HEALTHCOUNTCALC
 //typedef SubPieceTaskList HealthyDegreeCollection2;
// #else // !SAVAGE_USE_TINY_TASK_QUEUE
//	typedef multimap<UINT,SubPieceTask> HealthyDegreeCollection2;
// #endif // SAVAGE_USE_TINY_TASK_QUEUE


class PeerTunnel : public pool_object, private boost::noncopyable, public boost::enable_shared_from_this<PeerTunnel>
{
public:
	PeerTunnel(PeerConnection& connection, Downloader& downloader, int tunnelIndex);
	virtual ~PeerTunnel() { }

	virtual bool IsRequesting() const = 0;
	virtual bool IsRequesting(SubPieceUnit subPiece) const = 0;

	virtual void OnReceiveSubPiece(SubMediaPiecePtr subPiece) = 0;

	virtual void OnPieceNotFound(UINT pieceIndex) = 0;

	virtual void OnSubPieceNotFound(SubPieceUnit subPiece) = 0;

	virtual bool CheckRequestPieceTimeout() = 0;

	virtual DWORD GetSortValue() = 0;

	virtual DWORD GetUsedTime() = 0;
	virtual DWORD GetRealUsedTime() = 0;

	virtual bool RequestFromTaskQueue() = 0;

	virtual void AdjustWindowSize(UINT minSize) {};

	virtual UINT GetRequestCount() const = 0;

	inline virtual bool TryToDownload(SubPieceUnit subPiece, DWORD externalTimeout);

	inline virtual void ClearTaskQueue();
	//UINT GetTaskQueueSize() {return m_TaskQueue.size();}

	UINT GetWindowSize() const { return m_WindowSize; }

	size_t GetTaskQueueSize() const { return m_TaskQueue.size(); }


	PeerConnection& GetConnection() { return m_Connection; }
	const PeerConnection& GetConnection() const { return m_Connection; }

	inline bool AssignDownloadTask( SubPieceTask task );

	UINT GetTaskQueueMaxSize() {return m_taskQueueMaxSize;}

	int GetIndex() const { return m_TunnelIndex; }

	DWORD GetTunnelUsedTime(){ return m_RequestTime.elapsed32();}

	/// 是否是高速连接
	bool IsHighConnection()
	{
		// 是否10秒钟之内能够收到1片
		return GetUsedTime() < 10 * 1000;
	}
	double GetRequestSuccedRate() { return m_lastRequestSuccedRate; }

	bool IsUsed() const { return m_RequestTimes > 0; }
	void SetTaskMapPtr(HealthyDegreeCollection2* inMapPtr) { m_taskCollectionPtr = inMapPtr; }

	// Added by Tady 011511: Spark, to flash-starting.
	bool IsFreezing() const { return m_bIsFreezing; }
	void Freeze(bool val) { m_bIsFreezing = val; }

protected:
	virtual bool RequestSubPiece(SubPieceUnit subPiece, DWORD externalTimeOut) = 0;

protected:
	Downloader& m_Downloader;

	// 通道的预分配对列
	SubPieceTaskList m_TaskQueue;

	// 通道编号
	int m_TunnelIndex;

	// 请求发起时间
	time_counter m_RequestTime;

	// 请求内部超时世界
	DWORD m_InternalTimeout;
	
	UINT m_taskQueueMaxSize;

	UINT m_lastTaskQueueSize;

	UINT m_WindowSizeMin;
	UINT m_WindowSizeMax;

	PeerConnection& m_Connection;
        UINT m_WindowSize; 
       // 上一次SubPiece请求成功数
	double m_lastRequestSuccedRate;


	UINT m_RequestTimes;

	// Added by Tady, 072408:
	HealthyDegreeCollection2* m_taskCollectionPtr;

	// Added by Tady, 011511: For Spark --- flash-starting.
	bool m_bIsFreezing; 
};

std::ostream& operator<<(std::ostream& os, const PeerTunnel& val);

inline bool PeerTunnel::TryToDownload(SubPieceUnit subPiece, DWORD externalTimeout)
{
	SubPieceTask task(subPiece, externalTimeout);
	m_TaskQueue.push_back(task);
	m_lastTaskQueueSize ++;
	return true;
}

inline void PeerTunnel::ClearTaskQueue() 
{ 
	if (40 + m_lastTaskQueueSize - m_TaskQueue.size() > m_taskQueueMaxSize)
		m_taskQueueMaxSize = 40 + m_lastTaskQueueSize - m_TaskQueue.size();

	m_TaskQueue.clear(); 
	m_lastTaskQueueSize = 0; 
}

inline bool PeerTunnel::AssignDownloadTask( SubPieceTask task )
{
	m_TaskQueue.push_back( task );
	m_lastTaskQueueSize ++;
	return true;
}

class PeerTunnelFactory
{
public:
	static PeerTunnel* CreateTCP( PeerConnection& connection, Downloader& downloader, int tunnelIndex );
	static PeerTunnel* CreateUDP( PeerConnection& connection, Downloader& downloader, int tunnelIndex );
};

#endif
