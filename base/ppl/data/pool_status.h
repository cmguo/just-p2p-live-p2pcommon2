
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_DATA_POOL_STATUS_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_DATA_POOL_STATUS_H_

#include <ppl/data/int.h>
#include <ppl/util/macro.h>


namespace ppl { namespace data { 


class pool_status
{
public:
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

	pool_status()
	{
		FILL_ZERO(*this);
	}
};

} }


#endif
