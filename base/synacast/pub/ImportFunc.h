//2005-8-11 16:00 ��С������
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PUB_IMPORT_FUNC_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PUB_IMPORT_FUNC_H_

#ifdef NEED_NET
#include "tcpip.h"
#include "core.h"

//UM_SOCKET_ACCEPT_SUCCESS
//TCP��Ϣ�ṹ��Accept�õ�һ���µ�socket UM_SOCKET_ACCEPT_SUCCESS
typedef struct _MESSAGE_TCP_ACCEPT_SUCCESS{
	HOBJ	Target_obj;		
	HOBJ	Sender_obj;		
	UINT	Msg;			
	int		NewSocketHandle;//�½���Socket���
} MESSAGE_TCP_ACCEPT_SUCCESS;

//UM_SOCKET_ACCEPT_FAIL
//TCP��Ϣ�ṹ��Acceptʧ��
typedef struct _MESSAGE_TCP_ACCEPT_FAILED{
	HOBJ	Target_obj;		
	HOBJ	Sender_obj;		
	UINT	Msg;			
	int		ErrorCode;		//��������(WSA)
} MESSAGE_TCP_ACCEPT_FAILED;

//UM_SOCKET_CONNECT_SUCCESS
//TCP��Ϣ�ṹ��Connect�ɹ�
typedef struct _MESSAGE_TCP_CONNECT_SUCCES{
	HOBJ	Target_obj;		
	HOBJ	Sender_obj;		
	UINT	Msg;			//�޸��Ӳ���
} MESSAGE_TCP_CONNECT_SUCCES;

//UM_SOCKET_CONNECT_FAIL
//TCP��Ϣ�ṹ��Connectʧ��
typedef struct _MESSAGE_TCP_CONNECT_FAILED{
	HOBJ	Target_obj;		
	HOBJ	Sender_obj;		
	UINT	Msg;			
	int		ErrorCode;		//��������(WSA)
} MESSAGE_TCP_CONNECT_FAILED;

//UM_SOCKET_RECVPACKETS_SUCCESS
//TCP��Ϣ�ṹ��RecvPacket�ɹ�
typedef struct _MESSAGE_TCP_RECV_PACKET_SUCCES{
	HOBJ	Target_obj;		
	HOBJ	Sender_obj;		
	UINT	Msg;			
	char	*lpBuf;			//���ݻ�����
	int		nBufLen;		//��������С
} MESSAGE_TCP_RECV_PACKET_SUCCES;


#define UM_SOCKET_ACCEPT_AND_RECVPAKCT_SUCCESS UM_SOCKET_START + 100

//UM_SOCKET_ACCEPT_AND_RECVPAKCT_SUCCESS
//TCP��Ϣ�ṹ��Accept�õ�һ���µ�socket,����RecvPacketһ��Pakcet UM_SOCKET_ACCEPT_AND_RECVPAKCT_SUCCESS
typedef struct _MESSAGE_TCP_ACCEPT_AND_RECVPAKCT_SUCCESS{
	HOBJ	Target_obj;		
	HOBJ	Sender_obj;		
	UINT	Msg;			
	int		NewSocketHandle;//�½���Socket���
	char	*lpBuf;			//���ݻ�����
	int		nBufLen;		//��������С
} MESSAGE_TCP_ACCEPT_AND_RECVPAKCT_SUCCESS;


//UM_SOCKET_RECVPACKETS_FAIL
//TCP��Ϣ�ṹ��RecvPacketʧ��
typedef struct _MESSAGE_TCP_RECV_PACKET_FAILED{
	HOBJ	Target_obj;		//��Ϣ�����ߵľ��|ָ��
	HOBJ	Sender_obj;		//��Ϣ�����ߵľ��|ָ��
	UINT	Msg;			//��Ϣ���
	int		ErrorCode;		//��������(  -1:���ձ���ʧ�� 
//										   -2:���հ�ͷʧ�� 
//										   -3:��ͷ���ݷǷ�
//										   -4:�������ݳ�ʱ
//										   -5:�����Ѿ�����һ�˹ر�)
} MESSAGE_TCP_RECV_PACKET_FAILED;

