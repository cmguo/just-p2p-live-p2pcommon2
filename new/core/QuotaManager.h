
#ifndef _LIVE_P2PCOMMON2_NEW_CORE_QUOTA_MANAGER_H_
#define _LIVE_P2PCOMMON2_NEW_CORE_QUOTA_MANAGER_H_

#include "SubPieceTask.h"
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <map>
#include <set>

class PeerDownloader;
class PeerConnection;
class CStreamBuffer;
class Storage;
struct PEER_MINMAX;
class PeerTunnel;

/// 按照配额排序的PeerConnection集合
//typedef multimap< double,PeerConnection*,greater<double> > CPeerConnectionQuotaMap;

/// 按照预期的下载时间排序的PeerConnection集合
typedef std::multimap< DWORD, PeerConnection*> CPeerConnectionReceiveTimeMap;

typedef CPeerConnectionReceiveTimeMap CPeerConnectionQuotaMap;

typedef std::multimap<DWORD, PeerTunnel*> PeerTunnelRecvTimeMap;

typedef std::set<PeerConnection*> PeerConnectionCollection;

typedef boost::shared_ptr<PeerTunnel> PeerTunnelPtr;
typedef std::set<PeerTunnelPtr> PeerTunnelCollection;

const int min_piece_count = 300;
const int urgent_down_piece_count = 100;
const int extra_urgent_down_piece_count = 30;



// struct PieceTask2
// {
// 	SubPieceUnit subPieceUnit;
// 	DWORD externalTimeOut;
// 	PieceTask2( SubPieceUnit _subPieceUnit, DWORD _externalTimeOut) :externalTimeOut(_externalTimeOut){ subPieceUnit.PieceIndex = _subPieceUnit.PieceIndex; subPieceUnit.SubPieceIndex = _subPieceUnit.SubPieceIndex; }
// 	PieceTask2( UINT32 _pieceIndex, UINT8 _subPieceIndex, DWORD _externalTimeOut) :externalTimeOut(_externalTimeOut){ subPieceUnit.PieceIndex = _pieceIndex; subPieceUnit.SubPieceIndex = _subPieceIndex; } 
// };


/// 封装配额集合相关的操作
class QuotaManager : private boost::noncopyable
{
public:
	explicit QuotaManager(PeerDownloader& downloader, const PeerTunnelCollection& connections);

	void CalcTunnelReceiveTimes();

	void RequestFromTaskQueue();
	bool RequestFromTaskQueue2();
	/// 是否存在可以用来下载的Tunnel
	bool HasRecvTimeMap() const;

	void UpdateTunnelRecvTimeMap(PeerTunnelRecvTimeMap::iterator itr, UINT newRecvTime);

	std::pair<bool, PeerTunnel*> TryToDownload(SubPieceUnit subPiece, int& tryCount, DWORD externalTimeOut);

	void CalcHealthy(HealthyDegreeCollection &HealthyMap, UINT ResourceMaxIndex, const PEER_MINMAX& sourceMinMax) const;
	void CalcHealthy2(HealthyDegreeCollection &HealthyMap, UINT ResourceMaxIndex, const PEER_MINMAX& sourceMinMax) const; // Added by Tady, 070908
	void CalcHealthy2(HealthyDegreeCollection2 &HealthyMap, UINT ResourceMaxIndex, const PEER_MINMAX& sourceMinMax) /*const*/; // Added by Tady, 072408
//	void CalcHealthy3(HealthyDegreeCollection2 &taskQueue, UINT ResourceMaxIndex, const PEER_MINMAX& sourceMinMax) const; // Added by Tady, 072908
private:
	PeerTunnelRecvTimeMap m_RecvTimeMap;
        
        /// 所有的连接
        const PeerTunnelCollection& m_Tunnels;
        
        /// 下载管理器
        PeerDownloader& m_downloader;
	CStreamBuffer& m_streamBuffer;
	const Storage& m_storage;

	struct SubpieceRetryRec
	{
		SubPieceUnit	subPiece;
		UINT			times;
	};
	std::vector<SubpieceRetryRec> m_retrySubpieceVector;
};

#endif
