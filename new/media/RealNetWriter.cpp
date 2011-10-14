#include "StdAfx.h"

#include "RealNetWriter.h"
#include "common/MediaPiece.h"
#include "coreutils.h"



//////////////////////////////////////////////////////////////////////////
//   结构 RealMediaClient 的实现部分
//////////////////////////////////////////////////////////////////////////
RealMediaClient::RealMediaClient() 
: MediaClient()
{
	VIEW_DEBUG( "Create RealMediaClient ");
}

RealMediaClient::RealMediaClient( CNetWriter* netWriter, tcp_socket_ptr sock ) : MediaClient( netWriter, sock )
{
	VIEW_DEBUG( "Create RealMediaClient " << *sock);
}

HRESULT RealMediaClient::SetPlayableRange( 
	UINT HeaderIndex, 
	DWORD MinIndex,
	DWORD MaxIndex,
	const set<UINT16>& AudioPosistionSet)
{
	if( false == m_IsSending )
	{	// 还没有发送任何数据, 于是就要首先定位StartIndex
		DWORD StartIndex = m_NetWriter->LocateStartPosition();
		if( StartIndex == 0 )
		{
			NETWRITER_ERROR( "MediaClient LocationStartIndex Error");
			return E_LOCATION_ERROR;
		}		
		m_WillPlayIndex = StartIndex;
		// 由于Real的变态声音问题,所以定位后,有必须根据Real的声音问题,从m_WillPlayIndex开始
		// 重新做筛选,这个操作的目的是定位出Real的音频关键帧,只要从音频关键帧开始往播放器中
		// 丢数据,Real才会有声音
		if( AudioPosistionSet.size() > 0 )
		{	// 这个if是保证要有声音的Real才能进入这个程序段	
			const BYTE * RealBuffer = NULL;		// 用来定位Rmff数据分片的字节指针
			int RealLength = 0;					// 用来储存当前Rmff数据分片长度的
			UINT16 RealPosition = 0;			// 用来记录有与RealBuffer之间偏移位置的
			bool AlreadyPosition = false;		// 用来在程序内部置状态
			
			// 定位音视频同部点
			for (MediaDataPiecePtr dataPiece = m_NetWriter->GetFirst(m_WillPlayIndex); 
					(dataPiece && dataPiece->GetPieceIndex() <= MaxIndex);
					dataPiece = m_NetWriter->GetNext(dataPiece->GetPieceIndex()))
			{
				MonoMediaDataPiecePtr lpDataPacket = dataPiece->ToMonoPiece();
				if ( ! lpDataPacket )
					continue;
				UINT i = lpDataPacket->GetPieceIndex();
				if( lpDataPacket->GetHeaderPiece() > HeaderIndex )
				{
					NETWRITER_ERROR( "CRealNetWriter Send Media Data Gracefully End "<<*m_Socket );
					return E_SEND_ERROR;		
				}
				if( lpDataPacket->GetHeaderPiece() != HeaderIndex )
					continue;

				const BYTE * buffer = lpDataPacket->GetMediaData();
				UINT16 length = (UINT16)lpDataPacket->GetMediaDataLength();

				RealPosition = 0;

				while( RealPosition < length )
				{
					UINT16 ObjectVersion = UINT16_PARSE(buffer+RealPosition+0);	
					UINT16 DataLength = UINT16_PARSE(buffer+RealPosition+2);	
					UINT16 StreamNumber = UINT16_PARSE(buffer+RealPosition+4);
					//UINT32 TimeStamp = UINT32_PARSE(buffer+RealPosition+6);
					UINT8 Flags = UINT8_PARSE(buffer+RealPosition+11);
					
					if( ObjectVersion == 1 ||
						(Flags == 2 && AudioPosistionSet.find(StreamNumber) != AudioPosistionSet.end()) )
					{	// 如果是关键帧
						AlreadyPosition = true;
						RealBuffer = buffer + RealPosition;
						RealLength = length - RealPosition;
						m_WillPlayIndex = i;
						break;	//-----------------------------------------
					}												//    |
					RealPosition = RealPosition + DataLength;		//    |
				}													//    |
				//     <---------------------------------------------------
				if( AlreadyPosition == true ) 
				{
					bool SendOk = SendData( RealBuffer, RealLength );
					if( SendOk == false )
					{
						NETWRITER_ERROR( "CNetWriter Send Media Data Error "<<*m_Socket );
						return E_SEND_ERROR;
					}
					NETWRITER_INFO( "CNetWriter Sended Media Data " << m_WillPlayIndex );
					VIEW_INFO("PieceSend "<<i<<" End");
					m_LastPlayIndex = m_WillPlayIndex;
					m_WillPlayIndex = m_LastPlayIndex + 1;
					LIVE_ASSERT( m_WillPlayIndex != m_LastPlayIndex );
					LIVE_ASSERT( m_WillPlayIndex > m_LastPlayIndex );
					m_IsSending = true;
					break;	// 如果成功定位到了音频关键帧，就跳出这个循环-----
				}			//                                               |
			}				//                                               |
		}	//     <----------------------------------------------------------
		// 注意!! 此时的 m_IsSending 不一定就等于 true, 所以要做下面的判断。
		//      但是同时 m_IsSending 有可能等于true,所以一定要有下面的判断。
		else
		{
			m_IsSending = true;
		}
	}
	

	if( true == m_IsSending )
	{
		return SetPlayableRangeAfterSended(HeaderIndex,MinIndex,MaxIndex);
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
//   类 CRealNetWriter 的实现部分
//////////////////////////////////////////////////////////////////////////

CRealNetWriter::CRealNetWriter(CMediaServer* lpMediaServer, MonoMediaHeaderPiecePtr lpPacket )
: CNetWriter( lpMediaServer, lpPacket )
{
	// 加载音频流的编号序列
	const UINT16* info = (const UINT16*)m_lpMediaHead->GetInfo();
	UINT16 length = m_lpMediaHead->GetInfoLength() / 2;
	for( int i = 0; i < length; i ++ )
		m_PosistionSet.insert( info[i] );
}

CRealNetWriter::~CRealNetWriter()
{
	if( m_state != st_stop )
		this->Stop();
}

HRESULT CRealNetWriter::Stop()
{
	if( E_FAIL == CNetWriter::Stop() )
		return E_FAIL;
	// 丢掉剩下的所有的,这样保证能够顺利的播放到尾部
	for( RealMediaClientCollection::iterator iter = m_ClientHandles.begin(); iter != m_ClientHandles.end(); iter ++ )
	{
		iter->second.SetPlayableRange( m_lpMediaHead->GetPieceIndex(), m_MinIndex, m_MaxIndex, m_PosistionSet);
	}
	m_ClientHandles.clear();
	return S_OK;
}

HRESULT CRealNetWriter::SetPlayableRange(DWORD MinIndex,DWORD MaxIndex)
{
	if( m_state != st_start )
	{
		NETWRITER_ERROR( "SetPlayableRange-CRealNetWriter is not started, so can not stop! " );
		return E_FAIL;
	}
	
	NETWRITER_INFO( "CRealNetWriter::SetPlayableRange MinIndex = "<<MinIndex<<" MaxIndex = "<<MaxIndex );

	if( MinIndex > m_MinIndex ) m_MinIndex = MinIndex;
	if( MaxIndex > m_MaxIndex ) m_MaxIndex = MaxIndex;

	for( RealMediaClientCollection::iterator iter = m_ClientHandles.begin(); iter != m_ClientHandles.end();)
	{
		HRESULT hr = iter->second.SetPlayableRange( m_lpMediaHead->GetPieceIndex(), m_MinIndex, m_MaxIndex, m_PosistionSet );
		if( hr == E_SEND_ERROR || hr == E_FORECASR_NOT_GOOD )
		{
			m_ClientHandles.erase(iter++);
			continue;
		}
		LIVE_ASSERT( hr == S_OK || hr == E_LOCATION_ERROR );
		iter ++;
	}

	NETWRITER_DEBUG( "MinIndex : " << MinIndex << " MaxIndex : " << MaxIndex );
	return S_OK;
}

bool CRealNetWriter::OnNewClient(tcp_socket_ptr s)
{
	NETWRITER_EVENT("CRealNetWriter::OnNewClient " << s);
	LIVE_ASSERT( m_ClientHandles.find(s.get()) == m_ClientHandles.end() );
	RealMediaClient ClientHandle( this, s);
	// 将这个SOCKET挂接到 m_ClientHandles
	m_ClientHandles[s.get()] = ClientHandle;
	return true;
}

void CRealNetWriter::OnClientError(tcp_socket* sock, long errcode)
{
	m_ClientHandles.erase(sock);
}

bool CRealNetWriter::ContainsClient(tcp_socket* sock)
{
	return m_ClientHandles.find(sock) != m_ClientHandles.end();
}

void CRealNetWriter::DisconnectAllMediaClient()
{
	m_ClientHandles.clear();
}


CNetWriter* NetWriterFactory::CreateRealNetWriter(CMediaServer* mediaServer, MonoMediaHeaderPiecePtr headerPiece)
{
	return new CRealNetWriter(mediaServer, headerPiece);
}


