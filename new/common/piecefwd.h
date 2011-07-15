
#ifndef _LIVE_P2PCOMMON2_NEW_COMMON_PIECEFWD_H_
#define _LIVE_P2PCOMMON2_NEW_COMMON_PIECEFWD_H_

#include <synacast/protocol/piecefwd.h>

#include <boost/shared_ptr.hpp>




/// ÇÐÆ¬´æ´¢µÄdata piece
class MediaPiece;
class MediaDataPiece;
class MediaHeaderPiece;
typedef boost::shared_ptr<MediaPiece> MediaPiecePtr;
typedef boost::shared_ptr<MediaDataPiece> MediaDataPiecePtr;
typedef boost::shared_ptr<MediaHeaderPiece> MediaHeaderPiecePtr;

class UnfinishedMediaPiece;
typedef boost::shared_ptr<UnfinishedMediaPiece> UnfinishedMediaPiecePtr;


#ifdef _DEBUG
void CheckDataPiece( MediaDataPiecePtr piece );
void CheckHeaderPiece( MediaHeaderPiecePtr piece );
#else
inline void CheckDataPiece( MediaDataPiecePtr piece )
{}
inline void CheckHeaderPiece( MediaHeaderPiecePtr piece )
{}
#endif

#endif
