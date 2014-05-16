#include "StdAfx.h"

#include "NetWriter.h"
#include "common/MediaServer.h"
#include "common/StreamBuffer.h"
#include "common/MediaStorage.h"
#include "MediaServerListener.h"
#include "common/MediaPiece.h"
#include "AsfHelper.h"

#include "framework/socket.h"

#include <ppl/diag/trace.h>
/// 丢片间隙的报警时间上限
//const int WARNING_SENDED_TIMETICK_INTERVAL = 5000;


//////////////////////////////////////////////////////////////////////////
//  结构 MediaClient 的实现部分
//////////////////////////////////////////////////////////////////////////
MediaClient::MediaClient() :
m_NetWriter(NULL), 
m_IsSending(false),
m_WillPlayIndex(0),
m_LastPlayIndex(0), 
m_NeedStartWithKeyFrame( false ), 
m_KeyFrameLocated( false ), 
m_NeedChangeTimeStamps( false ), 
m_CheckedDataPacketCount( 0 ),m_isFirst(true),m_StartTimeStamp(0), m_StartTimeStampPL(0), m_lastSendTS(0)
{
	VIEW_DEBUG( "Create MediaClient ");
}

MediaClient::MediaClient( CNetWriter* netWriter, tcp_socket_ptr sock) :
m_Socket(sock),
m_NetWriter(netWriter), 
m_IsSending(false),
m_WillPlayIndex(0),
m_LastPlayIndex(0), 
m_NeedStartWithKeyFrame( false ), 
m_KeyFrameLocated( false ), 
m_NeedChangeTimeStamps( false ), 
m_CheckedDataPacketCount( 0 ),m_isFirst(true),m_StartTimeStamp(0), m_StartTimeStampPL(0), m_lastSendTS(0)
{	
	VIEW_DEBUG( "Create MediaClient " << *sock);
}

HRESULT MediaClient::SetPlayableRangeAfterSended( 
												 UINT HeaderIndex, 
												 DWORD MinIndex,
												 DWORD MaxIndex)
{
	LIVE_ASSERT( true == m_IsSending );
	
	bool SendOk = true;
	// 发送相应的数据
	for (MediaDataPiecePtr lpDataPacket = m_NetWriter->GetFirst(m_WillPlayIndex); 
			(lpDataPacket && lpDataPacket->GetPieceIndex() <= MaxIndex);
			lpDataPacket = m_NetWriter->GetNext(lpDataPacket->GetPieceIndex()))
	{
		UINT i = lpDataPacket->GetPieceIndex();
		if( lpDataPacket->GetHeaderPiece() > HeaderIndex )
		{	// 说明这个文件已经非常平滑的就播放完毕了啊，于是自然就要断开连接了
			NETWRITER_ERROR( "CNetWriter Send Media Data Gracefully End "<<*m_Socket );
			m_NetWriter->SavePlaytoIndex(i - 1);
			return E_SEND_ERROR;
		}
		if( lpDataPacket->GetHeaderPiece() != HeaderIndex )
			continue;

		// 检查是否是关键帧
		// 如果需要从关键帧开始，并且还没有定位到关键帧
// 		if ( m_NeedStartWithKeyFrame && false == m_KeyFrameLocated )
// 		{
// 			if ( m_CheckedDataPacketCount > 100 )
// 			{
// 				// 检查了一定量的报文后仍然没有检测到关键帧，则直接开始
// 				m_NeedStartWithKeyFrame = false;
// 				m_KeyFrameLocated = true;
// 				LIVE_ASSERT( false );
// 				//Tracer::Trace("no key frame is found %d\n", m_CheckedDataPacketCount);
// 			}
// 			else if ( AsfHelper::IsKeyFramePacket( lpDataPacket->GetMediaData(), lpDataPacket->GetMediaDataLength() ) )
// 			{
// 				// 检查到关键帧
// 				m_KeyFrameLocated = true;
// 				//Tracer::Trace("key frame is found %d\n", m_CheckedDataPacketCount);
// 			}
// 			else
// 			{
// 				// 非关键帧，跳过
// 				++m_CheckedDataPacketCount;
// 				continue;
// 			}
// 		}
	
		SendOk = this->DoSendDataPiece( lpDataPacket );
		if( SendOk == false )
		{
			NETWRITER_ERROR( "CNetWriter Send Media Data Error " << i <<*m_Socket );
			m_NetWriter->SavePlaytoIndex(i - 1);
			return E_SEND_ERROR;
		}
		NETWRITER_INFO( "CNetWriter Sended Media Data "<<i );
		VIEW_INFO("PieceSend "<<i<<" End");
		m_LastPlayIndex = i;

		m_NetWriter->m_dwLastIndex = m_LastPlayIndex + 1;
		//TRACE("Peer: %d \n", m_LastPlayIndex);
	}
	m_WillPlayIndex = m_LastPlayIndex + 1;
	m_NetWriter->SavePlaytoIndex(m_LastPlayIndex);
	
	//NETWRITER_WARN( "CNetWriter Now PlayIndex ! " << m_WillPlayIndex );
	LIVE_ASSERT( m_WillPlayIndex != m_LastPlayIndex );
	LIVE_ASSERT( m_WillPlayIndex > m_LastPlayIndex );
	
	return S_OK;
}



