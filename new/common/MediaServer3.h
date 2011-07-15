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



/// ý�������
class CMediaServer3 : public CMediaServer
{
public:
	/// port�ǽ��洫���Ĳ��ż����˿�
	CMediaServer3( MediaServerListener& listener, CStreamBuffer& lpStreamBuffer, USHORT port );
	~CMediaServer3();

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
	bool AddMediaHeader( const PPMediaHeaderPacketPtr& headerPiece );
	/// ��StreamBuffer���õģ�ʱ�������StreamBuffer����
	void SetPlayableRange( DWORD minIndex, DWORD maxIndex );

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

	UINT GetNowHeaderIndex() const { return m_CurrentHeaderIndex; }

protected:
	void Init();
	virtual bool OnSocketAccept(TCPServerSocket* sender, int sock, const InetSocketAddress& addr);

	/// ��ͷ����
	bool ChangeMediaHeader( u_int HeaderIndex );

protected:	// media service
	void ClosePreviousWriterSink();
	void CreateNewWriterSink( const PPMediaHeaderPacketPtr & mediaHeader );
	
	void CreateNewNetworkSink( const PPMediaHeaderPacketPtr & mediaHeader );
	
	void ServiceHeader( const PPMediaHeaderPacketPtr & mediaHeader );
	/************************************************************************/
	/* 
	ֻ���ŵ�ǰ����������һ���뵱ǰHeader Piece Index��һ�µ�Pieceʱ������
	*/
	/************************************************************************/
	UINT32 ServicePiecesToNextHeader( const UINT32 startIndex, const UINT32 endIndex );
	void ServiceOnePiece( const UINT32 pieceIndex );

private:
	/************************************************************************/
	/* 
	true, ����������HeaderPiece/DataPiece
	*/
	/************************************************************************/
	bool IsNewHeader( const UINT32 pieceIndex );
	bool IsHeaderPiece( const UINT32 pieceIndex );
	
private:	// utilities
	static bool IsVBR(const PPMediaHeaderPacketPtr& headerPiece);
	static bool IsOSVista();

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

	MediaServerListener& m_listener;
	CStreamBuffer& m_streamBuffer;
	const Storage& m_storage;
	/// �˿�
	USHORT m_port;
	scoped_ptr<TCPServerSocket> m_ServerSocket;

	/// ����ʵ�ʵ�����
	scoped_ptr<Writer>			m_Writer;
	scoped_ptr<NetworkSink>		m_NetworkSink;
	scoped_ptr<FileSink>		m_FileSink;
	wstring						m_FileSinkFileName;

	/// ��������Writerʹ�õ�Header Index
	u_int						m_CurrentHeaderIndex;

	/// ��ǰ���ŵ���λ��
	UINT						m_PlaytoIndex;

	/// ��ǰ���ŵ�ý������
	PLAYER_TYPE					m_mediaType;

	/// �Ƿ��ǵ�һ�� ��ͷ
	bool						m_IsFirstHeader;
};

#endif
