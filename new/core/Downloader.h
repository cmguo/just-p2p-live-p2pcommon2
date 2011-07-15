
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


///������Ҫ4��PeerConnection	
const UINT MIN_PEER_COUNT = 4;



/// �������ݵ�����
class Downloader : private boost::noncopyable
{
public:
	Downloader() { }
	virtual ~Downloader() { }

	virtual void AddTunnel( PeerConnection* pc ) = 0;
	virtual void RemoveTunnel( PeerConnection* pc ) = 0;

	/// ����
	virtual bool Start(bool hasMDS) = 0;
	virtual void Stop() = 0;

	/// PeerConnection���յ�һ��header pieceʱ����
	//virtual void OnHeaderPieceReceived(PPMediaHeaderPacketPtr packet, PeerConnection* connection) = 0;

	/// PeerConnection���յ�һ��data pieceʱ����
	//virtual void OnDataPieceReceived(PPMediaDataPacketPtr packet, PeerConnection* connection) = 0;
	virtual void OnSubPieceReceived(SubMediaPiecePtr packet, PeerConnection* connection) = 0;

	/// PeerConnection�ڽ���һ��pieceʧ��ʱ����
	virtual void OnPieceReceiveFailed(UINT pieceIndex, PeerConnection* connection) = 0;
	virtual void OnSubPieceReceivedFailed(SubPieceUnit subPiece, PeerConnection* connection) = 0;

	/// ������һ��Pieice
	virtual void RequestNextPiece() = 0;

	/// �ж�һ��PeerConnection�Ƿ����������������
	virtual bool CanDownload( PeerTunnelPtr tunnel ) const = 0;

	/// ���piece�Ƿ���Ҫ����
	virtual bool NeedDownload(UINT pieceIndex) const = 0;

	/// ���subpiece�Ƿ���Ҫ����
	virtual bool NeedDownload(SubPieceUnit subPiece) const = 0;

	/// �Ƿ��Ѿ������
	virtual bool IsRequested(UINT piece) const = 0;

	/// ����ӵ��ָ����piece��Դ
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

	/// �����Դ�����ֵ
	virtual UINT GetResourceMaxIndex() const = 0;

	/// sub piece�Ƿ��Ѿ�����
//	virtual bool HasSubPiece(UINT32 pieceIndex, UINT8 subPieceIndex) const = 0;

	/// �Ƿ���Ҫĳsub piece
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

