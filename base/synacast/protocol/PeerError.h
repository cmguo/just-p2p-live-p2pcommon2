
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_PEER_ERROR_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_PEER_ERROR_H_


/// �����룺peer���뿪
//const UINT16 PP_ERROR_LEAVE = 1;

const UINT16 PP_ERROR_CLOSE_FLAG = 0x8000;

/// �����룺�������pieceû���ҵ�
const UINT16 PP_ERROR_NO_PIECE = 0x0015;

/// �����룺�����ӵ�peerû�����Ƶ��(��Ҫ��kom.dll����)
const UINT16 PP_ERROR_NO_CHANNEL = 0x0003;


/// �����룺�������sub-pieceû���ҵ�
//const UINT16 PP_ERROR_NO_SUB_PIECE = 0x0015;



/// leaveԭ���Լ������Լ��ı���
const UINT16 PP_LEAVE_SELF_TO_SELF = 0x8001;

/// �����룺����ʱƵ��GUID����
const UINT16 PP_LEAVE_BAD_CHANNEL = 0x8002;

/// �����룺�汾��֧��
const UINT16 PP_LEAVE_VERSION_NOT_SUPPORT = 0x8003;

/// �����룺�ظ�������
const UINT16 PP_LEAVE_DUPLICATE_CONNECTION = 0x8004;

/// �����룺�ܾ�tcp��udp������
const UINT16 PP_LEAVE_REFUSE = 0x8005;


/// �����룺�汾��֧��
const UINT16 PP_LEAVE_NO_CONNECTION = 0x8011;

/// �����룻idleʱ�����
const UINT16 PP_LEAVE_LONG_IDLE = 0x8012;

/// �����룺���ӱ��ߵ�
const UINT16 PP_LEAVE_KICKED = 0x8013;

/// �����룺�����˳�
const UINT16 PP_LEAVE_QUIT = 0x8014;



/// �����룺�������
const UINT16 PP_LEAVE_NETWORK_ERROR = 0x8051;

/// �����룺�������ݴ���
const UINT16 PP_LEAVE_INVALID_PACKET = 0x8052;

/// �����룺���ֳ�ʱ
const UINT16 PP_LEAVE_HANDSHAKE_TIMEOUT = 0x8053;

/// �����룺�������ݴ���
const UINT16 PP_LEAVE_HANDSHAKE_ERROR = 0x8054;

/// �����룺���Է�p2p���ģ��������ռ���
const UINT16 PP_LEAVE_IGNORE_NON_P2P = 0x8055;



