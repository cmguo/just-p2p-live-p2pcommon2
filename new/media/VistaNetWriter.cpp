#include "StdAfx.h"

#include "VistaNetWriter.h"
#include "common/MediaPiece.h"


// const GUID GUID_ASF_FILE_PROPERTIES_OBJECT =
// 	{0x8CABDCA1, 0xA947, 0x11CF, {0x8E,0xE4,0x00,0xC0,0x0C,0x20,0x53,0x65} };

//////////////////////////////////////////////////////////////////////////
//   结构 VistaMediaClient 的实现部分
//////////////////////////////////////////////////////////////////////////
VistaMediaClient::VistaMediaClient() 
: MediaClient(), m_DataUnitSize(0),m_MmshIndex(0), m_PieceTimeStamp(0), m_LastPieceTimeStamp(0)
{
}

VistaMediaClient::VistaMediaClient(DWORD DataUnitSize) 
: MediaClient(),m_DataUnitSize(DataUnitSize), m_MmshIndex(0), m_PieceTimeStamp(0), m_LastPieceTimeStamp(0)
{
}

VistaMediaClient::VistaMediaClient( CNetWriter* netWriter, tcp_socket_ptr sock, DWORD DataUnitSize) 
: MediaClient( netWriter, sock ), m_DataUnitSize(DataUnitSize), m_MmshIndex(0), m_PieceTimeStamp(0), m_LastPieceTimeStamp(0)
{
}

HRESULT VistaMediaClient::SetPlayableRange( 
	UINT HeaderIndex, 
	DWORD MinIndex,
	DWORD MaxIndex)
{
	if( false == m_IsSending )
	{
		// 还没有发送任何数据, 于是就要首先定位StartIndex
		DWORD StartIndex = m_NetWriter->LocateStartPosition();

		if( StartIndex == 0 )
		{
			NETWRITER_ERROR( "MediaClient LocationStartIndex Error");
			return E_LOCATION_ERROR;
		}

		m_WillPlayIndex = StartIndex;
		m_IsSending = true;
		NETWRITER_INFO("MediaClient SetPlayableRange: StartIndex=" << StartIndex << ", MinMax=" << make_tuple(MinIndex, MaxIndex));
	}
	LIVE_ASSERT( true == m_IsSending );
	return SetPlayableRangeAfterSended(HeaderIndex, MinIndex, MaxIndex);
}

HRESULT VistaMediaClient::End()
{
	BYTE MmsEndOfStreamTag[12] = {0x24,0x45,0x08,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x08,0x00};
	bool SendOk = SendData( MmsEndOfStreamTag, 12 );
	if( SendOk == false )
	{
		NETWRITER_ERROR( "CNetWriter Send End Of Stream Tag Error " << *m_Socket << " " << m_LastSendPieceTick.elapsed() );
		return E_SEND_ERROR;
	}
	NETWRITER_INFO( "CNetWriter Send End Of Stream Tag OK " << *m_Socket << " " << m_LastSendPieceTick.elapsed() );

	return S_OK;	
}

void VistaMediaClient::OnIntervalTimer(int seconds)
{
	if ( m_LastSendPieceTick.elapsed() > 30*1000 )
	{
		m_MmshIndex ++;
		BYTE MmsResetOfStreamTag[13] = {0x24,0x4D,0x09,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x09,0x00, 0x00};
		memcpy( MmsResetOfStreamTag + 4, &m_MmshIndex, 4 );
		bool SendOk = SendData( MmsResetOfStreamTag, 13 );
		if( SendOk == false )
		{	// 我觉得这种情况很少见，所以没有必要去断开
			NETWRITER_ERROR( "CNetWriter Send Script Tag Error " << *m_Socket << " " << m_LastSendPieceTick.elapsed() );
		}
		else
		{
			NETWRITER_ERROR( "CNetWriter Send Script Tag OK " << *m_Socket << " " << m_LastSendPieceTick.elapsed() );
		}
		m_LastSendPieceTick.sync();
	}	
}

