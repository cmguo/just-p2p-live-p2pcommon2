
#ifndef _LIVE_P2PCOMMON2_NEW_COMMON_IPPOOL_STATISTICS_H_
#define _LIVE_P2PCOMMON2_NEW_COMMON_IPPOOL_STATISTICS_H_

#include <synacast/protocol/DataCollecting.h>

#include <ppl/util/ini_file.h>
#include <vector>

/*
struct PeerTrafficInfo
{
	/// ������ַ
	UINT32 DetectedIP;

	/// �ϴ��ֽ�������λ��KB
	UINT32 UploadBytes;

	/// �����ֽ�������λ��KB
	UINT32 DownloadBytes;

	PeerTrafficInfo()
	{
		this->Clear();
	}

	void Clear()
	{
		this->DetectedIP = 0;
		this->UploadBytes = 0;
		this->DownloadBytes = 0;
	}
};
*/

class IPPoolConfig
{
public:
	UINT MaxConnectFailedTimes;

	/// ��λ����
	UINT HelloInterval;

	/// ��λ����
	UINT DetectInterval;

	/// ��λ����
	UINT ConnectInterval;

	/// 
	UINT MaxContinuousConnectCheck;

	/// 
	UINT MaxPoolSizeNeedDoList;

	/// ��λ������
	UINT ValidDetectedRTT;

	IPPoolConfig()
	{
		this->MaxConnectFailedTimes = 3;
		this->HelloInterval = 120;
		this->DetectInterval = 120;
		this->ConnectInterval = 30;
		this->MaxContinuousConnectCheck = 50;
		this->MaxPoolSizeNeedDoList = 300;
		this->ValidDetectedRTT = 5000;
	}

	/// �����ļ��д洢��ʱ������Ϊ��λ
	void Load(ini_file& ini)
	{
		this->MaxConnectFailedTimes = ini.get_int(TEXT("MaxConnectFailedTimes"), this->MaxConnectFailedTimes);
		this->HelloInterval = ini.get_int(TEXT("HelloInterval"), this->HelloInterval);
		this->DetectInterval = ini.get_int(TEXT("DetectInterval"), this->DetectInterval);
		this->ConnectInterval = ini.get_int(TEXT("ConnectInterval"), this->ConnectInterval);
		this->MaxContinuousConnectCheck = ini.get_int(TEXT("MaxContinuousConnectCheck"), this->MaxContinuousConnectCheck);
		this->MaxPoolSizeNeedDoList = ini.get_int(TEXT("MaxPoolSizeNeedDoList"), this->MaxPoolSizeNeedDoList);
		this->ValidDetectedRTT = ini.get_int(TEXT("ValidDetectedRTT"), this->ValidDetectedRTT);
	}
};




/// peer��ַ�ص�ͳ������
class IPPoolStatistics : public IPPOOL_STATS
{
public:
	IPPoolStatistics()
	{
		this->Clear();
	}


};
#endif