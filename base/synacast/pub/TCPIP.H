//2005.08.11	��С������
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PUB_TCPIP_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PUB_TCPIP_H_

/*
����������
�����ߣ�����
�������ڣ�2005.08.08
��ǰ�汾��V1.0.0

�޸ļ�¼��

*/

#include <WINSOCK2.H>
#include "../pub/Core.h"

#define NETAPI WINAPI

#define	IO_COMPLETION	0
#define	IO_OVERLAPPED	1
//��ʼ��ϵͳ
//����˵��: pErrMsg  ���ش�����Ϣ ; nBufSize    pErrMsg��Ӧ��SIZE
bool		NETAPI	Startup(char *pErrMsg, int nBufSize);
typedef 
bool		(NETAPI * LPSTARTUP)(char *pErrMsg, int nBufSize);




//ϵͳ���
void		NETAPI	Clearup();
typedef
void		(NETAPI	* LPCLEARUP)();




//objectType �����б���
#define SOCKET_TCP	1			//��SOCKET����TCP��װ
#define SOCKET_PEER 2			//��TCP����PEERЭ���װ
#define SOCKET_HTTP_CLIENT	3	//HTTP�ͻ��˷�����
#define SOCKET_HTTP_SERVER	4	//HTTP�����������
#define SOCKET_UDP	5			//UDP������
#define SOCKET_PROXY	6		//����������

int			NETAPI	CreateServer(int objectType, int nPort, const char *pszAddress = NULL);
//����˵��: objectType   �׽�������,��objectType˵��
//			nPort		 �׽��ְ󶨶˿�
//			pszAddress   �׽��ְ�IP����ΪNULL���������IP�����򣬰󶨵�ָ����IP
//����ֵ:	-1  ϵͳ�����ڴ治��
//			-2  �����׽���ʧ��
//			-3  ������������ʧ��
//			>=0 ���ض�Ӧ�׽��־��
typedef
int			(NETAPI	*LPCREATESERVER)(int objectType, int nPort, const char *pszAddress);


int			NETAPI	CreateClient(int objectType, int nPort, const char *pszAddress = NULL);
//����˵��: objectType   �׽�������,��objectType�����б�
//			nPort		 �׽��ְ󶨶˿�
//			pszAddress   �׽��ְ�IP����ΪNULL���������IP�����򣬰󶨵�ָ����IP
//����ֵ:	-1  ϵͳ�����ڴ治��
//			-2  �����׽���ʧ��
//			>=0 ���ض�Ӧ�׽��־��
typedef
int			(NETAPI	*LPCREATECLIENT)(int objectType, int nPort, const char *pszAddress);

bool		NETAPI	ConnectHost(int sockHandle, LPSOCKADDR pSockAddr);
bool		NETAPI	ConnectHost2(int sockHandle, const char *pszAddress, int nPort);
//����˵��:  sockHandle  �׽��־��
//			 pszAddress, nPort Ŀ��IP�Ͷ˿�
//����ֵ:		true		�ɹ�
//				False		ʧ��
typedef bool (NETAPI *LPCONNECTHOST)(int sockHandle, LPSOCKADDR pSockAddr);
typedef bool (NETAPI *LPCONNECTHOST2)(int sockHandle, const char *pszAddress, int nPort);

void		NETAPI	AttachObject(int sockHandle, HANDLE selfHandle);
//���׽��ְ󶨵���Ӧ��Ŀ����
typedef void (NETAPI *LPATTACHOBJECT)(int sockHandle, HANDLE selfHandle);

void		NETAPI	DettachObject(int sockHandle);
//����׽�������Ŀ����
typedef void (NETAPI *LPDETTACHOBJECT)(int sockHandle);

bool		NETAPI	SendPackets(int	sockHandle, const void *lpBuf, int nBufLen);
//����TCP���ݱ�����Ӧ��PEER�� ����ֵ: true �ɹ���false ʧ��
typedef	bool (NETAPI *LPSENDPACKETS)(int	sockHandle, const void *lpBuf, int nBufLen);

