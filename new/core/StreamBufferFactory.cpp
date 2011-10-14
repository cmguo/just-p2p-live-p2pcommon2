
#include "StdAfx.h"

#include "StreamBufferFactory.h"

#include "common/MediaServer.h"
#include "Downloader.h"
#include "common/MediaPiece.h"
#include "common/BaseInfo.h"
#include "framework/log.h"

#include "util/BitMap.h"

#include <ppl/data/stlutils.h>
#include <ppl/data/guid_io.h>
#include <synacast/protocol/PacketStream.h>

/// streambuffer延迟的时间
const UINT STREAM_DELAY_TIME = 0 * 1000;



CStreamBuffer* StreamBufferFactory::MCCCreate()
{
	return new SourceStreamBuffer();
}

CStreamBuffer* StreamBufferFactory::PeerCreate()
{
	return new PeerStreamBuffer();
}




StreamBufferImpl::StreamBufferImpl(UINT minBufferTime) : m_lpMediaServer(NULL)
{
	APP_DEBUG("New StreamBufferImpl");
	m_State=st_none;
	m_lpMediaServer = NULL;

	m_minBufferTime = minBufferTime;
	LIVE_ASSERT(m_minBufferTime >= 30 * 1000);
	LIVE_ASSERT(m_minBufferTime <= 300 * 1000);
	//m_lpMediaServer = new CMediaServer(this, 8888);

}

StreamBufferImpl::~StreamBufferImpl()
{
	Stop();
	APP_DEBUG("Delete StreamBufferImpl");
}


void StreamBufferImpl::GenerateKey( const GUID& seed )
{
	const GUID tempData[4] = {
		{ 0xba5b7b9, 0xef3d, 0x488a, { 0xa4, 0x95, 0x85, 0xe6, 0x5f, 0x48, 0x12, 0x7f } }, 
		{ 0xeb945c67, 0xb23f, 0x4d3c, { 0x86, 0x66, 0x3c, 0x9e, 0xa8, 0x50, 0xd, 0xb5 } }, 
		{ 0x136341be, 0x7eb, 0x400c, { 0xba, 0xf, 0xf7, 0x78, 0x13, 0xb0, 0x63, 0xae } }, 
		{ 0xe4aa4a17, 0xd453, 0x4f5e, { 0xad, 0x3c, 0xe8, 0x6e, 0xf3, 0x6e, 0xf3, 0x8a } }, 
	};

	std::vector<BYTE> signKey;
	signKey.resize(sizeof(GUID) * 4);
    PacketOutputStream os(&signKey[0], signKey.size());
	for ( int i = 0; i < 4; ++i )
	{
		//memcpy(&signKey[i * sizeof(GUID)], &tempData[i], sizeof(GUID));
        os << tempData[i];
	}
    BYTE seedBytes[sizeof(GUID)];
    PacketOutputStream os2(seedBytes, sizeof(GUID));
	//const BYTE* seedBytes = reinterpret_cast<const BYTE*>(&seed);
    os2 << seed;

	for ( int i = 0; i < 4; ++i )
	{
		for ( size_t j = 0; j < sizeof(GUID); ++j )
		{
			BYTE& val = signKey[i * sizeof(GUID) + sizeof(GUID) - 1 - j];
			val ^= seedBytes[j];
		}
	}
	m_signer.reset( new DataSigner( signKey ) );
}



BitMap StreamBufferImpl::BuildTotalBitmap() const
{
	return m_storage.BuildBitmap();
}

UINT StreamBufferImpl::GetApproximateBufferTime() const
{
	double buftime = m_statistics.BufferTime;
	buftime = buftime * (100 - m_statistics.SkipPercent) / 100;
	return static_cast<UINT>(buftime);
}




void StreamBufferImpl::Clear()
{
	m_storage.Clear();

	m_statistics.Clear();
	m_base.Clear();
}

//状态迁移：									
//											/-------------------Reset()-----------------------\
//											^	  |											  ^
//											|	  v											  |
//st_none        ----Start()----->         st_waiting     -------AddFirstPiece()------->   st_running
//  ^											|											  |
//	|											|											  |
//	|											v											  v
//	\----------------------------------------Stop()-------------------------------------------/

bool StreamBufferImpl::Start()
{
	if( m_State != st_none )
	{
		STREAMBUFFER_ERROR("StreamBuffer already Started and m_State = "<<m_State);
		return false;
	}
	Clear();
	m_State = st_waiting;
	STREAMBUFFER_EVENT("StreamBuffer Start and m_State = "<<m_State);
	return true;
}

bool StreamBufferImpl::Stop()
{
	if( m_State == st_none )
	{
		STREAMBUFFER_ERROR("StreamBuffer already Stopped and m_State = "<<m_State);
		return false;
	}

	Clear();
	m_State = st_none;
	STREAMBUFFER_EVENT("StreamBuffer Stop and m_State = "<<m_State);
	return true;
}

