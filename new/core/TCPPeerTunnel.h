#ifndef _LIVE_P2PCOMMON2_NEW_CORE_TCPPEER_TUNNEL_H_
#define _LIVE_P2PCOMMON2_NEW_CORE_TCPPEER_TUNNEL_H_

#include "PeerTunnel.h"
#include <set>

class TCPPeerTunnel : public PeerTunnel
{
public:
	TCPPeerTunnel(PeerConnection& connection, Downloader& downloader, int tunnelIndex);

	virtual bool IsRequesting() const;

	virtual bool IsRequesting(SubPieceUnit subPiece) const;

	virtual void OnReceiveSubPiece(SubMediaPiecePtr subPiece);

	//virtual void OnReceivePiece(PPMediaDataPacketPtr piecePtr);

	virtual void OnPieceNotFound(UINT pieceIndex);

	virtual void OnSubPieceNotFound(SubPieceUnit subPiece);

	virtual bool CheckRequestPieceTimeout();

	virtual DWORD GetSortValue();

	virtual DWORD GetUsedTime();

	virtual bool RequestFromTaskQueue();

	virtual bool RequestTillFullWindow();
	virtual bool RequestTillFullWindow2();

	virtual UINT GetRequestCount() const;
	virtual void AdjustWindowSize(UINT minSize) 
	{
		m_WindowSize = m_avrgRTT  / m_avrgUsedTime + bool(m_avrgRTT  % m_avrgUsedTime); // == rtt / usedtime + 0.5
		if (m_WindowSize < m_WindowSizeMin)
		{
			m_WindowSize = m_WindowSizeMin;
		}
		else if (m_WindowSize > m_WindowSizeMax)
		{
			m_WindowSize = m_WindowSizeMax;
		}
	}

	DWORD GetRealUsedTime();

protected:
	virtual bool RequestSubPiece(SubPieceUnit subPiece, DWORD externalTimeout);

protected:


private:
	typedef std::list<std::pair<SubPieceUnit, DWORD> > RequestingSubPieceSet;

	// TCP的一个Tunnel一次可以请求多个SubPiece
	RequestingSubPieceSet m_RequestSubPieces;		

	UINT	m_TcpWindowSizeCal;
	UINT	m_maxRequestTime;

	UINT	m_avrgUsedTime;
	UINT	m_avrgRTT;

	void	UpdateAvrgUsedTime(UINT inLastUsedTime)
	{
		if (inLastUsedTime > m_maxRequestTime)
		{
			m_maxRequestTime = inLastUsedTime;
		}

		if (m_avrgUsedTime == 0)
			m_avrgUsedTime = inLastUsedTime;
		else
			m_avrgUsedTime = m_avrgUsedTime * 0.8 + inLastUsedTime * 0.2;

		if(m_avrgUsedTime < 10) m_avrgUsedTime = 10;
	}

	void	UpdateRTT(UINT inRTT)
	{
		if (m_avrgRTT == 0)
			m_avrgRTT = inRTT;
		else
			m_avrgRTT = m_avrgRTT * 0.8 + inRTT * 0.2;

		if (m_avrgRTT > 20000)
		{
			m_avrgRTT = 20000;
		}
		else if (m_avrgRTT < 10)
		{
			m_avrgRTT = 10;
		}
	}
};


#endif