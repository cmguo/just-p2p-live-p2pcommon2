#include "StdAfx.h"

#include "MmshNetWriter.h"
#include "common/MediaPiece.h"


//////////////////////////////////////////////////////////////////////////
//   结构 MmshMediaClient 的实现部分
//////////////////////////////////////////////////////////////////////////
MmshMediaClient::MmshMediaClient() 
: MediaClient()
{
}

MmshMediaClient::MmshMediaClient( CNetWriter* netWriter, tcp_socket_ptr sock) : MediaClient( netWriter, sock )
{
}

HRESULT MmshMediaClient::SetPlayableRange( 
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
	assert( true == m_IsSending );
	return SetPlayableRangeAfterSended(HeaderIndex,MinIndex,MaxIndex);
}

//////////////////////////////////////////////////////////////////////////
//   类 CMmshNetWriter 的实现部分
//////////////////////////////////////////////////////////////////////////

CMmshNetWriter::CMmshNetWriter( CMediaServer* lpMediaServer, MonoMediaHeaderPiecePtr lpPacket )
: CNetWriter( lpMediaServer, lpPacket )
{
}

CMmshNetWriter::~CMmshNetWriter()
{
	if( m_state != st_stop )
		this->Stop();
}

HRESULT CMmshNetWriter::Stop()
{
	if( E_FAIL == CNetWriter::Stop() )
		return E_FAIL;
	// 丢掉剩下的所有的,这样保证能够顺利的播放到尾部
	for( MmshMediaClientCollection::iterator iter = m_ClientHandles.begin(); iter != m_ClientHandles.end(); iter ++ )
	{
		iter->second.SetPlayableRange( m_lpMediaHead->GetPieceIndex(), m_MinIndex, m_MaxIndex);
	}
	m_ClientHandles.clear();
	return S_OK;
}

HRESULT CMmshNetWriter::SetPlayableRange(DWORD MinIndex,DWORD MaxIndex)
{
	if( m_state != st_start )
	{
		NETWRITER_ERROR( "SetPlayableRange-CMmshNetWriter is not started, so can not stop! " );
		return E_FAIL;
	}
	
	NETWRITER_INFO( "CMmshNetWriter::SetPlayableRange MinIndex = "<<MinIndex<<" MaxIndex = "<<MaxIndex << " " << m_ClientHandles.size());

	if( MinIndex > m_MinIndex ) m_MinIndex = MinIndex;
	if( MaxIndex > m_MaxIndex ) m_MaxIndex = MaxIndex;

	for( MmshMediaClientCollection::iterator iter = m_ClientHandles.begin(); iter != m_ClientHandles.end();)
	{
		HRESULT hr = iter->second.SetPlayableRange( m_lpMediaHead->GetPieceIndex(), m_MinIndex, m_MaxIndex );
		if( hr == E_SEND_ERROR || hr == E_FORECASR_NOT_GOOD )
		{
			NETWRITER_EVENT("CMmshNetWriter::OnClientError " << *iter->second.m_Socket << make_tuple(hr, m_ClientHandles.size()));
			m_ClientHandles.erase(iter++);
			continue;
		}
		assert( hr == S_OK || hr == E_LOCATION_ERROR );
		++iter;
	}

	NETWRITER_DEBUG( "MinIndex : " << MinIndex << " MaxIndex : " << MaxIndex );
	return S_OK;
}

bool CMmshNetWriter::OnClientReceive(tcp_socket_ptr s, const BYTE* data, size_t size)
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
		assert( 0 );
		NETWRITER_ERROR( "CNetWriter::ContainsClient has this SOCKET "<<*s );
		return true;
	}
	
	assert( m_lpMediaHead->GetMediaType() == PPMT_MEDIA_ASF_MMSH );
	// 发送Media的头部
	const BYTE *buffer = m_lpMediaHead->GetHeader();
	int length = m_lpMediaHead->GetHeaderLength();
	SendOk = SendDataToClient(s, buffer, length);
	if( SendOk == false )
	{
		NETWRITER_ERROR( "CNetWriter Send Media Header Error "<<*s );
		return false;
	}
	NETWRITER_DEBUG( "CNetWriter Send Media Header "<<m_lpMediaHead->GetPieceIndex() );
	VIEW_INFO("PieceSend "<<m_lpMediaHead->GetPieceIndex()<<" End");
	

	return OnNewClient(s);
}

void CMmshNetWriter::OnClientError(tcp_socket* sock, long errcode)
{
	NETWRITER_EVENT("CMmshNetWriter::OnClientError " << *sock << make_tuple(errcode, m_ClientHandles.size()));
	m_ClientHandles.erase( sock );
}

bool CMmshNetWriter::ContainsClient(tcp_socket* sock)
{
	return m_ClientHandles.find(sock) != m_ClientHandles.end();
}

bool CMmshNetWriter::OnNewClient(tcp_socket_ptr s)
{
	NETWRITER_EVENT("CMmshNetWriter::OnNewClient " << make_tuple(s, m_ClientHandles.size()));
	assert( m_ClientHandles.find(s.get()) == m_ClientHandles.end() );
	MmshMediaClient ClientHandle( this, s );
	// 将这个SOCKET挂接到 m_ClientHandles
	m_ClientHandles[s.get()] = ClientHandle;
	return true;
}

void CMmshNetWriter::DisconnectAllMediaClient()
{
	m_ClientHandles.clear();
}

CNetWriter* NetWriterFactory::CreateMmshMediaNetWriter(CMediaServer* mediaServer, MonoMediaHeaderPiecePtr headerPiece)
{
	return new CMmshNetWriter(mediaServer, headerPiece);
}


