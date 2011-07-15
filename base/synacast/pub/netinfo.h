
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PUB_NETINFO_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PUB_NETINFO_H_



/// eroc模块的统计和状态信息
struct NET_STATISTICS
{
	UINT64 TotalUDPReceiving;
	UINT64 TotalUDPReceivingComplete;
	UINT64 TotalUDPReceived;
	UINT64 TotalUDPReceiveError;

	UINT64 TotalSend;
	UINT64 TotalSendFailed;
	UINT64 TotalSendSucceeded;

	long LastUDPSendError;
	UINT64 TotalSendFailed2;
	long LastUDPReceiveError;

	UINT32 Reserved[1024 * 8 - 18];

	void Clear()
	{
		memset( this, 0, sizeof(NET_STATISTICS) );
	}
};

#endif