bool StreamBufferImpl::Reset(UINT index)
{
	if (m_State == st_none)
	{
		STREAMBUFFER_ERROR("StreamBuffer Not Start and m_State = "<<m_State);
		return false;
	}

	Clear();
	//Start后，将进入到等待第一个Piece下载的状态(即Reset后的状态)
	m_State = st_waiting;
	m_statistics.StartIndex = index;
	m_lpMediaServer->Reset();

	m_statistics.ResetTimes++;
	STREAMBUFFER_EVENT("StreamBuffer Reset to index "<<index <<" and m_State = "<<m_State);
	return true;
}

void StreamBufferImpl::OnFirstPiece(MediaDataPiecePtr lpPacket)
{
	LIVE_ASSERT(m_State == st_waiting);
	CheckDataPiece(lpPacket);

	m_State = st_running;//改变状态
	m_base.Reset(lpPacket);
	OnUpdateBase();
	STREAMBUFFER_EVENT("OnFirstPiece when add piece "<<lpPacket->GetPieceIndex()<<" into StreamBuffer, so Change m_state = "<<m_State);
}


bool StreamBufferImpl::AddHeaderPiece( MediaHeaderPiecePtr piece )
{
	m_storage.RemoveOldHeaders(m_config.MaxOldHeaderCount, m_lpMediaServer->GetNowHeaderIndex());
	if (!m_storage.AddHeaderPiece(piece))
		return false;

	m_lpMediaServer->AddMediaHeader(piece->ToMonoPiece());
	return true;
}

bool StreamBufferImpl::DoAddDataPiece(MediaDataPiecePtr packet)
{
	//然后再放到Buffer中
//	LIVE_ASSERT(packet->GetSize() > 1024 * 2);
	if (!m_storage.AddDataPiece(packet))
	{
		// 添加失败，需要删除这个packet
		return false;
	}
	UINT pieceIndex = packet->GetPieceIndex();
	if (m_statistics.FirstErrorPiece == 0 && pieceIndex > m_base.GetPieceIndex() && packet->GetTimeStamp() < m_base.GetTimeStamp())
	{
		m_statistics.FirstErrorPiece = pieceIndex;
		m_statistics.FirstErrorPieceTimeStamp = packet->GetTimeStamp();
		LIVE_ASSERT( false );
	}
	return true;
}


void StreamBufferImpl::UpdateBase()
{
	const UINT64 MAX_BASE_TIME_DIFF = 30 * 24 * 3600 * 1000u;	// 30天

	UINT64 elapsedBaseTime = m_base.GetElapsedTime();
	if (elapsedBaseTime <= MAX_BASE_TIME_DIFF)
		return;
	// 从Base时刻开始已经过了很长时间，需要更新BaseIndex/BaseTimeStamp/BaseTickCount
	// 避免时间跨度过大，造成时间计算出错
	PieceInfo firstPieceInfo = m_storage.GetFirstPiece();
	MediaDataPiecePtr firstPiece = firstPieceInfo.GetPiece();
//	LIVE_ASSERT(firstPiece != NULL);
	if (firstPiece && firstPiece->GetPieceIndex() > m_base.GetPieceIndex())
	{
		DoUpdateBase(firstPieceInfo);
		OnUpdateBase();
	}
}

void StreamBufferImpl::DoUpdateBase(const PieceInfo& piece)
{
	m_base.Update(piece.GetPiece(), piece.GetReceiveTime(), true);
}

void StreamBufferImpl::OnUpdateBase()
{
	// 将基准点的信息记录到共享内存中
	m_statistics.BasePieceIndex = m_base.GetPieceIndex();
	m_statistics.BaseTimeStamp = m_base.GetTimeStamp();
	m_statistics.BaseReceiveTime = m_base.GetReceiveTime();
}

void StreamBufferImpl::OnAppTimer(UINT times, UINT downSpeed)
{
	MANAGER_EVENT( "StreamBufferImpl::OnTimer" );

	if (m_State == st_running)
	{
		STREAMBUFFER_ERROR("StreamBufferImpl::OnTimer StreamBuffer Not Ruuning and m_State = "<<m_State);
		Push();
		Shrink();
		if (times % 2 == 0)
		{
			// OnAppTimer是1秒钟2次，这里1秒钟输出1次日志
			BitMap bitmap = BuildTotalBitmap();
			VIEW_INFO( "StreamBufferImpl::OnAppTimer - Skip: " << GetSkipIndex() );
			//m_storage.ViewStreamBuffer();
			VIEW_INFO("LocalBitmap "<<m_storage.GetMinIndex()<<" "<<m_storage.GetMaxIndex()<<" "<<GetSkipIndex()<<" "<<bitmap.GetResourceString()<<" End");
			//CORE_vlog(0, 0, "StreamBuffer: BufferTime=%d, PieceCount=%d, BufferSize=%d", m_statistics.BufferTime, m_statistics.PieceCount, m_statistics.BufferSize);
		}
	}

	UpdateStatistics(downSpeed);

	if( times % 8 == 1)
	{
		m_storage.Serialize();
	}
}

