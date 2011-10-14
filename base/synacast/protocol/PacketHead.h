
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_PACKET_HEAD_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_PACKET_HEAD_H_

#include <synacast/protocol/data/NetInfo.h>
#include <synacast/protocol/data/PeerAddress.h>
#include <synacast/protocol/data/DegreeInfo.h>
#include <synacast/protocol/base.h>


/// 老的udp报文头，用于以前的非安全的peer-tracker协议
struct OLD_UDP_PACKET_HEAD
{
	/// 应用类型
	UINT16 AppType;

	/// 命令字
	UINT8 Action;

	/// 命令字类型：1是请求，0表示成功的回应，其它为失败的回应
	INT8 ActionType;

	/// 魔法数
	UINT16 Magic;

	/// 版本号
	UINT16 ProtocolVersion;

	/// 一次Request-Resopnse过程的ID
	UINT32 TransactionID;

	enum { object_size = 12 };
};



/// tcp报文公共头
//typedef PACKET_COMMON_HEAD TCP_COMMON_HEAD;


/// udp报文的公共头
struct TCP_PACKET_HEAD
{
	/// 命令字
	UINT8 Action;

	/// 命令字类型：1是请求，0表示成功的回应，其它为失败的回应
	INT8 ReservedActionType;

	/// 版本号
	UINT16 ProtocolVersion;

	enum { object_size = 4 };

	void CheckValid( UINT16 protocolVer ) const
	{
		LIVE_ASSERT( 0 == this->ReservedActionType );
		LIVE_ASSERT( protocolVer == this->ProtocolVersion );
		LIVE_ASSERT( this->Action > 0 );
	}
};


// 新的报文定义


/// udp报文的公共头
struct NEW_UDP_PACKET_HEAD
{
	/// 命令字
	UINT8 Action;

	/// 保留
	INT8 ReservedActionType;

	/// 版本号
	UINT16 ProtocolVersion;

	/// 一次Request-Resopnse过程的ID
	UINT32 TransactionID;

	enum { object_size = 8 };

	void CheckValid( UINT16 protocolVer ) const
	{
		LIVE_ASSERT( 0 == this->ReservedActionType );
		LIVE_ASSERT( protocolVer == this->ProtocolVersion );
		LIVE_ASSERT( this->Action > 0 );
		LIVE_ASSERT( this->TransactionID != 0 );
	}
};


struct UDP_SESSION_INFO
{
	UINT32 SequenceID;
	UINT32 SessionKey;

	enum { object_size = 4 * 2 };
};




struct PACKET_PEER_INFO
{
	GUID ChannelGUID;
	GUID PeerGUID;

	PEER_ADDRESS Address;
	PEER_ADDRESS OuterAddress;
	PEER_ADDRESS DetectedRemoteAddress;

	DEGREE_INFO Degrees;

	UINT32 AppVersion;
	UINT8 IsEmbedded;
	PEER_CORE_INFO CoreInfo;

	enum { object_size = sizeof(GUID) * 2 + PEER_ADDRESS::object_size * 3 + DEGREE_INFO::object_size + 4 + 1 + PEER_CORE_INFO::object_size };

	void Clear()
	{
		FILL_ZERO( ChannelGUID );
		FILL_ZERO( PeerGUID );
		Address.Clear();
		OuterAddress.Clear();
		DetectedRemoteAddress.Clear();
		Degrees.Clear();
		AppVersion = 0;
		IsEmbedded = 0;
		CoreInfo.Clear();
	}

	void Init( const GUID& channelGUID, const GUID& peerGUID, const PEER_NET_INFO& netInfo, UINT32 appVersion )
	{
		this->ChannelGUID = channelGUID;
		this->PeerGUID = peerGUID;
		this->Address = netInfo.Address;
		//this->DetectedUDPAddress.IP = netInfo.Address.IP;
		//this->DetectedUDPAddress.Port = netInfo.Address.UdpPort;
		this->AppVersion = appVersion;
		this->CoreInfo = netInfo.CoreInfo;
		this->IsEmbedded = PPL_IS_EMBEDDED_SYSTEM;
	}

	void SetDetectedAddress( const PEER_ADDRESS& detectedAddr )
	{
		this->DetectedRemoteAddress = detectedAddr;
	}

	void Update( const PEER_ADDRESS& addr, const PEER_ADDRESS& outerAddr, const DEGREE_INFO& degrees )
	{
		this->Address = addr;
		this->OuterAddress = outerAddr;
		this->Degrees = degrees;
	}
	void UpdateCoreInfo( const PEER_CORE_INFO& coreInfo )
	{
		this->CoreInfo = coreInfo;
	}
};





/*

/// peer类型，分为普通peer和Source/SuperNode的peer
enum PeerTypeEnum
{
	NORMAL_PEER = 0, 
	SOURCE_PEER = 1, 
	MDS_PEER = 2, 
	MAS_PEER = 3, 
	PUBLISHER_PEER = 4,
};

*/




inline bool IsNormalPeer(const PEER_CORE_INFO& coreInfo)
{
	return coreInfo.PeerType == NORMAL_PEER;
}



#endif