bool MediaClient::SendData(const void* data, size_t size)
{
	TRACEOUT_T("Tady -> !!!!! SendQueue Size [%d] !!!!!", m_Socket->GetSendQueueSize());
	return m_Socket->send( static_cast<const BYTE*>( data ), size );
}


bool MediaClient::DoSendDataPiece( MediaDataPiecePtr dataPiece )
{
// 	if ( false == m_NeedChangeTimeStamps )
// 	{
// 		return this->m_Socket->Send( dataPiece->GetMediaData(), dataPiece->GetMediaDataLength() );
// 	}
// 
// 	pool_byte_buffer buf;
// 	buf.assign( dataPiece->GetMediaData(), dataPiece->GetMediaDataLength() );
// 
// 	if ( m_isFirst )
// 	{
// 		m_StartTimeStamp = AsfHelper::GetTimestamp(buf.data(), buf.size());
// 		m_StartTimeStampPL = AsfHelper::GetPresentTime(buf.data(), buf.size());
// //		m_StartTimeStampPL = m_StartTimeStamp;
// 		m_isFirst = false;
// 	}
// 
// 	if ( AsfHelper::ChanegeTimeStamps( buf.data(), buf.size(), m_StartTimeStamp, m_StartTimeStampPL, m_NetWriter->GetPreroll() ) )
// 	//if ( AsfHelper::ChanegeTimeStamps( buf.data(), buf.size(), m_StartTimeStamp, m_NetWriter->GetPreroll() ) )
// 	//if ( AsfHelper::ChanegeTimeStampsForVista( buf.data(), buf.size(), m_StartTimeStamp ) )
// 	{
// 		return this->m_Socket->Send( buf.data(), buf.size() );
// 	}
// 
// 	return this->m_Socket->Send( dataPiece->GetMediaData(), dataPiece->GetMediaDataLength() );

	UINT uiDataUnitSize = m_NetWriter->GetDataUnitSize();

	MonoMediaDataPiecePtr monoPiece = dataPiece->ToMonoPiece();
	if(  true == m_NeedChangeTimeStamps && uiDataUnitSize > 0 )
	{	// 成功读到 Asf 的DataUnitSize, 则转化成mmsh协议流来传输

		BYTE* PieceBuffer = const_cast<BYTE*>( monoPiece->GetMediaData() );
		UINT uiPieceSize = monoPiece->GetMediaDataLength();
		bool SendOk = true;
		pool_byte_buffer& buffer = m_NetWriter->GetPieceOperationBuffer();
		buffer.ensure_size(uiDataUnitSize);
		UINT16 IndexSize = 0;

		while( IndexSize < uiPieceSize )
		{
			memcpy(buffer.data(), PieceBuffer + IndexSize, uiDataUnitSize);
			if( true == m_isFirst )
			{
				m_StartTimeStamp = AsfHelper::GetTimestamp(buffer.data(), uiDataUnitSize);
				m_StartTimeStampPL = AsfHelper::GetPresentTime(buffer.data(), buffer.size());
				m_isFirst = false;
				m_lastPresentTS = m_NetWriter->GetPreroll();
			}

			//if( false == AsfHelper::ChanegeTimeStampsForVista(buffer.data(), uiDataUnitSize, m_StartTimeStamp) )
			if ( false == AsfHelper::ChanegeTimeStamps( buffer.data(), buffer.size(), (UINT32&)m_StartTimeStamp, (UINT32&)m_StartTimeStampPL, m_NetWriter->GetPreroll(), true, m_lastSendTS, m_lastPresentTS))
			{
//				NETWRITER_ERROR( "ChanegeTimeStamps Error " << PieceIndex << " " << IndexSize << " " << *m_Socket );
				break;
			}

			// 对数据包进行偏移处理
			SendOk = m_Socket->send( buffer.data(), uiDataUnitSize );
			if( SendOk == false )
			{
				return false;
			}

			IndexSize = IndexSize + (UINT16)uiDataUnitSize;
		}
		return true;
	}
	else
	{	// 如果读不到 Asf 的DataUnitSize, 保留传统方式 传输
		return this->SendData( monoPiece->GetMediaData(), monoPiece->GetMediaDataLength() );
	}
}