size_t StreamBufferImpl::GetRangeCount(  UINT rangeMinIndex, UINT rangeMaxIndex  ) const
{
	return m_storage.CountPieces(rangeMinIndex, rangeMaxIndex);
}

void StreamBufferImpl::SetPeerInformation( boost::shared_ptr<const PeerInformation> info )
{
	m_PeerInformation = info;
	m_SourceResource = m_PeerInformation->StatusInfo->GetSourceResource();
}

UINT StreamBufferImpl::GetPrepaDataLen()
{
	int prepaDataLen = 0;
	UINT minIndex = m_storage.GetMinIndex();
	UINT preloacteIndex = GetPrelocationIndex();
	LIMIT_MIN( minIndex, preloacteIndex );

	UINT curIndex = 0;
	if (m_base.GetTimeStamp() != 0)
	{
		UINT dataLen = m_SourceResource->GetTimedLength(m_base.GetElapsedTime());
		curIndex =m_base.GetPieceIndex() + dataLen;
	}
	curIndex = max(minIndex,curIndex);

	prepaDataLen = GetDownloadStartIndex() - curIndex;
	//	LIVE_ASSERT(prepaDataLen >= 0);
	LIMIT_MIN(prepaDataLen, 0);
	return prepaDataLen;
}

int StreamBufferImpl::GetPrepaDataTime()
{
#if 0
	return GetPrepaDataLen() * 120000 / m_SourceResource->GetMinMax().GetLength() ;
#else
	if (m_base.GetTimeStamp() != 0)
	{
		MediaDataPiecePtr skipPiece = m_storage.GetDataPiece(GetDownloadStartIndex() - 1);
		if (skipPiece != NULL)
		{
			UINT skipTS = skipPiece->GetTimeStamp();
			UINT curTS = m_base.GetTimeStamp() + m_base.GetElapsedTime();
			return skipTS - curTS;
		}
	}
	return 0;
#endif
}




SourceStreamBuffer::SourceStreamBuffer() : StreamBufferImpl(120 * 1000)
{
//	m_storage.SetNeedCacheSubPieces( true );
	m_totalDownloadedPieceCount = 0;
}

void SourceStreamBuffer::Clear()
{
	StreamBufferImpl::Clear();
	m_totalDownloadedPieceCount = 0;
}

bool SourceStreamBuffer::AddDataPiece( MediaDataPiecePtr piece )
{
	STREAMBUFFER_INFO("StreamBufferImpl::AddDataPiece " << make_tuple(piece->GetPieceIndex(), piece->GetPieceLength()));
	m_statistics.PieceSize = (UINT32)piece->GetPieceLength();

	UINT index = piece->GetPieceIndex();
	if (m_storage.IsEmpty())
	{
		// 第一片
		Reset(index);
		OnFirstPiece(piece);
	}
	if ( false == DoAddDataPiece(piece) )
		return false;
	++m_totalDownloadedPieceCount;
	return true;
}


void SourceStreamBuffer::Push()
{
	m_lpMediaServer->SetPlayableRange(m_storage.GetMinIndex(), m_storage.GetMaxIndex());
}

void SourceStreamBuffer::UpdateStatistics(UINT downSpeed)
{
	m_statistics.MinIndex = m_storage.GetMinIndex();
	m_statistics.MaxIndex = m_storage.GetMaxIndex();
	UINT bufferTime = m_storage.GetBufferTime();
	const UINT minBufferTime = 70 * 1000;
	if (bufferTime >= minBufferTime)
	{
		m_statistics.SkipPercent = 0;
	}
	else
	{
		double bufferPercent = bufferTime * 100.0 / minBufferTime;
		m_statistics.SkipPercent = (UINT8)( 100 - static_cast<UINT>(bufferPercent) );
		LIMIT_MIN_MAX(m_statistics.SkipPercent, 0, 100);
	}
	m_statistics.SkipIndex = m_statistics.MaxIndex;
	m_statistics.BufferSize = m_storage.GetBufferSize();
	m_statistics.PieceCount = (UINT32)m_storage.GetPieceCount();
	m_statistics.BufferTime = bufferTime;
	m_statistics.SkipBufferTime = bufferTime;
	m_statistics.TotalPushedPieceCount = (UINT32)m_totalDownloadedPieceCount; // source所有的片都可认为是pushed的了

	MediaDataPiecePtr minPiece = m_storage.GetDataPiece(m_statistics.MinIndex);
	MediaDataPiecePtr maxPiece = m_storage.GetDataPiece(m_statistics.MaxIndex);
	MediaDataPiecePtr skipPiece = m_storage.GetDataPiece(m_statistics.SkipIndex);
	m_statistics.MinTimeStamp = (minPiece) ? minPiece->GetTimeStamp() : 0;
	m_statistics.MaxTimeStamp = (maxPiece) ? maxPiece->GetTimeStamp() : 0;
	m_statistics.SkipTimeStamp = (skipPiece) ? skipPiece->GetTimeStamp() : 0;
	if (m_statistics.MinTimeStamp == 0 || m_statistics.SkipTimeStamp == 0)
	{
		m_statistics.SkipBufferTime = 0;
	}
	else
	{
		m_statistics.SkipBufferTime = static_cast<UINT32>( m_statistics.SkipTimeStamp - m_statistics.MinTimeStamp );
	}

	const DWORD min_buffer_time_for_report = ( m_config.CurrentTimeStampPosition < 100 ? m_config.CurrentTimeStampPosition : 100);
	if ( 0 == m_statistics.MinTimeStamp || m_statistics.BufferTime < min_buffer_time_for_report * 1000  )
	{
		m_statistics.CurrentTimeStamp = 0;
	}
	else
	{
		m_statistics.CurrentTimeStamp = m_statistics.MinTimeStamp + m_config.CurrentTimeStampPosition * 1000;
	}

}

