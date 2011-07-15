#ifndef _LIVE_P2PCOMMON2_NEW_MEDIA_COMMON_NET_WRITER_H_
#define _LIVE_P2PCOMMON2_NEW_MEDIA_COMMON_NET_WRITER_H_

#include "NetWriter.h"


struct WindowsMediaClient : public MediaClient
{
public:
	WindowsMediaClient( );
	WindowsMediaClient( CNetWriter* netWriter, tcp_socket_ptr sock);
	HRESULT SetPlayableRange( UINT HeaderIndex,				// 要对照的 HeaderIndex
							  DWORD MinIndex,					
							  DWORD MaxIndex);		// 是否发送所有数据
protected:
	int		m_PacketSize;										// 一个AsfDataUnit的大小
};


typedef map<tcp_socket*,WindowsMediaClient> WindowMediaClientCollection;

class CWindowsNetWriter : public CNetWriter
{
public:
	CWindowsNetWriter(CMediaServer* lpMediaServer, MonoMediaHeaderPiecePtr  lpPacket);
	~CWindowsNetWriter();

	HRESULT Stop();
	HRESULT SetPlayableRange(DWORD MinIndex,DWORD MaxIndex);
	void DisconnectAllMediaClient(); 
	PLAYER_TYPE GetPlayerType() const { return MEDIA_PLAYER; }

protected:
	virtual void OnClientError(tcp_socket* sock, long errcode);
	virtual bool ContainsClient(tcp_socket* sock);
	virtual bool OnNewClient(tcp_socket_ptr s);

protected:
	WindowMediaClientCollection m_ClientHandles;
};

#endif