CNetWriter::CNetWriter(CMediaServer* lpMediaServer, MonoMediaHeaderPiecePtr lpPacket) :
	m_state( st_none ),
	m_lpMediaHead( lpPacket ),
	m_lpMediaServer( lpMediaServer ),
	m_streamBuffer(lpMediaServer->GetStreamBuffer()), 
	m_storage(m_streamBuffer.GetStorage()), 
        m_MediaHeaderPosition(0),
        m_MinIndex(0),
	m_MaxIndex(0)
{
	m_IntervalTimer.set_callback(boost::bind(&CNetWriter::OnTimer, this));

	LIVE_ASSERT( lpPacket );
	LIVE_ASSERT( lpPacket->GetPieceType() == PPDT_MEDIA_HEADER );

	m_DataUnitSize = AsfHelper::GetDataUnitSizeFromHeader(lpPacket->GetHeader(), lpPacket->GetHeaderLength());
//	m_DataUnitSize = 0;
	const BYTE* buffer = m_lpMediaHead->GetHeader();
	int length = m_lpMediaHead->GetHeaderLength();
	
	if( buffer[0] == 'H' && 
		buffer[1] == 'T' && 
		buffer[2] == 'T' && 
		buffer[3] == 'P' )
	{	// 这就说明在Source处已经打上了HTTP的头部,于是就要抹掉HTTP的头部
		int i = 0;
		for( i = 4; i < length; i ++ )
		{
			if( buffer[i-4] == '\r' &&
				buffer[i-3] == '\n' &&
				buffer[i-2] == '\r' &&
				buffer[i-1] == '\n' )
			{	// 说明此时i就是 真正的MediaHeader的起始位置
				m_MediaHeaderPosition = i;
				break;			//---------------------------------|
			}					//                                 |
		}						//                                 |                   
		LIVE_ASSERT( i < length );	// 这说明头部不对啊                |
		//	   <---------------------------------------------------|
		LIVE_ASSERT( i > 0 );
		LIVE_ASSERT( m_MediaHeaderPosition > 0 );
	}
	else
	{	// 还好，这说明在Source处还没有打上HTTP的头部
		m_MediaHeaderPosition = 0;
	}
}

CNetWriter::~CNetWriter()
{
}

UINT CNetWriter::LocationStartIndex()
{	// 这个函数的目的是 定位 StartIndex 
	return max( m_storage.GetMinIndex(), m_streamBuffer.GetPrelocationIndex() );
}

