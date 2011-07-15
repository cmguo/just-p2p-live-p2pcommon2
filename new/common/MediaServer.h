#ifndef _LIVE_P2PCOMMON2_NEW_COMMON_MEDIA_SERVER_H_
#define _LIVE_P2PCOMMON2_NEW_COMMON_MEDIA_SERVER_H_

#include "writertypes.h"

#include <synacast/protocol/DataCollecting.h>
#include <synacast/protocol/piecefwd.h>
#include <ppl/net/socketfwd.h>

#include <boost/noncopyable.hpp>


class CStreamBuffer;
class Storage;

class MediaServerListener;


class MediaServerStatistics : public MEDIASERVER_STATS, private boost::noncopyable
{
public:
	MediaServerStatistics()
	{
		this->Clear();
	}


};


class CWriterCollection;
class CNetWriter;


/// 媒体服务器
class CMediaServer : private boost::noncopyable, public tcp_acceptor_listener
{
public:
	typedef boost::function<void (int)> ClientErrorCallback;
	/// port是界面传来的播放监听端口
	CMediaServer( MediaServerListener& listener, CStreamBuffer& lpStreamBuffer, USHORT port );
	~CMediaServer();

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
	bool AddMediaHeader( MonoMediaHeaderPiecePtr headerPiece );
	/// 给StreamBuffer调用的
	void SetPlayableRange( DWORD MinIndex, DWORD MaxIndex );

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

	UINT GetNowHeaderIndex() const { return m_NowHeaderIndex; }

	virtual const MediaServerStatistics& GetStatistics() const { return m_Statistics; }
	virtual MediaServerStatistics& GetStatisticsData() { return m_Statistics; }
	
	void SetClientErrorCallback(ClientErrorCallback cbHandler) { m_cbClientError = cbHandler; }
	void OnClientError(int errorcode) { if (m_cbClientError != NULL) m_cbClientError(errorcode); }

protected:
	void Init();
	virtual void on_socket_accept(tcp_acceptor* sender, tcp_socket* newClient, const InetSocketAddress& addr);

	/// 换头操作
	bool ChangeMediaHeader( UINT HeaderIndex );

	bool CreateNetWriter(const MonoMediaHeaderPiecePtr& header);
	
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

	MediaServerStatistics m_Statistics;

	MediaServerListener& m_listener;
	CStreamBuffer& m_streamBuffer;
	const Storage& m_storage;
	/// 端口
	USHORT m_port;
	boost::shared_ptr<tcp_acceptor> m_server;

	/// 负责实际的推流
	CNetWriter* m_netWriter;

	/// 现在正供Writer使用的Header Index
	UINT m_NowHeaderIndex;

	/// 当前播放到的位置
	UINT m_PlaytoIndex;

	/// 当前播放的媒体类型
	PLAYER_TYPE m_mediaType;

	/// 是否是第一次 换头
	bool m_isFirst;

	ClientErrorCallback m_cbClientError;
};
#endif