#define PROXY_UDP	0
#define PROXY_TCP	1
#define PROXY_HTTP	2

//UM_SOCKET_RECVFROM_SUCCESS
//UDP��Ϣ�ṹ��RECVFROM�ɹ�
typedef struct _MESSAGE_UDP_RECVFROM_SUCCESS{
	HOBJ	Target_obj;		
	HOBJ	Sender_obj;		
	UINT	Msg;			
	char	*lpBuf;			//���ݻ�����
	int		nBufLen;		//��������С
	LPSOCKADDR pSockAddr;	//������Դ��ַ��Ϣ
	UINT	ProxyType;		//"����"�����ͣ�
} MESSAGE_UDP_RECVFROM_SUCCESS;

//UM_SOCKET_RECVFROM_FAIL
//UDP��Ϣ�ṹ��RECVFROMʧ��
typedef struct _MESSAGE_UDP_RECVFROM_FAILED{
	HOBJ	Target_obj;		
	HOBJ	Sender_obj;		
	UINT	Msg;			
	int		ErrorCode;		//��������(WSA)
} MESSAGE_UDP_RECVFROM_FAILED;

extern NETINTERFACE NetInterface;

#define NET_Startup(pErrMsg, nBufSize)	\
	NetInterface.Startup(pErrMsg, nBufSize)

#define NET_Clearup() \
	NetInterface.Cleanup()

#define NET_CreateServer(objectType, nPort, pszAddress) \
	NetInterface.CreateServer(objectType, nPort, pszAddress)

#define NET_CreateClient(objectType, nPort, pszAddress) \
	NetInterface.CreateClient(objectType, nPort, pszAddress)

#define NET_TCP_ConnectHost(sockHandle, pSockAddr) \
	NetInterface.ConnectHost(sockHandle, pSockAddr)

#define NET_TCP_ConnectHost2(sockHandle, pszAddress, nPort) \
	NetInterface.ConnectHost2(sockHandle, pszAddress, nPort)

#define NET_AttachObject(sockHandle, selfHandle) \
	NetInterface.AttachObject(sockHandle, selfHandle)

#define NET_DettachObject(sockHandle) \
	NetInterface.DettachObject(sockHandle)

#define NET_TCP_SendPacket(sockHandle, lpBuf, nBufLen) \
	NetInterface.SendPackets(sockHandle, lpBuf, nBufLen)

#define NET_TCP_RecvPacket(sockHandle) \
	NetInterface.RecvPackets(sockHandle)

#define NET_UDP_SendTo(sockHandle, lpBuf, nBufLen, pSockAddr) \
	NetInterface.UdpSendTo(sockHandle, lpBuf, nBufLen, pSockAddr)

#define NET_UDP_SendTo2(sockHandle, lpBuf, nBufLen, pszAddress, nPort) \
	NetInterface.UdpSendTo2(sockHandle, lpBuf, nBufLen, pszAddress, nPort)

#define NET_UDP_RecvFrom(sockHandle, nRecvLen) \
	NetInterface.UdpRecvFrom(sockHandle, nRecvLen)

#define NET_ReleaseBuffer(lpBuf) \
	NetInterface.ReleaseBuffer(lpBuf) 

#define NET_CloseSocket(sockHandle) \
	NetInterface.CloseIt(sockHandle)

#define NET_GetConnectionCount() \
	NetInterface.GetConnectionNum()

#define NET_GetConnectionPendingCount() \
	NetInterface.GetConnectionPending()

#define NET_GetUploadSpeed() \
	NetInterface.GetUploadSpeed()

#define NET_GetDownloadSpeed() \
	NetInterface.GetDownloadSpeed()

#define NET_GetUploadBytes() \
	NetInterface.GetUploadBytes()

#define NET_GetDownloadBytes() \
	NetInterface.GetDownloadBytes()

#define NET_DoHTTPRecv(sockHandle) \
	NetInterface.DoHttpRecv(sockHandle)

#define NET_DoHTTPGet(sockHandle, pszUrl) \
	NetInterface.DoHttpRecv(sockHandle, pszUrl)
	
#define NET_TCP_Send(sockHandle, lpBuf, nBufLen) \
	NetInterface.TCPSend(sockHandle, lpBuf, nBufLen)

