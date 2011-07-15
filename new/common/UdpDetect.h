/********************************************************************
Copyright(c) 2004-2005 PPLive.com All rights reserved.                                                 
	filename: 	UdpDetect.h
	created:	2005-4-21   19:40
	author:		
	purpose:	
*********************************************************************/

#ifndef _LIVE_P2PCOMMON2_NEW_COMMON_UdpDetect_h_
#define _LIVE_P2PCOMMON2_NEW_COMMON__UdpDetect_h_



#include "util/flow.h"

#include <synacast/protocol/DataCollecting.h>
#include <synacast/protocol/DataIO.h>
#include <ppl/util/ini_file.h>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>



class CIPPool;
class PeerNetInfo;
class PeerStatusInfo;
class UDPConnectionlessPacketSender;
class PeerInformation;
class PeerManager;
class CLiveInfo;
struct PACKET_PEER_INFO;
struct NEW_UDP_PACKET_HEAD;

struct PEER_ADDRESS;
class PeerItem;
class SimpleSocketAddress;
class PeerExchangePacket;



/// udp̽������ͳ������
class UDPDetectorStatistics : public DETECTOR_STATS
{
public:
	FlowMeasure DetectFlow;
	FlowMeasure PeerExchangeFlow;
	int IgnoredPeerExchangeTargets;

	UDPDetectorStatistics()
	{
		this->Clear();
		this->IgnoredPeerExchangeTargets = 0;
	}

	// ������ͬ���������ڴ�����ȥ
	void SyncFlow()
	{
		this->DetectFlow.SyncTo( this->Detect.Traffic );
		this->PeerExchangeFlow.SyncTo( this->PeerExchange.Traffic );
	}

	// �������ݲɼ�����ӵ�
	void UpdateFlow()
	{
		this->DetectFlow.Update();
		this->PeerExchangeFlow.Update();
	}

	UINT64 GetTotalDownloadBytes() const
	{
		return this->DetectFlow.Download.GetTotalBytes() + this->PeerExchangeFlow.Download.GetTotalBytes();
	}
	UINT64 GetTotalUploadBytes() const
	{
		return this->DetectFlow.Upload.GetTotalBytes() + this->PeerExchangeFlow.Upload.GetTotalBytes();
	}
	UINT64 GetTotalDownloadPackets() const
	{
		return this->DetectFlow.Download.GetTotalPackets() + this->PeerExchangeFlow.Download.GetTotalPackets();
	}
	UINT64 GetTotalUploadPackets() const
	{
		return this->DetectFlow.Upload.GetTotalPackets() + this->PeerExchangeFlow.Upload.GetTotalPackets();
	}
};



/*
/// udp̽������ͳ������
class UDPDetectorStatistics
{
public:
	/// �ܹ��յ�hello���ĵĸ���
	int HelloPackets;

	/// �ܹ��յ�echo���ĵĸ���
	int EchoPackets;

	/// �ܹ�̽��Ĵ���
	int DetectTimes;

	int OnceHelloCount;
	int RealOnceHelloCount;
	int RealOnceDetectCount;

	int TotalPeerExchangeRequestTimes;

	UDPDetectorStatistics()
	{
		FILL_ZERO(*this);
	}
};
*/


class UDPDetectorConfig
{
public:
	bool DisablePeerExchange;

	UDPDetectorConfig()
	{
		this->DisablePeerExchange = false;
	};

	void Load( ini_file& ini )
	{
		this->DisablePeerExchange = ini.get_bool( _T("DisablePeerExchange"), this->DisablePeerExchange );
	}
};



/// UDP�鷺̽����
class CUdpDetect : private boost::noncopyable
{
public:
	explicit CUdpDetect(CIPPool& ipPool, PeerManager& peerManager, boost::shared_ptr<PeerInformation> peerInformation, boost::shared_ptr<UDPConnectionlessPacketSender> sender, const CLiveInfo& liveInfo);
	virtual ~CUdpDetect();

	/**
	 * @brief ����̽����
	 * @param doDetect �Ƿ���Ҫ��������̽��
	 */
	void Start(bool doDetect = false);

	/// ����ʱ��
	void OnAppTimer(UINT times);

	/// ��ȡͳ������
	const UDPDetectorStatistics& GetStatistics() const { return m_Statistics; }

	/// ����udp����
	bool HandleUDPPacket( data_input_stream& is, const NEW_UDP_PACKET_HEAD& head, const PACKET_PEER_INFO& packetPeerInfo, const SimpleSocketAddress& sockAddr );

	/// �������þܾ�Detect���� ��Ŀǰû��ʹ�ã�
	void SetRefuseDetect( bool refuse );

	/// ����Ƿ�������Detect����
	bool GetRefuseDetect();

protected:
	/// ����һ��̽��
	void DoDetect();

	/// ��ָ���ĵ�ַ����̽�⣬����ֵΪbool�ͣ���ʾ�Ƿ�ɹ�����̽�ⱨ��
	//bool DoHelloPeer(const PeerItem& peer);

	bool ExchangePeers( const SimpleSocketAddress& sockAddr, bool isResponse, UINT32 sendOffTime, UINT32 transactionID );

	/// ��ָ���ĵ�ַ����̽�⣬����ֵΪbool�ͣ���ʾ�Ƿ�ɹ�����̽�ⱨ��
	bool DoDetectPeer(const PeerItem& peer);

	/// ���� PeerExchange ����
	void HandlePeerExchange( data_input_stream& is, const NEW_UDP_PACKET_HEAD& head, const PACKET_PEER_INFO& packetPeerInfo, const SimpleSocketAddress& sockAddr );

	/// ����detect����
	void HandleDetect( data_input_stream& is, const NEW_UDP_PACKET_HEAD& head, const PACKET_PEER_INFO& packetPeerInfo, const SimpleSocketAddress& sockAddr );
	/// ����redetect����
	void HandleRedetect( data_input_stream& is, const NEW_UDP_PACKET_HEAD& head, const PACKET_PEER_INFO& packetPeerInfo, const SimpleSocketAddress& sockAddr );


private:
	PeerManager& m_PeerManager;
        const CLiveInfo& m_LiveInfo;	
        CIPPool& m_ipPool;
        boost::shared_ptr<const PeerNetInfo> m_NetInfo;
        boost::shared_ptr<const PeerStatusInfo> m_StatusInfo;	
        boost::shared_ptr<UDPConnectionlessPacketSender> m_PacketSender;
	boost::shared_ptr<PeerInformation> m_PeerInformation;
	UDPDetectorStatistics m_Statistics;
	bool m_needDetect;
	bool m_RefuseDetect;

	/// ͳ������

	UDPDetectorConfig m_Config;

	/// �������͵�peer-exchange���ģ������Ż����ܣ�����ÿ�η��Ͷ�Ҫ��ȡpeer�б�
	boost::shared_ptr<PeerExchangePacket> m_SentPeerExchangePacket;
	time_counter m_ExchangedPeerTime;


};

#endif