void SourceStreamBuffer::Shrink()
{
	if (m_storage.IsEmpty())
	{
		STREAMBUFFER_DEBUG("StreamBufferImpl::Shrink no pieces.");
		return;
	}
	size_t deletedCount = m_storage.RemoveExpired(m_base, m_minBufferTime);
	(void)deletedCount;
        UpdateBase();
}

UINT64 SourceStreamBuffer::GetSkipTimestamp() const
{
	UINT skipIndex = GetSkipIndex();
	if( skipIndex == 0) return 0;
	MediaDataPiecePtr packet = m_storage.GetDataPiece(skipIndex);
	if( ! packet ) return 0;
	return packet->GetTimeStamp();
}

PeerStreamBuffer::PeerStreamBuffer() : m_skipIndex(0), m_downloader(NULL)
{
//	m_storage.SetNeedCacheSubPieces(false);
}

void PeerStreamBuffer::Clear()
{
	StreamBufferImpl::Clear();
	m_feedback.Clear();
	m_skipIndex = 0;
	m_playIndex = 0;
	m_PrelocationIndex = 0;
}

bool PeerStreamBuffer::Reset(UINT index)
{
	if (!StreamBufferImpl::Reset(index))
		return false;
	m_skipIndex = index;
	m_playIndex = index;
	m_PrelocationIndex = index;
	return true;
}

BitMap PeerStreamBuffer::BuildTotalBitmap() const
{
	// min到skip之间的必须输出, skip到max之间的最多输出sourceminmax的2倍的长度
	UINT sourceMinMaxLength = m_SourceResource->GetLength();
	LIMIT_MIN(sourceMinMaxLength, 100);
	UINT maxLength = sourceMinMaxLength * 2;
	UINT minIndex = m_storage.GetMinIndex();
	if (minIndex > 0 && m_skipIndex > 0)
	{
		if (m_skipIndex >= minIndex)
		{
			// 加上min到skip之间的长度
			maxLength += (m_skipIndex - minIndex + 1);
		}
		else
		{
			LIVE_ASSERT(false);
		}
	}
	if (maxLength == 0)
	{
		maxLength = 3600;
	}
	return m_storage.BuildBitmap(maxLength);
}



bool PeerStreamBuffer::AddDataPiece( MediaDataPiecePtr piece )
{
	LIVE_ASSERT( PPDT_MEDIA_DATA == piece->GetPieceType() );
	CheckDataPiece(piece);

	STREAMBUFFER_INFO("StreamBufferImpl::AddDataPiece " << make_tuple(piece->GetPieceIndex(), piece->GetPieceLength()));
	m_statistics.PieceSize = (UINT32)piece->GetPieceLength();

	UINT index = piece->GetPieceIndex();

	if (NeedResetBuffer(piece) && IsValidPieceForReset(piece))
	{
		STREAMBUFFER_EVENT("recv piece index ="<<index<<" much more greater than m_skipIndex = "<<GetSkipIndex()<<" , so Reset StreamBuffer!");
		Reset(index);
	}
	if (m_storage.IsEmpty())
	{
		OnFirstPiece(piece);
		m_skipIndex = index;
	}
	else
	{
		LIVE_ASSERT(m_skipIndex != 0 && m_storage.GetMinIndex() != 0);
		// 过滤掉非法的piece
		if( index < m_skipIndex && index < m_storage.GetMinIndex() )
		{
			STREAMBUFFER_INFO("StreamBufferImpl::PieceTooOld " << index << " " << m_skipIndex << " " << m_storage.GetMinIndex());
			return false;
		}
		UINT sourceMinMaxLength = m_SourceResource->GetLength();
		LIMIT_MIN(sourceMinMaxLength, 100);
		UINT maxLength = sourceMinMaxLength * 2;
		if (index > m_skipIndex + maxLength)
		{
			STREAMBUFFER_ERROR("StreamBufferImpl::PieceTooNew, may be bad " << make_tuple(index, m_skipIndex, m_storage.GetMinIndex()) << " " << *m_SourceResource);
			return false;
		}
	}

	if ( false == DoAddDataPiece(piece) )
		return false;
	STREAMBUFFER_INFO( "StreamBuffer AddPiece index=" << index << " SkipIndex="<<GetSkipIndex() << (index>=GetSkipIndex() ? "  true" : "  false"));
	m_feedback.Record(piece->GetPieceIndex() <= m_skipIndex);
	return true;
}

