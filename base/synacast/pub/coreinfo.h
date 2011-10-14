
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PUB_COREINFO_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PUB_COREINFO_H_

#include <ppl/util/time_counter.h>
#include <ppl/util/macro.h>

const DWORD INFO_MAX_SYNC_INTERVAL = 500;


struct CORE_MEMORY_POOL_INFO
{
	UINT32 TotalBytes;			//
	UINT32 UsedBytes;			// 当前使用内存和系统分配内存对比，反映内存使用率，越高越好，不浪费内存
	UINT32 PeakUsedBytes;		// 
	UINT32 PeakTotalBytes;		// 峰值反映系统内存使用状况，峰值越小越好。与使用峰值对比，反映峰值使用率，越高越好
	UINT64 AllocatePoolCount;	// 内存池中分配内存次数
	UINT64 AllocateSystemCount;	// 实际分配系统内存次数，与池中分配相比，越小越好，说明内存池效果好，大部分内存分配不需要向系统申请

	/// 对于大块内存直接使用malloc的次数
	UINT64 LargeAllocCount;

	/// 对于大块内存直接使用free的次数
	UINT64 LargeFreeCount;

	/// 对于大块内存直接使用malloc的次数
	UINT64 FreeSystemCount;

	/// 当前使用的内存块数
	int UsedCount;
	/// 最高使用的内存块数
	int PeakUsedCount;
	/// 当前申请的内存块数
	int TotalCount;
	/// 最高申请的内存块数
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

/// eroc模块的统计和状态信息
struct CORE_STATISTICS
{
	CORE_MESSAGE_QUEUE_INFO MessageQueue;
	CORE_TIMER_QUEUE_INFO TimerQueue;
	CORE_MEMORY_POOL_INFO MemoryPool;
	UINT32 Reserved[1024 * 8];
};


#endif