#define NET_TCP_RecvN(sockHandle, nRecvLen) \
	NetInterface.RecvN(sockHandle, nRecvLen)

#define NET_GetPeerName(sockHandle, pSockAddr, namelen) \
	NetInterface.GetPeerName(sockHandle, pSockAddr, namelen)

#define NET_GetLocalName(sockHandle, pSockAddr, namelen) \
	NetInterface.GetLocalName(sockHandle, pSockAddr, namelen)

#define NET_GetSendingCount(sockHandle) \
	NetInterface.GetSendingNum(sockHandle)

#define NET_SetTransferTimeout(sockHandle, nTimeout) \
	NetInterface.SetTransferTimeout(sockHandle, nTimeout)

#define NET_SetConnectTimeout(sockHandle, nTimeout) \
	NetInterface.SetConnectTimeout(sockHandle, nTimeout)

#define NET_GetHostByName(pszHostName, hTargetObject) \
	NetInterface.GetHostIpByName(pszHostName, hTargetObject)


#endif //NEED_NET

#ifdef NEED_CORE
#include "core.h"

//extern COREINTERFACE CoreInterface; ��core.h�ж�����

//�ڴ����
#ifdef _DEBUG
#define CORE_MALLOC(size)	CoreInterface.Pool_malloc(size,__FILE__,__LINE__)
#define CORE_FREE(pVoid)	CoreInterface.Pool_free(pVoid,__FILE__,__LINE__)
#else
#define CORE_MALLOC(size)	CoreInterface.Pool_malloc(size,NULL,0)
#define CORE_FREE(pVoid)	CoreInterface.Pool_free(pVoid,NULL,0)
#endif


#define CORE_CLEAR			CoreInterface.Pool_clear

//��Ϣ����
#define CORE_SendObjectMessage(Target_obj, Sender_obj, Msg, PARAM1, PARAM2, PARAM3, PARAM4)  \
	CoreInterface.SendObjectMessage(Target_obj, Sender_obj, Msg, PARAM1, PARAM2, PARAM3, PARAM4)

#define CORE_GetObjectMessage(lpMsg)  \
	CoreInterface.GetObjectMessage(lpMsg)

#define CORE_ClearAllObjectMessage  \
	CoreInterface.ClearAllObjectMessage

#define CORE_ClearTargetObjectMessage(Target_obj)  \
	CoreInterface.ClearTargetObjectMessage(Target_obj)

#define CORE_CleanMemPool  \
	CoreInterface.CleanMemPool

#define CORE_GetMessageCount  \
	CoreInterface.GetMessageCount

#define CORE_WaitEvent(hEvent)  \
	CoreInterface.WaitEvent(hEvent)

#define CORE_WaitEventWithTimeOut(hEvent , timeout)  \
	CoreInterface.WaitEvent(hEvent, timeout)

#define CORE_SetQuitFlag  \
	CoreInterface.SetQuitFlag

#define CORE_log  \
	CoreInterface.log

#define CORE_vlog  \
	CoreInterface.vlog

#define CORE_SetLogLevel(Level)  \
	CoreInterface.SetLogLevel(Level)

#define CORE_LogOn(type) \
	CoreInterface.LogOn(type)

#define CORE_LogOff(type) \
	CoreInterface.LogOff(type)



#define CORE_InsertTimer(uElapse, Flag, Target_obj, Sender_obj, Msg, PARAM1, PARAM2, PARAM3, PARAM4) \
	CoreInterface.InsertTimer(uElapse, Flag, Target_obj, Sender_obj, Msg, PARAM1, PARAM2, PARAM3, PARAM4)

#define CORE_RemoveTimer(TimerID) \
	CoreInterface.RemoveTimer(TimerID)

#define CORE_CreateSharedMemory(Mapitem) \
	CoreInterface.CreateSharedMemory(Mapitem)

#define CORE_OpenSharedMemory(Mapitem) \
	CoreInterface.OpenSharedMemory(Mapitem)

#define CORE_CloseSharedMemory(Mapitem) \
	CoreInterface.CloseSharedMemory(Mapitem)


#endif //NEED_CORE






#endif