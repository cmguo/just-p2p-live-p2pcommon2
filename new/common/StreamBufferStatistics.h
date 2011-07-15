
#ifndef _LIVE_P2PCOMMON2_NEW_COMMON_STREAM_BUFFER_STATISTICS_H_
#define _LIVE_P2PCOMMON2_NEW_COMMON_STREAM_BUFFER_STATISTICS_H_

#include "util/flow.h"
#include <synacast/protocol/DataCollecting.h>

#include <ppl/util/ini_file.h>

class StreamBufferConfig
{
public:
	/// Ƭ��
	UINT ResetBufferGap;

	UINT MaxOldHeaderCount;

	/// ��λ������
	//UINT PeerMinBufferTime;

	/// ��λ������
	UINT TimelyPushGap;

	/// ��Ҫ��������У��
	bool NeedVerifyData;

	/// �����tracker��������peer���ĵ�ǰʱ�����λ�ã���Min��ʼ��������
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


/// StreamBuffer��ͳ������
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