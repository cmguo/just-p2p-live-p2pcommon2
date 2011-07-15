
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
	//TODO ʵ�� Publisher Upload �߼�

	checkPublishMapTimeOut();

	//����Connection's Bitmap

	CalPublishBitmap();
}

//ȥ����ʱ�ڵ�  Ҳ������Щ�ڵ���Լ���������
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

// ����Bitmap������Ҫ���͵�pieceindex ���ҷ����ÿ��connection
void PublisherUploader::CalPublishBitmap()
{
	for (UINT pieceIndex = m_Storage.GetMinIndex(); pieceIndex < m_Storage.GetMaxIndex(); pieceIndex ++)
	{
		//���piece 10���ڷ��͹�  ���� 
		if (m_publishPiecePushTimeOut.find(pieceIndex) != m_publishPiecePushTimeOut.end())
			continue;

		// OPT: ������԰��������ӵ�BM���ӵ�һ�𣬵ó�һ���ܵ�BM������ÿ���ж�һ��Piece�Ƿ���ڣ�ֻ��ҪO(1)
		// ����BM������Ϊһ���ܴ�Ļ�����
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

		// OPT: ������connection�����ҳ�δ�������ӣ�����pending�����������У�Ȼ��ÿ�ζ�ֻȡ��һ�����ӷ��ͣ�ֱ�������Ƴ�����
		{
			//����ÿ��connection pending��
			UINT max_pending = 40;
			PeerConnection *usePC = NULL;
			//���Piece��ҪSend
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

			// OPT: �����ڵ�PieceӦ�����ʼ��飬�����
			//{
				MediaDataPiecePtr dataPiece = m_Storage.GetDataPiece(pieceIndex);
				if (!dataPiece)
					continue;
			//}

			// NOTE: ����û����Ԥ���䣬ֱ�ӷ��ͣ����ܶ�������Ӧ��̫��
			if (max_pending != 40)
			{
				assert(usePC);
				SendMonoPiece(dataPiece, usePC);
				//������������ļ�����  10�������ٴη���
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
	//ss���ҷ���piece���� ����
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
	//ss���ҷ���piece���� ����
	assert(0);

	pc->DoSendPieceNotFound(pieceIndex);
}
*/
