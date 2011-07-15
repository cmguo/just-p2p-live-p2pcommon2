
#include "StdAfx.h"

#include "MediaStorage.h"
#include "StreamIndicator.h"
#include "MediaPiece.h"
#include "util/BitMap.h"
#include "framework/log.h"

#include <ppl/data/stlutils.h>
#include <ppl/util/random.h>



/// ɾ������pieceʱ����С�ο�ʱ��
const UINT STORAGE_MIN_EXPIRE_REFER_TIME = 50*1000;

/// �洢��piece������
const UINT STORAGE_PIECE_LIMIT = 30000;





bool Storage::AddDataPiece(MediaDataPiecePtr piece)
{
	CheckDataPiece(piece);
//	const BYTE* packetBuffer = packet->GetMediaData();

//	assert( packetBuffer[0] == (char)0x82 && packetBuffer[1] == (char)0 && packetBuffer[2] == (char)0 );
//	assert( ( packetBuffer[3] & (char)0x80 ) == (char)0x00 );	// no error correction
//	assert( packetBuffer[4] == (char)0x5D );					// payload length type OK

	UINT pieceIndex = piece->GetPieceIndex();
	//m_unfinishedDataPieces.Remove(pieceIndex);

	pair<PieceInfoCollection::iterator, bool> result = m_dataPieces.insert(make_pair(pieceIndex, PieceInfo(piece)));
	if (!result.second)
	{
		STREAMBUFFER_WARN("Recv an already had DataPacket, index = "<<pieceIndex);
		return false;
	}
	m_pieceSize = (UINT)piece->GetPieceLength();
	//LIMIT_MIN(m_MaxSubPieceCountPerPiece, packet->GetSubPieceCount(MAX_SUB_PIECE_DATA_SIZE));
	return true;
}

pair<bool, UINT> Storage::Delete(PieceInfoCollection::iterator iter)
{
	if (iter == m_dataPieces.end())
	{
		STREAMBUFFER_WARN("Storage::Delete cannot be found, so cann't del. ");
		return make_pair(false, 0);
	}
	return DoDelete(iter);
}

pair<bool, UINT> Storage::DoDelete(PieceInfoCollection::iterator iter)
{
	assert(iter != m_dataPieces.end());
	MediaDataPiecePtr piece = iter->second.GetPiece();
	UINT pieceIndex = 0;
	if (piece)
	{
		pieceIndex = piece->GetPieceIndex();
	}
	STREAMBUFFER_INFO( "SB: Storage::DoDelete " << iter->first );
	m_dataPieces.erase(iter); //ɾ�������ж�Ӧ�ļ�¼
	return make_pair(true, pieceIndex);
}

MediaDataPiecePtr Storage::GetDataPiece(UINT pieceIndex) const
{
	PieceInfoCollection::const_iterator iter = m_dataPieces.find(pieceIndex);
	if (iter == m_dataPieces.end())
		return MediaDataPiecePtr();
	return iter->second.GetPiece();
}

PieceInfo Storage::GetPieceInfo(UINT pieceIndex) const
{
	PieceInfoCollection::const_iterator iter = m_dataPieces.find(pieceIndex);
	if (iter == m_dataPieces.end())
		return PieceInfo();
	return iter->second;
}

bool Storage::HasDataPiece(UINT pieceIndex) const
{
	return GetDataPiece(pieceIndex);
}

UINT Storage::GetMinIndex() const
{
	// ���m_dataPiecesΪ�գ��򲻻����ѭ��
	PieceInfoCollection::const_iterator itr;
	for (itr = m_dataPieces.begin(); itr != m_dataPieces.end(); itr ++)
	{
		CheckDataPiece(itr->second.GetPiece());
		return itr->first;
	}
	return 0;
}

