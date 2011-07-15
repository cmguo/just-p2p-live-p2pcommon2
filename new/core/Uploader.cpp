
#include "StdAfx.h"

#include "Uploader.h"
#include "PeerManagerImpl.h"
#include "common/MediaStorage.h"
#include "PeerConnection.h"
#include "common/MediaPiece.h"
#include "common/appcore.h"
#include "framework/log.h"

#include <synacast/protocol/SubMediaPiece.h>
#include <ppl/data/stlutils.h>



Uploader::Uploader(CPeerManager& peerManager, const Storage& storage, const UploaderConfig& config) 
	: m_PeerManager(peerManager), m_Storage(storage), m_MaxUploadTimes(config.InitialMaxUploadTimesPerSubPiece)
{
}

Uploader::~Uploader()
{
}

/*
void Uploader::UploadMonoPiece(UINT pieceIndex, PeerConnection* pc)
{
	CheckUploadCounts();

#ifdef _WIN32_WCE
#pragma message("------wince不上传数据 PeerConnection::HandleSubPieceRequest")
	pc->DoSendPieceNotFound(subPieceUnit);
	return;
#endif

	PPMediaDataPacketPtr dataPiece = m_Storage.GetDataPiece(pieceIndex);
	if (dataPiece && CanUploadDataPiece(dataPiece))
	{
		// 获取到数据片，发送之
		SendDataPiece(dataPiece, pc);
	}
	else
	{
		PPMediaHeaderPacketPtr headerPiece = m_Storage.GetHeader(pieceIndex);
		if (headerPiece)
		{
			pc->SendHeaderPiece(headerPiece);
		}
		else
		{
			PEERCON_WARN("Peer " << *pc << " PeerConnection::HandleRequestOne: Request Piece Not Found. " << pieceIndex);
			pc->DoSendPieceNotFound(pieceIndex);
		}
	}
}
*/
/*
void Uploader::UploadSubPieces(UINT pieceIndex, PeerConnection* pc)
{
	CheckUploadCounts();

	PPMediaHeaderPacketPtr headerPiece = m_Storage.GetHeader(pieceIndex);
	if (headerPiece)
	{
		UPLOAD_DEBUG("Peer:" << *pc << " header piece sent " << pieceIndex);
		pc->SendHeaderPiece(headerPiece);
		return;
	}
	for (UINT16 index = 0; index < m_Storage.GetSubPieceCount(pieceIndex); ++index)
	{
		SubPieceUnit subPieceUnit(pieceIndex, index);
		SubPieceDataPacketPtr subPiece = m_Storage.GetSubPiece(pieceIndex, index);
		if (subPiece && CanUploadSubPiece(subPieceUnit))
		{
			UPLOAD_DEBUG("Peer:" << *pc << " sub piece sent " << make_tuple(pieceIndex, index));
			SendSubPiece(subPiece, pc);
		}
		else
		{
			pc->DoSendPieceNotFound(pieceIndex);
		}
	}
}


void Uploader::UploadSubPieces( UINT pieceIndex, UINT8 subPieceCount, const UINT16 subPieces[], PeerConnection* pc )
{
	CheckUploadCounts();

	UPLOAD_DEBUG("Peer:" << *pc << " PeerConnection::HandleSubPieceRequest " << pieceIndex << subPieceCount);
	PPMediaHeaderPacketPtr headerPiece = m_Storage.GetHeader(pieceIndex);
	if (headerPiece)
	{
		pc->SendHeaderPiece(headerPiece);
		return;
	}
	for (UINT16 index = 0; index < subPieceCount; ++index)
	{
		USHORT subPieceIndex = subPieces[index];
		SubPieceUnit subPieceUnit(pieceIndex, index);
		SubPieceDataPacketPtr subPiece = m_Storage.GetSubPiece(pieceIndex, subPieceIndex);
		if (subPiece && CanUploadSubPiece(subPieceUnit))
		{
			SendSubPiece(subPiece, pc);
		}
		else
		{
			pc->DoSendSubPieceNotFound(pieceIndex, subPieceIndex);
		}
	}
}*/