bool		NETAPI	RecvPackets(int	sockHandle);
//�Ӷ�Ӧ��PEER����TCP���ݣ� ����ֵ: true �ɹ���false ʧ��
typedef	bool (NETAPI *LPRECVPACKETS)(int	sockHandle);

//for httpclient
bool		NETAPI	DoHttpGet(int sockHandle, const char *pszUrl);
//HTTP�ͻ��˷��� GET �������� pszUrlΪĿ���ַ
typedef	bool (NETAPI *LPDOHTTPGET)(int sockHandle, const char *pszUrl);

//for httpserver
bool		NETAPI	DoHttpRecv(int sockHandle);
//HTTP�������˿�ʼ���ܿͻ�������
typedef  bool (NETAPI* LPDOHTTPRECV)(int sockHandle);

//for udp
bool		NETAPI	UdpSendTo(int sockHandle, const void *lpBuf, int nBufLen, LPSOCKADDR pSockAddr);
bool		NETAPI	UdpSendTo2(int sockHandle, const void *lpBuf, int nBufLen, const char *pszAddress, int nPort);
//����UDP���ݵ�Ŀ���ַ
//����˵��: sockHandle  ��Ӧ�׽��־��  lpBuf, nBufLen �������ݱ����ݺʹ�С pszAddress, nPort Ŀ��IP��ַ�Ͷ˿�
//����ֵ:	true �ɹ� �� false ʧ��
typedef	bool (NETAPI *LPUDPSENDTO)(int sockHandle, const void *lpBuf, int nBufLen, LPSOCKADDR pSockAddr);
typedef	bool (NETAPI *LPUDPSENDTO2)(int sockHandle, const void *lpBuf, int nBufLen, const char *pszAddress, int nPort);

//for udp
bool		NETAPI	UdpRecvFrom(int sockHandle, int nRecvLen);
//����UDP����
//����˵��: sockHandle  ��Ӧ�׽��־�� �� nRecvLen �����յ�������ݱ�
typedef	bool (NETAPI* LPUDPRECVFROM)(int sockHandle, int nRecvLen);

void		NETAPI	ReleaseBuffer(const void *lpBuf);
//�ϲ��ͷ���ʹ�õĻ�����
typedef	void (NETAPI* LPRELEASEBUFFER)(const void *lpBuf);

void		NETAPI	CloseIt(int sockHandle);
//�رն�Ӧ���׽��ַ���
typedef	void (NETAPI *LPCLOSEIT)(int sockHandle);

int			NETAPI	GetConnectionNum();
//��ǰTCP������
typedef int	(NETAPI *LPGETCONNECTIONNUM)();

int			NETAPI	GetConnectionPending();
//��ǰ�������ӵ�TCP��Ŀ
typedef	int	(NETAPI *LPGETCONNECTIONPENDING)();

long		NETAPI	GetUploadSpeed();
//�ϴ��ٶ�
typedef	long	(NETAPI *LPGETUPLOADSPEED)();

long		NETAPI	GetDownloadSpeed();
//�ϴ��ٶ�
typedef	long	(NETAPI *LPGETDOWNLOADSPEED)();

__int64	NETAPI	GetUploadBytes();
//�ϴ����ֽ�
typedef __int64 (NETAPI* LPGETUPLOADBYTES)();

__int64	NETAPI	GetDownloadBytes();
//�������ֽ�
typedef __int64 (NETAPI* LPGETDOWNLOADBYTES)();

bool	NETAPI	RecvN(int sockHandle, int nRecvLen);
//TCP����
typedef	bool	(NETAPI* LPRECVN)(int, int);

bool	NETAPI	GetPeerName(int sockHandle, struct sockaddr *pSockAddr, int *nameLen);
//ȡԶ��TCP�׽�����Ϣ
typedef bool (NETAPI *LPGETPEERNAME)(int, struct sockaddr*, int*);

bool		NETAPI	GetLocalName(int sockHandle, struct sockaddr *pSockAddr, int *nameLen);
//ȡ�����׽�����Ϣ
typedef bool	(NETAPI	*LPGETLOCALNAME)(int, struct sockaddr*, int*);