UINT Storage::GetMaxIndex() const
{
	const PieceInfoCollection& pieces = m_dataPieces;
	PieceInfoCollection::const_reverse_iterator itr;
	for (itr = pieces.rbegin(); itr != pieces.rend(); itr ++)
	{
		CheckDataPiece(itr->second.GetPiece());
		return itr->first;
	}	
	return 0;
}

MediaDataPiecePtr Storage::GetFirst(UINT pieceIndex) const
{
	PieceInfoCollection::const_iterator iter = m_dataPieces.lower_bound(pieceIndex);
	if (iter == m_dataPieces.end())
		return MediaDataPiecePtr();
	CheckDataPiece(iter->second.GetPiece());
	return iter->second.GetPiece();
}

MediaDataPiecePtr Storage::GetNext(UINT pieceIndex) const
{
	return this->GetFirst(pieceIndex + 1);
}


size_t Storage::RemoveExpired(const StreamIndicator& baseIndicator, UINT minBufferTime)
{
	UINT64 elapsedTime = baseIndicator.GetElapsedTime();
	assert(elapsedTime <= INT_MAX);
	if (elapsedTime <= minBufferTime)
	{
		return 0;
	}
	elapsedTime -= minBufferTime;
	size_t deletedCount = 0;
	PieceInfoCollection::iterator iter = m_dataPieces.begin();
	while (iter != m_dataPieces.end())
	{
		if (m_dataPieces.size() <= 1)
			break;
		MediaDataPiecePtr piece = iter->second.GetPiece();
		CheckDataPiece(piece);
		UINT pieceIndex = iter->first;
		// ����ɾ��upperBound��Ӧ��piece
		//assert(pieceIndex < upperBound);
		// ���upperBound�Ժ�һֱû�����ݣ����߾����Զ��������һƬ������ܻᷢ��upperBound��Ӧ��ƬҲ��ʱ�����
		if (pieceIndex >= baseIndicator.GetPieceIndex())
		{
			// peer�˵�BaseIndex��һ������С�ģ���Ҫ���Լ��
			assert( piece->GetTimeStamp() >= baseIndicator.GetTimeStamp() );
			UINT64 elapsedTimeStamp = piece->GetTimeStamp() - baseIndicator.GetTimeStamp();
			if (elapsedTimeStamp >= elapsedTime)
			{
				SOURCENEW_DEBUG("�ƽ� MinIndex="<<GetMinIndex()<<" MaxIndex="<<GetMaxIndex()<<" �ƽ�ʱ���="<<elapsedTime);
				break;
			}
		}
		// piece���ڣ�ɾ��֮
		STREAMBUFFER_WARN("Storage::RemoveExpired: " << make_tuple(pieceIndex, baseIndicator.GetPieceIndex()));
		DoDelete(iter++);
		++deletedCount;
	}
	while (m_dataPieces.size() > STORAGE_PIECE_LIMIT)
	{
		DoDelete(m_dataPieces.begin());
	}

	//m_unfinishedDataPieces.RemoveOld(GetMinIndex());
	return deletedCount;
}