/*
/// �����룺����������
const UINT16 PP_LEAVE_NO_DEGREE = -6;


/// �����룺����̫��
const UINT16 PP_LEAVE_BAD_BANDWIDTH = -8;

/// �����룺�Ͽ���mds������
const UINT16 PP_LEAVE_HANGUP_MDS = -11;

/// �����룺���ӱ��ߵ����Ҷ����ڲ�����������
const UINT16 PP_LEAVE_SOURCE_KICKED = -12;

/// �����룺���ӱ�source�ܾ�����Ϊ������peer
const UINT16 PP_LEAVE_INTERNAL_PEER_REJECTED = -13;

/// �����룺������Request ���Ǻܾö�û���ܵ�Data, ���ǲ�ͣ���ܵ�Announce
const UINT16 PP_LEAVE_REQUEST_NOT_RECV_DATA = -14;

/// �����룺UPT����û�л�����.
const UINT16 PP_LEAVE_NO_OPPORTUNITY = -15;

/// �����룺����peer���ͱ��ܾ�
const UINT16 PP_LEAVE_REFUSE_PEER_TYPE = -18;

/// �����룺���ӱ�source�ϵ�����Ϊ������peer
const UINT16 PP_LEAVE_SOURCE_KICKED_INTERNAL_PEER = -20;

/// �����룺���ӱ�source�ϵ�����Ϊ��ʱ��û������ͨѶ
const UINT16 PP_LEAVE_SOURCE_KICKED_IDLE_PEER = -21;

/// �����룺���ӱ�source�ϵ�����Ϊ��Դ���ص�
const UINT16 PP_LEAVE_SOURCE_KICKED_NON_OVERLAPPED_PEER = -22;

/// �����룺���ӱ�source�ϵ�����Ϊ�����ٶȹ���
const UINT16 PP_LEAVE_SOURCE_KICKED_TOOL_SLOW_PEER = -23;

/// �����룺���ӱ�source�ϵ�����Ϊqos����
const UINT16 PP_LEAVE_SOURCE_KICKED_LOW_QOS_PEER = -24;


/// �����룺udpt�����յ�finish����
const UINT16 PP_LEAVE_UDPT_FINISHED = -30;

/// �����룺���ͱ���ʧ��
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

/// leaveԭ�򣺱������ݴ���
const long PP_LEAVE_INVALID_PACKET = -1;

/// leaveԭ������ʱƵ��GUID����
const long PP_LEAVE_BAD_CHANNEL = -2;

/// leaveԭ���Լ������Լ��ı���
const long PP_LEAVE_SELF_TO_SELF = -3;

/// leaveԭ��idleʱ�����
const long PP_LEAVE_LONG_IDLE = -4;

/// leaveԭ�����ֳ�ʱ
const long PP_LEAVE_HANDSHAKE_TIMEOUT = -5;

/// leaveԭ������������
const long PP_LEAVE_NO_DEGREE = -6;

/// leaveԭ���ظ�������
const long PP_LEAVE_DUPLICATE_CONNECTION = -7;

/// leaveԭ�򣺴���̫��
const long PP_LEAVE_BAD_BANDWIDTH = -8;

/// leaveԭ�򣺳����˳�
const long PP_LEAVE_QUIT = -9;

/// leaveԭ�����ӱ��ߵ�
const long PP_LEAVE_KICKED = -10;

/// leaveԭ�򣺶Ͽ���mds������
const long PP_LEAVE_HANGUP_MDS = -11;

/// leaveԭ�����ӱ��ߵ����Ҷ����ڲ�����������
const long PP_LEAVE_SOURCE_KICKED = -12;

/// leaveԭ�����ӱ�source�ܾ�����Ϊ������peer
const long PP_LEAVE_INTERNAL_PEER_REJECTED = -13;

/// leaveԭ�򣺷�����Request ���Ǻܾö�û���ܵ�Data, ���ǲ�ͣ���ܵ�Announce
const long PP_LEAVE_REQUEST_NOT_RECV_DATA = -14;

/// leaveԭ��UPT����û�л�����.
const long PP_LEAVE_NO_OPPORTUNITY = -15;

/// leaveԭ���������
const long PP_LEAVE_NETWORK_ERROR = -16;

/// leaveԭ�򣺾ܾ�tcp��udp������
const long PP_LEAVE_REFUSE = -17;


/// leaveԭ�򣺸���peer���ͱ��ܾ�
const long PP_LEAVE_REFUSE_PEER_TYPE = -18;

/// leaveԭ�����ӱ�source�ϵ�����Ϊ������peer
const long PP_LEAVE_SOURCE_KICKED_INTERNAL_PEER = -20;

/// leaveԭ�����ӱ�source�ϵ�����Ϊ��ʱ��û������ͨѶ
const long PP_LEAVE_SOURCE_KICKED_IDLE_PEER = -21;

/// leaveԭ�����ӱ�source�ϵ�����Ϊ��Դ���ص�
const long PP_LEAVE_SOURCE_KICKED_NON_OVERLAPPED_PEER = -22;

/// leaveԭ�����ӱ�source�ϵ�����Ϊ�����ٶȹ���
const long PP_LEAVE_SOURCE_KICKED_TOOL_SLOW_PEER = -23;

/// leaveԭ�����ӱ�source�ϵ�����Ϊqos����
const long PP_LEAVE_SOURCE_KICKED_LOW_QOS_PEER = -24;




/// leaveԭ��udpt�����յ�finish����
const long PP_LEAVE_UDPT_FINISHED = -30;

/// leaveԭ�򣺷��ͱ���ʧ��
const long PP_LEAVE_SEND_DATA_FAILED = -51;


*/


#endif
