#include "StdAfx.h"

#include "MediaServer.h"
#include "StreamBuffer.h"
#include "MediaStorage.h"
#include "MediaPiece.h"
#include "media/NetWriter.h"
#include "media/MediaServerListener.h"
#include <synacast/protocol/MonoMediaPiece.h>
#include <ppl/net/asio/http_acceptor.h>


#define UM_STREAM_CHANGE	WM_USER+102

#ifndef EADDRINUSE
#define EADDRINUSE WSAEADDRINUSE
#endif


CMediaServer::CMediaServer( MediaServerListener& listener, CStreamBuffer& lpStreamBuffer, USHORT port ) :
        m_state(st_none),
	m_listener(listener),
        m_streamBuffer(lpStreamBuffer), 
	m_storage(lpStreamBuffer.GetStorage()),
        m_port(port),
        m_netWriter(NULL), 
	m_NowHeaderIndex(0),
	m_PlaytoIndex(0), 
	m_mediaType(NULL_PLAYER),
	m_isFirst(true)
{

	StartServer();
}

CMediaServer::~CMediaServer()
{
	StopServer();
	this->Stop();
}

HRESULT CMediaServer::Start( START_METHOD StartMethod, DWORD StartIndexTimeSpan, DWORD ShouldBufferTimeSpan )
{
	if( m_state == st_idle || m_state == st_run )
	{
		MEDIASERVER_ERROR( " CMediaServer has started! so can not start again!" );
		return E_FAIL;
	}

	m_state = st_idle;
	return S_OK;
}

void CMediaServer::Stop()
{
	if( m_state != st_idle && m_state != st_run )
	{
		MEDIASERVER_ERROR( " CMediaServer has stoped! so can not stop again!" );
		return;
	}
	Init();
	m_state = st_stop;
}

void CMediaServer::Init()//��ʼ��
{
	// ע�⣺һ��Ҫ��ɾ��WriterCollection,Ȼ����ɾ��MediaHeaderMap
	//      ��Ϊ��NetWrite�رյ�ʱ��Ҫ��֤�ܹ����ŵ��ļ���β�������Ի�ҪѰ�ҵ����Ӧ��MediaHeaderMap
	SAFE_DELETE( m_netWriter );

	m_NowHeaderIndex = 0;
	m_PlaytoIndex = 0;
}

void CMediaServer::Reset()
{
	m_PlaytoIndex = 0;
	m_NowHeaderIndex = 0;
	
	m_state = st_idle;
}

bool CMediaServer::AddMediaHeader( MonoMediaHeaderPiecePtr lpPacket)
{
	LIVE_ASSERT( lpPacket->GetPieceType() == PPDT_MEDIA_HEADER );

	if( !lpPacket )
	{
		STREAMBUFFER_ERROR( "Recv a Null Header Packet!" );
		return false;
	}

#ifdef _DEBUG
	if( m_NowHeaderIndex != 0 )
	{
		LIVE_ASSERT( m_storage.HasHeader(m_NowHeaderIndex) );
	}
#endif

	//����ǵ�һ�β��ŵ�ʱ������ChangeMediaHeader�������µ�Writer
	if( m_NowHeaderIndex == 0 )
	{
		STREAMBUFFER_INFO( "This is the first to add mediaheader, so Change Header" );
		ChangeMediaHeader( lpPacket->GetPieceIndex() );
	}

	return true;
}

bool CMediaServer::ChangeMediaHeader( UINT HeaderIndex )//����Ҫ�ĺ���
{
	//	LIVE_ASSERT( m_NewMediaHeaderQueue.size() > 0 );
	MediaHeaderPiecePtr headerPiece = m_storage.GetHeader(HeaderIndex);
	if ( ! headerPiece )
		return false;
	MonoMediaHeaderPiecePtr lpNewMediaHeader = headerPiece->ToMonoPiece();
	if( !lpNewMediaHeader )
		return false;

	MEDIASERVER_INFO( "TBD: Should Change New Header with HeaderIndex = "<<HeaderIndex );

	// ɾ���ɵ�NetWriter
	SAFE_DELETE( m_netWriter );
	VIEW_INFO("DelNetWriter "<<HeaderIndex<<" End");

	// ���Ͳ岥�����Ϣ,���ֲ��� ��β�����Ų�������������ֻҪ��治Ҫ̫����ͷ���ǻ�������
	if( m_isFirst == false )
	{	// ������ǵ�һ�λ�ͷ
		m_listener.NotifyGUI(UM_STREAM_CHANGE, 0, 0);
	}
	else
	{	// ����ǵ�һ�λ�ͷ
		m_isFirst = false;	
	}
	// �����µ�NetWriter
	return CreateNetWriter(lpNewMediaHeader);
}

void CMediaServer::SetPlayableRange( DWORD MinIndex, DWORD MaxIndex )
{
	if( m_netWriter )
	{
		m_netWriter->SetPlayableRange( MinIndex, MaxIndex );
	}
	else
	{
		MEDIASERVER_WARN( "TBD: There is no Writercollection so can not SetPlayableRange" );
	}

	if( MinIndex == 0 ) return;
	if( MaxIndex == 0 ) return;
	UINT SeeIndex = m_streamBuffer.GetSkipIndex();
	UINT64 SeeTimestamp = m_streamBuffer.GetSkipTimestamp();
	if( SeeIndex == 0 ) return;
	if( SeeTimestamp == 0 ) return;

	MediaDataPiecePtr lpPacket = m_storage.GetDataPiece( SeeIndex );
	UINT HeaderIndex = lpPacket->GetHeaderPiece();

	MediaDataPiecePtr startPacket = m_storage.GetNext( HeaderIndex );
	UINT64 startTimeStamp = startPacket->GetTimeStamp();
	UINT64 timeStamp = lpPacket->GetTimeStamp();
	UINT64 relativeTimeSpan = timeStamp - startTimeStamp;

	// �����ĿԴ�̣ܶ�minmax֮�����2��header����������߱��������쳣�����
	if( HeaderIndex > m_NowHeaderIndex 
		&& relativeTimeSpan > 3 * 1000 )	// ̫���˾���ʱ��Ҫ�������˵Graph�������Ῠס
	{
		ChangeMediaHeader( HeaderIndex );
	}
}



