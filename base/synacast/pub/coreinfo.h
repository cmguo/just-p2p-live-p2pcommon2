
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PUB_COREINFO_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PUB_COREINFO_H_

#include <ppl/util/time_counter.h>
#include <ppl/util/macro.h>

const DWORD INFO_MAX_SYNC_INTERVAL = 500;


struct CORE_MEMORY_POOL_INFO
{
	UINT32 TotalBytes;			//
	UINT32 UsedBytes;			// ��ǰʹ���ڴ��ϵͳ�����ڴ�Աȣ���ӳ�ڴ�ʹ���ʣ�Խ��Խ�ã����˷��ڴ�
	UINT32 PeakUsedBytes;		// 
	UINT32 PeakTotalBytes;		// ��ֵ��ӳϵͳ�ڴ�ʹ��״������ֵԽСԽ�á���ʹ�÷�ֵ�Աȣ���ӳ��ֵʹ���ʣ�Խ��Խ��
	UINT64 AllocatePoolCount;	// �ڴ���з����ڴ����
	UINT64 AllocateSystemCount;	// ʵ�ʷ���ϵͳ�ڴ����������з�����ȣ�ԽСԽ�ã�˵���ڴ��Ч���ã��󲿷��ڴ���䲻��Ҫ��ϵͳ����

	/// ���ڴ���ڴ�ֱ��ʹ��malloc�Ĵ���
	UINT64 LargeAllocCount;

	/// ���ڴ���ڴ�ֱ��ʹ��free�Ĵ���
	UINT64 LargeFreeCount;

	/// ���ڴ���ڴ�ֱ��ʹ��malloc�Ĵ���
	UINT64 FreeSystemCount;

	/// ��ǰʹ�õ��ڴ����
	int UsedCount;
	/// ���ʹ�õ��ڴ����
	int PeakUsedCount;
	/// ��ǰ������ڴ����
	int TotalCount;
	/// ���������ڴ����
	int PeakTotalCount;

	UINT32 Reserved[512 - 18];
	UINT32 Reserved2[1024 * 7];

};

struct CORE_MESSAGE_QUEUE_INFO
{
	UINT64 TotalCount;
	UINT32 PendingCount;
	UINT32 FreeCount;
	UINT32 PeakPendingCount;
	UINT32 PeakFreeCount;
	UINT64 TotalDequeueCount;
	UINT32 LastMessage;

	UINT32 Reserved[247];

	void Inc()
	{
		this->TotalCount++;
	}

	void SetPendingCount(UINT32 pendingCount)
	{
		this->PendingCount = pendingCount;
		LIMIT_MIN(this->PeakPendingCount, this->PendingCount);
	}
	void SetFreeCount(UINT32 freeCount)
	{
		this->FreeCount = freeCount;
		LIMIT_MIN(this->PeakFreeCount, this->FreeCount);
	}

	void SyncTo(CORE_MESSAGE_QUEUE_INFO& target) const
	{
		target.TotalCount = this->TotalCount;
		target.PendingCount = this->PendingCount;
		target.FreeCount = this->FreeCount;
		target.PeakPendingCount = this->PeakPendingCount;
		target.PeakFreeCount = this->PeakFreeCount;

		target.TotalDequeueCount = this->TotalDequeueCount;
		target.LastMessage = this->LastMessage;
	}
};

struct CORE_TIMER_QUEUE_INFO
{
	UINT64 TotalCount;
	UINT64 FireTimes;
	UINT32 PendingCount;
	UINT32 PeakPendingCount;
	UINT32 Reserved[250];

	void IncFired()
	{
		this->FireTimes++;
	}

	void Inc()
	{
		this->TotalCount++;
		this->PendingCount++;
		LIMIT_MIN(this->PeakPendingCount, this->PendingCount);
	}
	void Dec()
	{
		LIVE_ASSERT(this->PendingCount > 0);
		this->PendingCount--;
	}

	void SyncTo(ppl::util::time_counter& syncTime, CORE_TIMER_QUEUE_INFO& target) const
	{
		if ( syncTime.elapsed32() < INFO_MAX_SYNC_INTERVAL )
			return;
		syncTime.sync();
		target.TotalCount = this->TotalCount;
		target.FireTimes = this->FireTimes;
		target.PendingCount = this->PendingCount;
		target.PeakPendingCount = this->PeakPendingCount;
	}
};

/// erocģ���ͳ�ƺ�״̬��Ϣ
struct CORE_STATISTICS
{
	CORE_MESSAGE_QUEUE_INFO MessageQueue;
	CORE_TIMER_QUEUE_INFO TimerQueue;
	CORE_MEMORY_POOL_INFO MemoryPool;
	UINT32 Reserved[1024 * 8];
};


#endif