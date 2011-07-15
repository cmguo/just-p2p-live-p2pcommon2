
#include "StdAfx.h"

#include "MediaPiece.h"
#include "MediaPieceUtil.h"
#include "framework/log.h"

#include <synacast/protocol/DataSigner.h>
#include <synacast/protocol/PacketStream.h>
#include <synacast/protocol/SubMediaPiece.h>
#include <synacast/protocol/MonoMediaPiece.h>
#include <synacast/protocol/SignatureData.h>
#include <synacast/protocol/data/MediaPieceInfoIO.h>
#include <synacast/protocol/MonoMediaPieceIO.h>
#include <ppl/data/stlutils.h>
//#include <boost/foreach.hpp>

BOOST_STATIC_ASSERT(MAX_SUB_PIECE_DATA_SIZE < 1400);


#ifdef _DEBUG
void CheckDataPiece(MediaDataPiecePtr piece)
{
	assert(piece != NULL);
	assert(piece->GetPieceType() == PPDT_MEDIA_DATA);
	assert(piece->GetPieceIndex() > piece->GetHeaderPiece());
	//assert(piece->CheckValid());
}
void CheckHeaderPiece(MediaHeaderPiecePtr piece)
{
	assert(piece != NULL);
	assert(piece->GetPieceType() == PPDT_MEDIA_HEADER);
	assert(piece->GetPieceIndex() > 0);
	//assert(piece->CheckValid());
}
#endif



UnfinishedMediaPiece::UnfinishedMediaPiece(SubMediaPiecePtr subPiece)
{
	CheckSubPiece(subPiece);
	m_totalCount = subPiece->GetSubPieceCount();
	assert(m_totalCount < 64 && m_totalCount > 0);
	assert(subPiece->GetSubPieceIndex() < subPiece->GetSubPieceCount());
	m_subPieces[subPiece->GetSubPieceIndex()] = subPiece;
	m_pieceInfo = subPiece->GetPieceInfo();
	m_pieceLevel = subPiece->GetPieceLevel();
}

UnfinishedMediaPiece::~UnfinishedMediaPiece()
{
}

bool UnfinishedMediaPiece::IsFinished() const
{
	return m_subPieces.size() == m_totalCount;
}

bool UnfinishedMediaPiece::Contains(UINT8 subPieceIndex) const
{
	return m_subPieces.find(subPieceIndex) != m_subPieces.end();
}

SubMediaPiecePtr UnfinishedMediaPiece::GetSubPiece(UINT8 subPieceIndex) const
{
	return ptr_maps::find(m_subPieces, subPieceIndex);
}

bool UnfinishedMediaPiece::AddSubPiece(SubMediaPiecePtr subPiece)
{
	//VIEW_INFO("UnfinishedMediaDataPiece::AddSubPiece  ");
	CheckSubPiece(subPiece);
	assert(!m_subPieces.empty());
//	assert(subPiece->GetPieceType() == PPDT_MEDIA_DATA);
	//UINT pieceIndex = subPiece->GetPieceIndex();
	UINT8 subPieceIndex = subPiece->GetSubPieceIndex();
	if (m_totalCount != subPiece->GetSubPieceCount())
	{
		APP_ERROR("UnfinishedMediaDataPiece::AddSubPiece invalid total count " << subPiece->GetSubPieceCount());
		return false;
	}
	if (subPieceIndex >= subPiece->GetSubPieceCount())
	{
//		APP_ERROR("UnfinishedMediaDataPiece::AddSubPiece invalid sub piece index " << m_pieceIndex);
		return false;
	}
	if ( m_pieceInfo != subPiece->GetPieceInfo() )
	{
		APP_ERROR("UnfinishedMediaDataPiece::AddSubPiece invalid other piece info " << m_pieceInfo.PieceIndex);
		return false;
	}
	assert(subPieceIndex < m_totalCount);
	if (containers::contains(m_subPieces, subPieceIndex))
	{
		UDPT_ERROR("Storage::AddSubPiece subpiece already exists " << make_tuple(pieceIndex, m_totalCount, subPieceIndex));
		return false;
	}
	m_subPieces[subPieceIndex] = subPiece;
	return true;
}



