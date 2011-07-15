#include "StdAfx.h"

#include "CommonNetWriter.h"
#include "common/MediaPiece.h"


//////////////////////////////////////////////////////////////////////////
//   结构 WindowsMediaClient 的实现部分
//////////////////////////////////////////////////////////////////////////
WindowsMediaClient::WindowsMediaClient() 
: MediaClient()
{
// 	m_NeedStartWithKeyFrame = true;
// 	m_NeedChangeTimeStamps = true;
}

WindowsMediaClient::WindowsMediaClient( CNetWriter* netWriter, tcp_socket_ptr sock) : MediaClient( netWriter, sock )
{
// 	m_NeedStartWithKeyFrame = true;
// 	m_NeedChangeTimeStamps = true;
}

HRESULT WindowsMediaClient::SetPlayableRange( 
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
		//NETWRITER_INFO("MediaClient SetPlayableRange: StartIndex=" << StartIndex << ", MinMax=" << make_tuple(MinIndex, MaxIndex));
	}
	assert( true == m_IsSending );
	return SetPlayableRangeAfterSended(HeaderIndex,MinIndex,MaxIndex);
}

//////////////////////////////////////////////////////////////////////////
//   类 CWindowsNetWriter 的实现部分
//////////////////////////////////////////////////////////////////////////

CWindowsNetWriter::CWindowsNetWriter( CMediaServer* lpMediaServer, MonoMediaHeaderPiecePtr lpPacket )
: CNetWriter( lpMediaServer, lpPacket )
{
       // window xp, not need parse asf
	//this->ParseHeader();
}

CWindowsNetWriter::~CWindowsNetWriter()
{
	if( m_state != st_stop )
		this->Stop();
}

HRESULT CWindowsNetWriter::Stop()
{
	if( E_FAIL == CNetWriter::Stop() )
		return E_FAIL;
	// 丢掉剩下的所有的,这样保证能够顺利的播放到尾部
	for( WindowMediaClientCollection::iterator iter = m_ClientHandles.begin(); iter != m_ClientHandles.end(); iter ++ )
	{
		iter->second.SetPlayableRange( m_lpMediaHead->GetPieceIndex(), m_MinIndex, m_MaxIndex);
	}
	m_ClientHandles.clear();
	return S_OK;
}

HRESULT CWindowsNetWriter::SetPlayableRange(DWORD MinIndex,DWORD MaxIndex)
{
	if( m_state != st_start )
	{
		NETWRITER_ERROR( "SetPlayableRange-CWindowsNetWriter is not started, so can not stop! " );
		return E_FAIL;
	}
	
	if( MinIndex > m_MinIndex ) m_MinIndex = MinIndex;
	if( MaxIndex > m_MaxIndex ) m_MaxIndex = MaxIndex;

	for( WindowMediaClientCollection::iterator iter = m_ClientHandles.begin(); iter != m_ClientHandles.end();)
	{
		HRESULT hr = iter->second.SetPlayableRange( m_lpMediaHead->GetPieceIndex(), m_MinIndex, m_MaxIndex );
		if( hr == E_SEND_ERROR || hr == E_FORECASR_NOT_GOOD )
		{
			NETWRITER_EVENT("CWindowsNetWriter::OnClientError " << *iter->second.m_Socket << make_tuple(hr, m_ClientHandles.size()));
			m_ClientHandles.erase(iter++);
			continue;
		}
		assert( hr == S_OK || hr == E_LOCATION_ERROR );
		++iter;
	}
	
	return S_OK;
}


void CWindowsNetWriter::OnClientError(tcp_socket* sock, long errcode)
{
	NETWRITER_EVENT("CWindowsNetWriter::OnClientError " << *sock << make_tuple(errcode, m_ClientHandles.size()));
	m_ClientHandles.erase( sock );
}

bool CWindowsNetWriter::ContainsClient(tcp_socket* sock)
{
	return m_ClientHandles.find(sock) != m_ClientHandles.end();
}

bool CWindowsNetWriter::OnNewClient(tcp_socket_ptr s)
{
	NETWRITER_EVENT("CWindowsNetWriter::OnNewClient " << make_tuple(s, m_ClientHandles.size()));
	assert( m_ClientHandles.find(s.get()) == m_ClientHandles.end() );
	WindowsMediaClient ClientHandle( this, s );
	// 将这个SOCKET挂接到 m_ClientHandles
	m_ClientHandles[s.get()] = ClientHandle;
	return true;
}

void CWindowsNetWriter::DisconnectAllMediaClient()
{
	NETWRITER_EVENT("DisconnectAllMediaClient " << m_MaxIndex);
	m_ClientHandles.clear();
}

CNetWriter* NetWriterFactory::CreateWindowsMediaNetWriter(CMediaServer* mediaServer, MonoMediaHeaderPiecePtr headerPiece)
{
	return new CWindowsNetWriter(mediaServer, headerPiece);
}


