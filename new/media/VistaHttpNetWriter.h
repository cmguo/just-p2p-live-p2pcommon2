#ifndef _LIVE_P2PCOMMON2_NEW_MEDIA_VISTA_NET_WRITER_H_
#define _LIVE_P2PCOMMON2_NEW_MEDIA_VISTA_NET_WRITER_H_

#include "NetWriter.h"


struct VistaHttpMediaClient : public MediaClient
{
public:
	VistaHttpMediaClient();
	//VistaHttpMediaClient( DWORD DataUnitSize );
	VistaHttpMediaClient( CNetWriter* netWriter, tcp_socket_ptr sock, DWORD DataUnitSize);
	HRESULT SetPlayableRange( UINT HeaderIndex,				// Ҫ���յ� HeaderIndex
							  DWORD MinIndex,					
							  DWORD MaxIndex);					// �Ƿ�����������

protected:
	HRESULT SetPlayableRangeAfterSended( UINT HeaderIndex,		// Ҫ���յ� HeaderIndex
							  DWORD MinIndex,					
							  DWORD MaxIndex);					// �Ƿ�����������
	HRESULT SendPiece(MonoMediaDataPiecePtr lpDataPacket);
/*
	bool ChanegeTimeStamps(BYTE * const packetBuffer, const size_t maxPacketSize, const DWORD startTimeStamp);
	DWORD GetTimestamp(BYTE * const packetBuffer, const size_t maxPacketSize);
	size_t LocateTimeStampOffset( const BYTE * const packetBuffer, const size_t maxPacketSize );
*/
	size_t	m_DataUnitSize;										// һ��AsfDataUnit�Ĵ�С
	bool	m_bIsStartRound;
	UINT32	m_uSendCount;
};


typedef map<tcp_socket*,VistaHttpMediaClient> VistaHttpMediaClientCollection;

class CVistaHttpNetWriter : public CNetWriter
{
public:
	CVistaHttpNetWriter(CMediaServer* lpMediaServer, MonoMediaHeaderPiecePtr  lpPacket);
	~CVistaHttpNetWriter();

	HRESULT Stop();
	HRESULT SetPlayableRange(DWORD MinIndex,DWORD MaxIndex);
	void DisconnectAllMediaClient(); 
	PLAYER_TYPE GetPlayerType() const { return MEDIA_PLAYER; }

protected:
	virtual void OnClientError(tcp_socket* sock, long errcode);
	virtual bool ContainsClient(tcp_socket* sock);
	virtual bool OnNewClient(tcp_socket_ptr s);

protected:
	VistaHttpMediaClientCollection m_ClientHandles;
//	int		m_DataUnitSize;										// һ��AsfDataUnit�Ĵ�С
};

#endif