HRESULT CNetWriter::Start()
{
	if( m_state == st_start )
	{
		NETWRITER_ERROR( "CNetWriter is started, so can not start! " );
		return E_FAIL;
	}
	m_IntervalTimer.start( 1000 );

	m_dwLastIndex = 0;

	// 置状态
	m_state = st_start;
	
	return S_OK;
}

HRESULT CNetWriter::Stop()
{
	if( m_state != st_start )
	{
		NETWRITER_ERROR( "Stop-CNetWriter is not started, so can not stop! " );
		return E_FAIL;
	}
	
	m_PendingClients.clear();
	
	m_MinIndex = 0;
	m_MaxIndex = 0;
	
	m_IntervalTimer.stop();
	// 置状态
	m_state = st_stop;
	
	return S_OK;
}


void CNetWriter::OnTimer()
{
	OnIntervalTimer(m_IntervalTimer.get_times());
}

bool CNetWriter::OnAcceptConnection(tcp_socket_ptr newClient, const SocketAddress& addr)
{
	MEDIASERVER_INFO("CNetWriter::OnAcceptConnection " << *newClient << " from " << addr);
	tcp_socket_ptr client(newClient);
	LIVE_ASSERT(client->is_open());
	client->set_listener(this);
	// 启动收包
	if (!client->receive())
	{
		NETWRITER_ERROR( "CNetWriter DoHttpRecv Error "<<*newClient );
		return true;
	}
	if (m_PendingClients.insert(make_pair(client.get(), client)).second)
	{
		// 添加成功，需要释放client对socket对象的控制权
		return true;
	}
	return true;
}


bool CNetWriter::OnClientReceive(tcp_socket_ptr s, const BYTE* data, size_t size)
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
	
    if (strstr((char const *)data, "/secret.tmp") == NULL)
    {
        return false;
    }

	// 发送HTTP的头部
	SendOk = s->send(HTTP_HEADER, sizeof(HTTP_HEADER)-1 );
	if( SendOk == false )
	{
		NETWRITER_ERROR( "CNetWriter Send HTTP Header Error "<<*s );
		return false;
	}
	
	// 发送Media的头部
	const BYTE *buffer = m_lpMediaHead->GetHeader();
	int length = m_lpMediaHead->GetHeaderLength();
	SendOk = s->send( buffer+m_MediaHeaderPosition, length-m_MediaHeaderPosition );
	if( SendOk == false )
	{
		NETWRITER_ERROR( "CNetWriter Send Media Header Error "<<*s );
		return false;
	}
	NETWRITER_DEBUG( "CNetWriter Send Media Header "<<m_lpMediaHead->GetPieceIndex() );
	VIEW_INFO("PieceSend "<<m_lpMediaHead->GetPieceIndex()<<" End");
	
	return OnNewClient(s);
}

void CNetWriter::on_socket_receive(tcp_socket* sender, BYTE* data, size_t size)
{
	TCPClientSocketCollection::iterator iter = m_PendingClients.find(sender);
	if ( iter == m_PendingClients.end() )
	{
		LIVE_ASSERT(false);
		return;
	}

	tcp_socket_ptr sock = iter->second;
	m_PendingClients.erase(iter);
	NETWRITER_EVENT("CNetWriter::OnHttpRecvSuccess " << m_PendingClients.size() 
		<< *sender << " " << make_buffer_pair(data, size) << endl << string(reinterpret_cast<const char*>( data ), size));
	
	bool success = OnClientReceive(sock, data, size);
	
	if (!success)
	{
		NETWRITER_ERROR( "CNetWriter::OnClientReceive failed " << *sender );
	}
	
}

void CNetWriter::on_socket_receive_failed(tcp_socket* sender, int errcode)
{
	NETWRITER_ERROR("NetWriter::OnHttpRecvFaild " << make_pair(sender, errcode));
	OnClientError(sender, errcode);
	if (errcode == boost::asio::error::operation_aborted)
	{
		m_lpMediaServer->OnClientError(errcode);
	}
	m_PendingClients.erase(sender);
}

UINT CNetWriter::LocateStartPosition()
{
	return LocationStartIndex();
}

