#include "StdAfx.h"
#include "MkvNetWriter.h"
#include "common/MediaPiece.h"


//////////////////////////////////////////////////////////////////////////
//   结构 MkvMediaClient 的实现部分
//////////////////////////////////////////////////////////////////////////

MkvMediaClient::MkvMediaClient()
{
	PrepareForNextDataUnit();
}

MkvMediaClient::MkvMediaClient(CNetWriter* netWriter, tcp_socket_ptr sock):
MediaClient(netWriter, sock)
{
#ifdef WRITE_MKV_DATA_TO_FILE
    char szModuleName[MAX_PATH] = {'\0'}; 
    ::GetModuleFileName(NULL, szModuleName, sizeof(szModuleName)); 
    char szDriver[4] = {0}; 
    char szDir[MAX_PATH] = {0}; 
    _splitpath(szModuleName, szDriver, szDir, NULL, NULL); 
	stringstream fileName;
	fileName << szDriver;
	fileName << szDir;
	fileName << "MkvData-";
	fileName << time_counter::GetSystemTimeCount();
	fileName << ".mkv";
	m_DataFile.open(fileName.str().c_str(), std::ios::out | std::ios::binary);
#endif
	PrepareForNextDataUnit();
}

HRESULT MkvMediaClient::SetPlayableRange(UINT HeaderIndex, DWORD MinIndex, DWORD MaxIndex)
{
	if( false == m_IsSending )
	{
		// 还没有发送任何数据, 于是就要首先定位StartIndex
		DWORD StartIndex = m_NetWriter->LocateStartPosition();

		if( StartIndex == 0 )
		{
			NETWRITER_ERROR( "MkvMediaClient LocationStartIndex Error");
			return E_LOCATION_ERROR;
		}
		NETWRITER_INFO("CMkvNetWriter::MkvMediaClient SetPlayableRange: StartIndex=" << StartIndex << ", MinMax=" << make_tuple(MinIndex, MaxIndex));
		m_WillPlayIndex = StartIndex;
		m_IsSending = true;
	}
	LIVE_ASSERT( true == m_IsSending );

	return MkvMediaClient::SetPlayableRangeAfterSended(HeaderIndex,MinIndex,MaxIndex);
}

bool MkvMediaClient::SendToClient(UINT32 PacketIndex)
{
	if(m_DataUnitBuffer.size()>0)
	{
		bool SendOk = SendData( m_DataUnitBuffer.data(), m_DataUnitBuffer.size());
#ifdef WRITE_MKV_DATA_TO_FILE
		m_DataFile.write(m_DataUnitBuffer.data(), m_DataUnitBuffer.size());
#endif
		NETWRITER_DEBUG( "MkvMediaClient::SendToClient Send Media Data Index:" << PacketIndex <<"-"<<m_DataUnitBuffer.size() );
		if( SendOk == false )
		{
			NETWRITER_ERROR( "MkvMediaClient::SendToClient Send Media Data Error: " << PacketIndex <<*m_Socket );
			m_NetWriter->SavePlaytoIndex(PacketIndex - 1);
			return false;
		}
		PrepareForNextDataUnit();
	}
	return true;
}