HRESULT VistaMediaClient::SendPiece(MonoMediaDataPiecePtr lpDataPacket)
{
	bool sendOk = true;
	const BYTE* pieceBuffer = lpDataPacket->GetMediaData();
	DWORD pieceIndex = lpDataPacket->GetPieceIndex();
	UINT16 pieceSize = (UINT16)lpDataPacket->GetMediaDataLength();
	
	if( m_DataUnitSize > 0 )
	{	// 成功读到 Asf 的DataUnitSize, 则转化成mmsh协议流来传输

		UINT16 indexSize = 0;
		while( indexSize < pieceSize )
		{
			m_MmshIndex ++;
			BYTE MmsDataUnitTag[12] = {0x24,0x44,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00};
			WORD MmsDataUnitLength = (WORD)(m_DataUnitSize + 8);
			memcpy( MmsDataUnitTag + 2, &MmsDataUnitLength,2 );
			memcpy( MmsDataUnitTag + 4, &m_MmshIndex, 4 );
			memcpy( MmsDataUnitTag + 10, &MmsDataUnitLength,2 );
			sendOk = SendData( MmsDataUnitTag, 12 );
			if( sendOk == false )
			{
				NETWRITER_ERROR( "CNetWriter Send Mmsh Media Data Tag Error " << pieceIndex << " " << *m_Socket );
				m_NetWriter->SavePlaytoIndex(pieceIndex - 1);
				return E_SEND_ERROR;
			}

			sendOk = SendData( pieceBuffer + indexSize, m_DataUnitSize );
			if( sendOk == false )
			{
				NETWRITER_ERROR( "CNetWriter Send Mmsh Media Data Error " << pieceIndex << " " << *m_Socket );
				m_NetWriter->SavePlaytoIndex(pieceIndex - 1);
				return E_SEND_ERROR;
			}

			indexSize = indexSize + (UINT16)m_DataUnitSize;
		}
	}
	else
	{	// 如果读不到 Asf 的DataUnitSize, 保留 Mmsh 流传输的形式
		sendOk = SendData( pieceBuffer, pieceSize );
		if( sendOk == false )
		{
			NETWRITER_ERROR( "CNetWriter Send Media Data Error " << pieceIndex <<*m_Socket );
			m_NetWriter->SavePlaytoIndex(pieceIndex - 1);
			return E_SEND_ERROR;
		}
	}

	NETWRITER_INFO( "SendMediaData "<<*m_Socket<<" "<<pieceIndex<<" "<<m_LastSendPieceTick.elapsed());
	VIEW_INFO( "PieceSend " << pieceIndex << " " << m_LastSendPieceTick.elapsed() << " End");
	m_LastPieceTick.sync();
	m_LastPieceTimeStamp = lpDataPacket->GetTimeStamp();
	m_LastSendPieceTick.sync();
	m_LastPlayIndex = pieceIndex;
	
	return S_OK;
}

