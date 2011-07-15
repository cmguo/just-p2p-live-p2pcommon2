
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_CANDIDATE_PEER_INFO_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_CANDIDATE_PEER_INFO_H_

#include <ppl/data/int.h>
#include <ppl/util/macro.h>
#include <synacast/protocol/data/PeerAddress.h>
#include <synacast/protocol/data/NetInfo.h>
#include <synacast/protocol/DataIO.h>
#include <iosfwd>



/// peer地址信息
struct CANDIDATE_PEER_INFO
{
	/// peer地址
	PEER_ADDRESS Address;

	/// peer网络类型
	//	NET_TYPE NetworkType;

	PEER_CORE_INFO CoreInfo;

	enum { object_size = sizeof(PEER_ADDRESS) + sizeof(PEER_CORE_INFO) };

};


/// peer-exchange报文中Peer信息项
class PeerExchangeItem
{
public:
	/// peer地址
	PEER_ADDRESS Address;
	/// peer类型，lsb 4bit
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

