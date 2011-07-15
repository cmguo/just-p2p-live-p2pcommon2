
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_PIECEFWD_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_PIECEFWD_H_

#include <boost/shared_ptr.hpp>


class SubMediaPiece;
typedef boost::shared_ptr<SubMediaPiece> SubMediaPiecePtr;


class MonoMediaPiece;
class MonoMediaHeaderPiece;
class MonoMediaDataPiece;


typedef boost::shared_ptr<MonoMediaPiece> MonoMediaPiecePtr;
typedef boost::shared_ptr<MonoMediaHeaderPiece> MonoMediaHeaderPiecePtr;
typedef boost::shared_ptr<MonoMediaDataPiece> MonoMediaDataPiecePtr;



/// 检查piece的有效性
#ifdef _DEBUG

void CheckDataPiece(const MonoMediaDataPiece* piece);
void CheckHeaderPiece(const MonoMediaHeaderPiece* piece);

void CheckDataPiece(MonoMediaDataPiecePtr piece);
void CheckHeaderPiece(MonoMediaHeaderPiecePtr piece);
void CheckSubPiece(SubMediaPiecePtr subPiece);

#else

inline void CheckDataPiece(const MonoMediaDataPiece* piece) { }
inline void CheckHeaderPiece(const MonoMediaHeaderPiece* piece) { }

inline void CheckDataPiece(MonoMediaDataPiecePtr piece) { }
inline void CheckHeaderPiece(MonoMediaHeaderPiecePtr piece) { }
inline void CheckSubPiece(SubMediaPiecePtr subPiece) { }

#endif



#endif