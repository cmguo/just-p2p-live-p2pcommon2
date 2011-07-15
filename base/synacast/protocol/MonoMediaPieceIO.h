
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_MONO_MEDIA_PIECEIO_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_MONO_MEDIA_PIECE_IOH_

#include <synacast/protocol/MonoMediaPiece.h>
#include <synacast/protocol/data/MediaPieceInfoIO.h>


class MonoMediaPieceIO
{
public:
	static void WritePiece( data_output_stream& os, const MonoMediaDataPiece& dataPiece )
	{
		MediaDataPieceInfo info;
		info.TimeStamp = dataPiece.TimeStamp;
		info.HeaderPiece = dataPiece.HeaderPiece;
		info.MediaDataLength = dataPiece.MediaData.size();
		os  << dataPiece.GetPieceInfo() 
			<< info 
			<< dataPiece.MediaData;
	}

	static void WritePiece( data_output_stream& os, const MonoMediaHeaderPiece& headerPiece )
	{
		MediaHeaderPieceInfo info;
		info.MediaType = headerPiece.MediaType;
		info.InfoLength = static_cast<UINT16>( headerPiece.InfoData.size() );
		info.HeaderLength = headerPiece.HeaderData.size();
		info.ProfileLength = static_cast<UINT16>( headerPiece.ProfileData.size() );
		os  << headerPiece.GetPieceInfo() 
			<< info 
			<< headerPiece.InfoData 
			<< headerPiece.HeaderData 
			<< headerPiece.ProfileData;
	}

	static bool ReadPieceBody( data_input_stream& is, MonoMediaDataPiece& dataPiece )
	{
		MediaDataPieceInfo info;
		if ( is >> info && is.read_buffer( info.MediaDataLength, dataPiece.MediaData ) )
		{
			dataPiece.TimeStamp = info.TimeStamp;
			dataPiece.HeaderPiece = info.HeaderPiece;
			return true;
		}
		return false;
	}

	static bool ReadPieceBody( data_input_stream& is, MonoMediaHeaderPiece& headerPiece )
	{
		MediaHeaderPieceInfo info;
		if ( is >> info && is.available() >= info.InfoLength + info.HeaderLength + info.ProfileLength && info.HeaderLength <= PPL_MAX_PIECE_DATA_LENGTH )
		{
			headerPiece.MediaType = info.MediaType;
			is.read_buffer( info.InfoLength, headerPiece.InfoData );
			is.read_buffer( info.HeaderLength, headerPiece.HeaderData );
			is.read_buffer( info.ProfileLength, headerPiece.ProfileData );
			return is;
		}
		return false;
	}
};


#endif