size_t Storage::RemoveExpired(const StreamIndicator& baseIndicator, UINT minBufferTime, UINT upperBound, UINT lowerBound)
{
	assert(upperBound >= lowerBound);
	size_t deletedCount = 0;
	// ���� base ���������ɾ��ʱ��
	UINT64 elapsedTime = baseIndicator.GetElapsedTime();
	UINT64 elapsedTimeStamp = 0;
	assert(elapsedTime <= INT_MAX);
	if (elapsedTime > minBufferTime)
	{
		// ���� elapsedTime ���� �����ƽ�ʱ��� - 70s
		elapsedTime -= minBufferTime;

		// ���� skipIndex ���������ɾ��ʱ��
		UINT skipIndex = upperBound;
		if( skipIndex == 0 )
		{
			return 0;
		}
		PieceInfoCollection::iterator skipIter = m_dataPieces.find(skipIndex);
		assert( skipIter != m_dataPieces.end());
		MediaDataPiecePtr skipPiece = skipIter->second.GetPiece();
		CheckDataPiece(skipPiece);
		if( skipPiece->GetTimeStamp() - baseIndicator.GetTimeStamp() <= STORAGE_MIN_EXPIRE_REFER_TIME)
		{
			return 0;
		}

		// ���� elapsedTimeForSkip ���� SkipIndexʱ��� - 50s
		UINT64 elapsedTimeForSkip = skipPiece->GetTimeStamp() - baseIndicator.GetTimeStamp() - STORAGE_MIN_EXPIRE_REFER_TIME;
		// ��ʱ elapsedTime ���� min(�����ƽ�ʱ��� - 70s, SkipIndexʱ��� - 50s)
		elapsedTime = min( elapsedTime, elapsedTimeForSkip);

		PieceInfoCollection::iterator iter = m_dataPieces.begin();
		while (iter != m_dataPieces.end())
		{
			MediaDataPiecePtr piece = iter->second.GetPiece();
			CheckDataPiece(piece);
			UINT pieceIndex = iter->first;
			// ����ɾ��upperBound��Ӧ��piece
			//assert(pieceIndex < upperBound);
			// ���upperBound�Ժ�һֱû�����ݣ����߾����Զ��������һƬ������ܻᷢ��upperBound��Ӧ��ƬҲ��ʱ�����
			if (pieceIndex > upperBound)
				break;
			if (pieceIndex >= baseIndicator.GetPieceIndex())
			{
				// peer�˵�BaseIndex��һ������С�ģ���Ҫ���Լ��
				elapsedTimeStamp = piece->GetTimeStamp() - baseIndicator.GetTimeStamp();
				if (elapsedTimeStamp >= elapsedTime)
				{
					break;
				}
			}
			// piece���ڣ�ɾ��֮
			STREAMBUFFER_WARN("Storage::RemoveExpired: " << make_tuple(baseIndicator.GetPieceIndex(), pieceIndex, piece->GetTimeStamp()));
			DoDelete(iter++);
			++deletedCount;
			if (m_dataPieces.size() < 400)
			{
			//	assert(false);
			}
		}
	}

	// �˴������֮��ģ���Ϊ�˻ر�һ���鲻������BUG�����
	while (m_dataPieces.size() > STORAGE_PIECE_LIMIT)
	{
		DoDelete(m_dataPieces.begin());
		++deletedCount;
	}
	while (!m_dataPieces.empty())
	{
		// ���˵���upperBound���̫Զ��piece
		UINT tempPieceIndex = m_dataPieces.begin()->first;
		if (tempPieceIndex >= lowerBound)
			break;
		DoDelete(m_dataPieces.begin());
		++deletedCount;
	}
#if defined(_WIN32_WCE)
#pragma message("------wince�����ƻ��������������Ƭ��Ϊ600")
	while (m_dataPieces.size() > 600)
	{
		DoDelete(m_dataPieces.begin());
		++deletedCount;
	}
#endif

//	m_unfinishedDataPieces.RemoveOld(GetMinIndex());
	return deletedCount;
}

size_t Storage::CountPieces(UINT rangeMin, UINT rangeMax) const
{
	return maps::count_range(m_dataPieces, rangeMin, rangeMax);
}

PieceInfo Storage::GetFirstPiece() const
{
	if (m_dataPieces.empty())
		return PieceInfo();
	return m_dataPieces.begin()->second;
}

MediaDataPiecePtr Storage::GetLastPiece() const
{
	if (m_dataPieces.empty())
		return MediaDataPiecePtr();
	return m_dataPieces.rbegin()->second.GetPiece();
}

