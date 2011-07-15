
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_CANDIDATE_PEER_INFO_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_CANDIDATE_PEER_INFO_H_

#include <ppl/data/int.h>
#include <ppl/util/macro.h>
#include <synacast/protocol/data/PeerAddress.h>
#include <synacast/protocol/data/NetInfo.h>
#include <synacast/protocol/DataIO.h>
#include <iosfwd>



/// peer��ַ��Ϣ
struct CANDIDATE_PEER_INFO
{
	/// peer��ַ
	PEER_ADDRESS Address;

	/// peer��������
	//	NET_TYPE NetworkType;

	PEER_CORE_INFO CoreInfo;

	enum { object_size = sizeof(PEER_ADDRESS) + sizeof(PEER_CORE_INFO) };

};


/// peer-exchange������Peer��Ϣ��
class PeerExchangeItem
{
public:
	/// peer��ַ
	PEER_ADDRESS Address;
	/// peer���ͣ�lsb 4bit
	UINT8 PeerType;

	PeerExchangeItem()
	{
		Address.Clear();
		PeerType = 0;
	}

	enum { object_size = sizeof(PEER_ADDRESS) + sizeof(UINT8) };
};





struct INNER_CANDIDATE_PEER_INFO
{
	PEER_ADDRESS	DetectedAddress;
	PEER_ADDRESS    StunServerAddress;
	PEER_CORE_INFO  CoreInfo;
	UINT8			PublicHostCount;
	//PEER_ADDRESS	PublicHosts[PPL_MAX_PUBLIC_HOST_COUNT];

	enum { object_size = sizeof(PEER_ADDRESS) * 2 + sizeof(PEER_CORE_INFO) + 1 };

};



#endif

