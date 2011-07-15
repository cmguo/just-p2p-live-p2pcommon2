
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_MEDIA_PIECE_INFOIO_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_MEDIA_PIECE_INFOIO_H_

#include <synacast/protocol/data/MediaPieceInfo.h>
#include <synacast/protocol/DataIO.h>
#include <ppl/io/data_input_stream.h>
#include <ppl/io/data_output_stream.h>


inline data_output_stream& operator<<(data_output_stream& os, const MediaSubPieceInfo& info)
{
	return os 
		<< info.PieceLevel 
		<< info.SubPieceCount 
		<< info.SubPieceIndex;
}

inline data_input_stream& operator>>(data_input_stream& is, MediaSubPieceInfo& info)
{
	return is 
		>> info.PieceLevel 
		>> info.SubPieceCount 
		>> info.SubPieceIndex;
}


inline data_output_stream& operator<<(data_output_stream& os, const MediaPieceInfo& info)
{
	return os 
		<< info.PieceType 
		<< info.PieceIndex 
		<< info.PieceLength;
}

inline data_input_stream& operator>>(data_input_stream& is, MediaPieceInfo& info)
{
	return is 
		>> info.PieceType 
		>> info.PieceIndex 
		>> info.PieceLength;
}

inline data_input_stream& operator>>( data_input_stream& is, MediaDataPieceInfo& info )
{
	return is 
		>> info.TimeStamp 
		>> info.HeaderPiece 
		>> info.MediaDataLength;
}

inline data_output_stream& operator<<( data_output_stream& os, const MediaDataPieceInfo& info )
{
	return os 
		<< info.TimeStamp 
		<< info.HeaderPiece 
		<< info.MediaDataLength;
}

inline data_input_stream& operator>>( data_input_stream& is, MediaHeaderPieceInfo& info )
{
	return is 
		>> info.MediaType 
		>> info.InfoLength 
		>> info.HeaderLength 
		>> info.ProfileLength;
}

inline data_output_stream& operator<<( data_output_stream& os, const MediaHeaderPieceInfo& info )
{
	return os 
		<< info.MediaType 
		<< info.InfoLength 
		<< info.HeaderLength 
		<< info.ProfileLength;
}


#endif