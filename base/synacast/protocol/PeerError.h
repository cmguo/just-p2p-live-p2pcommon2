
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_PEER_ERROR_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_PEER_ERROR_H_


/// 错误码：peer已离开
//const UINT16 PP_ERROR_LEAVE = 1;

const UINT16 PP_ERROR_CLOSE_FLAG = 0x8000;

/// 错误码：所请求的piece没有找到
const UINT16 PP_ERROR_NO_PIECE = 0x0015;

/// 错误码：所连接的peer没有这个频道(主要有kom.dll发出)
const UINT16 PP_ERROR_NO_CHANNEL = 0x0003;


/// 错误码：所请求的sub-piece没有找到
//const UINT16 PP_ERROR_NO_SUB_PIECE = 0x0015;



/// leave原因：自己发给自己的报文
const UINT16 PP_LEAVE_SELF_TO_SELF = 0x8001;

/// 错误码：握手时频道GUID错误
const UINT16 PP_LEAVE_BAD_CHANNEL = 0x8002;

/// 错误码：版本不支持
const UINT16 PP_LEAVE_VERSION_NOT_SUPPORT = 0x8003;

/// 错误码：重复连接数
const UINT16 PP_LEAVE_DUPLICATE_CONNECTION = 0x8004;

/// 错误码：拒绝tcp或udp的连接
const UINT16 PP_LEAVE_REFUSE = 0x8005;


/// 错误码：版本不支持
const UINT16 PP_LEAVE_NO_CONNECTION = 0x8011;

/// 错误码；idle时间过长
const UINT16 PP_LEAVE_LONG_IDLE = 0x8012;

/// 错误码：连接被踢掉
const UINT16 PP_LEAVE_KICKED = 0x8013;

/// 错误码：程序退出
const UINT16 PP_LEAVE_QUIT = 0x8014;



/// 错误码：网络错误
const UINT16 PP_LEAVE_NETWORK_ERROR = 0x8051;

/// 错误码：报文数据错误
const UINT16 PP_LEAVE_INVALID_PACKET = 0x8052;

/// 错误码：握手超时
const UINT16 PP_LEAVE_HANDSHAKE_TIMEOUT = 0x8053;

/// 错误码：报文数据错误
const UINT16 PP_LEAVE_HANDSHAKE_ERROR = 0x8054;

/// 错误码：忽略非p2p报文，如数据收集的
const UINT16 PP_LEAVE_IGNORE_NON_P2P = 0x8055;



/*
/// 错误码：连接数不足
const UINT16 PP_LEAVE_NO_DEGREE = -6;


/// 错误码：带宽太差
const UINT16 PP_LEAVE_BAD_BANDWIDTH = -8;

/// 错误码：断开到mds的连接
const UINT16 PP_LEAVE_HANGUP_MDS = -11;

/// 错误码：连接被踢掉，且短期内不允许再连接
const UINT16 PP_LEAVE_SOURCE_KICKED = -12;

/// 错误码：连接被source拒绝，因为是内网peer
const UINT16 PP_LEAVE_INTERNAL_PEER_REJECTED = -13;

/// 错误码：发出了Request 但是很久都没有受到Data, 但是不停地受到Announce
const UINT16 PP_LEAVE_REQUEST_NOT_RECV_DATA = -14;

/// 错误码：UPT连接没有机会了.
const UINT16 PP_LEAVE_NO_OPPORTUNITY = -15;

/// 错误码：根据peer类型被拒绝
const UINT16 PP_LEAVE_REFUSE_PEER_TYPE = -18;

/// 错误码：连接被source断掉，因为是内网peer
const UINT16 PP_LEAVE_SOURCE_KICKED_INTERNAL_PEER = -20;

/// 错误码：连接被source断掉，因为长时间没有数据通讯
const UINT16 PP_LEAVE_SOURCE_KICKED_IDLE_PEER = -21;

/// 错误码：连接被source断掉，因为资源不重叠
const UINT16 PP_LEAVE_SOURCE_KICKED_NON_OVERLAPPED_PEER = -22;

/// 错误码：连接被source断掉，因为下载速度过低
const UINT16 PP_LEAVE_SOURCE_KICKED_TOOL_SLOW_PEER = -23;

/// 错误码：连接被source断掉，因为qos过低
const UINT16 PP_LEAVE_SOURCE_KICKED_LOW_QOS_PEER = -24;


/// 错误码：udpt连接收到finish报文
const UINT16 PP_LEAVE_UDPT_FINISHED = -30;

/// 错误码：发送报文失败
const UINT16 PP_LEAVE_SEND_DATA_FAILED = -51;
*/