void Uploader::UploadSubPiece(SubPieceUnit subPieceUnit, PeerConnection* pc)
{
	CheckUploadCounts();

	if (subPieceUnit.SubPieceIndex == static_cast<UINT8>(-1))
	{
		UploadMonoPiece(subPieceUnit.PieceIndex, pc);
		return;
	}

	//assert(subPieceUnit.SubPieceIndex != -1);
	SubMediaPiecePtr subPiece = m_Storage.GetSubPiece(subPieceUnit.PieceIndex, subPieceUnit.SubPieceIndex);
	if (subPiece && CanUploadSubPiece(subPiece))
	{
		SendSubPiece(subPiece, pc);
	}
	else
	{
		pc->SendSubPieceNotFound(subPieceUnit.PieceIndex, subPieceUnit.SubPieceIndex);
	}
}

void Uploader::UploadSubPiece(UINT64 inTS, PeerConnection* pc)
{
	CheckUploadCounts();

	
	//assert(subPieceUnit.SubPieceIndex != -1);
	SubMediaPiecePtr subPiece = m_Storage.GetSubPiece(inTS);
	if (subPiece && CanUploadSubPiece(subPiece))
	{
		SendSubPiece(subPiece, pc);
	}
// 	else
// 	{
// 		pc->SendSubPieceNotFound(subPieceUnit.PieceIndex, subPieceUnit.SubPieceIndex);
// 	}
}

void Uploader::SendSubPiece( SubMediaPiecePtr subPiece, PeerConnection* pc )
{
	if (pc->SendSubPiece(subPiece))
	{
		UploadInfo& info = m_UploadInfo[subPiece->GetSubPieceUnit()];
		info.UploadTimes++;
		info.LastUploadTime.sync();
		
		if (m_rand.Next() % 16 == 7)
		{
			UPLOAD_DEBUG("Uploader::SendSubPiece " << m_MaxUploadTimes << " Unit:" << subPiece->GetSubPieceUnit() 
				<< " " << make_tuple(info.UploadTimes, info.LastUploadTime));
		}
	}
}
/*
void Uploader::SendDataPiece( PPMediaDataPacketPtr dataPiece, PeerConnection* pc )
{
	if (pc->SendDataPiece(dataPiece))
	{
		size_t subPieceCount = CalcSubPieceCountForPiece(dataPiece->GetMediaDataLength());
		for (UINT16 subPieceIndex = 0; subPieceIndex < subPieceCount; ++subPieceIndex)
		{
			SubPieceUnit subPieceUnit(dataPiece->GetPieceIndex(), subPieceIndex);
			m_UploadInfo[subPieceUnit].UploadTimes++;
			m_UploadInfo[subPieceUnit].LastUploadTime.Sync();
		}
	}
}*/

void Uploader::CheckUploadCounts()
{
	CheckMaxUploadTimes();
	if (m_LastCheckTime.elapsed() > 500)
	{
		m_LastCheckTime.sync();
		SubPieceUnit firstSubPieceUnit;
		size_t uploadCount = 0;
		if (!m_UploadInfo.empty())
		{
			firstSubPieceUnit = m_UploadInfo.begin()->first;
			uploadCount = m_UploadInfo.begin()->second.UploadTimes;
		}
		SubPieceUnit upperBound(m_Storage.GetMinIndex(), 0);
		size_t erasedCount = maps::erase_lower(m_UploadInfo, upperBound);
		(void)erasedCount;
                UPLOAD_DEBUG("Uploader::CheckUploadCounts " << erasedCount << " " << firstSubPieceUnit << " " << uploadCount << " " << upperBound << " " << m_UploadInfo.size());
	}
}

