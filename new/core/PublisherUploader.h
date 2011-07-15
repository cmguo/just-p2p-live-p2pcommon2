
#ifndef _LIVE_P2PCOMMON2_NEW_CORE_PUBLISHER_UPLOADER_H_
#define _LIVE_P2PCOMMON2_NEW_CORE_PUBLISHER_UPLOADER_H_

#include "Uploader.h"
#include <map>

typedef std::map<UINT, time_counter> publishPiecePushTimeOut;

class PublisherUploader : public Uploader
{
public:
	explicit PublisherUploader(CPeerManager& peerManager, const Storage& storage, const UploaderConfig& config);
	virtual ~PublisherUploader();

	virtual void OnAppTimer(UINT times);

	virtual void UploadSubPiece(SubPieceUnit subPieceUnit, PeerConnection* pc);
	//virtual void UploadSubPieces(UINT pieceIndex, PeerConnection* pc);
	//virtual void UploadMonoPiece(UINT pieceIndex, PeerConnection* pc);

private:
	void CalPublishBitmap();
	publishPiecePushTimeOut m_publishPiecePushTimeOut;
	void checkPublishMapTimeOut();

};


#endif