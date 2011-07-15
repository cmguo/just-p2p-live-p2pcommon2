
#include "StdAfx.h"

#include "MediaPieceUtil.h"
#include "MediaPiece.h"
#include "framework/log.h"

#include <synacast/protocol/SubMediaPiece.h>
#include <synacast/protocol/MonoMediaPieceIO.h>
#include <synacast/protocol/DataSigner.h>
#include <synacast/protocol/PacketStream.h>
#include <boost/foreach.hpp>



bool MediaPieceUtil::Verify( const MediaPiece& piece, DataSignerPtr signer )
{
	if ( ! signer )
		return true;
	// 需要签名的数据除了subpiece data之外，还包含piece info
	// sub piece data中还包含签名结果
	pool_byte_buffer& buf = GetBufferRef();
	buf.ensure_size( MediaPieceInfo::object_size );
	PacketOutputStream os( buf.data(), buf.size() );
	os << piece.GetPieceInfo();
	BOOST_FOREACH( SubMediaPiecePtr subPiece, piece.GetSubPieces() )
	{
		buf.append( subPiece->SubPieceData );
	}
	if ( buf.size() < MediaPieceInfo::object_size + 1 + SignatureData::static_size )
		return false;
	size_t datalen = buf.size() - 1 - SignatureData::static_size;
	if ( SignatureData::static_size != buf[datalen] )
		return false;
	const BYTE* signature = buf.data() + datalen + 1;
	return signer->Verify( buf.data(), datalen, signature );
}

bool MediaPieceUtil::Sign( const MonoMediaDataPiece& dataPiece, DataSignerPtr signer )
{
	pool_byte_buffer& buf = GetBufferRef();
	PrepareSignBuffer( dataPiece );
	PacketOutputStream os( buf.data(), buf.size() );
	MonoMediaPieceIO::WritePiece( os, dataPiece );
	assert( os.position() == dataPiece.GetSize() );
	return SignBuffer( os, dataPiece, signer );
}

bool MediaPieceUtil::Sign( const MonoMediaHeaderPiece& headerPiece, DataSignerPtr signer )
{
	pool_byte_buffer& buf = GetBufferRef();
	PrepareSignBuffer( headerPiece );
	PacketOutputStream os( buf.data(), buf.size() );
	MonoMediaPieceIO::WritePiece( os, headerPiece );
	assert( os.position() == headerPiece.GetSize() );
	return SignBuffer( os, headerPiece, signer );
}

MonoMediaPiecePtr MediaPieceUtil::MakeMonoPiece(const MediaPiece& slicedPiece)
{
	pool_byte_buffer& buf = GetBufferRef();
	// 首先，将所有的subpiece的data组装起来
	const MediaPieceInfo& pieceInfo = slicedPiece.GetPieceInfo();
	pool_byte_buffer& pieceData = buf;
	pieceData.resize(0);
	for (size_t i = 0; i < slicedPiece.GetSubPieces().size(); ++i)
	{
		const SubMediaPiece& subPiece = *slicedPiece.GetSubPieces()[i];
		assert(subPiece.GetSubPieceIndex() == i);
		pieceData.append(subPiece.SubPieceData);
	}
	assert(pieceData.size() == pieceInfo.PieceLength + 1 + SignatureData::static_size);

	// 解析piece data部分
	PacketInputStream is( pieceData.data(), pieceInfo.PieceLength );
	MonoMediaPiecePtr piece;
	if ( PPDT_MEDIA_DATA == pieceInfo.PieceType )
	{
		MonoMediaDataPiece* dataPiece = new MonoMediaDataPiece( slicedPiece.GetPieceLevel(), pieceInfo.PieceIndex );
		piece.reset( dataPiece );
		if ( false == MonoMediaPieceIO::ReadPieceBody( is, *dataPiece ) )
		{
			VIEW_ERROR("MediaPiece::MakeMonoPiece invalid data piece body " << make_tuple(pieceInfo.PieceIndex, pieceInfo.PieceType));
			return MonoMediaPiecePtr();
		}
	}
	else if ( PPDT_MEDIA_HEADER == pieceInfo.PieceType )
	{
		MonoMediaHeaderPiece* headerPiece = new MonoMediaHeaderPiece( slicedPiece.GetPieceLevel(), pieceInfo.PieceIndex );
		piece.reset( headerPiece );
		if ( false == MonoMediaPieceIO::ReadPieceBody( is, *headerPiece ) )
		{
			VIEW_ERROR("MediaPiece::MakeMonoPiece invalid header piece body " << make_tuple(pieceInfo.PieceIndex, pieceInfo.PieceType));
			return MonoMediaPiecePtr();
		}
	}
	else
	{
		VIEW_ERROR("MediaPiece::MakeMonoPiece unrecognized piece type " << pieceInfo.PieceType);
		assert(false);
		return MonoMediaPiecePtr();
	}
	assert( piece->GetPieceBodyLength() == pieceInfo.PieceLength );
	if ( pieceInfo.PieceLength < piece->GetPieceBodyLength() )
	{
		VIEW_ERROR("MediaPiece::MakeMonoPiece invalid piece body length " << make_tuple(pieceInfo.PieceIndex, pieceInfo.PieceType));
		assert(false);
		return MonoMediaPiecePtr();
	}
	return piece;
}


void MediaPieceUtil::PrepareSignBuffer( const MonoMediaPiece& piece )
{
	pool_byte_buffer& buf = GetBufferRef();
	buf.ensure_size( piece.GetSize() + 1 + SignatureData::static_size );
}

bool MediaPieceUtil::SignBuffer( data_output_stream& os, const MonoMediaPiece& piece, DataSignerPtr signer )
{
	pool_byte_buffer& buf = GetBufferRef();
	// 对piece数据进行签名
	SignatureData signature;
	if ( false == signer->Sign( buf.data(), piece.GetSize(), signature ) )
		return false;
	// 将签名结果附加在piece数据后面
	os.write_uint8( SignatureData::static_size );
	os.write_n( signature.data(), signature.size() );
	return true;
}