UINT CNetWriter::GetPlaytoIndex() const
{
	return m_lpMediaServer->GetPlaytoIndex();
}

void CNetWriter::SavePlaytoIndex(UINT playtoIndex)
{
	m_lpMediaServer->SavePlaytoIndex(playtoIndex);
	MediaServerStatistics& statistics = m_lpMediaServer->GetStatisticsData();
	statistics.PlayToIndex = playtoIndex;
	if ( 0 == statistics.FirstPlayIndex )
	{
		statistics.FirstPlayIndex = playtoIndex;
		statistics.FirstPlayTime = time_counter().get_count();
		MediaDataPiecePtr dataPiece = m_lpMediaServer->GetStorage().GetDataPiece( playtoIndex );
		//LIVE_ASSERT( dataPiece );
		if ( dataPiece )
		{
			statistics.FirstPlayPieceTimeStamp = dataPiece->GetTimeStamp();
		}
	}
}

UINT CNetWriter::GetServerPort() const
{
	return m_lpMediaServer->GetPort();
}

void CNetWriter::Reset()
{
	Stop();
	Start();
}


CNetWriter* NetWriterFactory::Create(CMediaServer* mediaServer, MonoMediaHeaderPiecePtr headerPiece)
{
	LIVE_ASSERT( headerPiece );
	if( !headerPiece )
		return NULL;

	UINT mediaType = headerPiece->GetMediaType();
	if (mediaType == PPMT_MEDIA_WMF_SAMPLE2)
	{
		LIVE_ASSERT(false);
		return NULL;
		//mediaServer->StopServer();
		//return CreateNewSampleNetWriter(mediaServer, headerPiece);
	}

	mediaServer->StartServer();
	
	if( mediaType == PPMT_MEDIA_RMFF_FILE ||
		mediaType == PPMT_MEDIA_RMFF_HTTP ||
		mediaType == PPMT_MEDIA_RMFF_RTSP )
	{	// Rmff系列用 CRealNetWriter
		return CreateRealNetWriter(mediaServer, headerPiece);
	}
	else if( mediaType == PPMT_MEDIA_ASF_MMSH )
	{	// Mmsh系列用 CMmshNetWriter
		return CreateMmshMediaNetWriter(mediaServer, headerPiece);
	}
	else if( mediaType == PPMT_MEDIA_MKV_FILE || mediaType == PPMT_MEDIA_FLV_FILE)
	{
		return CreateMkvNetWriter(mediaServer, headerPiece);
	}


//	return CreateWindowsMediaNetWriter(mediaServer, headerPiece);
#if defined(_PPL_SILVER_LIGHT_VERSION)
	return CreateVistaMediaNetWriter(mediaServer, headerPiece);
#endif
	
#if defined(_PPL_PLATFORM_MSWIN)

#if !defined(_WIN32_WCE)

	int GraphMode = mediaServer->GetListener().GetGraphMode();
        if( IsVBR(headerPiece))
	{	// 如果是 VBR 完全采用 VistaMediaNetWriter 来构造
		return CreateVistaMediaNetWriter(mediaServer, headerPiece);
	}

	if( GraphMode == 0 )
	{
// 		if( IsVBR(headerPiece) )
// 		{	// 如果是 VBR 完全采用 VistaMediaNetWriter 来构造
// 			return CreateVistaMediaNetWriter(mediaServer, headerPiece);
// 		}

		// 正常模式 对 Vista 和 Windows 自动适应
		OSVERSIONINFO osvi;
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx (&osvi);

		if( osvi.dwMajorVersion <= 5 )
		{	// 是 Windows, 采用传统的文件流方式输出
			return CreateWindowsMediaNetWriter(mediaServer, headerPiece);
		}
		else if( osvi.dwMajorVersion == 6 )
		{	// 是 Vista, 采用 修正时间戳后的HTTP流 方式输出
			return CreateVistaHttpMediaNetWriter(mediaServer, headerPiece);
		}
		else
		{
			LIVE_ASSERT(0);
			return CreateVistaHttpMediaNetWriter(mediaServer, headerPiece);
		}
	}
	else if( GraphMode == 1 )
	{	// 强制使用 MMSH, 主要在 没有声卡驱动, 下载缓冲100%, 但是播放器任然不停地来回缓冲
		return CreateVistaMediaNetWriter(mediaServer, headerPiece);	
	}
	else if( GraphMode == 2 )
	{	// Graph方式,强制使用 修正时间戳后的HTTP流, 主要在有声音无图像的时候使用
		//return CreateVistaMediaNetWriter(mediaServer, headerPiece);
		return CreateVistaHttpMediaNetWriter(mediaServer, headerPiece);
	}
	
	return CreateWindowsMediaNetWriter(mediaServer, headerPiece);

#else

//#pragma message("------Windows CE使用普通asf方式的播放策略")
	return CreateWindowsMediaNetWriter(mediaServer, headerPiece);

#endif

#else

	int netWriterMode = mediaServer->GetListener().GetNetWriterMode();
	if ( 1 == netWriterMode )
	{
		return CreateMmshMediaNetWriter(mediaServer, headerPiece);
	}
	else if ( 2 == netWriterMode )
	{
		return CreateVistaMediaNetWriter(mediaServer, headerPiece);
	}
	else if ( 3 == netWriterMode )
	{
		return CreateVistaHttpMediaNetWriter(mediaServer, headerPiece);
	}
	return CreateWindowsMediaNetWriter(mediaServer, headerPiece);

#endif
}