long		NETAPI	GetSendingNum(int sockHandle);
//ȡ���ڷ��͵����ݱ�����
typedef	long	(NETAPI	*LPGETSENDINGNUM)(int);

bool		NETAPI	TCPSend(int sockHandle, const void *lpBuf, int nBufLen);
//����TCP����
typedef bool	(NETAPI *LPTCPSEND)(int sockHandle, const void *lpBuf, int nBufLen);

bool		NETAPI	GetProxyPacket(int sockHandle, const void *lpBuf, int nBufLen, LPSOCKADDR pSockAddr);
//ȡPEER��������
typedef bool	(NETAPI *LPGETPROXYPACKET)(int ,const void*, int ,LPSOCKADDR);


bool		NETAPI	GetProxyPacket2(int sockHandle, const void *lpBuf, int nBufLen, 
									const char *pszAddress, int nPort);
//ȡPEER��������
typedef bool	(NETAPI *LPGETPROXYPACKET2)(int ,const void*, int , const char*, int);

void	NETAPI	SetTransferTimeout(int sockHandle, int nTimeout);
//�趨ͨѶ��ʱʱ��,��λΪ��
typedef	void	(NETAPI*	LPSETTRANSFERTIMEOUT)(int, int);

void	NETAPI	SetConnectTimeout(int sockHandle, int nTimeout);
//�趨���ӳ�ʱʱ��,��λΪ��
typedef void	(NETAPI*	LPSETCONNECTTIMEOUT)(int, int);

bool	NETAPI	GetHostIpByName(const char *pszHostName, HOBJ hTargetObject);
//ͨ������ȡIP
typedef bool	(NETAPI*	LPGETHOSTIPBYNAME)(const char*, HOBJ);

bool	NETAPI	JoinGroup(int sockHandle, const char *pszGroupIP, UINT nTTL, bool bLoopBack);
//�����鲥����
typedef bool	(NETAPI*	LPJOINGROUP)(int sockHandle, const char *pszGroupIP, UINT nTTL, bool bLoopBack);

void	NETAPI	LeaveGroup(int sockHandle, const char *pszGroupIP);
//����鲥����
typedef	void	(NETAPI*	LPLEAVEGROUP)(int sockHandle, const char *pszGroupIP);

void	NETAPI	SetMaxAccept(int sockHandle, WORD nMaxAcceptNum, WORD nFreezeTime, bool bWriteLog);
//WSAAceept����������
typedef void	(NETAPI*	LPSETMAXACCEPT)(int sockHandle, WORD nMaxAcceptNum, WORD nFreezeTime, bool bWriteLog);

void	NETAPI  SetMaxSendLen(int nMaxSendLen);
//�趨������ֽ���
typedef void	(NETAPI*	LPSETMAXSENDLEN)(int nMaxSendLen);

void	NETAPI  SetMaxRecvLen(int nMaxRecvLen);
//�趨�������ֽ���
typedef void	(NETAPI*	LPSETMAXRECVLEN)(int nMaxRecvLen);

