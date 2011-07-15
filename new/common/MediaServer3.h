#ifndef _LIVE_P2PCOMMON2_NEW_COMMON_MEDIASERVER3_H_
#define _LIVE_P2PCOMMON2_NEW_COMMON_MEDIASERVER3_H_

#include "MediaServer.h"

#include <map>
using std::map;

#include <boost/scoped_ptr.hpp>
#include <boost/any.hpp>
using boost::scoped_ptr;
using boost::any;

#include <writer/writer.h>
using namespace Synacast::StreamWriter;

#include <writer/FileSink.h>
#include <writer/NetworkSink.h>
using namespace Synacast::StreamSink;



/// 媒体服务器
class CMediaServer3 : public CMediaServer
{
public:
	/// port是界面传来的播放监听端口
	CMediaServer3( MediaServerListener& listener, CStreamBuffer& lpStreamBuffer, USHORT port );
	~CMediaServer3();

	MediaServerListener& GetListener() { return m_listener; }

	HRESULT Start( START_METHOD StartMethod, DWORD StartIndexTimeSpan, DWORD ShouldBufferTimeSpan );
	void Stop();
	/// Reset用于在Streambuffer需要Rebuffer使用.
	void Reset();

	/// 读取实际的监听端口
	USHORT GetPort() const { return m_port; }

	/// 获取播放的媒体类型
	PLAYER_TYPE GetPlayerType() const { return m_mediaType; }

	/// 给StreamBuffer调用的
	bool AddMediaHeader( const PPMediaHeaderPacketPtr& headerPiece );
	/// 给StreamBuffer调用的，时间控制由StreamBuffer管理
	void SetPlayableRange( DWORD minIndex, DWORD maxIndex );

	/// 保存当前播放到的位置
	void SavePlaytoIndex(UINT playtoIndex);
	/// 获取当前播放到的位置
	UINT GetPlaytoIndex() const { return m_PlaytoIndex; }

	/// 启动服务器监听
	void StartServer();

	/// 停止服务器
	void StopServer();

	/// 获取所关联的StreamBuffer
	CStreamBuffer& GetStreamBuffer() { return m_streamBuffer; }
	const Storage& GetStorage() { return m_storage; }


	/// 立即推掉所有的数据片
	void DisconnectAllMediaClient();

	UINT GetNowHeaderIndex() const { return m_CurrentHeaderIndex; }

protected:
	void Init();
	virtual bool OnSocketAccept(TCPServerSocket* sender, int sock, const InetSocketAddress& addr);

	/// 换头操作
	bool ChangeMediaHeader( u_int HeaderIndex );

protected:	// media service
	void ClosePreviousWriterSink();
	void CreateNewWriterSink( const PPMediaHeaderPacketPtr & mediaHeader );
	
	void CreateNewNetworkSink( const PPMediaHeaderPacketPtr & mediaHeader );
	
	void ServiceHeader( const PPMediaHeaderPacketPtr & mediaHeader );
	/************************************************************************/
	/* 
	只播放当前流，遇到第一个与当前Header Piece Index不一致的Piece时，跳出
	*/
	/************************************************************************/
	UINT32 ServicePiecesToNextHeader( const UINT32 startIndex, const UINT32 endIndex );
	void ServiceOnePiece( const UINT32 pieceIndex );

private:
	/************************************************************************/
	/* 
	true, 新数据流的HeaderPiece/DataPiece
	*/
	/************************************************************************/
	bool IsNewHeader( const UINT32 pieceIndex );
	bool IsHeaderPiece( const UINT32 pieceIndex );
	
private:	// utilities
	static bool IsVBR(const PPMediaHeaderPacketPtr& headerPiece);
	static bool IsOSVista();

protected:
	enum MediaServerState{
		/// 刚初始化，没有启动
		st_none,
		/// 已启动，但是没有启NetWriter
		st_idle,
		/// 已启动，已经启了NetWriter
		st_run,	
		/// 已停止
		st_stop,
	} m_state;

	MediaServerListener& m_listener;
	CStreamBuffer& m_streamBuffer;
	const Storage& m_storage;
	/// 端口
	USHORT m_port;
	scoped_ptr<TCPServerSocket> m_ServerSocket;

	/// 负责实际的推流
	scoped_ptr<Writer>			m_Writer;
	scoped_ptr<NetworkSink>		m_NetworkSink;
	scoped_ptr<FileSink>		m_FileSink;
	wstring						m_FileSinkFileName;

	/// 现在正供Writer使用的Header Index
	u_int						m_CurrentHeaderIndex;

	/// 当前播放到的位置
	UINT						m_PlaytoIndex;

	/// 当前播放的媒体类型
	PLAYER_TYPE					m_mediaType;

	/// 是否是第一次 换头
	bool						m_IsFirstHeader;
};

#endif
