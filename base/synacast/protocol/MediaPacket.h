
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_MEDIA_PACKET_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_MEDIA_PACKET_H_

/**
 * @file
 * @brief 包含媒体相关的报文类：subpiece请求报文，subpiece数据报文，header piece和data piece类
 */


#include <synacast/protocol/PacketBase.h>
#include <synacast/protocol/data/SubPieceUnit.h>
#include <synacast/protocol/base.h>
#include <synacast/protocol/ReaderTypes.h>
#include <synacast/protocol/piecefwd.h>
#include <synacast/protocol/DataIO.h>

#include <synacast/protocol/SubMediaPiece.h>
#include <synacast/protocol/SignatureData.h>
#include <synacast/protocol/data/MediaPieceInfoIO.h>

#include <ppl/io/variant_reader.h>
#include <ppl/io/variant_writer.h>

#include <vector>

using ppl::io::variant_reader_util;
using ppl::io::variant_reader_uint8;
using ppl::io::variant_reader_uint16;
using ppl::io::variant_writer_uint16;
using ppl::io::variant_reader_util;




inline data_output_stream& operator<<( data_output_stream& os, const SubPieceUnit& info )
{
	return os 
		<< info.PieceIndex 
		<< info.SubPieceIndex;
}
inline data_input_stream& operator>>( data_input_stream& is, SubPieceUnit& info )
{
	return is 
		>> info.PieceIndex 
		>> info.SubPieceIndex;
}




/// sub-piece请求报文
class SubPieceUnitRequestPacket : public PacketBase
{
public:
	/// sub-piece 切片尺寸，接下来是低4位的RequestCount和高4位的RequestByTSCount，然后是数组：SubPieces和TSOfPiece
	UINT16 SubPieceLength;
	std::vector<SubPieceUnit> SubPieces;
	std::vector<UINT64> TSOfPiece;

	SubPieceUnitRequestPacket() : PacketBase( PPT_SUB_PIECE_REQUEST )
	{
		SubPieceLength = 0;
	}
	explicit SubPieceUnitRequestPacket(SubPieceUnit subPieceUnit) : PacketBase( PPT_SUB_PIECE_REQUEST )
	{
		SubPieceLength = 0;
		SubPieces.resize(1);
		SubPieces[0] = subPieceUnit;
	}

	explicit SubPieceUnitRequestPacket(UINT64 inTS) : PacketBase( PPT_SUB_PIECE_REQUEST )
	{
		SubPieceLength = 0;
		TSOfPiece.resize(1);
		TSOfPiece[0] = inTS;
	}

	virtual bool read_object(data_input_stream& is)
	{
		UINT8 val = 0;
		if ( is >> SubPieceLength >> val )
		{
			/// 低4位是subpiece个数
			UINT8 subPieceCount = val & 0x0F;
			/// 高4位是TimeStamp个数
			UINT8 tsCount = (val & 0xF0) >> 4;
			return is.read_vector( subPieceCount, SubPieces, SubPieceUnit::object_size ) && is.read_vector( tsCount, TSOfPiece, sizeof(UINT64) );
		}
		return false;
	}

	virtual void write_object(data_output_stream& os) const
	{
		LIVE_ASSERT( SubPieces.size() <= 15 );
		LIVE_ASSERT( TSOfPiece.size() <= 15 );
		UINT8 lowByte = static_cast<UINT8>( SubPieces.size() ) & 0x0F;
		UINT8 highByte = static_cast<UINT8>( TSOfPiece.size() ) & 0x0F;
		UINT8 val = (highByte << 4) | lowByte;
		if ( SubPieces.size() == 15 )
		{
			SubPieces.size();
		}
		os << SubPieceLength << val << SubPieces << TSOfPiece;
	}

	virtual size_t get_object_size() const
	{
		LIVE_ASSERT( SubPieces.size() <= 15 );
		LIVE_ASSERT( TSOfPiece.size() <= 15 );
		if ( SubPieces.size() == 15 )
		{
			SubPieces.size();
		}
		return sizeof(UINT16) + sizeof(UINT8) + SubPieceUnit::object_size * SubPieces.size() + sizeof(UINT64) * TSOfPiece.size();
	}

	// Added by Tady, 073108: For multi-subPiece request.
	virtual void AddOneSubPieceUnit(SubPieceUnit inSubPieceUnit)
	{
		SubPieces.push_back(inSubPieceUnit);
		LIVE_ASSERT(SubPieces.size() <= 15 );
	}

	virtual void AddOneSubPieceUnit(UINT64 inTS)
	{
		TSOfPiece.push_back(inTS);
		LIVE_ASSERT(TSOfPiece.size() <= 15 );
	}

	virtual void Clear() 
	{
		SubPieces.clear();
		TSOfPiece.clear();
	}

	virtual UINT GetRequestCount() 
	{ 
		return SubPieces.size(); 
	}

	virtual UINT GetRequestByTSCount() 
	{ 
		return TSOfPiece.size();
	}
};


class SubPiecePacket : public PacketBase
{
public:
	SubMediaPiecePtr SubPiece;

	explicit SubPiecePacket( SubMediaPiecePtr piece ) : PacketBase( PPT_SUB_PIECE_DATA ), SubPiece( piece )
	{
	}
	SubPiecePacket() : PacketBase( PPT_SUB_PIECE_DATA ), SubPiece( new SubMediaPiece )
	{
	}

	virtual bool read_object( data_input_stream& is )
	{
		is  >> SubPiece->SubPieceInfo 
			>> SubPiece->PieceInfo 
			>> ppl::io::variant_reader_uint16::make( SubPiece->SubPieceData );
		return is && SubPiece->PieceInfo.PieceLength + 1 + SignatureData::static_size >= SubPiece->SubPieceData.size();
	}

	virtual void write_object( data_output_stream& os ) const
	{
		os  << SubPiece->SubPieceInfo 
			<< SubPiece->PieceInfo 
			<< ppl::io::variant_writer_uint16::make( SubPiece->SubPieceData );
	}

	virtual size_t get_object_size() const
	{
		LIVE_ASSERT( SubPiece );
		return MediaSubPieceInfo::object_size 
			+ MediaPieceInfo::object_size 
			+ sizeof(UINT16) + SubPiece->SubPieceData.size();
	}
};


#endif

