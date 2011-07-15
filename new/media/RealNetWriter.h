#ifndef _LIVE_P2PCOMMON2_NEW_MEDIA_REAL_NETWRITER_H_
#define _LIVE_P2PCOMMON2_NEW_MEDIA_REAL_NETWRITER_H_

#include "NetWriter.h"
#include <set>

struct RealMediaClient : public MediaClient
{
public:
	RealMediaClient();
	RealMediaClient( CNetWriter* netWriter, tcp_socket_ptr sock );
	HRESULT SetPlayableRange( UINT HeaderIndex,				// 要对照的 HeaderIndex
							  DWORD MinIndex,					
							  DWORD MaxIndex,
							  const std::set<UINT16>& AudioPosistionSet	// 音频流编号的数据集合
							  );		// 是否发送所有数据
};

typedef map<tcp_socket*, RealMediaClient> RealMediaClientCollection;

class CRealNetWriter : public CNetWriter
{
public:
	CRealNetWriter( CMediaServer* lpMediaServer, MonoMediaHeaderPiecePtr  lpPacket );
	~CRealNetWriter();

	HRESULT Stop();
	HRESULT SetPlayableRange(DWORD MinIndex,DWORD MaxIndex);
	void DisconnectAllMediaClient();
	PLAYER_TYPE GetPlayerType() const { return REAL_PLAYER; }

protected:
	virtual void OnClientError(tcp_socket* sock, long errcode);
	virtual bool ContainsClient(tcp_socket* sock);
	virtual bool OnNewClient(tcp_socket_ptr s);

protected:
	set<UINT16> m_PosistionSet;
	RealMediaClientCollection m_ClientHandles;
};


#endif