size_t PeerStreamBuffer::PushStream(UINT& skipIndex)
{
	LIVE_ASSERT(m_downloader != NULL);
	LIVE_ASSERT(m_skipIndex > 0);
	LIVE_ASSERT(m_skipIndex >= m_storage.GetMinIndex());
	if (m_storage.IsEmpty())
	{
		STREAMBUFFER_WARN("StreamBufferImpl::IncreaseSkipIndex StreamBuffer isn't contain any DataPacket!");
		return 0;
	}

	size_t pushedCount = 0;
	//pushedCount = PushContinuously(skipIndex);
	//pushedCount += PushTimely(base, skipIndex);

	UINT64 CurrentTimestamp = m_base.GetElapsedTime();
	
	UINT StartIndex = m_skipIndex + 1;
	UINT MaxIndex = m_storage.GetMaxIndex();
	bool IsTimelyPush = true;			//  按照时间戳推进
	bool IsContinuousPush = true;		//  连续推进

	// 如果Skip对应的时间戳 > 理论推进时间戳 那么 就没有必要 "按时间戳推进"了。所以把 IsTimelyPush 赋为false
	if( m_storage.HasDataPiece(m_skipIndex) )
	{
		MediaDataPiecePtr skipPiece = m_storage.GetDataPiece(m_skipIndex);
		CheckDataPiece(skipPiece);
		if( skipPiece->GetTimeStamp() + m_config.TimelyPushGap > CurrentTimestamp + m_base.GetTimeStamp() )
		{
			IsTimelyPush = false;
		}
	}

	// 下面就是真正的 SkipIndex 推进算法了
	for( UINT i = StartIndex; i <= MaxIndex; i ++ )
	{
		if( m_storage.HasDataPiece(i) == true)
		{	// 这一片 是数据片, SkipIndex向前推进
			MediaDataPiecePtr piece = m_storage.GetDataPiece(i);
			CheckDataPiece(piece);
			if( piece->GetTimeStamp() + m_config.TimelyPushGap > CurrentTimestamp + m_base.GetTimeStamp() )
			{	// 此时 i 对应的Piece的时间戳 > 理论推进时间戳 后面就没有必要"按时间戳推进"
				IsTimelyPush = false;
			}
			if( IsTimelyPush == true || IsContinuousPush == true )
			{	// 只要还处于能够推进状态，就要把 i 赋值到 m_skipIndex
				m_skipIndex = i;
				VIEW_DEBUG("PushStream "<<m_skipIndex<<" End");
				pushedCount ++;
				PPL_TRACE("buffer forwarding " << make_tuple(m_base.GetPieceIndex(), m_storage.GetMinIndex(), m_skipIndex, m_skipIndex - m_storage.GetMinIndex()));
			}
		}
		else if( m_storage.HasHeader(i))
		{	// 这一片 是头部片, SkipIndex向前推进
		}
		else if( m_storage.HasHeader(i+1))
		{	// Source在切换头部的时候可能出现1片空隙,这里是判断这一片是否是空隙
		}
		else if( i < m_PrelocationIndex )
		{	// 在无效的范围内啦,故应该考虑直接推过去
			// 相当于把 m_SkipIndex 推进到 m_PrelocationIndex 那里去
		}
		//else if( IsTimelyPush == true && m_downloader->IsRequested(i) == true )
		//else if( IsTimelyPush == true && m_downloader->IsRequested(i) == true )
		//{	// 这一片已经发起了Request, 并且没有过时间限制, SkipIndex向前推进
		//	IsContinuousPush = false;
		//}
		else if( IsTimelyPush == true && m_downloader->GetRequestCount(i) >= 8 && m_downloader->IsRequested(i)==false )
		{	// 现在没有请求该片 已经请求了8次  （相当于8次没有下到，放弃）
			VIEW_INFO( "PeerStreamBuffer::PushStream index="<<i<< " StartIndex=" << StartIndex << " RquestCount="<<m_downloader->GetRequestCount(i)<<" not requesting" );
			IsContinuousPush = false;	// 停止连续
		}
		else if( IsTimelyPush == true && m_downloader->GetRequestCount(i) >= 9 && m_downloader->IsRequested(i)==true )
		{	// 现在正在请求该片 包括这次,已经请求了9次   （相当于8次没有下到，放弃）
			VIEW_INFO( "PeerStreamBuffer::PushStream index="<<i<< " StartIndex=" << StartIndex << " RquestCount="<<m_downloader->GetRequestCount(i)<<" requesting" );
			IsContinuousPush = false;	// 停止连续
		}
		else if( IsTimelyPush == true && m_downloader->HasResource(i) == false )
		{	// 这一片 没有被下载的 能力, 并且没有过时间限制, SkipIndex向前推进
			IsContinuousPush = false;
		}
		else
		{	// SkipIndex 不符合以上条件,停止向前推进
			PPL_TRACE("buffer forwarding failed " << make_tuple(m_base.GetPieceIndex(), m_storage.GetMinIndex(), m_skipIndex, m_skipIndex - m_storage.GetMinIndex()) << make_tuple(pushedCount, i, i - m_skipIndex));
			break;
		}
	}
	
	if( m_skipIndex > 0)
	{
		UINT64 SkipTimestamp = m_storage.GetDataPiece(m_skipIndex)->GetTimeStamp();
		(void)SkipTimestamp;
                VIEW_INFO("TimeStamp "<<(SkipTimestamp+m_config.TimelyPushGap)<<" "<<(CurrentTimestamp + m_base.GetTimeStamp())<<" End" );
	}

	if ( pushedCount > 0 )
	{
		PPL_TRACE("buffer forwarding total " << make_tuple(m_base.GetPieceIndex(), m_storage.GetMinIndex()) << make_tuple(m_skipIndex, m_skipIndex - m_storage.GetMinIndex(), pushedCount));
	}

	return pushedCount;
}

