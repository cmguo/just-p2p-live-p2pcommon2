
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_SUBMEDIA_PIECE_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_SUBMEDIA_PIECE_H_

#include <synacast/protocol/data/MediaPieceInfo.h>
#include <synacast/protocol/data/SubPieceUnit.h>
#include <ppl/data/int.h>



class SubMediaPiece : public pool_object, private boost::noncopyable
{
public:
	MediaSubPieceInfo SubPieceInfo;
	MediaPieceInfo PieceInfo;
	pool_byte_buffer SubPieceData;

	SubMediaPiece()
	{
	}

	explicit SubMediaPiece( const MediaPieceInfo& pieceInfo, UINT8 pieceLevel, UINT8 subPieceCount, UINT8 subPieceIndex, UINT16 dataLen, const BYTE* data )
		: PieceInfo( pieceInfo )
	{
		assert( dataLen < 1500 );
		assert( dataLen <= 1344 );
		SubPieceInfo.PieceLevel = pieceLevel;
		SubPieceInfo.SubPieceCount = subPieceCount;
		SubPieceInfo.SubPieceIndex = subPieceIndex;
		SubPieceData.assign( data, dataLen );
	}

/*
	virtual bool read_object( data_input_stream& is )
	{
		is  >> SubPieceInfo 
			>> PieceInfo 
			>> ppl::io::variant_reader_uint16::make( SubPieceData );
		return is && PieceInfo.PieceLength + 1 + SignatureData::static_size >= SubPieceData.size();
	}

	virtual void write_object( data_output_stream& os ) const
	{
		os  << SubPieceInfo 
			<< PieceInfo 
			<< ppl::io::variant_writer_uint16::make( SubPieceData );
	}

	virtual size_t get_object_size() const
	{
		return MediaSubPieceInfo::object_size 
			+ MediaPieceInfo::object_size 
			+ sizeof(UINT16) + SubPieceData.size();
	}
*/

	const MediaSubPieceInfo& GetSubPieceInfo() const { return SubPieceInfo; }
	const MediaPieceInfo& GetPieceInfo() const { return PieceInfo; }

	UINT8 GetPieceType() const { return PieceInfo.PieceType; }
	UINT32 GetPieceIndex() const { return PieceInfo.PieceIndex; }
	UINT32 GetPieceLength() const { return PieceInfo.PieceLength; }

	UINT8 GetSubPieceIndex() const { return SubPieceInfo.SubPieceIndex; }
	UINT8 GetSubPieceCount() const { return SubPieceInfo.SubPieceCount; }
	UINT8 GetPieceLevel() const { return SubPieceInfo.PieceLevel; }

	SubPieceUnit GetSubPieceUnit() const
	{
		return SubPieceUnit( PieceInfo.PieceIndex, SubPieceInfo.SubPieceIndex );
	}
};


#endif