MediaPiece::MediaPiece( UnfinishedMediaPiecePtr subPieces ) : m_pieceLevel( 0 )
{
	assert(subPieces->IsFinished());
	assert(subPieces->GetTotalCount() > 0);
	assert(subPieces->GetTotalCount() < 250);
	m_subPieces.resize(subPieces->GetTotalCount());
	STL_FOR_EACH_CONST(SubPieceCollection, subPieces->GetSubPieces(), iter)
	{
		assert(iter->first < m_subPieces.size());
		m_subPieces[iter->first] = iter->second;
	}
}

MediaPiece::MediaPiece( MonoMediaPiecePtr monoPiece, const pool_byte_buffer& pieceData ) : m_pieceLevel( monoPiece->GetPieceLevel() )
{
	assert( pieceData.size() > MediaPieceInfo::object_size );
	size_t slicedDataLen = pieceData.size() - MediaPieceInfo::object_size;
	size_t subPieceCount = ( slicedDataLen + MAX_SUB_PIECE_DATA_SIZE - 1) / MAX_SUB_PIECE_DATA_SIZE;
	assert(subPieceCount <= 250);

	assert(subPieceCount <= 64);
	LIMIT_MAX( subPieceCount, 250 );
	m_subPieces.clear();
	m_subPieces.resize( subPieceCount );
	for (UINT8 subPieceIndex = 0; subPieceIndex < subPieceCount; ++subPieceIndex)
	{
		size_t startPos = MAX_SUB_PIECE_DATA_SIZE * subPieceIndex;
		size_t len = MAX_SUB_PIECE_DATA_SIZE;
		assert( startPos < slicedDataLen );
		if ( startPos >= slicedDataLen )
		{
			break;
		}
		if ( startPos + MAX_SUB_PIECE_DATA_SIZE > slicedDataLen )
		{
			len = slicedDataLen - startPos;
		}
		m_subPieces[subPieceIndex].reset( new SubMediaPiece( monoPiece->GetPieceInfo(), m_pieceLevel, subPieceCount, subPieceIndex, len, pieceData.data() + MediaPieceInfo::object_size + startPos ) );
	}
}

MediaPiece::~MediaPiece()
{

}

bool MediaPiece::InitBase()
{
	if ( m_subPieces.empty() )
	{
		assert(false);
		return false;
	}

	size_t totallen = 0;
	for (size_t index = 0; index < m_subPieces.size(); ++index)
	{
		assert(m_subPieces[index]);
		totallen += m_subPieces[index]->SubPieceData.size();
	}

	const SubMediaPiece& subPiece = *m_subPieces[0];
	assert(subPiece.GetPieceType() == PPDT_MEDIA_DATA || subPiece.GetPieceType() == PPDT_MEDIA_HEADER);
	m_pieceInfo = subPiece.GetPieceInfo();
	m_pieceLevel = subPiece.GetPieceLevel();
	if ( totallen != m_pieceInfo.PieceLength + 1 + SignatureData::static_size )
	{
		assert( false );
		return false;
	}
	return true;
}




MediaDataPiece::MediaDataPiece( UnfinishedMediaPiecePtr subPieces ) : MediaPiece( subPieces )
{
}

MediaDataPiece::MediaDataPiece( MonoMediaDataPiecePtr monoPiece, const pool_byte_buffer& pieceData ) : MediaPiece( monoPiece, pieceData )
{
	assert(monoPiece->GetPieceIndex() > monoPiece->GetHeaderPiece());
}

bool MediaDataPiece::Init()
{
	if ( false == InitBase() )
		return false;
	assert( m_subPieces.size() > 0 );
	const SubMediaPiece& subPiece = *m_subPieces[0];
	if ( subPiece.GetPieceType() != PPDT_MEDIA_DATA )
	{
		assert( false );
		return false;
	}
	PacketInputStream is( subPiece.SubPieceData.data(), subPiece.SubPieceData.size() );
	return ( is >> m_dataPieceInfo ) && ( MediaDataPieceInfo::object_size + m_dataPieceInfo.MediaDataLength == m_pieceInfo.PieceLength );
}

MonoMediaDataPiecePtr MediaDataPiece::ToMonoPiece() const
{
	if (m_pieceInfo.PieceType != PPDT_MEDIA_DATA)
	{
		VIEW_ERROR("MediaPiece::MakeDataPacket unrecognized piece type " << m_pieceInfo.PieceType);
		assert(false);
		return MonoMediaDataPiecePtr();
	}
	return boost::static_pointer_cast<MonoMediaDataPiece>( MediaPieceUtil::MakeMonoPiece( *this ) );
}

