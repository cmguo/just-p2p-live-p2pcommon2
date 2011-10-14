
#ifndef _LIVE_P2PCOMMON2_NEW_COMMON_IPINFO_H_
#define _LIVE_P2PCOMMON2_NEW_COMMON_IPINFO_H_

#include "framework/memory.h"

#include <synacast/protocol/data/PeerAddress.h>
#include <synacast/protocol/data/CandidatePeerInfo.h>
#include <boost/noncopyable.hpp>


/// ippool��Peer��ַ�Ķ˿��������
enum IPPOOL_CONNECT_FLAGS
{
	/// δָ����
	IPPOOL_CONNECT_NONE = 0x0000, 

	/// �����˿ڿ���������
	IPPOOL_CONNECT_NORMAL = 0x0001, 

	/// ����������80�˿�
	IPPOOL_CONNECT_80 = 0x0002, 

	/// ����ʹ��udpt��ʽ����
	IPPOOL_CONNECT_UDPT = 0x0003, 

	/// ����
	IPPOOL_CONNECT_BLOCKED = 0x0004, 
};


enum CandidatePeerTypeEnum
{
	CANDIDATE_FROM_TRACKER = 1, 
	CANDIDATE_FROM_UDP_ECHO = 2, 
	CANDIDATE_FROM_TCP_ECHO = 3, 
	CANDIDATE_FROM_UDP_HELLO = 4, 
	CANDIDATE_FROM_REDIRECT = 5, 
};




class PeerItem : public pool_object
{
public:
	/// ������Peer����������Ϣ����ַ����������
	CANDIDATE_PEER_INFO Info;

	/// �������Է���RTT(ms)
	UINT RTT;

	PEER_ADDRESS StunServerAddress;

	/// ���ӵı�־
	WORD ConnectFlags;

	/// �Ƿ���̽��
	bool CanDetect;

	/// ���Ӵ���
	WORD ConnectTimes;

	void Init()
	{
		FILL_ZERO(*this);
		this->RTT = UINT_MAX;
		this->ConnectFlags = IPPOOL_CONNECT_NONE;
		this->CanDetect = false;
	}
};


/// peer��ַ��Ϣ��
class CIPInfo : public PeerItem
{
public:
	/// ��ʼ��¼��ʱ��
	UINT StartTime;

	/// ���һ��̽���ʱ��
	UINT LastDetectTime;

	/// ���һ������ʱ��
	UINT LastConnectTime;

	/// ���һ������ʧ�ܵ�ʱ��
	UINT LastConnectFailedTime;

	/// ���һ�����ӶϿ�ʱ��
	UINT LastDisconnectTime;

	/// �ϴε����ӳ���ʱ��
//	UINT LastConnectionTime;

	/// ̽�����
	WORD DetectTimes;

	/// ̽��ɹ�����
	WORD DetectSucceededTimes;

	/// ���ӳɹ�����
	WORD ConnectSucceededTimes;

	/// ����ʧ�ܴ���
	WORD ConnectFailedTimes;
	WORD UDPConnectFailedTimes;
	WORD TCPConnectFailedTimes;

	/// �Ͽ����ӵĴ���
	WORD DisconnectTimes;

	/// ״̬
//	enum IPPoolState state;



	explicit CIPInfo(const PEER_ADDRESS& addr)
	{
		Clear();
		this->Info.Address = addr;
	}
	CIPInfo(const CANDIDATE_PEER_INFO& info)
	{
		Clear();
		this->Info = info;
	}

	/// ��ȡpeer��ַ
	const PEER_ADDRESS& GetAddress() const { return this->Info.Address; }

	/// ��ȡ̽��ʧ�ܴ���
	UINT GetDetectFailedTimes() const
	{
		if (DetectTimes >= DetectSucceededTimes)
			return DetectTimes - DetectSucceededTimes;
//		LIVE_ASSERT(false);
		return 0;
	}

	/// ��ȡ����ʧ�ܴ���
	UINT GetConnectFailedTimes() const
	{
		if (ConnectTimes >= ConnectSucceededTimes)
			return ConnectTimes - ConnectSucceededTimes;
	//	LIVE_ASSERT(false);
		return 0;
	}

	/// �Ƿ�û��̽��ɹ���
	bool IsUndetectable() const
	{
		return DetectTimes > 0 && DetectSucceededTimes == 0;
	}

	/// �Ƿ�û�����ӳɹ���
	bool IsUnconnectable() const
	{
		return ConnectTimes > 0 && ConnectSucceededTimes == 0;
	}

	/// ת��Ϊ�ַ���
	string ToString() const
	{
		char str[1025] = { 0 };
#if defined(_PPL_PLATFORM_MSWIN)
        _snprintf(str, 1024, "%s %u %u %u %u %u %u %hu %hu %hu %hu %hu %hu %hu", 
            this->GetAddress().ToString().c_str(), 
            this->RTT, 
            this->LastDetectTime, 
            this->LastConnectTime, 
            this->LastConnectFailedTime, 
            this->LastConnectFailedTime, 
            this->LastDisconnectTime, 
            this->DetectTimes, 
            this->DetectSucceededTimes, 
            this->ConnectTimes, 
            this->ConnectSucceededTimes, 
            this->ConnectFailedTimes, 
            this->DisconnectTimes, 
            this->ConnectFlags);
        return str;
#else
        snprintf(str, 1024, "%s %u %u %u %u %u %u %hu %hu %hu %hu %hu %hu %hu", 
            this->GetAddress().ToString().c_str(), 
            this->RTT, 
            this->LastDetectTime, 
            this->LastConnectTime, 
            this->LastConnectFailedTime, 
            this->LastConnectFailedTime, 
            this->LastDisconnectTime, 
            this->DetectTimes, 
            this->DetectSucceededTimes, 
            this->ConnectTimes, 
            this->ConnectSucceededTimes, 
            this->ConnectFailedTimes, 
            this->DisconnectTimes, 
            this->ConnectFlags);
#endif
		
		return str;
	}

private:
	///�����Ϣ
	void Clear()
	{
		memset(this, 0, sizeof(CIPInfo));
		ConnectFlags = IPPOOL_CONNECT_NONE;
		this->CanDetect = false;
	}
};

#endif
