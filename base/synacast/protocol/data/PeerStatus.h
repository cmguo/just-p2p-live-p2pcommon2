
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_PEER_STATUS_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_PEER_STATUS_H_

#include <synacast/protocol/data/DegreeInfo.h>
#include <ppl/data/int.h>
#include <ppl/util/macro.h>


/// peer״̬��Ϣ
struct PEER_STATUS
{
	/// ʣ���ϴ�����(��ʱû��ʹ��)
	UINT16 UploadBWLeft;

	/// ��������(�ϴ��ٶ�)
	UINT16 Qos;

	/// ʣ���
	INT8 DegreeLeft;

	/// ����
	UINT8 OutDegree;

	/// ���
	UINT8 InDegree;

	/// ��֡��
	UINT8 SkipPercent;

	enum { object_size = 8 };

	int GetTotalDegree() const
	{
		return this->DegreeLeft + this->OutDegree + this->InDegree;
	}
};





/// peer״̬��Ϣ������status/minmax
struct PEER_STATUS_INFO
{
	/// ���Ӻ����ص�״̬��Ϣ
	PEER_STATUS Status;

	/// ��Դ��Χ
	PEER_MINMAX MinMax;

	/// ���
	void Clear()
	{
		this->MinMax.Clear();
		FILL_ZERO(this->Status);
		this->Status.SkipPercent = 100;
	}

	enum { object_size = PEER_STATUS::object_size + PEER_MINMAX::object_size };

};




/*
struct EXTRA_PEER_NET_INFO
{
UINT32 DetectedIP;
UINT16 DetectedUDPPort;
INT8 DegreeLeft;
UINT8 InDegree;
};*/





/// ������peer״̬��Ϣ
class PeerStatusEx
{
public:
	/// ʣ���ϴ�����(��ʱû��ʹ��)
	UINT16 UploadBWLeft;

	/// ��������(�ϴ��ٶ�)
	UINT16 Qos;

	/// �������Ϣ
	DEGREE_INFO Degrees;

	/// ��֡��
	UINT8 SkipPercent;

	enum { object_size = sizeof(UINT16) * 2 + sizeof(DEGREE_INFO) + sizeof(UINT8) };

	PeerStatusEx()
	{
		UploadBWLeft = 0;
		Qos = 0;
		Degrees.Clear();
		// ��ʼ��SkipPercent��Ϊ100����Ӧ�Ļ�����Ϊ0
		SkipPercent = 100;
	}

	PeerStatusEx( const PEER_STATUS& status, const DEGREE_INFO& degreeInfo )
	{
		UploadBWLeft = status.UploadBWLeft;
		Qos = status.Qos;
		Degrees = degreeInfo;
		SkipPercent = status.SkipPercent;
	}
};



#endif
