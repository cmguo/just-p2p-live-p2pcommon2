
#ifndef _LIVE_P2PCOMMON2_NEW_COMMON_STREAM_BUFFER_STATISTICS_H_
#define _LIVE_P2PCOMMON2_NEW_COMMON_STREAM_BUFFER_STATISTICS_H_

#include "util/flow.h"
#include <synacast/protocol/DataCollecting.h>

#include <ppl/util/ini_file.h>

class StreamBufferConfig
{
public:
	/// 片数
	UINT ResetBufferGap;

	UINT MaxOldHeaderCount;

	/// 单位：毫秒
	//UINT PeerMinBufferTime;

	/// 单位：毫秒
	UINT TimelyPushGap;

	/// 需要对数据做校验
	bool NeedVerifyData;

	/// 报告给tracker（和其它peer）的当前时间戳的位置（从Min开始的秒数）
	UINT CurrentTimeStampPosition;

	StreamBufferConfig()
	{
		this->ResetBufferGap = 5000;
		this->MaxOldHeaderCount = 20;
		//this->PeerMinBufferTime = 70 * 1000;
		this->TimelyPushGap = 20 * 1000;
		this->NeedVerifyData = true;
		this->CurrentTimeStampPosition = 80;
	}

	void Load(ini_file& ini)
	{
		this->ResetBufferGap = ini.get_int(_T("ResetBufferGap"), this->ResetBufferGap);
		this->MaxOldHeaderCount = ini.get_int(_T("MaxOldHeaderCount"), this->MaxOldHeaderCount);
		//this->PeerMinBufferTime = ini.GetInteger("PeerMinBufferTime", this->PeerMinBufferTime / 1000) * 1000;
		this->TimelyPushGap = ini.get_int(_T("TimelyPushGap"), this->TimelyPushGap / 1000) * 1000;
		this->NeedVerifyData = ini.get_bool(_T("NeedVerifyData"), this->NeedVerifyData);
		this->CurrentTimeStampPosition = ini.get_int(_T("CurrentTimeStampPosition"), this->CurrentTimeStampPosition);
	}
};


/// StreamBuffer的统计数据
class StreamBufferStatistics : public STREAMBUFFER_STATS
{
public:
	RateMeasure MediaFlow;

	StreamBufferStatistics()
	{
		Clear();
	}
};

#endif