UINT Storage::GetBufferSize() const
{
	if (m_dataPieces.empty())
		return 0;
	return GetMaxIndex() - GetMinIndex() + 1;
}
UINT Storage::GetBufferTime() const
{
	MediaDataPiecePtr firstPiece = GetFirstPiece().GetPiece();
	MediaDataPiecePtr lastPiece = GetLastPiece();
	if ( ! firstPiece || ! lastPiece )
		return 0;
	return static_cast<UINT>( lastPiece->GetTimeStamp() - firstPiece->GetTimeStamp() );
}

BitMap Storage::BuildBitmap() const
{
	if (m_dataPieces.empty())
		return BitMap();
	UINT minIndex = GetMinIndex();
	UINT maxIndex = GetMaxIndex();
	if ((minIndex == 0) && (maxIndex == 0))
		return BitMap();
	assert(minIndex > 0);
	assert(minIndex <= maxIndex);
	UINT length = maxIndex - minIndex + 1;
	const PieceInfoCollection& pieceColl = m_dataPieces;
	PieceInfoCollection::const_iterator BeginItr = pieceColl.find(minIndex);
	PieceInfoCollection::const_iterator EndItr   = pieceColl.find(maxIndex);

	BitMap bitmap(minIndex, length);
// 	for(int i = 0; i < length; i ++ )
// 	{
// 		bitmap.SetBit(minIndex + i);
// 	}
	
	//2.����Bitmap
	for (PieceInfoCollection::const_iterator itr = BeginItr; itr != EndItr; itr++ )
	{
		assert(bitmap.IsInRange(itr->first));
		bitmap.SetBit(itr->first);
	}

	return bitmap;
}

BitMap Storage::BuildBitmap(UINT maxLength) const
{
	assert(maxLength > 0);
	if (IsEmpty())
		return BitMap();
	UINT minIndex = GetMinIndex();
	UINT maxIndex = GetMaxIndex();
	if ((minIndex == 0) && (maxIndex == 0))
		return BitMap();
	assert(minIndex > 0);
	assert(minIndex <= maxIndex);
	UINT length = maxIndex - minIndex + 1;
	LIMIT_MAX(length, maxLength);
	maxIndex = minIndex + length - 1;
	const PieceInfoCollection& pieceColl = m_dataPieces;
	PieceInfoCollection::const_iterator BeginItr = pieceColl.find(minIndex);
	PieceInfoCollection::const_iterator EndItr   = pieceColl.upper_bound(maxIndex);

	BitMap bitmap(minIndex, length);
	//2.����Bitmap
	for (PieceInfoCollection::const_iterator itr = BeginItr; itr != EndItr; itr++ )
	{
		assert(bitmap.IsInRange(itr->first));
		bitmap.SetBit(itr->first);
	}
	return bitmap;
}

void Storage::Serialize() const
{
	SOURCENEW_INFO( "Storage: Size=" << m_dataPieces.size() );
	for(PieceInfoCollection::const_iterator itr = m_dataPieces.begin(); itr != m_dataPieces.end(); itr ++)
	{
		const PieceInfo& piece = itr->second;
                (void)piece;
		SOURCENEW_INFO( "Storage: Piece Index=" << piece.GetPiece()->GetPieceIndex() << " TimeStamp=" << piece.GetPiece()->GetTimeStamp() << " PieceLength=" << piece.GetPiece()->GetPieceLength() << " HeaderIndex=" << piece.GetPiece()->GetHeaderPiece() );
	}
	SOURCENEW_INFO( "Storage: Serialize End" );
}






Storage::Storage() : m_pieceSize(5600)
{
}

Storage::~Storage()
{
	STREAMBUFFER_INFO("Storage::~Storage");
	Clear();
}

void Storage::Clear()
{
	m_dataPieces.clear();
	//m_unfinishedDataPieces.Clear();
	// ����Ҫ���headerƬ
//	m_headers.clear();
}


MediaHeaderPiecePtr Storage::GetHeader(UINT pieceIndex) const
{
	return ptr_maps::find(m_headers, pieceIndex);
}

