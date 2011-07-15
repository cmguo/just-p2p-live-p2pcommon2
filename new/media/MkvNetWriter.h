#ifndef _LIVE_P2PCOMMON2_NEW_MEDIA_MKV_NET_WRITER_H_
#define _LIVE_P2PCOMMON2_NEW_MEDIA_MKV_NET_WRITER_H_

#include "NetWriter.h"
#include <string>

using std::string;

#ifdef _DEBUG
//#define WRITE_MKV_DATA_TO_FILE
#endif

#ifdef WRITE_MKV_DATA_TO_FILE
#include <fstream>
using std::fstream;
using std::stringstream;
#endif

struct MkvMediaClient : public MediaClient
{
	typedef std::basic_string<BYTE> buffer;
	MkvMediaClient();
	MkvMediaClient( CNetWriter* netWriter, tcp_socket_ptr sock);
#ifdef WRITE_MKV_DATA_TO_FILE
	~MkvMediaClient()
	{
		m_DataFile.close();
	}
#endif
	HRESULT SetPlayableRange( UINT HeaderIndex,				// 要对照的 HeaderIndex
							  DWORD MinIndex,					
							  DWORD MaxIndex);		// 是否发送所有数据

protected:
	HRESULT SetPlayableRangeAfterSended( UINT HeaderIndex,		// 要对照的 HeaderIndex
							  DWORD MinIndex,					
							  DWORD MaxIndex);		// 是否发送所有数据
	bool SendToClient(UINT32 PacketIndex);
private:
	void PrepareForNextDataUnit()
	{
		m_DataUnitBuffer.erase(m_DataUnitBuffer.begin(),m_DataUnitBuffer.end());
		m_PacketSize = 0;
		m_PiecesIndexWanted = 0;
		m_piecesNumber = 0;
	}

	bool IsInDataUnit()
	{
		 return (m_PiecesIndexWanted != 0);
	}

private:
	buffer m_DataUnitBuffer; // data units' buffer
	UINT8 m_PiecesIndexWanted;
	UINT8 m_piecesNumber;
	UINT32 m_PacketSize;

#ifdef WRITE_MKV_DATA_TO_FILE
	fstream m_DataFile;
#endif
};

typedef map<tcp_socket*,MkvMediaClient*> MkvMediaClientCollection;

class CMkvNetWriter : public CNetWriter  
{
public:
	CMkvNetWriter(CMediaServer* lpMediaServer, MonoMediaHeaderPiecePtr lpPacket);
	virtual ~CMkvNetWriter();

	HRESULT Stop();
	HRESULT SetPlayableRange(DWORD MinIndex,DWORD MaxIndex);
	void DisconnectAllMediaClient(); 
	PLAYER_TYPE GetPlayerType() const { return MKV_PLAYER; }

protected:
	virtual void OnClientError(tcp_socket* sock, long errcode);
	virtual bool ContainsClient(tcp_socket* sock);
	virtual bool OnNewClient(tcp_socket_ptr s);

protected:
	MkvMediaClientCollection m_ClientHandles;
};

#endif //MKV_NET_WRITER_H_
