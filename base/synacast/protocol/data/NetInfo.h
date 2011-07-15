
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_NET_INFO_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_NET_INFO_H_

#include <synacast/protocol/data/PeerAddress.h>


/// peer������Ϣ
struct PEER_CORE_INFO
{
	UINT8 PeerType : 4;
	/// �������ͣ�����1������2��UPNP3
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

	/// ���
	void Clear()
	{
		FILL_ZERO(*this);
	}

};



/// peer������Ϣ
struct PEER_NET_INFO
{
	/// peer��������
	PEER_CORE_INFO CoreInfo;

	/// peer��ַ
	PEER_ADDRESS Address;

};


#endif