bool NetWriterFactory::IsVBR(MonoMediaHeaderPiecePtr headerPiece)
{
	const BYTE *buffer = headerPiece->GetHeader();
	int length = headerPiece->GetHeaderLength();
	
	string header(reinterpret_cast<const char*>( buffer ), length);
	string vbrstring( "I\0s\0V\0B\0R\0\0\0\1\0", sizeof("I\0s\0V\0B\0R\0\0\0\1\0")-1 );

	if( header.find(vbrstring) != string::npos )
	{	// 
		return true;
	}
	else
	{	// 
		return false;
	}
}

MediaDataPiecePtr CNetWriter::GetFirst(UINT pieceIndex) const
{
	return m_storage.GetFirst(pieceIndex);
}

MediaDataPiecePtr CNetWriter::GetNext(UINT pieceIndex) const
{
	return m_storage.GetNext(pieceIndex);
}

void CNetWriter::ParseHeader()
{
	if ( ! m_lpMediaHead )
	{
		LIVE_ASSERT( false );
		return;
	}
	const BYTE *buffer = m_lpMediaHead->GetHeader();
	int length = m_lpMediaHead->GetHeaderLength();
	if ( length < m_MediaHeaderPosition )
	{
		LIVE_ASSERT( false );
		return;
	}
	buffer += m_MediaHeaderPosition;
	length -= m_MediaHeaderPosition;


	try
	{
		m_AsfHeaderObject.Parse( reinterpret_cast<const char*>( buffer ), length );
		if ( m_AsfHeaderObject.GetFilePropertiesObject() )
		{
			//Tracer::Trace("NetWriter::ParserHeader preroll=%I64d\n", m_AsfHeaderObject.GetFilePropertiesObject()->GetPreroll());
		}
	}
	catch ( std::exception& e )
	{
		NETWRITER_ERROR("CNetWriter::ParseHeader " << e.what());
	}
}

UINT32 CNetWriter::GetPreroll() const
{
	if ( NULL == m_AsfHeaderObject.GetFilePropertiesObject() )
		return 5000;

	UINT64 preroll = m_AsfHeaderObject.GetFilePropertiesObject()->GetPreroll();
	LIVE_ASSERT( preroll > 0 && preroll < UINT_MAX );
	LIMIT_MIN( preroll, 1 );
	LIMIT_MAX( preroll, UINT_MAX );
	return static_cast<UINT32>( preroll );
}

bool CNetWriter::SendDataToClient( tcp_socket_ptr sock, const void* data, size_t size )
{
	return sock->send(data, size);
}
