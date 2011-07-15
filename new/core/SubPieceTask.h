#ifndef _LIVE_P2PCOMMON2_NEW_CORE_SUBPIECE_TASK_H_
#define _LIVE_P2PCOMMON2_NEW_CORE_SUBPIECE_TASK_H_

#include <synacast/protocol/data/SubPieceUnit.h>
#include <map>
#include <list>

#define _Savage 1 // Added by Tady, 072408: Control use pre-distribute or not. 
// 1 means not use it, and use Savage-mode. Enjoy it! :)
#if _Savage

#define NO_HEALTHCOUNTCALC 0
#define MRP_SUPPORTED 1

#if MRP_SUPPORTED
#define LIGHT_MRP 1
#endif // MRP_SUPPORTED

#endif // _Savage

struct PieceTask
{
	UINT pieceIndex;
	DWORD externalTimeOut;
	PieceTask( UINT _pieceIndex, DWORD _externalTimeOut) :pieceIndex(_pieceIndex), externalTimeOut(_externalTimeOut){}
};

typedef std::multimap<UINT,PieceTask> HealthyDegreeCollection;

struct SubPieceTask
{
	SubPieceUnit subPiece;
	DWORD externalTimeOut;
	SubPieceTask (SubPieceUnit _subPiece, DWORD _externalTimeOut): subPiece(_subPiece), externalTimeOut(_externalTimeOut) {}
	SubPieceTask () : subPiece(0), externalTimeOut(0) { }
	SubPieceTask( UINT32 _pieceIndex, UINT8 _subPieceIndex, DWORD _externalTimeOut) :externalTimeOut(_externalTimeOut){ subPiece.PieceIndex = _pieceIndex; subPiece.SubPieceIndex = _subPieceIndex; } 
};


typedef std::list<SubPieceTask> SubPieceTaskList;

typedef std::multimap<UINT, SubPieceTask> SubPieceTaskMultiMap;

#if NO_HEALTHCOUNTCALC
typedef SubPieceTaskList HealthyDegreeCollection2;
#else 
typedef SubPieceTaskMultiMap HealthyDegreeCollection2;
#endif


enum PacketHandlingResultEnum
{
	PACKET_HANDLING_OK = 0, 
	PACKET_HANDLING_ERROR = 1, 
	PACKET_HANDLING_IGNORE = 2, 
};


#endif