MediaDataPiecePtr MediaDataPiece::FromMonoPiece( MonoMediaDataPiecePtr monoPiece, DataSignerPtr signer )
{
	if ( false == MediaPieceUtil::Sign( *monoPiece, signer ) )
		return MediaDataPiecePtr();

	MediaDataPiecePtr piece( new MediaDataPiece( monoPiece, MediaPieceUtil::GetBuffer() ) );
	if ( false == piece->Init() )
	{
		assert(false);
		return MediaDataPiecePtr();
	}
	return piece;
}

MediaDataPiecePtr MediaDataPiece::FromSubPieces( UnfinishedMediaPiecePtr subPieces, DataSignerPtr signer )
{
	if ( false == subPieces->IsFinished() )
		return MediaDataPiecePtr();
	if ( PPDT_MEDIA_DATA != subPieces->GetPieceType() )
	{
		VIEW_ERROR("MediaDataPiece::FromSubPieces invalid piece type " << subPieces->GetPieceType());
		assert( false );
		return MediaDataPiecePtr();
	}
	MediaDataPiecePtr piece( new MediaDataPiece( subPieces ) );
	if ( false == piece->Init() )
	{
		assert(false);
		return MediaDataPiecePtr();
	}
	//if ( false == MediaPieceUtil::Verify( *piece, signer ) )
	//	return MediaDataPiecePtr();
	return piece;
}

MediaHeaderPiece::MediaHeaderPiece( UnfinishedMediaPiecePtr subPieces ) : MediaPiece( subPieces )
{
}

MediaHeaderPiece::MediaHeaderPiece( MonoMediaHeaderPiecePtr monoPiece, const pool_byte_buffer& pieceData ) : MediaPiece( monoPiece, pieceData )
{
}

bool MediaHeaderPiece::Init()
{
	if ( false == InitBase() )
		return false;
	assert( m_subPieces.size() > 0 );
	const SubMediaPiece& subPiece = *m_subPieces[0];
	if ( subPiece.GetPieceType() != PPDT_MEDIA_HEADER )
	{
		assert( false );
		return false;
	}
	PacketInputStream is( subPiece.SubPieceData.data(), subPiece.SubPieceData.size() );
	return ( is >> m_headerPieceInfo ) 
		&& ( MediaHeaderPieceInfo::object_size + m_headerPieceInfo.InfoLength + m_headerPieceInfo.HeaderLength + m_headerPieceInfo.ProfileLength == m_pieceInfo.PieceLength );
}

MonoMediaHeaderPiecePtr MediaHeaderPiece::ToMonoPiece() const
{
	if (m_pieceInfo.PieceType != PPDT_MEDIA_HEADER)
	{
		VIEW_ERROR("MediaHeaderPiece::ToMonoPiece MakeHeaderPiece piece type " << m_pieceInfo.PieceType);
		assert(false);
		return MonoMediaHeaderPiecePtr();
	}
	return boost::static_pointer_cast<MonoMediaHeaderPiece>( MediaPieceUtil::MakeMonoPiece( *this ) );
}

MediaHeaderPiecePtr MediaHeaderPiece::FromMonoPiece( MonoMediaHeaderPiecePtr monoPiece, DataSignerPtr signer )
{
	if ( false == MediaPieceUtil::Sign( *monoPiece, signer ) )
		return MediaHeaderPiecePtr();

	MediaHeaderPiecePtr piece( new MediaHeaderPiece( monoPiece, MediaPieceUtil::GetBuffer() ) );
	if ( false == piece->Init() )
	{
		assert(false);
		return MediaHeaderPiecePtr();
	}
	return piece;
}

MediaHeaderPiecePtr MediaHeaderPiece::FromSubPieces( UnfinishedMediaPiecePtr subPieces, DataSignerPtr signer )
{
	if ( false == subPieces->IsFinished() )
		return MediaHeaderPiecePtr();
	if ( PPDT_MEDIA_HEADER != subPieces->GetPieceType() )
	{
		VIEW_ERROR("MediaHeaderPiece::FromSubPieces invalid piece type " << subPieces->GetPieceType());
		assert( false );
		return MediaHeaderPiecePtr();
	}
	MediaHeaderPiecePtr piece( new MediaHeaderPiece( subPieces ) );
	if ( false == piece->Init() )
	{
		assert(false);
		return MediaHeaderPiecePtr();
	}
	if ( false == MediaPieceUtil::Verify( *piece, signer ) )
		return MediaHeaderPiecePtr();
	return piece;
}






