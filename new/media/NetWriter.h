#ifndef _LIVE_P2PCOMMON2_NEW_MEDIA_NET_WRITER_H_
#define _LIVE_P2PCOMMON2_NEW_MEDIA_NET_WRITER_H_


#include "common/writertypes.h"
#include "common/piecefwd.h"
#include "SourceReaderLib/AsfHeaderObject.h"
#include "framework/timer.h"
#include "framework/log.h"

#include <synacast/protocol/MonoMediaPiece.h>
#include <ppl/net/socketfwd.h>

#include <boost/noncopyable.hpp>


class CMediaServer;
class SocketAddress;

#define E_SEND_ERROR		-10		// �������ݰ�ʧ��
#define E_LOCATION_ERROR	-20		// ��λʧ��
#define E_FORECASR_NOT_GOOD	-30		// Ԥ��Ľ����������


//const int ALLOW_PAENDDING_COUNT = 50;

typedef std::map<tcp_socket*, tcp_socket_ptr>	TCPClientSocketCollection;

class CNetWriter;
class CStreamBuffer;
class Storage;


/// ����һ���������ͻ��˵�����
struct MediaClient : public pool_object
{
public:
	/// ���Ӧ�Ĳ���������Ӧ�ľ��
	tcp_socket_ptr	m_Socket;
	CNetWriter* m_NetWriter;

	/// �Ƿ��Ѿ���ʼ�򲥷�����������
	bool  m_IsSending;

	/// ��ǰӦ�ö�����һƬ
	UINT m_WillPlayIndex;

	/// ��һ�ζ���Piece�ı��
	UINT m_LastPlayIndex;

protected:
	/// �Ƿ���Ҫ��asf��key frame��ʼ���
	bool m_NeedStartWithKeyFrame;

	/// �Ѿ���λ����key frame
	bool m_KeyFrameLocated;

	/// �ƺ�����Ҫ�޸�ʱ���
	bool m_NeedChangeTimeStamps;

	/// ����˶��ٸ�data packet
	UINT m_CheckedDataPacketCount;

	bool	m_isFirst;
	DWORD	m_StartTimeStamp;
	DWORD	m_StartTimeStampPL;
	UINT32	m_lastSendTS;
	UINT32	m_lastPresentTS;


public:
	MediaClient();
	MediaClient( CNetWriter* netWriter, tcp_socket_ptr sock );

protected:
	HRESULT SetPlayableRangeAfterSended( UINT HeaderIndex,		// Ҫ���յ� HeaderIndex
							  DWORD MinIndex,					
							  DWORD MaxIndex);		// �Ƿ�����������

	/// ��������
	bool SendData(const void* data, size_t size);

	bool DoSendDataPiece( MediaDataPiecePtr dataPiece );
};



/// ���������������Ĳ������ͻ���������
class CNetWriter : private boost::noncopyable, public tcp_socket_listener
{
public:
	CNetWriter(CMediaServer* lpMediaServer, MonoMediaHeaderPiecePtr lpPacket);
	virtual ~CNetWriter();

	virtual HRESULT Start();
	virtual HRESULT Stop();

	MediaDataPiecePtr GetFirst(UINT pieceIndex) const;
	MediaDataPiecePtr GetNext(UINT pieceIndex) const;

	/// ����
	void Reset();

	virtual HRESULT SetPlayableRange(DWORD MinIndex,DWORD MaxIndex) = 0;
	virtual PLAYER_TYPE GetPlayerType() const { return NULL_PLAYER; }
	UINT LocationStartIndex();

	virtual bool OnAcceptConnection(tcp_socket_ptr newClient, const SocketAddress& addr);

	/// ��ȡ���Ŷ˿�
	virtual UINT16 GetPort() const { return 0; }

	virtual void DisconnectAllMediaClient() = 0; 

	/// ��λ��ʼ�Ĳ���λ��
	UINT LocateStartPosition();

	/// ��ȡ��ǰ���ŵ���λ��
	UINT GetPlaytoIndex() const;

