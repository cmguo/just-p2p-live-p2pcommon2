#ifndef _LIVE_P2PCOMMON2_NEW_MEDIA_VISTA_NET_WRITER_H_
#define _LIVE_P2PCOMMON2_NEW_MEDIA_VISTA_NET_WRITER_H_

#include "NetWriter.h"
#include <ppl/util/time_counter.h>


struct VistaMediaClient : public MediaClient
{
public:
	VistaMediaClient();
	VistaMediaClient( DWORD DataUnitSize );
	VistaMediaClient( CNetWriter* netWriter, tcp_socket_ptr sock, DWORD DataUnitSize);
	HRESULT SetPlayableRange( UINT HeaderIndex,				// 要对照的 HeaderIndex
							  DWORD MinIndex,					
							  DWORD MaxIndex);		// 是否发送所有数据

	HRESULT End();
	void OnIntervalTimer(int seconds);
protected:
	HRESULT SetPlayableRangeAfterSended( UINT HeaderIndex,		// 要对照的 HeaderIndex
							  DWORD MinIndex,					
							  DWORD MaxIndex);					// 是否发送所有数据
	HRESULT SendPiece(MonoMediaDataPiecePtr lpDataPacket);
	UINT64 GetRelativePieceTimeStamp() const { return m_PieceTimeStamp + m_BaseTick.elapsed(); }

	int		m_DataUnitSize;										// 一个AsfDataUnit的大小
	DWORD	m_MmshIndex;										// mmsh的数据包的seqno
	time_counter	m_LastSendPieceTick;								// 上一次SendPiece的时间

	time_counter m_BaseTick;										// 匀速推 计算机基准时间
	UINT64 m_PieceTimeStamp;										// 匀速推 P2P基准时间戳

	time_counter m_LastPieceTick;								// 上一次推Piece的 计算机基准时间
	UINT64 m_LastPieceTimeStamp;									// 上一次推Piece的 P2P基准时间戳
};


typedef map<tcp_socket*,VistaMediaClient> VistaMediaClientCollection;

class CVistaNetWriter : public CNetWriter
{
public:
	CVistaNetWriter(CMediaServer* lpMediaServer, MonoMediaHeaderPiecePtr  lpPacket);
	~CVistaNetWriter();

	HRESULT Stop();
	HRESULT SetPlayableRange(DWORD MinIndex,DWORD MaxIndex);
	void DisconnectAllMediaClient(); 
	PLAYER_TYPE GetPlayerType() const { return MEDIA_PLAYER; }

protected:
	virtual bool OnClientReceive(tcp_socket_ptr sock, const BYTE* data, size_t size);
	virtual void OnClientError(tcp_socket* sock, long errcode);
	virtual bool ContainsClient(tcp_socket* sock);
	virtual bool OnNewClient(tcp_socket_ptr s);

	virtual void OnIntervalTimer(int seconds);

protected:
	VistaMediaClientCollection m_ClientHandles;
//	DWORD m_DataUnitSize;
};

#endif