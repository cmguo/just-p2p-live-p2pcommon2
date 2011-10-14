#include "StdAfx.h"

#include "VistaHttpNetWriter.h"
#include "common/MediaPiece.h"
#include "AsfBase.h"
#include "AsfHelper.h"

// Added for test.
#include <ppl/diag/trace.h>
// const GUID GUID_ASF_FILE_PROPERTIES_OBJECT =
// 	{0x8CABDCA1, 0xA947, 0x11CF, {0x8E,0xE4,0x00,0xC0,0x0C,0x20,0x53,0x65} };





//////////////////////////////////////////////////////////////////////////
//   结构 VistaHttpMediaClient 的实现部分
//////////////////////////////////////////////////////////////////////////
VistaHttpMediaClient::VistaHttpMediaClient() 
: MediaClient(), m_DataUnitSize(0)
{
//	m_NeedStartWithKeyFrame = true;
}

//VistaHttpMediaClient::VistaHttpMediaClient(DWORD DataUnitSize) 
//: MediaClient(), m_DataUnitSize(DataUnitSize)
//{
////	m_NeedStartWithKeyFrame = true;
//}

VistaHttpMediaClient::VistaHttpMediaClient( CNetWriter* netWriter, tcp_socket_ptr sock, DWORD DataUnitSize) 
: MediaClient( netWriter, sock ), m_DataUnitSize(DataUnitSize)
{
//	m_NeedStartWithKeyFrame = true;
}

HRESULT VistaHttpMediaClient::SetPlayableRange( 
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
	return SetPlayableRangeAfterSended(HeaderIndex,MinIndex,MaxIndex);
}