void Uploader::OnAppTimer(UINT times)
{
	if (times % APP_TIMER_TIMES_PER_SECOND != 0)
	{
		return ;
	}

	// 速度无效，按照上传SubPiece上传次数限制
	if( m_PeerManager.GetConfig().Uploader.MaxUploadSpeed == 0 )
	{
		return ;
	}
	
	int seconds = times / APP_TIMER_TIMES_PER_SECOND;
	if (seconds % 60 != 0)
	{
		return;
	}

	CheckMaxUploadTimes();

	UINT oldMaxUploadTimes = (UINT)m_MaxUploadTimes;
	UINT uploadSpeed = m_PeerManager.GetLongTimeUploadFlow().GetRate();

	float rate = 2.0;
	if (uploadSpeed > 0)
		rate = ( float(m_PeerManager.GetConfig().Uploader.MaxUploadSpeed * 1000 ) / float(uploadSpeed) );
	LIMIT_MIN_MAX(rate,0.5,2.0);

	// 按照速度超过的比例调整
	float adjustRate = ( rate - 1 ) / 2;		// 1/2的速度调整，减轻波动
	m_MaxUploadTimes += int( m_MaxUploadTimes * adjustRate );
	LIMIT_MIN_MAX(m_MaxUploadTimes,1,10000);	// 1倍上传到1万倍上传，400Kbps ~ 4Gbps

// 	if (uploadSpeed > m_PeerManager.GetConfig().Uploader.MaxUploadSpeed )
// 	{
// 		// 超过了最大上传速度，下调subpiece上传次数限制
// 		if (m_MaxUploadTimes > 0)
// 		{
// 			m_MaxUploadTimes--;
// 		}
// 	}
// 	else
// 	{
// 		m_MaxUploadTimes++;
// 	}
        (void)oldMaxUploadTimes;
	UPLOAD_DEBUG("Uploader adjust MaxUploadSpeed: " << m_PeerManager.GetConfig().Uploader.MaxUploadSpeed << " Adjust Rate: " << adjustRate << " " << make_tuple(uploadSpeed, oldMaxUploadTimes, m_MaxUploadTimes));
}
/*
bool Uploader::CanUploadDataPiece( PPMediaDataPacketPtr dataPiece )
{
	UINT pieceIndex = dataPiece->GetPieceIndex();
	size_t subPieceCount = CalcSubPieceCountForPiece(dataPiece->GetMediaDataLength());
	size_t overrunnedCount = 0;
	for (UINT16 subPieceIndex = 0; subPieceIndex < subPieceCount; ++subPieceIndex)
	{
		// 计算超出上传限制的subpiece片数
		if (true == CanUploadSubPiece(SubPieceUnit(pieceIndex, subPieceIndex)))
		{
			return true;
			//overrunnedCount++;
		}
	}
	return false;
	// 如果超限的片数大于总subpiece片数的一半，则认为是不能再上传
	// return overrunnedCount * 2 > subPieceCount;
}
*/

bool Uploader::CanUploadSubPiece( SubMediaPiecePtr subPiece )
{
	// 不限制header
	if (subPiece->GetPieceType() == PPDT_MEDIA_HEADER)
		return true;
	SubPieceUnit subPieceUnit = subPiece->GetSubPieceUnit();
	return CanUploadSubPiece(subPieceUnit);
}

bool Uploader::CanUploadSubPiece( SubPieceUnit subPieceUnit )
{
	assert(m_MaxUploadTimes > 0);
	// return m_UploadTimes[subPieceUnit] <= m_MaxUploadTimes;
	if ( m_UploadInfo[subPieceUnit].UploadTimes < m_MaxUploadTimes )
		return true;

	UploadInfo& info = m_UploadInfo[subPieceUnit];
	UPLOAD_DEBUG( "Uploader::CanUploadSubPiece - " << subPieceUnit 
		<< " ticks: " << make_tuple( info.UploadTimes, info.LastUploadTime, time_counter().get_count(), info.LastUploadTime.elapsed() ));
	// 8s之后还有SS要，就给他；8s之内不能冗余发送，认为8s时间足够SS之间交换数据
	return 8000 < info.LastUploadTime.elapsed();
}

void Uploader::CheckMaxUploadTimes()
{
	assert(m_MaxUploadTimes > 0);
}

bool Uploader::CanUploadPiece( MediaPiecePtr piece )
{
	if ( PPDT_MEDIA_HEADER == piece->GetPieceType() )
		return true;
	SubPieceUnit subPieceUnit(piece->GetPieceIndex(), 0);
	return CanUploadSubPiece(subPieceUnit);
}

void Uploader::UploadMonoPiece( UINT pieceIndex, PeerConnection* pc )
{
	// 请求全部的sub-piece
	MediaPiecePtr piece = m_Storage.GetMediaPiece(pieceIndex);
	if (piece)
	{
		if (CanUploadPiece(piece))
		{
			SendMonoPiece(piece, pc);
			return;
		}
	}
	pc->SendPieceNotFound(pieceIndex);
}

bool Uploader::SendMonoPiece( MediaPiecePtr piece, PeerConnection* pc )
{
	UINT8 subPieceCount = piece->GetSubPieceCount();
	for (UINT8 subPieceIndex = 0; subPieceIndex < subPieceCount; ++subPieceIndex)
	{
		SubMediaPiecePtr subPiece(piece->GetSubPiece(subPieceIndex));
		if (subPiece)
		{
			if ( false == pc->SendSubPiece(subPiece) )
				return false;
		}
		else
		{
			assert(false);
		}
	}
	return true;
}