//void PeerStreamBuffer::PushPlay()
//{
//	if(m_skipIndex == 0 || m_playIndex == 0)
//	{
//		return;
//	}
//
//	const PPMediaDataPacket* pieceSkip = (const PPMediaDataPacket*)GetDataPiece(m_skipIndex);
//	CheckDataPiece(pieceSkip);
//	LIVE_ASSERT( pieceSkip->GetTimeStamp() >= m_base.GetTimeStamp() );
//	DWORD SkipTimestamp = pieceSkip->GetTimeStamp();
//	//? TimeStamp可能翻转
//	if( SkipTimestamp < 20*1000 )
//	{
//		return;
//	}
//
//	const PieceInfoCollection& pieces = GetPieces().m_storage;
//	for (PieceInfoCollection::const_iterator iter = pieces.begin();
//		iter != pieces.end(); ++iter)
//	{
//		//? TimeStamp可能翻转
//		if( iter->second->GetTimeStamp() + 20*1000 >= SkipTimestamp )
//		{
//			break;
//		}
//		m_playIndex = iter->first;
//	}
//}

bool PeerStreamBuffer::IsValidPieceForReset(MediaDataPiecePtr piece) const
{
//	const PEER_MINMAX& sourceMinmax = m_SourceResource->GetMinMax();
//	UINT pieceIndex = piece->GetPieceIndex();
//	return pieceIndex < (sourceMinmax.MaxIndex + 2*60*60*10) && (pieceIndex + 2000) > sourceMinmax.MinIndex;
	return m_SourceResource->CheckPieceIndexValid(piece->GetPieceIndex());
}

void PeerStreamBuffer::Push()
{
	// 推进 SkipIndex
	UINT oldSkipIndex = m_skipIndex;
	size_t pushedCount = PushStream(m_skipIndex);
	UINT diff = m_skipIndex - oldSkipIndex;
	LIVE_ASSERT(diff >= pushedCount);
	m_feedback.Push(pushedCount);
	m_feedback.Skip(diff - pushedCount);

	// 推进 PlayIndex
	//UINT oldPlayIndex = m_playIndex;
	//PushPlay();
	//if (m_playIndex > oldPlayIndex)
	if( m_storage.GetMinIndex() < m_skipIndex && m_PrelocationIndex < m_skipIndex )
	{
		//m_lpMediaServer->SetPlayableRange(GetMinIndex(), m_playIndex);
		m_lpMediaServer->SetPlayableRange(max(m_storage.GetMinIndex(),m_PrelocationIndex), m_skipIndex);
	}
}