SubMediaPiecePtr Storage::GetSubPiece(UINT pieceIndex, UINT8 subPieceIndex) const
{
	//SubMediaPiecePtr subPiece = m_unfinishedDataPieces.GetSubPiece(pieceIndex, subPieceIndex);
	//if ( subPiece )
	//	return subPiece;
	SubMediaPiecePtr subPiece;
	PieceInfo pieceInfo = this->GetPieceInfo(pieceIndex);
	if (pieceInfo.IsValid())
	{
		// ��piece����ȡsub piece
		subPiece = pieceInfo.GetSubPiece(subPieceIndex);
		//assert( subPiece );
	}
	else
	{
		// ���headers
		//subPiece = m_unfinishedHeaderPieces.GetSubPiece(pieceIndex, subPieceIndex);
		//if ( subPiece )
		//	return subPiece;
		MediaHeaderPiecePtr headerPiece = GetHeader(pieceIndex);
		if ( headerPiece )
		{
			subPiece = SubMediaPiecePtr(headerPiece->GetSubPiece(subPieceIndex));
			//assert( subPiece );
		}
	}
	return subPiece;
}

SubMediaPiecePtr Storage::GetSubPiece(UINT64 inTS) const
{	
	SubMediaPiecePtr subPiece;
	if (m_dataPieces.size() > 2)
	{
		MediaDataPiecePtr minPiece = m_dataPieces.begin()->second.GetPiece();
		MediaDataPiecePtr maxPiece = m_dataPieces.rbegin()->second.GetPiece();
		if (inTS >= minPiece->GetTimeStamp() && inTS <= maxPiece->GetTimeStamp())
		{
			double indexDurationOf1s = (double)(maxPiece->GetPieceIndex() - minPiece->GetPieceIndex()) / (double)(maxPiece->GetTimeStamp() - minPiece->GetTimeStamp()) ;
			UINT32 iterIndex = minPiece->GetPieceIndex() + indexDurationOf1s * (inTS - minPiece->GetTimeStamp());

			LIMIT_MIN_MAX(iterIndex, minPiece->GetPieceIndex(), maxPiece->GetPieceIndex());
			PieceInfoCollection::const_iterator iter = m_dataPieces.lower_bound(iterIndex);

			assert( iter != m_dataPieces.end() );
			while (iter != m_dataPieces.begin() && inTS < iter->second.GetPiece()->GetTimeStamp())
			{
				iter--;
			}
			while (iter != m_dataPieces.end() && inTS > iter->second.GetPiece()->GetTimeStamp())
			{
				iter++;
			}	
			
			if (iter != m_dataPieces.end())
			{
				RandomGenerator random;
				subPiece = iter->second.GetSubPiece(random.Next() % iter->second.GetSubPieceCount());
			}
		}
	}

	return subPiece;
}


bool Storage::HasHeader(UINT pieceIndex) const
{
	return GetHeader(pieceIndex);
}

size_t Storage::RemoveOldHeaders(size_t maxCount, UINT upperBound)
{
	size_t deletedCount = 0;
	while( m_headers.size() > maxCount )
	{
		MediaHeaderPieceCollection::iterator iter = m_headers.begin();
		if( iter->first < upperBound )
		{
			m_headers.erase( iter->first );
			++deletedCount;
		}
		else
		{
			STREAMBUFFER_INFO("New MediaHeader Index is less than older headers, this is abnormal " << make_pair(iter->first, upperBound));
			break;
		}
	}
	return deletedCount;
}

bool Storage::AddHeaderPiece(MediaHeaderPiecePtr piece)
{
	CheckHeaderPiece(piece);
	STREAMBUFFER_INFO("Recv an MediaHeaderPacket, index = " << piece->GetPieceIndex());

	// ��������ͬһ��ͷ��
	if( containers::contains(m_headers, piece->GetPieceIndex()) )
	{
		STREAMBUFFER_INFO( " Header "<<piece->GetPieceIndex()<<" is existed!" );
		return false;
	}
	// �����µ�
	m_headers[piece->GetPieceIndex()] = piece;
	return true;
}