typedef struct _NETINTERFACE{
	DWORD			dwVersion;
	LPSTARTUP		Startup;
	LPCLEARUP		Cleanup;
	LPCREATESERVER	CreateServer;
	LPCREATECLIENT	CreateClient;
	LPCONNECTHOST	ConnectHost;
	LPCONNECTHOST2	ConnectHost2;
	LPATTACHOBJECT	AttachObject;
	LPDETTACHOBJECT	DettachObject;
	LPSENDPACKETS	SendPackets;
	LPRECVPACKETS	RecvPackets;
	LPDOHTTPGET		DoHttpGet;
	LPDOHTTPRECV	DoHttpRecv;
	LPUDPSENDTO		UdpSendTo;
	LPUDPSENDTO2	UdpSendTo2;
	LPUDPRECVFROM	UdpRecvFrom;
	LPRELEASEBUFFER	ReleaseBuffer;
	LPCLOSEIT		CloseIt;

	LPGETCONNECTIONNUM	GetConnectionNum;
	LPGETCONNECTIONPENDING	GetConnectionPending;
	LPGETUPLOADSPEED	GetUploadSpeed;
	LPGETDOWNLOADSPEED	GetDownloadSpeed;
	LPGETUPLOADBYTES	GetUploadBytes;
	LPGETDOWNLOADBYTES	GetDownloadBytes;
	LPRECVN				RecvN;
	LPGETPEERNAME		GetPeerName;
	LPGETLOCALNAME		GetLocalName;
	LPGETSENDINGNUM		GetSendingNum;
	LPTCPSEND			TCPSend;
	LPGETPROXYPACKET	GetProxyPacket;
	LPGETPROXYPACKET2	GetProxyPacket2;
	LPSETTRANSFERTIMEOUT		SetTransferTimeout;
	LPSETCONNECTTIMEOUT			SetConnectTimeout;
	LPGETHOSTIPBYNAME			GetHostIpByName;
	LPJOINGROUP					JoinGroup;
	LPLEAVEGROUP				LeaveGroup;
	LPSETMAXACCEPT				SetMaxAccept;
	LPSETMAXSENDLEN				SetMaxSendLen;
	LPSETMAXRECVLEN				SetMaxRecvLen;
	DWORD			dwReserved[21];
} NETINTERFACE, *LPNETINTERFACE;

void	NETAPI	TS_XXXX(LPNETINTERFACE lpNetInterface);

//�ײ��յ�ͨѶ��Ϣ�󣬽���Ϣ�׸��ϲ�����Ӧ����ϢID
//SendObjectMessage( HOBJ Target_obj, HOBJ Sender_obj, UINT Msg,
//					   UINT PARAM_1, UINT PARAM_2, UINT PARAM_3, UINT PARAM_4);
//�ײ�����Ӧ��SendObjectMessage�����У�Sender_objΪ��Ӧ���׽��־����
//Target_objΪͨ��AttachObject����������õ�Ŀ����

//MESSAGE ID
#define UM_SOCKET_START		1000

#define UM_SOCKET_ACCEPT_SUCCESS	UM_SOCKET_START + 1	//���������ܿͻ������ӳɹ�
//��Ӧ���� param1 : sockHandle	�½����ӵ��׽��־��
#define UM_SOCKET_ACCEPT_FAIL		UM_SOCKET_START + 2	//���������ܿͻ�������ʧ��
//��Ӧ���� param1 : nErrCode	��Ӧ���׽��ִ��󷵻�ֵ

#define UM_SOCKET_CONNECT_SUCCESS	UM_SOCKET_START + 3 //�ͻ������ӷ������ɹ�
//��Ӧ���� ��
#define UM_SOCKET_CONNECT_FAIL		UM_SOCKET_START + 4 //�ͻ������ӷ�����ʧ��
//��Ӧ���� param1 : nErrCode	��Ӧ���׽��ִ��󷵻�ֵ

#define UM_SOCKET_RECVPACKETS_SUCCESS UM_SOCKET_START + 5	//����TCP���ݱ��ɹ�
//��Ӧ���� param1: const void *pszBuf �������ݱ�����
//		   param2: int nBufLen		  �������ݱ���С

#define UM_SOCKET_RECVPACKETS_FAIL	  UM_SOCKET_START + 6	//����TCP���ݱ�ʧ��
//��Ӧ���� param1 : nErrCode	���󷵻�ֵ -1:���ձ���ʧ�� 
//										   -2:���հ�ͷʧ�� 
//										   -3:��ͷ���ݷǷ�
//										   -4:�������ݳ�ʱ
//										   -5:�����Ѿ�����һ�˹ر�

#define UM_SOCKET_RECVFROM_SUCCESS	  UM_SOCKET_START + 7	//����UDP���ݳɹ�
//��Ӧ���� param1: const void *pszBuf �������ݱ�����
//		   param2: int nBufLen		  �������ݱ���С
//		   param3: LPSOCKADDR pSockAddr ������ԴIP��Ϣ
#define UM_SOCKET_RECVFROM_FAIL		  UM_SOCKET_START + 8	//����UDP����ʧ��
//��Ӧ���� param1 : nErrCode	���󷵻�ֵ 