void PeerStreamBuffer::UpdateStatistics(UINT downSpeed)
{
	UINT oldBufferTime = m_statistics.BufferTime;

	m_statistics.MinIndex = m_storage.GetMinIndex();
	m_statistics.MaxIndex = m_storage.GetMaxIndex();

	const UINT max_min_buffer_time = 20;
	const UINT minBufferTime = max_min_buffer_time * 1000;
	//if (downSpeed > 300 * 1024)
	//{
	//	minBufferTime = 5 * 1000;
	//}
	//else if (downSpeed > 150 * 1024)
	//{
	//	minBufferTime = 10 * 1000;
	//}
	//else if (downSpeed > 60 * 1024)
	//{
	//	minBufferTime = 15 * 1000;
	//}

	// 如果片数太少，pieceDuration为0
	double pieceDuration = m_SourceResource->CalcAveragePieceDuration();
	double pieceRate = m_SourceResource->CalcAveragePieceRate();
//	UINT minBufferSize = static_cast<UINT>(minBufferTime * pieceRate / 1000);
	UINT minBufferSize = m_SourceResource->CalcPieceCount( minBufferTime );
//	UINT huntIndex = m_base.GetPieceIndex() + static_cast<UINT>(m_base.GetElapsedTime() * pieceRate / 1000);
	UINT pieceCount = m_feedback.GetPushed();
	LIVE_ASSERT(m_skipIndex >= m_statistics.MinIndex);
	UINT bufferSize = m_skipIndex - m_statistics.MinIndex + 1;
	LIVE_ASSERT(pieceCount <= bufferSize);
// 	if (pieceCount > bufferSize)
// 	{
	//	STREAMBUFFER_DEBUG("StreamPusher::UpdateStatistics pieceCount invalid" << make_tuple(pieceCount, bufferSize));
//	}
	UINT bufferTime = static_cast<UINT>(pieceCount * pieceDuration);
	PEER_MINMAX minmax = { m_statistics.MinIndex, m_statistics.MaxIndex };
	(void)pieceRate;
        (void)minmax;
        STREAMBUFFER_DEBUG("StreamPusher::UpdateStatistics " << minmax << make_tuple(pieceCount, bufferSize, minBufferSize) << make_tuple(bufferTime, minBufferTime) << make_tuple(pieceDuration, pieceRate, m_SourceResource->GetLength()));

	if (minBufferSize <= 1)
		minBufferSize = 100;
	if (bufferSize <= 1)
		bufferSize = 100;
	// 限制minBufferSize的最大值，避免minBufferSize估算出错导致缓冲区计算有问题
//	LIMIT_MAX(minBufferSize, max_min_buffer_time * 100);
	LIMIT_MIN(bufferSize, minBufferSize);
	LIMIT_MAX(pieceCount, bufferSize);
	LIVE_ASSERT(minBufferSize > 0 && bufferSize > 0);
	UINT bufferPercent = (pieceCount >= bufferSize) ? 100 : static_cast<UINT>(pieceCount * 100.0 / bufferSize);
	LIVE_ASSERT(bufferPercent <= 100);
	LIMIT_MAX(bufferPercent, 100);

	UINT sourceLength20s = m_SourceResource->GetTimedLength(minBufferTime);
	if ( bufferTime < minBufferTime && sourceLength20s > 0 )
	{
		// 如果buffertime小于最小参考值20s，并且有sourceminmax，则使用source20s长度内的数据片计算
		UINT refIndex = m_storage.GetMinIndex() + sourceLength20s;
		UINT refCount = m_storage.CountRange(refIndex);
		bufferPercent = static_cast<UINT>( refCount * 100.0 / sourceLength20s );
		bufferSize = sourceLength20s;
		pieceCount = refCount;
		VIEW_INFO("calc buffer percent " << make_tuple(m_storage.GetMinIndex(), refIndex, refCount) << make_tuple(sourceLength20s, bufferPercent, bufferTime));
		LIMIT_MIN_MAX(bufferPercent, 0, 100);
	}

	m_statistics.SkipPercent = (UINT8)( 100 - bufferPercent );
	m_statistics.BufferSize = bufferSize;
	m_statistics.BufferTime = bufferTime;
	m_statistics.PieceCount = pieceCount;
	m_statistics.SkipIndex = m_skipIndex;
	m_statistics.HuntIndex = m_PrelocationIndex;
	m_statistics.TotalSkippedPieceCount = (UINT32)m_feedback.GetTotalSkipped();
	m_statistics.TotalPushedPieceCount = (UINT32)m_feedback.GetTotalPushed();

	MediaDataPiecePtr minPiece = m_storage.GetDataPiece(m_statistics.MinIndex);
	MediaDataPiecePtr maxPiece = m_storage.GetDataPiece(m_statistics.MaxIndex);
	MediaDataPiecePtr skipPiece = m_storage.GetDataPiece(m_statistics.SkipIndex);
	m_statistics.MinTimeStamp = (minPiece) ? minPiece->GetTimeStamp() : 0;
	m_statistics.MaxTimeStamp = (maxPiece) ? maxPiece->GetTimeStamp() : 0;
	m_statistics.SkipTimeStamp = (skipPiece) ? skipPiece->GetTimeStamp() : 0;
	if (m_statistics.MinTimeStamp == 0 || m_statistics.SkipTimeStamp == 0)
	{
		m_statistics.SkipBufferTime = 0;
	}
	else
	{
		m_statistics.SkipBufferTime = static_cast<UINT32>( m_statistics.SkipTimeStamp - m_statistics.MinTimeStamp );
	}

	UINT newBufferTime = m_statistics.BufferTime;
	UINT oldIndex = oldBufferTime / 1000;
	UINT newIndex = newBufferTime / 1000;
	UINT usedTime = static_cast<UINT>( m_StartTime.elapsed() );
	if ( oldIndex < STREAMBUFFER_STATS::MAX_BUFFERRING_TIME_COUNT )
	{
		for ( UINT index = oldIndex + 1; index <= newIndex; ++index )
		{
			LIVE_ASSERT( index >= 1 );
			if ( index <= STREAMBUFFER_STATS::MAX_BUFFERRING_TIME_COUNT )
			{
				m_statistics.BufferringUsedTime[ index - 1 ] = usedTime;
			}
		}
	}

	m_statistics.DownloadedPieceCount = m_storage.GetPieceCount();

	STREAMBUFFER_INFO(
		"PeerStreamBuffer:SKIP (BufferSize,PieceCount,SkipPercent)=" 
		<< make_tuple(m_statistics.BufferSize, m_statistics.PieceCount, (int)m_statistics.SkipPercent) 
		<< " (BufferTime,TotalPushedPieceCount,SkipIndex=" 
		<< make_tuple(m_statistics.BufferTime, m_statistics.TotalPushedPieceCount, m_statistics.SkipIndex) 
		<< " MinMax=" << minmax
		);
}