MediaPiecePtr Storage::GetMediaPiece(UINT pieceIndex) const
{
	MediaDataPiecePtr piece = GetDataPiece(pieceIndex);
	if ( piece )
	{
		return piece;
	}
	return GetHeader(pieceIndex);
}

void Storage::ViewStreamBuffer() const
{
	VIEW_INFO("Storage::ViewStreamBuffer - LocalBitmap " << GetMinIndex() << " " << GetMaxIndex() << " " << BuildBitmap().GetResourceString() << " End");
}

size_t Storage::CountRange( UINT refIndex ) const
{
	return maps::count_range(this->m_dataPieces, this->GetMinIndex(), refIndex);
}



#ifdef _PPL_RUN_TEST

#include <ppl/util/test_case.h>


class MediaDataStorageTestCase : public ppl::util::test_case
{
	virtual void DoRun()
	{
/*		Storage stg;
		assert(stg.GetPieceCount() == 0);

		string dataPacketHeader = "11111";
#pragma warning(disable:4309)
		dataPacketHeader[0] = 0x82;
#pragma warning(default:4309)
		dataPacketHeader[1] = 0;
		dataPacketHeader[2] = 0;
		dataPacketHeader[3] = 9;
		dataPacketHeader[4] = 0x5d;

		//bool success = stg.AddRequest(1, 0);
		//assert(success);
		//assert(stg.GetSize() == 1);
		string data1 = dataPacketHeader + "4444";
		PPMediaDataPacketPtr piece1(new PPMediaDataPacket(2342, 23420, 3, data1.size(), data1.data()));
		bool success = stg.AddDataPiece(piece1);
		assert(success);
		assert(stg.GetPieceCount() == 1);
		assert(stg.GetDataPiece(2342) == piece1);
		assert(stg.GetFirst(2) == piece1);
		assert(stg.GetNext(2) == piece1);
		assert(stg.GetFirst(2342) == piece1);
		assert(!stg.GetNext(2342));
		assert(!stg.GetFirst(2343));

		PPMediaDataPacketPtr piece2(new PPMediaDataPacket(3, 20, 2, data1.size(), data1.data()));
		PPMediaHeaderPacketPtr headerPiece(new PPMediaHeaderPacket(112, PPMT_MEDIA_WMF2, 0, NULL, 4, "5555", 0, NULL));
		success = stg.AddDataPiece(piece2);
		assert(success);
		assert(stg.GetPieceCount() == 2);
		assert(stg.GetDataPiece(3) == piece2);
		assert(stg.GetFirst(3) == piece2);
		assert(stg.GetNext(3) == piece1);
		assert(stg.GetSubPieceCount(3) == 1);
		assert(stg.GetPossibleSubPieceCount(3, 0) == 1);
		assert(stg.GetSubPieceCount(132) == 0);
		assert(stg.GetPossibleSubPieceCount(132, 0) == 1);

		success = stg.AddHeaderPiece(headerPiece);
		assert(success);
		assert(stg.GetPieceCount() == 2);
		assert(stg.GetHeader(112) == headerPiece);
		assert(stg.GetBufferSize() == 2342 - 2 + 1);
		assert(stg.GetBufferTime() == 23420 - 20);
		assert(stg.GetMinIndex() == 2);
		assert(stg.GetMaxIndex() == 2342);
*/
/*		assert(stg.CountPieces(5, 500) == 0);
		assert(stg.CountPieces(4, 2341) == 0);
		assert(stg.CountPieces(3, 2341) == 1);
		assert(stg.CountPieces(2, 2341) == 1);
		assert(stg.CountPieces(1, 2341) == 1);
		assert(stg.CountPieces(1, 2342) == 2);
		assert(stg.CountPieces(1, 5000) == 2);

		success = stg.AddDataPiece(piece2);
		assert(!success);
		assert(stg.GetPieceCount() == 2);

		assert(!stg.GetDataPiece(0));
		assert(!stg.GetDataPiece(1));
		assert(!stg.GetDataPiece(2));
		assert(stg.GetDataPiece(3));
		assert(!stg.GetDataPiece(4));

		assert(!stg.HasDataPiece(0));
		assert(!stg.HasDataPiece(1));
		assert(!stg.HasDataPiece(2));
		assert(stg.HasDataPiece(3));
		assert(!stg.HasDataPiece(4));
		assert(stg.HasDataPiece(112) == false);
	//	assert(stg.HasHeader(112) == true);

//		SubMediaPiecePtr subPiece0(new SubMediaPiece(145, 233, 112, 3, 0, 1, "1"));
//		SubMediaPiecePtr subPiece1(new SubMediaPiece(145, 233, 112, 3, 1, 2, "22"));
//		SubMediaPiecePtr subPiece2(new SubMediaPiece(145, 233, 112, 3, 2, 3, "333"));

		{
			Storage stg;
			pair<bool, PPMediaDataPacketPtr> result = stg.AddSubPiece(subPiece2);
			assert(result.first == true);
			assert(! result.second);
			assert(stg.GetUnfinished().HasSubPiece(145) == true);
			assert(stg.GetUnfinished().HasSubPiece(145, 2) == true);
			assert(stg.GetUnfinished().HasSubPiece(145, 1) == false);
			assert(stg.GetUnfinished().HasSubPiece(145, 0) == false);
			assert(stg.GetUnfinished().HasSubPiece(145, 3) == false);
			assert(stg.GetUnfinished().GetSubPiece(145, 2) == subPiece2);
			vector<UINT8> interests;
			stg.GetUnfinished().GetInterested(145, interests);
			assert(interests.size() == 2);
			assert(interests[0] == 0);
			assert(interests[1] == 1);

			result = stg.AddSubPiece(subPiece2);
			assert(result.first == false);
			assert(!result.second);

			result = stg.AddSubPiece(subPiece0);
			assert(result.first == true);
			assert(!result.second);
			assert(stg.GetUnfinished().HasSubPiece(145) == true);
			assert(stg.GetUnfinished().HasSubPiece(145, 1) == false);
			assert(stg.GetUnfinished().HasSubPiece(145, 2) == true);
			assert(stg.GetUnfinished().HasSubPiece(145, 0) == true);
			assert(stg.GetUnfinished().HasSubPiece(145, 3) == false);
			assert(stg.GetUnfinished().HasSubPiece(145) == false);
			stg.GetUnfinished().GetInterested(145, interests);
			assert(interests.size() == 1);
			assert(interests[0] == 1);


			result = stg.AddSubPiece(subPiece1);
			assert(result.first == true);
			assert(result.second);
			assert(stg.GetUnfinished().HasSubPiece(145) == false);
			assert(stg.GetUnfinished().HasSubPiece(145, 1) == false);
			assert(stg.GetUnfinished().HasSubPiece(145, 2) == false);
			assert(stg.GetUnfinished().HasSubPiece(145, 0) == false);
			assert(stg.GetUnfinished().HasSubPiece(145, 3) == false);
			assert(stg.GetUnfinished().HasSubPiece(145) == false);
			stg.GetUnfinished().GetInterested(145, interests);
			assert(interests.size() == 0);

			assert(result.second->GetMediaDataLength() == 6);
			assert(memcmp(result.second->GetMediaData(), "122333", 6) == 0);
		}

		stg.Clear();
		assert(stg.GetPieceCount() == 0);

		piece2.reset();
		piece1.reset();
*/

	}
};

CPPUNIT_TEST_SUITE_REGISTRATION(MediaDataStorageTestCase);



#endif