	/// ���浱ǰ���ŵ���λ��
	void SavePlaytoIndex(UINT playtoIndex);

	/// ��ȡ�������˿�
	UINT GetServerPort() const;

	UINT32 GetPreroll() const;

	UINT   GetDataUnitSize() { return m_DataUnitSize; }

	pool_byte_buffer& GetPieceOperationBuffer() { return m_PieceOperationBuffer; }

protected:
	void OnTimer();
	/// ���1��Ķ�ʱ��
	virtual void OnIntervalTimer(int seconds) { }

	virtual void on_socket_receive(tcp_socket* sender, BYTE* data, size_t size);
	virtual void on_socket_receive_failed(tcp_socket* sender, int errcode);

	virtual bool OnClientReceive(tcp_socket_ptr sock, const BYTE* data, size_t size);
	virtual void OnClientError(tcp_socket* sock, long errcode) { }
	virtual bool OnNewClient(tcp_socket_ptr sock) { return false; }
	virtual bool ContainsClient(tcp_socket* sock) { return false; }


	void ParseHeader();

	bool SendDataToClient(tcp_socket_ptr sock, const void* data, size_t size);

public:
	CMediaServer * GetMediaServer() const { return m_lpMediaServer; }
	DWORD m_dwLastIndex;

protected:
	enum MediaServerState{
		st_none,
		st_start,
		st_stop
	} m_state;

	periodic_timer m_IntervalTimer;

	MonoMediaHeaderPiecePtr m_lpMediaHead;
	CMediaServer * const m_lpMediaServer;
	CStreamBuffer& m_streamBuffer;
	const Storage& m_storage;

	///������MediaHeader���ܵ�ͷ���е����λ��
	int m_MediaHeaderPosition;

	DWORD m_MinIndex;
	DWORD m_MaxIndex;

	TCPClientSocketCollection m_PendingClients;

	/// Ϊ�����ڴ���������Ż����ܵ�piece���ݻ�����
	pool_byte_buffer m_PieceOperationBuffer;

	Synacast::Format::AsfHeaderObject m_AsfHeaderObject;

	UINT		m_DataUnitSize;										// һ��AsfDataUnit�Ĵ�С
};



/// NetWriter���๤��
class NetWriterFactory
{
public:
	/// ���� vista ����wmpý���NetWriter
	static CNetWriter* CreateVistaMediaNetWriter(CMediaServer* mediaServer, MonoMediaHeaderPiecePtr headerPiece);
	
	/// ���� vista ����wmpý���NetWriter
	static CNetWriter* CreateVistaHttpMediaNetWriter(CMediaServer* mediaServer, MonoMediaHeaderPiecePtr headerPiece);

	/// ���� windows ����wmpý���NetWriter
	static CNetWriter* CreateWindowsMediaNetWriter(CMediaServer* mediaServer, MonoMediaHeaderPiecePtr headerPiece);

	/// ���� mmsh Э��� NetWriter
	static CNetWriter* CreateMmshMediaNetWriter(CMediaServer* mediaServer, MonoMediaHeaderPiecePtr headerPiece);

	/// ����realý���NetWriter
	static CNetWriter* CreateRealNetWriter(CMediaServer* mediaServer, MonoMediaHeaderPiecePtr headerPiece);
	
	/// ����mkvý���NetWriter
	static CNetWriter* CreateMkvNetWriter(CMediaServer* mediaServer, MonoMediaHeaderPiecePtr headerPiece);

	/// ����ʹ����sample��ʽ��NetWriter
//	static CNetWriter* CreateNewSampleNetWriter(CMediaServer* mediaServer, PPMediaHeaderPacketPtr headerPiece);

	/// ����NetWriter
	static CNetWriter* Create(CMediaServer* mediaServer, MonoMediaHeaderPiecePtr headerPiece);
	
private:

	/// �ж��ǲ��� VBR
	static bool IsVBR(MonoMediaHeaderPiecePtr headerPiece);
};



#endif