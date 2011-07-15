#ifndef _LIVE_P2PCOMMON2_NEW_CORE_UDPPEER_TUNNEL_H_
#define _LIVE_P2PCOMMON2_NEW_CORE_UDPPEER_TUNNEL_H_

#include "PeerTunnel.h"
#include <boost/array.hpp>

#include <map>

#define _LIGHT_TIMEOUT

#ifdef _LIGHT_TIMEOUT
#include <list>
#endif

#define MAXARRAYSIZE 100

class UDPPeerTunnel : public PeerTunnel
{
public:
	UDPPeerTunnel(PeerConnection& connection, Downloader& downloader, int tunnelIndex);
	virtual ~UDPPeerTunnel();
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
	void ClearTaskQueue();


	void UpdataWindowSize(UINT usedTime, bool requestSucced);
	void AdjustWindowSize(UINT minSize);
	DWORD GetRealUsedTime() { return m_avrgUsedTime;}

	virtual bool TryToDownload(SubPieceUnit subPiece, DWORD externalTimeout);

protected:
	virtual bool RequestSubPiece(SubPieceUnit subPiece, DWORD externalTimeout);
	virtual bool RequestSubPiece2(SubPieceUnit subPiece, DWORD externalTimeout); // Added by Tady,080808: For multi-subpieces-request.
	void		 RequestMRP();
	void		 CheckTimeoutMRP();

protected:
	DWORD             m_SumDeltaUsedTime;
	DWORD             m_SumRequestSuccedCount;


private:

#ifndef _LIGHT_TIMEOUT
	typedef std::map<SubPieceUnit, std::pair<DWORD, DWORD> > RequestingSubPieceSet;
#else
	typedef std::list<std::pair<SubPieceUnit, std::pair<DWORD, DWORD> > > RequestingSubPieceSet;
	typedef std::map<SubPieceUnit, UINT> RequestedSubpieceSet;
	RequestedSubpieceSet m_Requested;
	int m_orderlessCount;
	UINT m_orderlessRate;
#endif

	// TCP��һ��Tunnelһ�ο���������SubPiece
	RequestingSubPieceSet m_RequestSubPieces;		
	
	// ����Piece��ʱ������
	boost::array<UINT, MAXARRAYSIZE> m_usedTimeArray;
	boost::array<bool, MAXARRAYSIZE> m_requestSuccedArray;

	size_t m_UseArraySize;

	// ǰ20��SubPiece����ɹ���
	double m_nowRequestSuccedRate;

	UINT m_avrgUsedTime; // ���ݵ�����ʱ���ƽ���������������ٶȡ�
	UINT m_avrgRTT;		 // һ������������ƽ��ʱ�䣬��������ʱ�ӡ�
	void UpdateRTT(UINT inRTT);

	// ���� usedTime �� requestSucced �Ĵ���
	UINT m_calcUsedTimeCount;

	UINT m_udpIntTimeOutMin;
	UINT m_udpIntTimeOutMax;
	UINT m_maxRequestTime;
	
	//////////////////////////////////////////////////////////////////////////
	// Added for Debug
	int m_debugID;
	
	time_counter m_lastTaskTimer;

};


#endif