#define UM_SOCKET_HTTPGET_SUCCESS		UM_SOCKET_START + 9	//http get �ɹ�
//��Ӧ���� param1: const void *pszBuf �������ݱ�����
//		   param2: int nBufLen		  �������ݱ���С
#define UM_SOCKET_HTTPGET_FAIL			UM_SOCKET_START + 10	//http get ʧ��
//��Ӧ���� param1 : nErrCode	���󷵻�ֵ -1:���ӷ�����ʧ�� 
//										   -2:��������ʧ�� 
//										   -3:��������ʧ��
//										   -4:���������Ҳ�����ͷ����λ��
//										   -5:ϵͳ�������뻺����ʧ��	  
//										   -6:���պ�����ʧ��
//										   -7:�������ݳ�ʱ

#define UM_SOCKET_RECVN_SUCCESS		UM_SOCKET_START + 11	//RECVN�ɹ�
//��Ӧ����	param1: const void *lpBuf	�������ݱ�����
//			param2: int nBufLen			�������ݱ���С

#define UM_SOCKET_RECVN_FAIL		UM_SOCKET_START + 12	//RECVNʧ��
//��Ӧ����	param1:	nErrCode	���󷵻�ֵ -1:ϵͳ�������뻺����ʧ��
//										   -2:���պ�����ʧ��
//										   -3:�����Ѿ�����һ�˹ر�
//										   -4:�������ݳ�ʱ

#define UM_SOCKET_HTTPRECV_SUCCESS		UM_SOCKET_START + 13	//http server recv�ɹ�
//��Ӧ���� param1: const void *pszBuf �������ݱ�����
//		   param2: int nBufLen		  �������ݱ���С

#define UM_SOCKET_HTTPRECV_FAIL			UM_SOCKET_START + 14	//http server recvʧ��
//��Ӧ���� param1 : nErrCode	���󷵻�ֵ -1:���������Ҳ�����ͷ����λ��
//										   -2:ϵͳ�������뻺����ʧ��
//										   -3:���պ�����ʧ��
//										   -4:�����Ѿ��Ͽ�
//										   -5:�������ݳ�ʱ

#define UM_SOCKET_PROXYRECV_SUCCESS		UM_SOCKET_START + 14	//ȡ�������ݳɹ�
//��Ӧ���� param1: const void *pszBuf �������ݱ�����
//		   param2: int nBufLen		  �������ݱ���С

#define UM_SOCKET_PROXYRECV_FAIL		UM_SOCKET_START + 15	//ȡ��������ʧ��
//��Ӧ���� param1 : nErrCode	���󷵻�ֵ -1:����Ŀ�������ʧ��
//										   -2:����PEERЭ�����ݱ�ʧ��
//										   -3:����PEERЭ�����ݱ�ʧ��
//										   -4:�Է������Ѿ��Ͽ�
//										   -5:�������ݳ�ʱ

#define UM_SOCKET_GETHOSTBYNAME_SUCCESS	UM_SOCKET_START + 16	//ͨ������ȡIP�ɹ�
//��Ӧ����	param1: const char *pszHostName	�������û���ѯ��
//			param2:	 DWORD	dwIP			����IP

#define UM_SOCKET_GETHOSTBYNAME_FAIL	UM_SOCKET_START + 17	//ͨ������ȡIPʧ��
//��Ӧ����	param1: int	nErrCode			�׽��ִ������

#define UM_SOCKET_END		2000


#define NETWORK			2000

#define NETWORK_INFO(message)		LOG(NETWORK, __INFO, message)
#define NETWORK_EVENT(message)		LOG(NETWORK, __EVENT, message)
#define NETWORK_WARN(message)		LOG(NETWORK, __WARN, message)
#define NETWORK_ERROR(message)		LOG(NETWORK, __ERROR, message)
#define NETWORK_DEBUG(message)		LOG(NETWORK, __DEBUG, message)




#endif