HRESULT VistaMediaClient::SetPlayableRangeAfterSended( 
												 UINT HeaderIndex,
												 DWORD MinIndex,
												 DWORD MaxIndex)
{
	LIVE_ASSERT( true == m_IsSending );
	//bool SendOk = true;

	if( MinIndex == 0 ) return S_OK;
	if( MaxIndex == 0 ) return S_OK;

	MediaDataPiecePtr lpMinDataPacket = m_NetWriter->GetFirst(MinIndex);
	if (!lpMinDataPacket)
		return S_OK;
	UINT64 MinDataTimeStamp = lpMinDataPacket->GetTimeStamp();
	UINT64 MinPlayTimeStamp = MinDataTimeStamp + 10*1000;

	if( m_PieceTimeStamp == 0  )
	{	
		// 还没有 推过流

		UINT64 FirstPieceTimeStamp = 0;

		for (MediaDataPiecePtr dataPiece = m_NetWriter->GetFirst(m_WillPlayIndex); 
				(dataPiece && dataPiece->GetPieceIndex() <= MaxIndex);
				dataPiece = m_NetWriter->GetNext(dataPiece->GetPieceIndex()))
		{
			MonoMediaDataPiecePtr lpDataPacket = dataPiece->ToMonoPiece();
			if ( ! lpDataPacket )
				continue;
			UINT i = lpDataPacket->GetPieceIndex();
			if( lpDataPacket->GetHeaderPiece() > HeaderIndex )
			{	// 说明这个文件已经非常平滑的就播放完毕了啊，于是自然就要断开连接了
				NETWRITER_ERROR( "CNetWriter Send Media Data Gracefully End "<<*m_Socket );
				m_NetWriter->SavePlaytoIndex(i - 1);
				return E_SEND_ERROR;
			}
			if( lpDataPacket->GetHeaderPiece() != HeaderIndex )
				continue;

			if( FirstPieceTimeStamp > 0 && FirstPieceTimeStamp+10*1000<lpDataPacket->GetTimeStamp())
			{	// 开始快推10s的数据给播放器
				break;
			}
			
			HRESULT SendResult = SendPiece(lpDataPacket);
			if( SendResult != S_OK )
			{
				return SendResult;
			}

			// 将推给播放器的最后一片的时间戳做为 基准时间戳
			m_BaseTick.sync();
			m_PieceTimeStamp = lpDataPacket->GetTimeStamp();

			if( FirstPieceTimeStamp == 0 )
			{
				FirstPieceTimeStamp = lpDataPacket->GetTimeStamp();
			}
		}
	}
	else
	{
		// 已经推过流了
		UINT64 RelativePieceTimeStamp = GetRelativePieceTimeStamp() + 500;	// +500 ms 是为了避免离散性引起的差异
		for (MediaDataPiecePtr dataPiece = m_NetWriter->GetFirst(m_WillPlayIndex); 
				(dataPiece && dataPiece->GetPieceIndex() <= MaxIndex);
				dataPiece = m_NetWriter->GetNext(dataPiece->GetPieceIndex()))
		{
			MonoMediaDataPiecePtr lpDataPacket = dataPiece->ToMonoPiece();
			if ( ! lpDataPacket )
				continue;
			UINT i = lpDataPacket->GetPieceIndex();
			if( lpDataPacket->GetHeaderPiece() > HeaderIndex )
			{	// 说明这个文件已经非常平滑的就播放完毕了啊，于是自然就要断开连接了
				NETWRITER_ERROR( "CNetWriter Send Media Data Gracefully End "<<*m_Socket );
				m_NetWriter->SavePlaytoIndex(i - 1);
				return E_SEND_ERROR;
			}
			if( lpDataPacket->GetHeaderPiece() != HeaderIndex )
				continue;

			if( lpDataPacket->GetTimeStamp() > RelativePieceTimeStamp && 
				lpDataPacket->GetTimeStamp() > MinPlayTimeStamp )
			{
				break;
			}
			
			HRESULT SendResult = SendPiece(lpDataPacket);
			if( SendResult != S_OK )
			{
				return SendResult;
			}
		}

		// 如果连续1s秒没有推流，则调整基准时间戳
		//if( m_LastPieceTick.GetElapsed() > 1000 )
		//{
		//	m_BaseTick = m_LastPieceTick;
		//	m_PieceTimeStamp = m_LastPieceTimeStamp;
		//	VIEW_INFO("VistaMediaClient Base Chanege: "<<m_LastPieceTick.GetElapsed() );
		//}
	}	

	m_WillPlayIndex = m_LastPlayIndex + 1;
	m_NetWriter->m_dwLastIndex = m_LastPlayIndex + 1;
	m_NetWriter->SavePlaytoIndex(m_LastPlayIndex);	

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
//   类 CVistaNetWriter 的实现部分
//////////////////////////////////////////////////////////////////////////

CVistaNetWriter::CVistaNetWriter( CMediaServer* lpMediaServer, MonoMediaHeaderPiecePtr lpPacket )
: CNetWriter( lpMediaServer, lpPacket )
{
	// 计算出 m_DataUnitSize
// 	const BYTE* buffer = lpPacket->GetHeader();
// 	const UINT length = lpPacket->GetHeaderLength();
// 
// 	//分析ASF文件剩下的字节,关键是把 ASF_File_Properties_Object 的重要信息解析出来
// 	DWORD FilePropObjGuidIndex = 0;
// 	for( ; FilePropObjGuidIndex < length-24-104; FilePropObjGuidIndex++)
// 	{
// 		if( GUID_ASF_FILE_PROPERTIES_OBJECT == *((GUID*)(buffer+FilePropObjGuidIndex)) ) 
// 			break;
// 	}
// 	if( FilePropObjGuidIndex >= length-24-104 )
// 	{
// 		m_DataUnitSize = 0;
// 	}
// 	// ASF文件的切片数目
// 	DWORD m_PacketNumber = *(DWORD*)(buffer+FilePropObjGuidIndex+56);
// 	// ASF文件的每个切片的长度
// 	DWORD m_PacketLength = *(DWORD*)(buffer+FilePropObjGuidIndex+92);
// 	DWORD BakPacketLength = *(DWORD*)(buffer+FilePropObjGuidIndex+96);
// 	if( BakPacketLength != m_PacketLength )
// 	{
// 		m_DataUnitSize = 0;
// 	}
// 	m_DataUnitSize = m_PacketLength;
}

CVistaNetWriter::~CVistaNetWriter()
{
	if( m_state != st_stop )
		this->Stop();
}

HRESULT CVistaNetWriter::Stop()
{
	if( E_FAIL == CNetWriter::Stop() )
		return E_FAIL;
	// 丢掉剩下的所有的,这样保证能够顺利的播放到尾部
	for( VistaMediaClientCollection::iterator iter = m_ClientHandles.begin(); iter != m_ClientHandles.end(); iter ++ )
	{
		iter->second.SetPlayableRange( m_lpMediaHead->GetPieceIndex(), m_MinIndex, m_MaxIndex);
		iter->second.End();
	}
	m_ClientHandles.clear();
	return S_OK;
}

HRESULT CVistaNetWriter::SetPlayableRange(DWORD MinIndex,DWORD MaxIndex)
{
	if( m_state != st_start )
	{
		NETWRITER_ERROR( "SetPlayableRange-CVistaNetWriter is not started, so can not stop! " );
		return E_FAIL;
	}
	
	//NETWRITER_INFO( "CVistaNetWriter::SetPlayableRange MinIndex = "<<MinIndex<<" MaxIndex = "<<MaxIndex << " " << m_ClientHandles.size());

	if( MinIndex > m_MinIndex ) m_MinIndex = MinIndex;
	if( MaxIndex > m_MaxIndex ) m_MaxIndex = MaxIndex;

	for( VistaMediaClientCollection::iterator iter = m_ClientHandles.begin(); iter != m_ClientHandles.end();)
	{
		HRESULT hr = iter->second.SetPlayableRange( m_lpMediaHead->GetPieceIndex(), m_MinIndex, m_MaxIndex );
		if( hr == E_SEND_ERROR || hr == E_FORECASR_NOT_GOOD )
		{
			NETWRITER_EVENT("CVistaNetWriter::OnClientError " << *iter->second.m_Socket << make_tuple(hr, m_ClientHandles.size()));
			iter->second.End();
			m_ClientHandles.erase(iter++);
			continue;
		}
		LIVE_ASSERT( hr == S_OK || hr == E_LOCATION_ERROR );
		++iter;
	}

	//NETWRITER_DEBUG( "MinIndex : " << MinIndex << " MaxIndex : " << MaxIndex );
	return S_OK;
}


bool CVistaNetWriter::OnClientReceive(tcp_socket_ptr s, const BYTE* data, size_t size)
{
	if( m_state != st_start )
	{
		NETWRITER_ERROR( "OnHttpRecvSuccess-CNetWriter is not started, so can not stop! " << *s);
		return false;
	}
	
	bool SendOk;
	
	// 检查 m_ClientHandles 是否有此套接字，如果有此套接字，则退出
	if( ContainsClient(s.get()) )
	{
		LIVE_ASSERT( 0 );
		NETWRITER_ERROR( "CNetWriter::ContainsClient has this SOCKET "<<*s );
		return true;
	}

	if( m_DataUnitSize > 0)
	{	// 成功读到 Asf 的DataUnitSize, 则转化成mmsh协议流来传输
		string RecvString( reinterpret_cast<const char*>( data ),size );
		size_t nPos1 = RecvString.find("Pragma: xPlayStrm=");
		size_t nPos2 = RecvString.find("Pragma: client-id=");
		size_t nPos3 = RecvString.find("Pragma: stream-switch-count=");
		size_t nPos4 = RecvString.find("Pragma: stream-switch-entry=");
		if( nPos1 == string::npos || nPos2 == string::npos || nPos3 == string::npos || nPos4 == string::npos )
		{	// 是 mmsh 的 试探请求
			const BYTE *buffer = m_lpMediaHead->GetHeader();
			int length = m_lpMediaHead->GetHeaderLength();
	
			// 没有找到字符
			char http_header[10*1024];
			sprintf(http_header, MMSH_HTTP_FIRST_HEADER, 12+length-m_MediaHeaderPosition);
	
			// 发送HTTP的头部
			SendOk = SendDataToClient( s, reinterpret_cast<const BYTE*>( http_header ), strlen(http_header) );
			if( SendOk == false )
			{
				NETWRITER_ERROR( "CNetWriter Send MMSH HTTP Header Error "<<*s );
				return false;
			}
	
			// 发送MMSH头部Tag 
			BYTE MmsHeaderTag[12] = {0x24,0x48,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x0C,0x00,0x00};
			WORD MmsHeaderLength = (WORD)(length - m_MediaHeaderPosition + 8);
			memcpy( MmsHeaderTag + 2, &MmsHeaderLength,2 );
			memcpy( MmsHeaderTag + 10, &MmsHeaderLength,2 );
			SendOk = SendDataToClient( s, MmsHeaderTag, 12 );
			if( SendOk == false )
			{
				NETWRITER_ERROR( "CNetWriter Send Mmsh Header Tag Error "<<*s );
				return false;
			}
		
			// 发送Media的头部
			SendOk = SendDataToClient( s, buffer+m_MediaHeaderPosition, length-m_MediaHeaderPosition );
			if(	SendOk == false )
			{
				NETWRITER_ERROR( "CNetWriter Send Media Header Error "<<*s );
				return false;
			}
			NETWRITER_DEBUG( "CNetWriter Send Media Header "<<m_lpMediaHead->GetPieceIndex() );
			VIEW_INFO("PieceSend "<<m_lpMediaHead->GetPieceIndex()<<" End");
	
			return true;
		}
		else
		{	// 是 mmsh 的 正式请求
			SendOk = SendDataToClient(s, MMSH_HTTP_SECOND_HEADER, strlen(MMSH_HTTP_SECOND_HEADER) );
			if( SendOk == false )
			{
				NETWRITER_ERROR( "CNetWriter Send MMSH HTTP Header Error "<<*s );
				return false;
			}

			const BYTE *buffer = m_lpMediaHead->GetHeader();
			int length = m_lpMediaHead->GetHeaderLength();

			// 发送MMSH头部Tag 
			BYTE MmsHeaderTag[12] = {0x24,0x48,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x0C,0x00,0x00};
			WORD MmsHeaderLength = (WORD)(length - m_MediaHeaderPosition + 8);
			memcpy( MmsHeaderTag + 2, &MmsHeaderLength,2 );
			memcpy( MmsHeaderTag + 10, &MmsHeaderLength,2 );
			SendOk = SendDataToClient( s, MmsHeaderTag, 12 );
			if( SendOk == false )
			{
				NETWRITER_ERROR( "CNetWriter Send Mmsh Header Tag Error "<<*s );
				return false;
			}
			
			// 发送Media的头部
			SendOk = SendDataToClient( s, buffer+m_MediaHeaderPosition, length-m_MediaHeaderPosition );
			if( SendOk == false )
			{
				NETWRITER_ERROR( "CNetWriter Send Media Header Error "<<*s );
				return false;
			}
			NETWRITER_DEBUG( "CNetWriter Send Media Header "<<m_lpMediaHead->GetPieceIndex() );
			VIEW_INFO("PieceSend "<<m_lpMediaHead->GetPieceIndex()<<" End");

			return OnNewClient(s);	
		}
	}
	else
	{	// 如果读不到 Asf 的DataUnitSize, 保留 Mmsh 流传输的形式

		// 发送HTTP的头部
		SendOk = SendDataToClient( s, HTTP_HEADER, sizeof(HTTP_HEADER)-1 );
		if( SendOk == false )
		{
			NETWRITER_ERROR( "CNetWriter Send HTTP Header Error "<<*s );
			return false;
		}
		
		// 发送Media的头部
		const BYTE *buffer = m_lpMediaHead->GetHeader();
		int length = m_lpMediaHead->GetHeaderLength();
		SendOk = SendDataToClient( s, buffer+m_MediaHeaderPosition, length-m_MediaHeaderPosition );
		if( SendOk == false )
		{
			NETWRITER_ERROR( "CNetWriter Send Media Header Error "<<*s );
			return false;
		}
		NETWRITER_DEBUG( "CNetWriter Send Media Header "<<m_lpMediaHead->GetPieceIndex() );
		VIEW_INFO("PieceSend "<<m_lpMediaHead->GetPieceIndex()<<" End");
		
		return OnNewClient(s);
	}
}

void CVistaNetWriter::OnClientError(tcp_socket* sock, long errcode)
{
	NETWRITER_EVENT("CVistaNetWriter::OnClientError " << *sock << make_tuple(errcode, m_ClientHandles.size()));
	if( m_ClientHandles.find(sock) != m_ClientHandles.end() )
	{
		m_ClientHandles.find(sock)->second.End();
	}
	m_ClientHandles.erase( sock );
}

bool CVistaNetWriter::ContainsClient(tcp_socket* sock)
{
	return m_ClientHandles.find(sock) != m_ClientHandles.end();
}

bool CVistaNetWriter::OnNewClient(tcp_socket_ptr s)
{
	NETWRITER_EVENT("CVistaNetWriter::OnNewClient " << make_tuple(s, m_ClientHandles.size()));
	LIVE_ASSERT( m_ClientHandles.find(s.get()) == m_ClientHandles.end() );
	VistaMediaClient ClientHandle( this, s, m_DataUnitSize );
	// 将这个SOCKET挂接到 m_ClientHandles
	m_ClientHandles[s.get()] = ClientHandle;
	return true;
}

void CVistaNetWriter::DisconnectAllMediaClient()
{
	NETWRITER_INFO("DisconnectAllMediaClient");
	for( VistaMediaClientCollection::iterator iter = m_ClientHandles.begin(); iter != m_ClientHandles.end(); iter ++ )
	{
		iter->second.End();
	}
	m_ClientHandles.clear();
}

void CVistaNetWriter::OnIntervalTimer(int seconds)
{
	SetPlayableRange(m_MinIndex, m_MaxIndex);
	for( VistaMediaClientCollection::iterator iter = m_ClientHandles.begin(); iter != m_ClientHandles.end(); iter ++ )
	{
		iter->second.OnIntervalTimer(seconds);
	}
}

CNetWriter* NetWriterFactory::CreateVistaMediaNetWriter(CMediaServer* mediaServer, MonoMediaHeaderPiecePtr headerPiece)
{
	return new CVistaNetWriter(mediaServer, headerPiece);
}