const UINT32 PP_REFUSE_TCP = 1;
const UINT32 PP_REFUSE_UDP = 2;
const UINT32 PP_REFUSE_NO_DEGREE = 3;
const UINT32 PP_REFUSE_INNER_PEER = 4;
const UINT32 PP_REFUSE_PEER = 5;


const UINT32 PP_KICK_IDLE = 1;
const UINT32 PP_KICK_DOWNLOAD_SLOW = 2;
const UINT32 PP_KICK_LOW_QOS = 3;
const UINT32 PP_KICK_NOT_OVERLAPPED = 4;
const UINT32 PP_KICK_LAN = 5;
const UINT32 PP_KICK_DENY = 6;





/*

/// leave原因：报文数据错误
const long PP_LEAVE_INVALID_PACKET = -1;

/// leave原因：握手时频道GUID错误
const long PP_LEAVE_BAD_CHANNEL = -2;

/// leave原因：自己发给自己的报文
const long PP_LEAVE_SELF_TO_SELF = -3;

/// leave原因；idle时间过长
const long PP_LEAVE_LONG_IDLE = -4;

/// leave原因：握手超时
const long PP_LEAVE_HANDSHAKE_TIMEOUT = -5;

/// leave原因：连接数不足
const long PP_LEAVE_NO_DEGREE = -6;

/// leave原因：重复连接数
const long PP_LEAVE_DUPLICATE_CONNECTION = -7;

/// leave原因：带宽太差
const long PP_LEAVE_BAD_BANDWIDTH = -8;

/// leave原因：程序退出
const long PP_LEAVE_QUIT = -9;

/// leave原因：连接被踢掉
const long PP_LEAVE_KICKED = -10;

/// leave原因：断开到mds的连接
const long PP_LEAVE_HANGUP_MDS = -11;

/// leave原因：连接被踢掉，且短期内不允许再连接
const long PP_LEAVE_SOURCE_KICKED = -12;

/// leave原因：连接被source拒绝，因为是内网peer
const long PP_LEAVE_INTERNAL_PEER_REJECTED = -13;

/// leave原因：发出了Request 但是很久都没有受到Data, 但是不停地受到Announce
const long PP_LEAVE_REQUEST_NOT_RECV_DATA = -14;

/// leave原因：UPT连接没有机会了.
const long PP_LEAVE_NO_OPPORTUNITY = -15;

/// leave原因：网络错误
const long PP_LEAVE_NETWORK_ERROR = -16;

/// leave原因：拒绝tcp或udp的连接
const long PP_LEAVE_REFUSE = -17;


/// leave原因：根据peer类型被拒绝
const long PP_LEAVE_REFUSE_PEER_TYPE = -18;

/// leave原因：连接被source断掉，因为是内网peer
const long PP_LEAVE_SOURCE_KICKED_INTERNAL_PEER = -20;

/// leave原因：连接被source断掉，因为长时间没有数据通讯
const long PP_LEAVE_SOURCE_KICKED_IDLE_PEER = -21;

/// leave原因：连接被source断掉，因为资源不重叠
const long PP_LEAVE_SOURCE_KICKED_NON_OVERLAPPED_PEER = -22;

/// leave原因：连接被source断掉，因为下载速度过低
const long PP_LEAVE_SOURCE_KICKED_TOOL_SLOW_PEER = -23;

/// leave原因：连接被source断掉，因为qos过低
const long PP_LEAVE_SOURCE_KICKED_LOW_QOS_PEER = -24;




/// leave原因：udpt连接收到finish报文
const long PP_LEAVE_UDPT_FINISHED = -30;

/// leave原因：发送报文失败
const long PP_LEAVE_SEND_DATA_FAILED = -51;


*/


#endif
