
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_BASE_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_BASE_H_


/**
* @file
* @brief 包含协议等相关的一些常量定义
*/


#include <ppl/data/int.h>


/// 协议版本
const UINT16 SYNACAST_VERSION = 0x0201;
const UINT16 SYNACAST_VERSION_3 = 0x0300;

const UINT16 SYNACAST_VERSION_EX_1	= 0x0301;


/// tracker请求报文使用的版本号
const UINT16 SYNACAST_VERSION_REQUEST = SYNACAST_VERSION_EX_1;


/// 报文魔法数
const UINT16 SYNACAST_MAGIC = 0xAB98;


/// 表示请求报文
const INT8 PT_ACTION_TYPE_REQUEST = 1;

/// 表示应答报文
const INT8 PT_ACTION_TYPE_RESPONSE = 0;


const UINT16 PPL_P2P_LIVE2 = 1001;
//const UINT16 PPL_P2P_LOGON = 1002;
//const UINT16 PPL_P2P_LIVE1 = 1003;



const UINT8 PPL_P2P_CONNECTION_ACTION_FLAG = 0x80;



#if defined(_WIN32_WCE)
const UINT8 PPL_IS_EMBEDDED_SYSTEM = 1;
#else
const UINT8 PPL_IS_EMBEDDED_SYSTEM = 0;
#endif

const UINT8 PPL_PROTOCOL_PLATFORM = PPL_IS_EMBEDDED_SYSTEM;


const INT8 PT_ERROR_NO_CHANNEL		= -1;
const INT8 PT_ERROR_NO_PEER			= -2;
const INT8 PT_ERROR_PEER_EXIST		= -3;
const INT8 PT_ERROR_AUTH_FAILED		= -4;
const INT8 PT_ERROR_SPECIAL_CHANNEL	= -5;
const INT8 PT_ERROR_ZONE_DENIED		= -6;
const INT8 PT_ERROR_LANGUAGE_DENIED		= -7;
const INT8 PT_ERROR_OBSOLETE_PEER_VERSION	= -8;


/// udp tracker类型
const BYTE PPL_TRACKER_TYPE_UDP = 0;
/// tcp tracker类型
const BYTE PPL_TRACKER_TYPE_TCP = 1;
/// http tracker类型
const BYTE PPL_TRACKER_TYPE_HTTP = 2;





/// peer-peer协议命令字
enum PEER_PACKET_TYPE
{
	PPT_ERROR				= 0x40,

	PPT_DETECT				= 0x49,	// routing
	PPT_REDETECT			= 0x4A,	// routing

	PPT_HANDSHAKE			= 0x50, 

	PPT_SUB_PIECE_REQUEST	= 0x52, 
	PPT_SUB_PIECE_DATA		= 0x53, 
	PPT_HUFFMAN_ANNOUNCE	= 0x55,

	PPT_PEER_EXCHANGE		= 0x70, 

	PPT_DATA_COLLECTING_REQUEST		= 0x31,
	PPT_DATA_COLLECTING_RESPONSE	= 0x32,

	PPT_DHT_REQUEST = 0x21, 
	PPT_DHT_RESPONSE = 0x22, 

};


/// 媒体报文类型
enum PEER_MEDIA_PACKET_TYPE
{
	PPDT_MEDIA_DATA		= 1,
	PPDT_MEDIA_HEADER	= 2,
	PPDT_MEDIA_PAYLOAD	= 3, 
	PPDT_MEDIA_END		= 4, 
};


enum PeerNetTypeEnum
{
	PNT_INVALID = 0, 
	PNT_INNER = 1, 
	PNT_OUTER = 2, 
	PNT_UPNP = 3, 
};

/// peer类型，分为普通peer和Source/SuperNode的peer
enum PeerTypeEnum
{
	NORMAL_PEER = 0, 
	SOURCE_PEER = 1, 
	MDS_PEER = 2, 
	MAS_PEER = 3, 
	PUBLISHER_PEER = 4,
};

enum CookieTypeEnum
{
	COOKIE_NONE		= 0, 
	COOKIE_TIME		= 1, 
};

#define CHECK_SUM_XOR   0x47
#define CHECK_SUM_XOR2  0x434D

#endif

