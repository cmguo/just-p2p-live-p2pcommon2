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


/// ý�������
class CMediaServer : private boost::noncopyable, public tcp_acceptor_listener
{
public:
	typedef boost::function<void (int)> ClientErrorCallback;
	/// port�ǽ��洫���Ĳ��ż����˿�
	CMediaServer( MediaServerListener& listener, CStreamBuffer& lpStreamBuffer, USHORT port );
	~CMediaServer();

	MediaServerListener& GetListener() { return m_listener; }

	HRESULT Start( START_METHOD StartMethod, DWORD StartIndexTimeSpan, DWORD ShouldBufferTimeSpan );
	void Stop();
	/// Reset������Streambuffer��ҪRebufferʹ��.
	void Reset();

	/// ��ȡʵ�ʵļ����˿�
	USHORT GetPort() const { return m_port; }

	/// ��ȡ���ŵ�ý������
	PLAYER_TYPE GetPlayerType() const { return m_mediaType; }

	/// ��StreamBuffer���õ�
	bool AddMediaHeader( MonoMediaHeaderPiecePtr headerPiece );
	/// ��StreamBuffer���õ�
	void SetPlayableRange( DWORD MinIndex, DWORD MaxIndex );

	/// ���浱ǰ���ŵ���λ��
	void SavePlaytoIndex(UINT playtoIndex);
	/// ��ȡ��ǰ���ŵ���λ��
	UINT GetPlaytoIndex() const { return m_PlaytoIndex; }

	/// ��������������
	void StartServer();

	/// ֹͣ������
	void StopServer();

	/// ��ȡ��������StreamBuffer
	CStreamBuffer& GetStreamBuffer() { return m_streamBuffer; }
	const Storage& GetStorage() { return m_storage; }


	/// �����Ƶ����е�����Ƭ
	void DisconnectAllMediaClient();

	UINT GetNowHeaderIndex() const { return m_NowHeaderIndex; }

	virtual const MediaServerStatistics& GetStatistics() const { return m_Statistics; }
	virtual MediaServerStatistics& GetStatisticsData() { return m_Statistics; }
	
	void SetClientErrorCallback(ClientErrorCallback cbHandler) { m_cbClientError = cbHandler; }
	void OnClientError(int errorcode) { if (m_cbClientError != NULL) m_cbClientError(errorcode); }

protected:
	void Init();
	virtual void on_socket_accept(tcp_acceptor* sender, tcp_socket* newClient, const InetSocketAddress& addr);

	/// ��ͷ����
	bool ChangeMediaHeader( UINT HeaderIndex );

	bool CreateNetWriter(const MonoMediaHeaderPiecePtr& header);
	
protected:
	enum MediaServerState{
		/// �ճ�ʼ����û������
		st_none,
		/// ������������û����NetWriter
		st_idle,
		/// ���������Ѿ�����NetWriter
		st_run,	
		/// ��ֹͣ
		st_stop,
	} m_state;

	MediaServerStatistics m_Statistics;

	MediaServerListener& m_listener;
	CStreamBuffer& m_streamBuffer;
	const Storage& m_storage;
	/// �˿�
	USHORT m_port;
	boost::shared_ptr<tcp_acceptor> m_server;

	/// ����ʵ�ʵ�����
	CNetWriter* m_netWriter;

	/// ��������Writerʹ�õ�Header Index
	UINT m_NowHeaderIndex;

	/// ��ǰ���ŵ���λ��
	UINT m_PlaytoIndex;

	/// ��ǰ���ŵ�ý������
	PLAYER_TYPE m_mediaType;

	/// �Ƿ��ǵ�һ�� ��ͷ
	bool m_isFirst;

	ClientErrorCallback m_cbClientError;
};
#endif