void CMediaServer::on_socket_accept(tcp_acceptor* sender, tcp_socket* newClient, const InetSocketAddress& addr)
{
	tcp_socket_ptr client( newClient );
	MEDIASERVER_INFO("CMediaServer::OnSocketAccept " << *newClient << " from " << addr);
	// return false��ʾ�����ӱ��ܾ�
	if (NULL == m_netWriter)
	{
		MEDIASERVER_ERROR( "THE WRITERCOLLECTION IS DIE, SO CAN NOT PLAY ANYTHING!" );
		return;
	}
	client->set_send_timeout(10); // Added by Tady, 082609: : For VistaHttpNetWriter. The Asf-reader in DShow is a freak! 
	m_netWriter->OnAcceptConnection(client, addr);
}

void CMediaServer::SavePlaytoIndex(UINT playtoIndex)
{
	if (m_PlaytoIndex < playtoIndex)
	{
		m_PlaytoIndex = playtoIndex;
	}
}

void CMediaServer::StartServer()
{
	if (m_server)
		return;
	// �󶨼����˿�,����˿ڱ�ռ,���Զ�+1,ֱ����û�б�ռ�Ķ˿�Ϊֹ
	m_server.reset(new http_acceptor());
	m_server->set_listener(this);

	LIVE_ASSERT(m_port != 0);
	for (int i = 0; i < 100; ++i)
	{
		if (m_server->open(m_port))
		{
			LIVE_ASSERT(m_server->is_open());
			MEDIASERVER_EVENT( "CMediaServer: Create HTTP Server OK port = " << m_port );
			break;
		}
		int errcode = m_server->last_error();
		MEDIASERVER_ERROR( "CMediaServer: Create HTTP Server Failed " << ::WSAGetLastError() 
			<< " errcode=" << make_pair(m_port, errcode) );
		if (errcode < 0)
		{
			if (errcode == -1)
			{
				MEDIASERVER_ERROR( "CMediaServer: Create HTTP Server ϵͳ�����ڴ治�� ");
				LIVE_ASSERT(!"Create Media Server Failed.");
				return;
			}
			if (errcode == -2)
			{
				MEDIASERVER_ERROR( "CMediaServer: Create HTTP Server �����׽���ʧ�� port = " << m_port );
			}
			else if (errcode == -3)
			{
				MEDIASERVER_ERROR( "CMediaServer: Create HTTP Server ������������ʧ�� port = " << m_port );
			}
			else
			{
				MEDIASERVER_ERROR( "CMediaServer: Create HTTP Server FAILED. Unrecognized errcode. port = " << m_port );
				LIVE_ASSERT(!"Create Media Server Failed.");
			//	return;
			}
		}
		else
		{
			if (errcode == EADDRINUSE)
			{
				MEDIASERVER_ERROR( "CMediaServer: Create HTTP Server failed. port is in use. " << m_port );
			}
			else
			{
				MEDIASERVER_ERROR( "CMediaServer: Create HTTP Server FAILED. Unrecognized winsock errcode. port = " << m_port );
				//LIVE_ASSERT(!"Create Media Server Failed.");
			//	return;
			}
		}
		++m_port;
	}

	// ���ù����ڴ��еĲ��Ŷ˿�,���Ǳ�¶�������
}

void CMediaServer::StopServer()
{
	m_server.reset();
}



void CMediaServer::DisconnectAllMediaClient()
{
	if( m_netWriter != NULL)
	{
		m_netWriter->DisconnectAllMediaClient();		
	}
}

bool CMediaServer::CreateNetWriter(const MonoMediaHeaderPiecePtr& header)
{
	m_netWriter = NetWriterFactory::Create(this, header);
	UINT HeaderIndex = header->GetPieceIndex();

	m_Statistics.HeaderIndex = HeaderIndex;
	m_Statistics.FirstPlayIndex = 0;
	m_Statistics.FirstPlayTime = 0;
	m_Statistics.FirstPlayPieceTimeStamp = 0;

	LIVE_ASSERT(m_netWriter != NULL);
	VIEW_INFO("CreateNetWriter "<<header->GetPieceIndex()<<" End");

	m_netWriter->Start();

	m_mediaType = m_netWriter->GetPlayerType();
	UINT16 playPort = m_netWriter->GetPort();
	if (playPort != 0)
	{
		//LIVE_ASSERT(m_port == playPort);
		m_port = playPort;
	}

	if( m_NowHeaderIndex != 0 )
	{
		MEDIASERVER_INFO( "TBD: MediaServer HeaderIndex = "<<m_NowHeaderIndex );
	}
	LIVE_ASSERT( HeaderIndex != 0 );
	MEDIASERVER_INFO( "TBD: MediaServer New HeaderIndex = "<<HeaderIndex );
	
	// �û�NetWriter
	m_NowHeaderIndex = HeaderIndex;
	return true;
}