#ifdef _PPL_RUN_TEST

#include <ppl/util/random.h>
#include <ppl/util/test_case.h>

class UnfinishedMediaDataPieceTestCase : public ppl::util::test_case
{
	virtual void DoRun()
	{
		const size_t data_size = 5 * MAX_SUB_PIECE_DATA_SIZE + 600;
		string data(data_size, '1');
		MonoMediaDataPiecePtr monoPiece(new MonoMediaDataPiece(1123, 234, 345, data.size(), data.data()));
		vector<BYTE> key(32, 5);
		DataSignerPtr signer( new DataSigner( key ) );
		MediaDataPiecePtr subpiece = MediaDataPiece::FromMonoPiece( monoPiece, signer );
		assert( subpiece->GetSubPieceCount() == 6 );

		MediaPieceInfo pieceInfo = monoPiece->GetPieceInfo();
		pieceInfo.PieceIndex++;
		SubMediaPiecePtr subPieceError1(new SubMediaPiece(pieceInfo, 2, 3, 0, 2, (const BYTE*)"22"));
		SubMediaPiecePtr subPieceError2(new SubMediaPiece(pieceInfo, 2, 3, 0, 3, (const BYTE*)"333"));
		SubMediaPiecePtr subPieceError3(new SubMediaPiece(pieceInfo, 2, 3, 0, 4, (const BYTE*)"4444"));
		SubMediaPiecePtr subPieceError4(new SubMediaPiece(pieceInfo, 2, 33, 0, 5, (const BYTE*)"55555"));

		UnfinishedMediaPiecePtr piecePtr( new UnfinishedMediaPiece( subpiece->GetSubPiece( 1 ) ) );
		UnfinishedMediaPiece& piece = *piecePtr;
		assert(!piece.IsFinished());
		assert(piece.GetTotalCount() == 6);
		assert(piece.GetReceivedCount() == 1);
		assert(piece.Contains(1));
		assert(!piece.Contains(0));
		assert(!piece.Contains(2));
		assert(!piece.Contains(3));
		assert(!piece.GetSubPiece(0));
		assert(!piece.GetSubPiece(2));
		assert(!piece.GetSubPiece(3));
		assert(piece.GetSubPiece(1) == subpiece->GetSubPiece( 1 ));
		assert( ! MediaDataPiece::FromSubPieces( piecePtr, DataSignerPtr() ) );
		assert( ! MediaHeaderPiece::FromSubPieces( piecePtr, DataSignerPtr() ) );

		assert(piece.AddSubPiece( subpiece->GetSubPiece( 0 ) ));
		assert(!piece.IsFinished());
		assert(piece.GetTotalCount() == 6);
		assert(piece.GetReceivedCount() == 2);
		assert(piece.Contains(1));
		assert(piece.Contains(0));
		assert(!piece.Contains(2));
		assert(piece.GetSubPiece(0) == subpiece->GetSubPiece( 0 ));
		assert(!piece.GetSubPiece(2));
		assert(piece.GetSubPiece(1) == subpiece->GetSubPiece( 1 ));
		assert( ! MediaDataPiece::FromSubPieces( piecePtr, DataSignerPtr() ) );
		assert( ! MediaHeaderPiece::FromSubPieces( piecePtr, DataSignerPtr() ) );

		assert(!piece.AddSubPiece( subpiece->GetSubPiece( 0 ) ));
		assert(piece.GetReceivedCount() == 2);

		assert(!piece.AddSubPiece(subPieceError1));
		assert(piece.GetReceivedCount() == 2);
		assert(!piece.AddSubPiece(subPieceError2));
		assert(piece.GetReceivedCount() == 2);
		assert(!piece.AddSubPiece(subPieceError3));
		assert(piece.GetReceivedCount() == 2);
		assert(!piece.AddSubPiece(subPieceError4));
		assert(piece.GetReceivedCount() == 2);

		assert(piece.AddSubPiece( subpiece->GetSubPiece( 2 ) ));
		assert(piece.AddSubPiece( subpiece->GetSubPiece( 3 ) ));
		assert(piece.AddSubPiece( subpiece->GetSubPiece( 4 ) ));
		assert(piece.AddSubPiece( subpiece->GetSubPiece( 5 ) ));
		assert(piece.IsFinished());
		assert(piece.GetTotalCount() == 6);
		assert(piece.GetReceivedCount() == 6);
		assert(piece.Contains(1));
		assert(piece.Contains(0));
		assert(piece.Contains(2));
		assert(piece.Contains(3));
		assert(piece.Contains(4));
		assert(piece.Contains(5));
		assert(piece.GetSubPiece(0) == subpiece->GetSubPiece( 0 ));
		assert(piece.GetSubPiece(2) == subpiece->GetSubPiece( 2 ));
		assert(piece.GetSubPiece(1) == subpiece->GetSubPiece( 1 ));
		assert(piece.GetSubPiece(3) == subpiece->GetSubPiece( 3 ));
		assert(piece.GetSubPiece(4) == subpiece->GetSubPiece( 4 ));
		assert(piece.GetSubPiece(5) == subpiece->GetSubPiece( 5 ));

		{
			MediaDataPiecePtr dataPiece = MediaDataPiece::FromSubPieces( piecePtr, signer );
			//MediaHeaderPiecePtr headerPiece = MediaHeaderPiece::FromSubPieces( piecePtr, signer );
			//assert( ! headerPiece );

			assert(dataPiece);
			assert(dataPiece->GetPieceIndex() == 1123);
			assert(dataPiece->GetTimeStamp() == 234);
			assert(dataPiece->GetHeaderPiece() == 345);
			assert(dataPiece->GetMediaDataLength() == data_size);

			MonoMediaDataPiecePtr monoDataPiece = dataPiece->ToMonoPiece();
			assert(memcmp(monoDataPiece->GetMediaData(), data.data(), data_size) == 0);
			assert( monoDataPiece->HeaderPiece == 345 );
			assert( monoDataPiece->TimeStamp == 234 );
			assert( monoDataPiece->PieceIndex == 1123 );
			assert( monoDataPiece->GetMediaDataLength() == data_size );
		}
	}
};


