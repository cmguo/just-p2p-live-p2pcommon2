
#include "StdAfx.h"

#include "PublisherUploader.h"
#include "common/GloalTypes.h"
#include "PeerManagerImpl.h"
#include "PeerConnection.h"
#include "common/MediaStorage.h"
#include "common/MediaPiece.h"
#include "framework/log.h"


PublisherUploader::PublisherUploader(CPeerManager& peerManager, const Storage& storage, const UploaderConfig& config) 
	: Uploader( peerManager, storage, config )
{
	m_publishPiecePushTimeOut.clear();
}

PublisherUploader::~PublisherUploader()
{
}

void PublisherUploader::OnAppTimer(UINT times)
{
	//TODO 实现 Publisher Upload 逻辑

	checkPublishMapTimeOut();

	//计算Connection's Bitmap

	CalPublishBitmap();
}

//去除超时节点  也就是那些节点可以继续被发送
void PublisherUploader::checkPublishMapTimeOut()
{
	for(publishPiecePushTimeOut::iterator iter = m_publishPiecePushTimeOut.begin();
		iter != m_publishPiecePushTimeOut.end();)
		{
			if (iter->second.elapsed() > 10 *1000)
				m_publishPiecePushTimeOut.erase(iter++);
			else
				iter++;
		}
}

// 根据Bitmap计算需要发送的pieceindex 并且分配给每个connection
void PublisherUploader::CalPublishBitmap()
{
	for (UINT pieceIndex = m_Storage.GetMinIndex(); pieceIndex < m_Storage.GetMaxIndex(); pieceIndex ++)
	{
		//这个piece 10秒内发送过  跳过 
		if (m_publishPiecePushTimeOut.find(pieceIndex) != m_publishPiecePushTimeOut.end())
			continue;

		// OPT: 这里可以把所有连接的BM叠加到一起，得出一个总的BM，这样每次判断一个Piece是否存在，只需要O(1)
		// 叠加BM可以作为一个很大的或运算
		{
			bool hasPiece = false;
			STL_FOR_EACH_CONST( PeerConnectionCollection, m_PeerManager.GetConnections(), itr)
			{
				PeerConnection* pc = *itr;

				assert(!pc->IsUDP());

				if (pc->HasPiece(pieceIndex))
				{
					hasPiece = true;
					break;
				}
			}
			if (hasPiece) continue;
		}

		// OPT: 从所有connection里面找出未满的连接，按照pending个数升序排列，然后每次都只取第一个连接发送，直到满，移出集合
		{
			//限制每个connection pending数
			UINT max_pending = 40;
			PeerConnection *usePC = NULL;
			//这个Piece需要Send
			STL_FOR_EACH_CONST( PeerConnectionCollection, m_PeerManager.GetConnections(), itr_p)
			{
				PeerConnection* pc = *itr_p;
				assert(!pc->IsUDP());

				if (pc->GetSendPending() < (int)max_pending)
				{
					max_pending = pc->GetSendPending();
					usePC = pc;
				}
			}

			// OPT: 不存在的Piece应该在最开始检查，清理掉
			//{
				MediaDataPiecePtr dataPiece = m_Storage.GetDataPiece(pieceIndex);
				if (!dataPiece)
					continue;
			//}

			// NOTE: 这里没有做预分配，直接发送，可能对网络适应不太好
			if (max_pending != 40)
			{
				assert(usePC);
				SendMonoPiece(dataPiece, usePC);
				//加入正在请求的集合中  10秒后可以再次发送
				m_publishPiecePushTimeOut.insert(std::make_pair(pieceIndex,time_counter()));
				VIEW_INFO("SendDataPiece: "<<pieceIndex<<" to "<<usePC);
			}
			else 
			{
				break;
			}
		}
	}
}

void PublisherUploader::UploadSubPiece( SubPieceUnit subPieceUnit, PeerConnection* pc )
{

}
/*
void PublisherUploader::UploadSubPieces( UINT pieceIndex, PeerConnection* pc )
{
	PPMediaHeaderPacketPtr headerPiece = m_Storage.GetHeader(pieceIndex);
	if (headerPiece)
	{
		UPLOAD_DEBUG("Peer:" << *pc << " header piece sent " << pieceIndex);
		pc->SendHeaderPiece(headerPiece);
		return;
	}
	//ss向我发了piece请求 错误
	assert(0);
	pc->DoSendPieceNotFound(pieceIndex);
}*/

/*
void PublisherUploader::UploadMonoPiece( UINT pieceIndex, PeerConnection* pc )
{
	PPMediaHeaderPacketPtr headerPiece = m_Storage.GetHeader(pieceIndex);
	if (headerPiece)
	{
		UPLOAD_DEBUG("Peer:" << *pc << " header piece sent " << pieceIndex);
		pc->SendHeaderPiece(headerPiece);
		return;
	}
	//ss向我发了piece请求 错误
	assert(0);

	pc->DoSendPieceNotFound(pieceIndex);
}
*/