/*
size_t VistaHttpMediaClient::LocateTimeStampOffset( const BYTE * const packetBuffer, const size_t maxPacketSize )
{
	LIVE_ASSERT( ( packetBuffer[3] & 0x80 ) == 0 );	// no error correction
	LIVE_ASSERT( packetBuffer[4] == 0x5D );					// payload length type OK

	LengthType packetLengthType = (LengthType)( ( packetBuffer[3] & 0x60 ) >> 5 );
	LengthType paddingLengthType = (LengthType)( ( packetBuffer[3] & 0x18 ) >> 3 );
	LengthType sequenceType = (LengthType)( ( packetBuffer[3] & 0x06 ) >> 1 );

	// data unit
	size_t packetInfoOffset = 5;	// 0x820000, 09/08/11, 5D
	return packetInfoOffset + 
		TypeToLength( packetLengthType ) + 
		TypeToLength( sequenceType ) + 
		TypeToLength( paddingLengthType );
}

DWORD VistaHttpMediaClient::GetTimestamp(BYTE * const packetBuffer, const size_t maxPacketSize)
{
	size_t timeStampOffset = LocateTimeStampOffset( packetBuffer, maxPacketSize );
	DWORD* sendTime = (DWORD*)(packetBuffer + timeStampOffset);
	return *sendTime;
}

bool VistaHttpMediaClient::ChanegeTimeStamps(BYTE * const packetBuffer, const size_t maxPacketSize, const DWORD startTimeStamp)
{
	bool single = ( ( packetBuffer[3] & 0x01 ) != 0x01 );
	size_t timeStampOffset = LocateTimeStampOffset( packetBuffer, maxPacketSize );
	
	DWORD* sendTime = (DWORD*)(packetBuffer + timeStampOffset);
	LIVE_ASSERT(*sendTime >= startTimeStamp);
	*sendTime = *sendTime - startTimeStamp;
	
	size_t payloadOffset = timeStampOffset + 4 + 2;		// send time, duration
	size_t payloadCount = 1;
	if ( ! single )
	{
		LIVE_ASSERT( ( packetBuffer[payloadOffset] & 0x80 ) == 0x80 );
		payloadOffset += 1;								// multi start with payloads info, 1B
		payloadCount = packetBuffer[payloadOffset - 1] & 0x3F;
	}

	if ( payloadOffset > 13 )
	{
		//cout << "payload offset: " << payloadOffset << endl;
	}

	size_t parsedPayload = 0;
	while( parsedPayload < payloadCount )
	{
		if ( payloadOffset + 15 >= maxPacketSize )	// 检查下面要访问的内存合法
		{
			break;
		}

		/// is key frame
		// packetBuffer[payloadOffset] & (BYTE)0x80 == (BYTE)0x80;
		
		/// stream number
		// BYTE streamID = packetBuffer[payloadOffset] & 0x7F;

		/// object ID
		// BYTE objectID = packetBuffer[payloadOffset+1];

		/// offset
		// UINT32 offset = *( (UINT32*)(packetBuffer + payloadOffset + 2) );
		size_t replicatedDataLength = packetBuffer[payloadOffset + 6];
		size_t replicatedDataOffset = 7;
		size_t representTimeOffset = 4;
		/// object size
		// DWORD objectSize = *( (UINT32*) (packetBuffer + payloadOffset + replicatedDataOffset) );
		DWORD* presentTime = (DWORD*)( packetBuffer + payloadOffset + replicatedDataOffset + representTimeOffset );
		if( *presentTime  > startTimeStamp )
		{
			*presentTime = *presentTime - startTimeStamp;
		}	
		else
		{
// 			LIVE_ASSERT( false );
 			return false;
		}

		if ( single )
		{
			break;
		}

		parsedPayload += 1;

		size_t payloadLengthOffset = replicatedDataOffset + replicatedDataLength;
		if ( payloadOffset + payloadLengthOffset + 2 >= maxPacketSize )	// 检查下面访问payloadLength是否合法
		{
			break;
		}
		UINT16 payloadLength = *(UINT16*)( packetBuffer + payloadOffset + payloadLengthOffset );
		payloadOffset += (payloadLength + payloadLengthOffset + 2);
	}
	return true;
}
*/
HRESULT VistaHttpMediaClient::SendPiece(MonoMediaDataPiecePtr lpDataPacket)
{
	bool SendOk = true;
	BYTE* PieceBuffer = const_cast<BYTE*>( lpDataPacket->GetMediaData() );
	DWORD PieceIndex = lpDataPacket->GetPieceIndex();
	 
	UINT16 PieceSize = (UINT16)lpDataPacket->GetMediaDataLength();
	
	if( m_DataUnitSize > 0 )
	{	// 成功读到 Asf 的DataUnitSize, 则转化成mmsh协议流来传输
		pool_byte_buffer& buffer = m_NetWriter->GetPieceOperationBuffer();
		buffer.ensure_size(m_DataUnitSize);
		UINT16 IndexSize = 0;
		while( IndexSize < PieceSize )
		{
			memcpy(buffer.data(), PieceBuffer + IndexSize, m_DataUnitSize);
			LIVE_ASSERT(m_DataUnitSize == buffer.size());
			if( true == m_isFirst )
			{
				m_StartTimeStamp = AsfHelper::GetTimestamp(buffer.data(), m_DataUnitSize);
				m_StartTimeStampPL = AsfHelper::GetPresentTime(buffer.data(), buffer.size());
//				m_StartTimeStampPL = m_NetWriter->GetPreroll();
				if (m_StartTimeStamp > AsfHelper::MAX_TIMESTAMP - 2000 || m_StartTimeStampPL > AsfHelper::MAX_TIMESTAMP - 2000)
				{
					IndexSize = IndexSize + (UINT16)m_DataUnitSize;
					continue;
				}
				m_isFirst = false;
				m_bIsStartRound = true;
				m_uSendCount = 0;
				m_lastSendTS = 0;
				m_lastPresentTS = m_NetWriter->GetPreroll();
			}

//			if( false == ChanegeTimeStamps(buffer.data(), m_DataUnitSize, m_StartTimeStamp) )
			if (m_bIsStartRound == true)
			{
				if (m_uSendCount > 500)
				{
					m_bIsStartRound = false;
				}
				m_uSendCount++;
			}
			
			if ( false == AsfHelper::ChanegeTimeStamps( buffer.data(), buffer.size(), (UINT32&)m_StartTimeStamp, (UINT32&)m_StartTimeStampPL, m_NetWriter->GetPreroll(), m_bIsStartRound , m_lastSendTS, m_lastPresentTS) )
			{
				NETWRITER_ERROR( "ChanegeTimeStamps Error " << PieceIndex << " " << IndexSize << " " << *m_Socket );
				break;
			}

			// 对数据包进行偏移处理
			SendOk = SendData( buffer.data(), m_DataUnitSize );
			if( SendOk == false )
			{
				NETWRITER_ERROR( "CNetWriter Send Mmsh Media Data Error " << PieceIndex << " " << *m_Socket );
				m_NetWriter->SavePlaytoIndex(PieceIndex - 1);
				return E_SEND_ERROR;
			}

			IndexSize = IndexSize + (UINT16)m_DataUnitSize;
		}
	}
	else
	{	// 如果读不到 Asf 的DataUnitSize, 保留传统方式 传输
		SendOk = SendData( PieceBuffer, PieceSize );
		if( SendOk == false )
		{
			NETWRITER_ERROR( "CNetWriter Send Media Data Error " << PieceIndex <<*m_Socket );
			m_NetWriter->SavePlaytoIndex(PieceIndex - 1);
			return E_SEND_ERROR;
		}
	}
	
	return S_OK;
}


