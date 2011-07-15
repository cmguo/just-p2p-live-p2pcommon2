
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_DATA_POOL_STATUS_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_DATA_POOL_STATUS_H_

#include <ppl/data/int.h>
#include <ppl/util/macro.h>


namespace ppl { namespace data { 


class pool_status
{
public:
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

	pool_status()
	{
		FILL_ZERO(*this);
	}
};

} }


#endif
