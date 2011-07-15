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

#define E_SEND_ERROR		-10		// 发送数据包失败
#define E_LOCATION_ERROR	-20		// 定位失败
#define E_FORECASR_NOT_GOOD	-30		// 预测的结果不够理想


//const int ALLOW_PAENDDING_COUNT = 50;

typedef std::map<tcp_socket*, tcp_socket_ptr>	TCPClientSocketCollection;

class CNetWriter;
class CStreamBuffer;
class Storage;


/// 代表一个播放器客户端的连接
struct MediaClient : public pool_object
{
public:
	/// 相对应的播放器所对应的句柄
	tcp_socket_ptr	m_Socket;
	CNetWriter* m_NetWriter;

	/// 是否已经开始向播放器发送数据
	bool  m_IsSending;

	/// 当前应该丢到哪一片
	UINT m_WillPlayIndex;

	/// 上一次丢的Piece的编号
	UINT m_LastPlayIndex;

protected:
	/// 是否需要从asf的key frame开始输出
	bool m_NeedStartWithKeyFrame;

	/// 已经定位到了key frame
	bool m_KeyFrameLocated;

	/// 似乎否需要修改时间戳
	bool m_NeedChangeTimeStamps;

	/// 检查了多少个data packet
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
	HRESULT SetPlayableRangeAfterSended( UINT HeaderIndex,		// 要对照的 HeaderIndex
							  DWORD MinIndex,					
							  DWORD MaxIndex);		// 是否发送所有数据

	/// 发送数据
	bool SendData(const void* data, size_t size);

	bool DoSendDataPiece( MediaDataPiecePtr dataPiece );
};



/// 负责往连接上来的播放器客户端推数据
class CNetWriter : private boost::noncopyable, public tcp_socket_listener
{
public:
	CNetWriter(CMediaServer* lpMediaServer, MonoMediaHeaderPiecePtr lpPacket);
	virtual ~CNetWriter();

	virtual HRESULT Start();
	virtual HRESULT Stop();

	MediaDataPiecePtr GetFirst(UINT pieceIndex) const;
	MediaDataPiecePtr GetNext(UINT pieceIndex) const;

	/// 重置
	void Reset();

	virtual HRESULT SetPlayableRange(DWORD MinIndex,DWORD MaxIndex) = 0;
	virtual PLAYER_TYPE GetPlayerType() const { return NULL_PLAYER; }
	UINT LocationStartIndex();

	virtual bool OnAcceptConnection(tcp_socket_ptr newClient, const SocketAddress& addr);

	/// 获取播放端口
	virtual UINT16 GetPort() const { return 0; }

	virtual void DisconnectAllMediaClient() = 0; 

	/// 定位起始的播放位置
	UINT LocateStartPosition();

	/// 获取当前播放到的位置
	UINT GetPlaytoIndex() const;

	/// 保存当前播放到的位置
	void SavePlaytoIndex(UINT playtoIndex);

	/// 获取服务器端口
	UINT GetServerPort() const;

	UINT32 GetPreroll() const;

	UINT   GetDataUnitSize() { return m_DataUnitSize; }

	pool_byte_buffer& GetPieceOperationBuffer() { return m_PieceOperationBuffer; }

protected:
	void OnTimer();
	/// 间隔1秒的定时器
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

	///真正的MediaHeader在总的头部中的相对位置
	int m_MediaHeaderPosition;

	DWORD m_MinIndex;
	DWORD m_MaxIndex;

	TCPClientSocketCollection m_PendingClients;

	/// 为减少内存申请次数优化性能的piece数据缓冲区
	pool_byte_buffer m_PieceOperationBuffer;

	Synacast::Format::AsfHeaderObject m_AsfHeaderObject;

	UINT		m_DataUnitSize;										// 一个AsfDataUnit的大小
};



/// NetWriter的类工厂
class NetWriterFactory
{
public:
	/// 创建 vista 下面wmp媒体的NetWriter
	static CNetWriter* CreateVistaMediaNetWriter(CMediaServer* mediaServer, MonoMediaHeaderPiecePtr headerPiece);
	
	/// 创建 vista 下面wmp媒体的NetWriter
	static CNetWriter* CreateVistaHttpMediaNetWriter(CMediaServer* mediaServer, MonoMediaHeaderPiecePtr headerPiece);

	/// 创建 windows 下面wmp媒体的NetWriter
	static CNetWriter* CreateWindowsMediaNetWriter(CMediaServer* mediaServer, MonoMediaHeaderPiecePtr headerPiece);

	/// 创建 mmsh 协议的 NetWriter
	static CNetWriter* CreateMmshMediaNetWriter(CMediaServer* mediaServer, MonoMediaHeaderPiecePtr headerPiece);

	/// 创建real媒体的NetWriter
	static CNetWriter* CreateRealNetWriter(CMediaServer* mediaServer, MonoMediaHeaderPiecePtr headerPiece);
	
	/// 创建mkv媒体的NetWriter
	static CNetWriter* CreateMkvNetWriter(CMediaServer* mediaServer, MonoMediaHeaderPiecePtr headerPiece);

	/// 创建使用新sample方式的NetWriter
//	static CNetWriter* CreateNewSampleNetWriter(CMediaServer* mediaServer, PPMediaHeaderPacketPtr headerPiece);

	/// 创建NetWriter
	static CNetWriter* Create(CMediaServer* mediaServer, MonoMediaHeaderPiecePtr headerPiece);
	
private:

	/// 判断是不是 VBR
	static bool IsVBR(MonoMediaHeaderPiecePtr headerPiece);
};



#endif