HRESULT VistaHttpMediaClient::SetPlayableRangeAfterSended( 
												 UINT HeaderIndex,
												 DWORD MinIndex,
												 DWORD MaxIndex)
{
	LIVE_ASSERT( true == m_IsSending );
	
	//bool SendOk = true;
	// 发送相应的数据
	for (MediaDataPiecePtr dataPiece = m_NetWriter->GetFirst(m_WillPlayIndex); 
			(dataPiece && dataPiece->GetPieceIndex() <= MaxIndex);
			dataPiece = m_NetWriter->GetNext(dataPiece->GetPieceIndex()))
	{
		MonoMediaDataPiecePtr lpDataPacket = dataPiece->ToMonoPiece();
		if ( ! lpDataPacket )
		{
			LIVE_ASSERT( false );
			continue;
		}
		UINT i = lpDataPacket->GetPieceIndex();
		if( lpDataPacket->GetHeaderPiece() > HeaderIndex )
		{	// 说明这个文件已经非常平滑的就播放完毕了啊，于是自然就要断开连接了
			NETWRITER_ERROR( "CNetWriter Send Media Data Gracefully End "<<*m_Socket );
			m_NetWriter->SavePlaytoIndex(i - 1);
			return E_SEND_ERROR;
		}
		if( lpDataPacket->GetHeaderPiece() != HeaderIndex )
			continue;
		
		HRESULT SendResult = SendPiece(lpDataPacket);
		if( SendResult != S_OK )
		{
			return SendResult;
		}
		NETWRITER_INFO( "CNetWriter Sended Media Data "<<i );
		VIEW_INFO("PieceSend "<<i<<" End");
		m_LastPlayIndex = i;

		m_NetWriter->m_dwLastIndex = m_LastPlayIndex + 1;
	}
	m_WillPlayIndex = m_LastPlayIndex + 1;
	m_NetWriter->SavePlaytoIndex(m_LastPlayIndex);
	
	LIVE_ASSERT( m_WillPlayIndex != m_LastPlayIndex );
	LIVE_ASSERT( m_WillPlayIndex > m_LastPlayIndex );
	
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
//   类 CVistaHttpNetWriter 的实现部分
//////////////////////////////////////////////////////////////////////////

CVistaHttpNetWriter::CVistaHttpNetWriter( CMediaServer* lpMediaServer, MonoMediaHeaderPiecePtr lpPacket )
: CNetWriter( lpMediaServer, lpPacket )
{
	this->ParseHeader();

	// 计算出 m_DataUnitSize
// 	BYTE* buffer = const_cast<BYTE*>(lpPacket->GetHeader());
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
// 	
// 	// ASF文件的切片数目
// 	DWORD m_PacketNumber = *(DWORD*)(buffer+FilePropObjGuidIndex+56);
// 	// ASF文件的每个切片的长度
// 	DWORD m_PacketLength = *(DWORD*)(buffer+FilePropObjGuidIndex+92);
// 	DWORD BakPacketLength = *(DWORD*)(buffer+FilePropObjGuidIndex+96);
// 
// 	if( BakPacketLength != m_PacketLength )
// 	{
// 		m_DataUnitSize = 0;
// 	}
// 	m_DataUnitSize = m_PacketLength;
//	m_DataUnitSize = AsfHelper::GetDataUnitSizeFromHeader(buffer, length);
// 
// 	DWORD position = 0;
// 
// 	if( buffer[0] == 'H' && 
// 		buffer[1] == 'T' && 
// 		buffer[2] == 'T' && 
// 		buffer[3] == 'P' )
// 	{	// 这就说明在Source处已经打上了HTTP的头部,于是就要抹掉HTTP的头部
// 		UINT i = 0;
// 		for( i = 4; i < length; i ++ )
// 		{
// 			if( buffer[i-4] == '\r' &&
// 				buffer[i-3] == '\n' &&
// 				buffer[i-2] == '\r' &&
// 				buffer[i-1] == '\n' )
// 			{	// 说明此时i就是 真正的MediaHeader的起始位置
// 				position = i;
// 				break;			//---------------------------------|
// 			}					//                                 |
// 		}						//                                 |                   
// 		LIVE_ASSERT( i < length );	// 这说明头部不对啊                |
// 		//	   <---------------------------------------------------|
// 		LIVE_ASSERT( i > 0 );
// 	}
// 	else
// 	{	// 还好，这说明在Source处还没有打上HTTP的头部
// 		position = 0;
// 	}
}

CVistaHttpNetWriter::~CVistaHttpNetWriter()
{
	if( m_state != st_stop )
		this->Stop();
}

HRESULT CVistaHttpNetWriter::Stop()
{
	if( E_FAIL == CNetWriter::Stop() )
		return E_FAIL;
	// 丢掉剩下的所有的,这样保证能够顺利的播放到尾部
	for( VistaHttpMediaClientCollection::iterator iter = m_ClientHandles.begin(); iter != m_ClientHandles.end(); iter ++ )
	{
		iter->second.SetPlayableRange( m_lpMediaHead->GetPieceIndex(), m_MinIndex, m_MaxIndex);
	}
	m_ClientHandles.clear();
	return S_OK;
}

HRESULT CVistaHttpNetWriter::SetPlayableRange(DWORD MinIndex,DWORD MaxIndex)
{
	if( m_state != st_start )
	{
		NETWRITER_ERROR( "SetPlayableRange-CVistaHttpNetWriter is not started, so can not stop! " );
		return E_FAIL;
	}

	if( MinIndex > m_MinIndex ) m_MinIndex = MinIndex;
	if( MaxIndex > m_MaxIndex ) m_MaxIndex = MaxIndex;

	for( VistaHttpMediaClientCollection::iterator iter = m_ClientHandles.begin(); iter != m_ClientHandles.end();)
	{
		HRESULT hr = iter->second.SetPlayableRange( m_lpMediaHead->GetPieceIndex(), m_MinIndex, m_MaxIndex );
		if( hr == E_SEND_ERROR || hr == E_FORECASR_NOT_GOOD )
		{
			NETWRITER_EVENT("CVistaHttpNetWriter::OnClientError " << *iter->second.m_Socket << make_tuple(hr, m_ClientHandles.size()));
			m_ClientHandles.erase(iter++);
			continue;
		}
		LIVE_ASSERT( hr == S_OK || hr == E_LOCATION_ERROR );
		++iter;
	}

	return S_OK;
}

void CVistaHttpNetWriter::OnClientError(tcp_socket* sock, long errcode)
{
	NETWRITER_EVENT("CVistaHttpNetWriter::OnClientError " << *sock << make_tuple(errcode, m_ClientHandles.size()));
	m_ClientHandles.erase( sock );
}

bool CVistaHttpNetWriter::ContainsClient(tcp_socket* sock)
{
	return m_ClientHandles.find(sock) != m_ClientHandles.end();
}

bool CVistaHttpNetWriter::OnNewClient(tcp_socket_ptr s)
{
	NETWRITER_EVENT("CVistaHttpNetWriter::OnNewClient " << make_tuple(s, m_ClientHandles.size()));
	LIVE_ASSERT( m_ClientHandles.find(s.get()) == m_ClientHandles.end() );
	VistaHttpMediaClient ClientHandle( this, s, m_DataUnitSize );
	// 将这个SOCKET挂接到 m_ClientHandles
	m_ClientHandles[s.get()] = ClientHandle;
	return true;
}

void CVistaHttpNetWriter::DisconnectAllMediaClient()
{
	NETWRITER_INFO("DisconnectAllMediaClient");
	m_ClientHandles.clear();
}

CNetWriter* NetWriterFactory::CreateVistaHttpMediaNetWriter(CMediaServer* mediaServer, MonoMediaHeaderPiecePtr headerPiece)
{
	return new CVistaHttpNetWriter(mediaServer, headerPiece);
}
