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
	HRESULT SetPlayableRange( UINT HeaderIndex,				// Ҫ���յ� HeaderIndex
							  DWORD MinIndex,					
							  DWORD MaxIndex);		// �Ƿ�����������

	HRESULT End();
	void OnIntervalTimer(int seconds);
protected:
	HRESULT SetPlayableRangeAfterSended( UINT HeaderIndex,		// Ҫ���յ� HeaderIndex
							  DWORD MinIndex,					
							  DWORD MaxIndex);					// �Ƿ�����������
	HRESULT SendPiece(MonoMediaDataPiecePtr lpDataPacket);
	UINT64 GetRelativePieceTimeStamp() const { return m_PieceTimeStamp + m_BaseTick.elapsed(); }

	int		m_DataUnitSize;										// һ��AsfDataUnit�Ĵ�С
	DWORD	m_MmshIndex;										// mmsh�����ݰ���seqno
	time_counter	m_LastSendPieceTick;								// ��һ��SendPiece��ʱ��

	time_counter m_BaseTick;										// ������ �������׼ʱ��
	UINT64 m_PieceTimeStamp;										// ������ P2P��׼ʱ���

	time_counter m_LastPieceTick;								// ��һ����Piece�� �������׼ʱ��
	UINT64 m_LastPieceTimeStamp;									// ��һ����Piece�� P2P��׼ʱ���
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