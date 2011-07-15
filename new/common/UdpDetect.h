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



/// udp探测器的统计数据
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

	// 将数据同步到共享内存里面去
	void SyncFlow()
	{
		this->DetectFlow.SyncTo( this->Detect.Traffic );
		this->PeerExchangeFlow.SyncTo( this->PeerExchange.Traffic );
	}

	// 后来数据采集里面加的
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
/// udp探测器的统计数据
class UDPDetectorStatistics
{
public:
	/// 总共收到hello报文的个数
	int HelloPackets;

	/// 总共收到echo报文的个数
	int EchoPackets;

	/// 总共探测的次数
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



/// UDP洪泛探测器
class CUdpDetect : private boost::noncopyable
{
public:
	explicit CUdpDetect(CIPPool& ipPool, PeerManager& peerManager, boost::shared_ptr<PeerInformation> peerInformation, boost::shared_ptr<UDPConnectionlessPacketSender> sender, const CLiveInfo& liveInfo);
	virtual ~CUdpDetect();

	/**
	 * @brief 启动探测器
	 * @param doDetect 是否需要主动发起探测
	 */
	void Start(bool doDetect = false);

	/// 主定时器
	void OnAppTimer(UINT times);

	/// 获取统计数据
	const UDPDetectorStatistics& GetStatistics() const { return m_Statistics; }

	/// 处理udp报文
	bool HandleUDPPacket( data_input_stream& is, const NEW_UDP_PACKET_HEAD& head, const PACKET_PEER_INFO& packetPeerInfo, const SimpleSocketAddress& sockAddr );

	/// 可以设置拒绝Detect请求 （目前没有使用）
	void SetRefuseDetect( bool refuse );

	/// 获得是否设置了Detect请求
	bool GetRefuseDetect();

protected:
	/// 进行一次探测
	void DoDetect();

	/// 向指定的地址发起探测，返回值为bool型，表示是否成功发送探测报文
	//bool DoHelloPeer(const PeerItem& peer);

	bool ExchangePeers( const SimpleSocketAddress& sockAddr, bool isResponse, UINT32 sendOffTime, UINT32 transactionID );

	/// 向指定的地址发起探测，返回值为bool型，表示是否成功发送探测报文
	bool DoDetectPeer(const PeerItem& peer);

	/// 处理 PeerExchange 报文
	void HandlePeerExchange( data_input_stream& is, const NEW_UDP_PACKET_HEAD& head, const PACKET_PEER_INFO& packetPeerInfo, const SimpleSocketAddress& sockAddr );

	/// 处理detect报文
	void HandleDetect( data_input_stream& is, const NEW_UDP_PACKET_HEAD& head, const PACKET_PEER_INFO& packetPeerInfo, const SimpleSocketAddress& sockAddr );
	/// 处理redetect报文
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

	/// 统计数据

	UDPDetectorConfig m_Config;

	/// 将被发送的peer-exchange报文，用于优化性能，避免每次发送都要获取peer列表
	boost::shared_ptr<PeerExchangePacket> m_SentPeerExchangePacket;
	time_counter m_ExchangedPeerTime;


};

#endif