HRESULT MkvMediaClient::SetPlayableRangeAfterSended(UINT HeaderIndex, DWORD MinIndex, DWORD MaxIndex)
{
	LIVE_ASSERT( true == m_IsSending );
	for ( MediaDataPiecePtr dataPiece = m_NetWriter->GetFirst(m_WillPlayIndex);
		dataPiece && dataPiece->GetPieceIndex() <= MaxIndex;
		dataPiece = m_NetWriter->GetNext(dataPiece->GetPieceIndex()))
	{
		MonoMediaDataPiecePtr lpDataPacket = dataPiece->ToMonoPiece();
		if ( ! lpDataPacket )
			continue;
		NETWRITER_DEBUG( "CMkvNetWriter::MkvMediaClient SetPlayableRangeAfterSended Piece index:" << lpDataPacket->GetPieceIndex() <<" Piece length:"<<lpDataPacket->GetMediaDataLength());
		if( lpDataPacket->GetHeaderPiece() > HeaderIndex )
		{	// 说明这个文件已经非常平滑的就播放完毕了啊，于是自然就要断开连接了
			NETWRITER_DEBUG( "MkvMediaClient::SetPlayableRangeAfterSended Send Media Data Gracefully End "<<*m_Socket );
			m_NetWriter->SavePlaytoIndex(lpDataPacket->GetPieceIndex() - 1);
			return E_SEND_ERROR;
		}
		if( lpDataPacket->GetHeaderPiece() != HeaderIndex )
			continue;
		if(lpDataPacket->GetPieceIndex()!= m_WillPlayIndex)
		{
			if(IsInDataUnit())
			{
				NETWRITER_ERROR("MkvMediaClient::SetPlayableRangeAfterSended - InDataUnit Missed: "<< m_WillPlayIndex <<"Actual:"<<lpDataPacket->GetPieceIndex()<<"SubDataIndex:"<< m_PiecesIndexWanted);
				PrepareForNextDataUnit();
			}
			else
			{
				NETWRITER_ERROR("MkvMediaClient::SetPlayableRangeAfterSended - NotInDataUnit Missed: "<< m_WillPlayIndex <<"Actual:"<<lpDataPacket->GetPieceIndex());
			}
		}
		//piece is OK, let's check data units
		const BYTE* data = lpDataPacket->GetMediaData();
		const UINT32 length = (UINT32)lpDataPacket->GetMediaDataLength();
		UINT32 off = 0;
		while(off < length)
		{
			UINT8 piecesIndex = *((UINT8 *)(data + off));
			UINT8 piecesNumber = *((UINT8 *)(data + off + 1));
			UINT16 pieceSize = *((UINT16 *)(data + off + 2));
			UINT32 packetSize = 0;
			if( piecesNumber == 1 && piecesIndex == 0) // a single data unit
			{
				packetSize = pieceSize;
				m_PacketSize = packetSize;
				m_PiecesIndexWanted = 0;
				m_piecesNumber = piecesNumber;
				off += 4;
				m_DataUnitBuffer.append(data+off, packetSize);
				off += packetSize;	
			}
			else if( piecesNumber > 1 )  //sliced data unit
			{
				packetSize = *((UINT32 *)(data + off + 4));				
				off += 8;
				if(IsInDataUnit())
				{
					if( m_PacketSize == packetSize && m_PiecesIndexWanted == piecesIndex && m_piecesNumber == piecesNumber)
					{
						m_DataUnitBuffer.append(data + off, pieceSize);
						m_PiecesIndexWanted++;
						off += pieceSize;
						if(m_PiecesIndexWanted == m_piecesNumber)
						{
							if(!SendToClient(lpDataPacket->GetPieceIndex()))
							{
								return E_SEND_ERROR;
							}
						}
					}
					else // Data pieces is error, skip this data unit
					{
						off += pieceSize;
						PrepareForNextDataUnit();
						continue;
					}
				}
				else // Not in a data unit
				{
					if(piecesIndex != 0)
					{
						//corrupted data unit, skip this piece
						NETWRITER_ERROR("MkvMediaClient::SetPlayableRangeAfterSended - Corrupted data unit : PieceIndex:"<<lpDataPacket->GetPieceIndex()<<
						"Off:"<<off << "SubDataUnitIndex: "<< piecesIndex);
						off += pieceSize;
						continue;  
					}
					else // go into a data unit
					{						
						//send complete data to client first
						if(!SendToClient(lpDataPacket->GetPieceIndex()))
						{
							return E_SEND_ERROR;
						}
						m_PacketSize = packetSize;
						m_piecesNumber = piecesNumber;
						m_DataUnitBuffer.append(data + off, pieceSize);
						m_PiecesIndexWanted++;
						off += pieceSize;
						continue;
					}
				}
			}
			else if( piecesNumber == 0) // This is a format error!
			{
				NETWRITER_ERROR("MkvMediaClient::SetPlayableRangeAfterSended - Error: PieceNumber is 0 : PieceIndex:"<<lpDataPacket->GetPieceIndex()<<
					"Off:"<<off);
				m_NetWriter->SavePlaytoIndex(lpDataPacket->GetPieceIndex() - 1);
				return E_SEND_ERROR;
			}
		}
		LIVE_ASSERT(off == length);
		//send complete data units to client
		if(!IsInDataUnit())
		{
			if(!SendToClient(lpDataPacket->GetPieceIndex()))
			{
				return E_SEND_ERROR;
			}
		}
		m_LastPlayIndex = lpDataPacket->GetPieceIndex();
		m_NetWriter->m_dwLastIndex = m_LastPlayIndex + 1;
		m_WillPlayIndex = m_LastPlayIndex + 1;
	}
	m_WillPlayIndex = m_LastPlayIndex + 1;
	m_NetWriter->SavePlaytoIndex(m_LastPlayIndex);
	LIVE_ASSERT( m_WillPlayIndex > m_LastPlayIndex );
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
//   类 CMkvNetWriter 的实现部分
//////////////////////////////////////////////////////////////////////////

CMkvNetWriter::CMkvNetWriter(CMediaServer* lpMediaServer, MonoMediaHeaderPiecePtr lpPacket):
CNetWriter(lpMediaServer, lpPacket)
{

}

CMkvNetWriter::~CMkvNetWriter()
{
	if( m_state != st_stop )
		this->Stop();
}

HRESULT CMkvNetWriter::Stop()
{
	if( E_FAIL == CNetWriter::Stop() )
		return E_FAIL;
	// 丢掉剩下的所有的,这样保证能够顺利的播放到尾部
	for( MkvMediaClientCollection::iterator iter = m_ClientHandles.begin(); iter != m_ClientHandles.end(); iter ++ )
	{
		iter->second->SetPlayableRange( m_lpMediaHead->GetPieceIndex(), m_MinIndex, m_MaxIndex );
		delete iter->second;
	}
	m_ClientHandles.clear();
	return S_OK;
}

HRESULT CMkvNetWriter::SetPlayableRange(DWORD MinIndex,DWORD MaxIndex)
{
	if( m_state != st_start )
	{
		NETWRITER_ERROR( "SetPlayableRange-CMkvNetWriter is not started, so can not stop! " );
		return E_FAIL;
	}
	
	//NETWRITER_INFO( "CMkvNetWriter::SetPlayableRange MinIndex = "<<MinIndex<<" MaxIndex = "<<MaxIndex );

	if( MinIndex > m_MinIndex ) m_MinIndex = MinIndex;
	if( MaxIndex > m_MaxIndex ) m_MaxIndex = MaxIndex;

	for( MkvMediaClientCollection::iterator iter = m_ClientHandles.begin(); iter != m_ClientHandles.end();)
	{
		HRESULT hr = iter->second->SetPlayableRange( m_lpMediaHead->GetPieceIndex(), m_MinIndex, m_MaxIndex);
		if( hr == E_SEND_ERROR || hr == E_FORECASR_NOT_GOOD )
		{
			delete iter->second;
			m_ClientHandles.erase(iter++);

			continue;
		}
		LIVE_ASSERT( hr == S_OK || hr == E_LOCATION_ERROR );
		iter ++;
	}

	//NETWRITER_DEBUG( "MinIndex : " << MinIndex << " MaxIndex : " << MaxIndex );
	return S_OK;
}

void CMkvNetWriter::DisconnectAllMediaClient()
{
	for( MkvMediaClientCollection::iterator iter = m_ClientHandles.begin(); iter != m_ClientHandles.end(); iter ++ )
	{
		delete iter->second;
	}
	m_ClientHandles.clear();
}


bool CMkvNetWriter::OnNewClient(tcp_socket_ptr s)
{
	NETWRITER_EVENT("CMkvNetWriter::OnNewClient " << s);
	LIVE_ASSERT(m_ClientHandles.find(s.get()) == m_ClientHandles.end());
	MkvMediaClient* pClientHandle = new MkvMediaClient(this, s);
	// 将这个SOCKET挂接到 m_ClientHandles
	m_ClientHandles[s.get()] = pClientHandle;
	return true;
}

void CMkvNetWriter::OnClientError(tcp_socket* sock, long errcode)
{
	NETWRITER_EVENT("CMkvNetWriter::OnClientError " << *sock << " ErrCode:" <<errcode);
	MkvMediaClientCollection::iterator iter = m_ClientHandles.find(sock);
	if ( iter == m_ClientHandles.end() )
	{
		LIVE_ASSERT(false);
		return;
	}
	delete iter->second;
	m_ClientHandles.erase(iter);
}

bool CMkvNetWriter::ContainsClient(tcp_socket* sock)
{
	return m_ClientHandles.find(sock)!=m_ClientHandles.end();
}

//////////////////////////////////////////////////////////////////////////
//   类 NetWriterFactory 的实现部分
//////////////////////////////////////////////////////////////////////////

CNetWriter* NetWriterFactory::CreateMkvNetWriter(CMediaServer* mediaServer, MonoMediaHeaderPiecePtr headerPiece)
{
	return new CMkvNetWriter(mediaServer, headerPiece);
}