bool PeerStreamBuffer::NeedDownload(UINT index) const
{
	if (index <= m_skipIndex)
	{
		STREAMBUFFER_ERROR("StreamBufferImpl::NeedDownload no need download "<<index<<" because it's not in range or less than SkipIndex.");
		return false;//不下载Source范围外差的太多的Piece和无需下载的Piece
	}

	return !m_storage.HasDataPiece(index);
}

void PeerStreamBuffer::Shrink()
{
	if (m_storage.IsEmpty())
	{
		STREAMBUFFER_DEBUG("StreamBufferImpl::Shrink no pieces.");
		return;
	}
	UINT boundLength = 3000;
	if (m_SourceResource->GetLength() > 0)
	{
		boundLength = m_SourceResource->GetLength() * 5 / 2;
		LIMIT_MIN(boundLength, 3000);
	}
	UINT upperBound = GetSkipIndex();
	UINT lowerBound = 0;
	if (upperBound > boundLength)
	{
		lowerBound = upperBound - boundLength;
	}
	size_t deletedCount = m_storage.RemoveExpired(m_base, m_minBufferTime + STREAM_DELAY_TIME, upperBound, lowerBound);
	m_feedback.Discard(deletedCount);
	if (m_storage.IsEmpty())
	{
		// 重置StreamBuffer，清除所有的piece，重置相应的记录
		m_skipIndex = 0;
		Clear();
		m_State = st_waiting;
	}
	else if (m_skipIndex < m_storage.GetMinIndex())
	{
		// 应该是在m_storage.RemoveExpired中删除了m_skipIndex对应的片
		LIVE_ASSERT(false);
		m_skipIndex = m_storage.GetMinIndex();
		m_feedback.Push(1);
	}
	else
	{
		//UpdateBase();
	}
	LIVE_ASSERT(m_skipIndex >= m_storage.GetMinIndex());
}

bool PeerStreamBuffer::NeedResetBuffer(MediaDataPiecePtr piece) const
{
	return piece->GetPieceIndex() > GetSkipIndex() + m_config.ResetBufferGap;
}

UINT64 PeerStreamBuffer::GetSkipTimestamp() const
{
	if( m_skipIndex == 0)
		return 0;
	MediaDataPiecePtr packet = m_storage.GetDataPiece(m_skipIndex);
	if( ! packet )
		return 0;
	return packet->GetTimeStamp();
}

void PeerStreamBuffer::SetPrelocationIndex( UINT PrelocationIndex )
{
	m_PrelocationIndex = PrelocationIndex;
	UINT index = 0;
	for( index = PrelocationIndex-1; index >= m_storage.GetMinIndex(); index -- )
	{
		if( m_storage.HasDataPiece(index) == true )
			m_PrelocationIndex = index;	
		else break;
	}
	VIEW_INFO( "RealPreloactionIndex " << index << " End");
	m_lpMediaServer->DisconnectAllMediaClient();
}

void PeerStreamBuffer::DoUpdateBase( const PieceInfo& piece )
{
	m_base.Update(piece.GetPiece(), piece.GetReceiveTime(), false);
}

DataSignerPtr PeerStreamBuffer::GetSigner()
{
	return m_config.NeedVerifyData ? m_signer : DataSignerPtr();
}





#ifdef _PPL_RUN_TEST

#include <ppl/data/stlutils.h>

class MapsTest : public ppl::util::test_case
{
	virtual void DoRun()
	{
		map<int, int> m;
		m[1] = 1;
		m[3] = 3;
		m[5] = 5;
		m[6] = 7;
		m[7] = 1;
		m[9] = 1;
		m[11] = 1;
		LIVE_ASSERT(maps::count_range(m, 2, 10) == 5);
		LIVE_ASSERT(maps::count_range(m, 3, 10) == 5);
		LIVE_ASSERT(maps::count_range(m, 2, 11) == 6);
	}
};

CPPUNIT_TEST_SUITE_REGISTRATION(MapsTest);

#endif


