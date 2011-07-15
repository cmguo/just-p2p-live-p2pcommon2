
#ifndef _LIVE_P2PCOMMON2_NEW_COMMON_IPPOOL_STATISTICS_H_
#define _LIVE_P2PCOMMON2_NEW_COMMON_IPPOOL_STATISTICS_H_

#include <synacast/protocol/DataCollecting.h>

#include <ppl/util/ini_file.h>
#include <vector>

/*
struct PeerTrafficInfo
{
	/// 外网地址
	UINT32 DetectedIP;

	/// 上传字节数，单位：KB
	UINT32 UploadBytes;

	/// 下载字节数，单位：KB
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

	/// 单位：秒
	UINT HelloInterval;

	/// 单位：秒
	UINT DetectInterval;

	/// 单位：秒
	UINT ConnectInterval;

	/// 
	UINT MaxContinuousConnectCheck;

	/// 
	UINT MaxPoolSizeNeedDoList;

	/// 单位：毫秒
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

	/// 配置文件中存储的时间以秒为单位
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




/// peer地址池的统计数据
class IPPoolStatistics : public IPPOOL_STATS
{
public:
	IPPoolStatistics()
	{
		this->Clear();
	}


};
#endif