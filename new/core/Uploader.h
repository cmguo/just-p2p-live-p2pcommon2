
#ifndef _LIVE_P2PCOMMON2_NEW_CORE_UPLOADER_H_
#define _LIVE_P2PCOMMON2_NEW_CORE_UPLOADER_H_

#include "common/piecefwd.h"
#include <synacast/protocol/data/SubPieceUnit.h>
#include <ppl/util/time_counter.h>
#include <ppl/util/random.h>
#include <boost/noncopyable.hpp>
#include <map>

class CPeerManager;
class PeerConnection;
class Storage;
class UploaderConfig;


struct UploadInfo
{
	size_t UploadTimes;
        time_counter LastUploadTime;
	
	UploadInfo() : UploadTimes(0), LastUploadTime(0)
	{
	}
};

typedef std::map<SubPieceUnit, UploadInfo> SubPieceUploadInfoCollection;



class Uploader : private boost::noncopyable
{
public:
	explicit Uploader(CPeerManager& peerManager, const Storage& storage, const UploaderConfig& config);
	virtual ~Uploader();

	virtual void UploadSubPiece(SubPieceUnit subPieceUnit, PeerConnection* pc);
	virtual void UploadSubPiece(UINT64 inTS, PeerConnection* pc);

//	virtual void UploadSubPieces(UINT pieceIndex, UINT8 subPieceCount, const UINT16 subPieces[], PeerConnection* pc);
//	virtual void UploadSubPieces(UINT pieceIndex, PeerConnection* pc);
//	virtual void UploadMonoPiece(UINT pieceIndex, PeerConnection* pc);

	virtual void OnAppTimer(UINT times);

protected:
	void SendSubPiece(SubMediaPiecePtr subPiece, PeerConnection* pc);
//	void SendDataPiece(PPMediaDataPacketPtr dataPiece, PeerConnection* pc);

	//	bool CanUploadDataPiece(PPMediaDataPacketPtr dataPiece);
	bool CanUploadPiece(MediaPiecePtr piece);
	bool CanUploadSubPiece(SubPieceUnit subPieceUnit);
	bool CanUploadSubPiece(SubMediaPiecePtr subPiece);
	void CheckMaxUploadTimes();
	CPeerManager& m_PeerManager;
	const Storage& m_Storage;

	void UploadMonoPiece( UINT pieceIndex, PeerConnection* pc );

	bool SendMonoPiece( MediaPiecePtr piece, PeerConnection* pc );

private:
	SubPieceUploadInfoCollection m_UploadInfo;
	time_counter m_LastCheckTime;
	size_t m_MaxUploadTimes;
	RandomGenerator m_rand;

	void CheckUploadCounts();
};

#endif


