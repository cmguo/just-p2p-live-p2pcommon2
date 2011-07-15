
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_NET_INFO_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_NET_INFO_H_

#include <synacast/protocol/data/PeerAddress.h>


/// peer基本信息
struct PEER_CORE_INFO
{
	UINT8 PeerType : 4;
	/// 网络类型：内网1、公网2、UPNP3
	UINT8 PeerNetType : 2;
	UINT8 Reserved : 2; // MSB

	UINT8 NATType : 4;
	UINT8 Reserved2 : 4;
	UINT8 Reserved1[6];

	enum { object_size = 8 };

	PEER_CORE_INFO()
		: PeerType(0)
		, PeerNetType(0)
		, Reserved(0)
		, NATType(0)
		, Reserved2(0)
	{
		memset(Reserved1, 0, sizeof(Reserved1));
	}

	/// 清空
	void Clear()
	{
		FILL_ZERO(*this);
	}

};



/// peer网络信息
struct PEER_NET_INFO
{
	/// peer网络类型
	PEER_CORE_INFO CoreInfo;

	/// peer地址
	PEER_ADDRESS Address;

};


#endif

