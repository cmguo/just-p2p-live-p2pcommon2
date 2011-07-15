
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_TRACKER_PROTOCOL_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_TRACKER_PROTOCOL_H_

//#error should not be included

#include <synacast/protocol/data/NetInfoIO.h>
#include <synacast/protocol/data/PeerMinMaxIO.h>
#include <synacast/protocol/data/PeerAddressIO.h>
#include <ppl/io/data_input_stream.h>
#include <ppl/io/data_output_stream.h>

using ppl::io::data_input_stream;
using ppl::io::data_output_stream;


/// peer-trackerЭ���������
enum ActionTypeEnum
{
	PT_ACTION_JOIN					= 0x01, 
	PT_ACTION_LIST					= 0x02, 
	PT_ACTION_KEEPALIVE				= 0x03, 
	PT_ACTION_LEAVE					= 0x04, 
	PT_ACTION_REGISTER				= 0x05, 
};

enum PTSActionEnum
{
	PTS_ACTION_JOIN_REQUEST			= 0x06, 
	PTS_ACTION_JOIN_RESPONSE		= 0x07, 

	PTS_ACTION_LIST_PEERS_REQUEST	= 0x08, 
	PTS_ACTION_LIST_PEERS_RESPONSE	= 0x09, 

	PTS_ACTION_KEEP_ALIVE_REQUEST	= 0x0A, 
	PTS_ACTION_KEEP_ALIVE_RESPONSE	= 0x0B, 

	PTS_ACTION_REGISTER_REQUEST		= 0x0C, 
	PTS_ACTION_REGISTER_RESPONSE	= 0x0D, 

	PTS_ACTION_LEAVE_REQUEST		= 0x0E, 
	PTS_ACTION_SOURCE_EXIT_REQUEST	= 0x0F, 

	PTS_ACTION_ERROR_RESPONSE		= 0x13, 

	PTS_ACTION_PROXY_MESSAGE		= 0x10, 
};



struct SECURE_REQUEST_HEAD
{
	/// Peer�����͡���·���͡�nat����
	PEER_CORE_INFO CoreInfo;

	/// �Ƿ���Ƕ��ʽϵͳ�ã�Windows Desktop=0��Windows CE=1
	UINT8 Platform;

	/// �ں˰汾��
	UINT32 AppVersion;

	/// ����ϵͳ���Ա��
	UINT16 Language;

	enum { object_size = PEER_CORE_INFO::object_size + 1 + 4 + 2 };
};

struct SECURE_RESPONSE_HEAD
{
	/// ��Ƶ��source����Դ��Χ
	PEER_MINMAX SourceMinMax;

	/// ��Ŀ��piece��С��Ŀǰû��ʹ�ã�
	UINT32 PieceSize;

	/// ��Ŀ�������ʣ�byte/s��Ŀǰû��ʹ�ã�
	UINT32 ByteRate;

	/// ��Ƶ����peer����
	UINT32 PeerCount;

	/// tracker������peer��ַ��ʵ��ֻ��ip/udpport��Ч��tcpportΪ0��
	PEER_ADDRESS DetectedAddress;

	enum { object_size = PEER_MINMAX::object_size + sizeof(UINT32) * 3 + PEER_ADDRESS::object_size };
};


inline data_output_stream& operator<<( data_output_stream& os, const SECURE_REQUEST_HEAD& head )
{
	return os 
		<< head.CoreInfo 
		<< head.Platform 
		<< head.AppVersion 
		<< head.Language;
}

inline data_input_stream& operator>>( data_input_stream& is, SECURE_REQUEST_HEAD& head )
{
	return is 
		>> head.CoreInfo 
		>> head.Platform 
		>> head.AppVersion 
		>> head.Language;
}


inline data_output_stream& operator<<( data_output_stream& os, const SECURE_RESPONSE_HEAD& head )
{
	return os 
		<< head.SourceMinMax 
		<< head.PieceSize 
		<< head.ByteRate 
		<< head.PeerCount 
		<< head.DetectedAddress;
}

inline data_input_stream& operator>>( data_input_stream& is, SECURE_RESPONSE_HEAD& head )
{
	return is 
		>> head.SourceMinMax 
		>> head.PieceSize 
		>> head.ByteRate 
		>> head.PeerCount 
		>> head.DetectedAddress;
}

#endif