class PieceInfoTestCase : public ppl::util::test_case
{
	virtual void DoRun()
	{
		string data(5 * 1024 + 600, '1');
		MonoMediaDataPiecePtr piece(new MonoMediaDataPiece(1123, 234, 345, data.size(), data.data()));
		vector<BYTE> key(32, 5);
		//piece->Sign(key);
		{
		//	PieceInfo pieceInfo(piece, false);
		//	CheckPieceInfo(pieceInfo, piece);
		}
		{
		//	PieceInfo pieceInfo(piece, true);
		//	CheckPieceInfo(pieceInfo, piece);
		}
	}

	void CheckPieceInfo(const PieceInfo& pieceInfo, MonoMediaDataPiecePtr piece)
	{
		assert(pieceInfo.GetSubPieceCount() == 6);
		for (UINT8 i = 0; i < 5; ++i)
		{
			SubMediaPiecePtr subPiece = pieceInfo.GetSubPiece(i);
			assert(subPiece->SubPieceData.size() == 1024);
			assert(subPiece->GetPieceIndex() == 1123);
//			assert(subPiece->GetTimeStamp() == 234);
//			assert(subPiece->GetHeaderPiece() == 345);
		}
		SubMediaPiecePtr subPiece = pieceInfo.GetSubPiece(5);
		assert(subPiece->SubPieceData.size() == 600);
		assert(subPiece->GetPieceIndex() == 1123);
//		assert(subPiece->GetTimeStamp() == 234);
//		assert(subPiece->GetHeaderPiece() == 345);
		//assert(pieceInfo.GetMonoPiece() == piece);
		//assert(pieceInfo.GetMediaData() == piece->GetMediaData());
		assert(pieceInfo.GetMediaDataLength() == piece->GetMediaDataLength());
		assert(pieceInfo.GetSubPieceCount() == 6);
	}
};

class PieceTestCase : public ppl::util::test_case
{
	virtual void DoRun()
	{
		MonoMediaDataPiecePtr dataPiece(new MonoMediaDataPiece(112, 13, 14, 5, "hello"));
		assert(dataPiece.use_count() == 1);
		assert(dataPiece.unique());
		MonoMediaDataPiecePtr piece = dataPiece;
		assert(dataPiece.use_count() == 2);
		assert(!dataPiece.unique());
		assert(piece.use_count() == 2);
		assert(!piece.unique());
	}
};


CPPUNIT_TEST_SUITE_REGISTRATION(PieceTestCase);
CPPUNIT_TEST_SUITE_REGISTRATION(UnfinishedMediaDataPieceTestCase);
CPPUNIT_TEST_SUITE_REGISTRATION(PieceInfoTestCase);

#endif





