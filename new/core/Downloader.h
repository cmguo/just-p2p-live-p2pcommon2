
#ifndef _LIVE_P2PCOMMON2_NEW_CORE_DOWNLOADER_H_
#define _LIVE_P2PCOMMON2_NEW_CORE_DOWNLOADER_H_

#include "QuotaManager.h"
#include "PeerTunnel.h"
#include <synacast/protocol/piecefwd.h>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

class CPeerManager;
class PeerConnection;
class CStreamBuffer;
class UnfinishedSubPieceStorage;
class PeerInformation;
class PeerTunnel;
typedef boost::shared_ptr<PeerTunnel> PeerTunnelPtr;
class TCPPeerConnectionInfo;
class UDPPeerConnectionInfo;
class PeerHandshakeInfo;
typedef boost::shared_ptr<PeerTunnel> PeerTunnelPtr;


///最少需要4个PeerConnection	
const UINT MIN_PEER_COUNT = 4;



/// 负责数据的下载
class Downloader : private boost::noncopyable
{
public:
	Downloader() { }
	virtual ~Downloader() { }

	virtual void AddTunnel( PeerConnection* pc ) = 0;
	virtual void RemoveTunnel( PeerConnection* pc ) = 0;

	/// 启动
	virtual bool Start(bool hasMDS) = 0;
	virtual void Stop() = 0;

	/// PeerConnection在收到一个header piece时调用
	//virtual void OnHeaderPieceReceived(PPMediaHeaderPacketPtr packet, PeerConnection* connection) = 0;

	/// PeerConnection在收到一个data piece时调用
	//virtual void OnDataPieceReceived(PPMediaDataPacketPtr packet, PeerConnection* connection) = 0;
	virtual void OnSubPieceReceived(SubMediaPiecePtr packet, PeerConnection* connection) = 0;

	/// PeerConnection在接收一个piece失败时调用
	virtual void OnPieceReceiveFailed(UINT pieceIndex, PeerConnection* connection) = 0;
	virtual void OnSubPieceReceivedFailed(SubPieceUnit subPiece, PeerConnection* connection) = 0;

	/// 请求下一个Pieice
	virtual void RequestNextPiece() = 0;

	/// 判断一个PeerConnection是否可以用来下载数据
	virtual bool CanDownload( PeerTunnelPtr tunnel ) const = 0;

	/// 检查piece是否需要下载
	virtual bool NeedDownload(UINT pieceIndex) const = 0;

	/// 检查subpiece是否需要下载
	virtual bool NeedDownload(SubPieceUnit subPiece) const = 0;

	/// 是否已经请求过
	virtual bool IsRequested(UINT piece) const = 0;

	/// 是有拥有指定的piece资源
	virtual bool HasResource(UINT piece) const = 0;

	virtual void AddRequest(UINT pieceIndex, PeerConnection* pc, UINT timeout) = 0;
	virtual void AddRequest(SubPieceUnit subPiece, PeerConnection* connection, UINT externalTimeout) = 0;
	virtual size_t RemoveRequest(UINT pieceIndex, PeerConnection* pc) = 0;
	virtual size_t RemoveRequests(PeerConnection* pc) = 0;
	virtual size_t RemoveSubPieceRequest(SubPieceUnit subPiece, const PeerConnection* connection) = 0;

	virtual int GetRequestCount(UINT pieceIndex) = 0;

	virtual void CheckRequestByConnection( PeerConnection* conn ) = 0;

//	virtual SubMediaPiecePtr GetSubPiece(UINT pieceIndex, USHORT subPieceIndex) const = 0;

	//virtual pair<bool, PPMediaDataPacketPtr> AddSubPiece(SubMediaPiecePtr subPiece) = 0;

	/// 获得资源的最大值
	virtual UINT GetResourceMaxIndex() const = 0;

	/// sub piece是否已经存在
//	virtual bool HasSubPiece(UINT32 pieceIndex, UINT8 subPieceIndex) const = 0;

	/// 是否需要某sub piece
	virtual bool NeedSubPiece( UINT32 pieceIndex, UINT8 subPieceIndex ) const = 0;

	//virtual size_t GetReceivedSubPieceCount(UINT pieceIndex) const = 0;
	//virtual size_t GetSubPieceCount(UINT pieceIndex) const = 0;
	//virtual size_t GetInterestedSubPieces(UINT pieceIndex, vector<UINT16>& interestedSubPieceIndexes) const = 0;
	//virtual bool HasSubPiece(UINT pieceIndex) const = 0;
};



class DownloaderFactory
{
public:
	static Downloader* PeerCreate(CPeerManager& owner, CStreamBuffer& streamBuffer, boost::shared_ptr<PeerInformation> peerInformation);
	static Downloader* CreateTrivial();
};
#endif

