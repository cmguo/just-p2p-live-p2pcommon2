//2005.08.11	张小兵定标
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PUB_TCPIP_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PUB_TCPIP_H_

/*
网络服务组件
创建者：付刚
创建日期：2005.08.08
当前版本：V1.0.0

修改记录：

*/

#include <WINSOCK2.H>
#include "../pub/Core.h"

#define NETAPI WINAPI

#define	IO_COMPLETION	0
#define	IO_OVERLAPPED	1
//初始化系统
//参数说明: pErrMsg  返回错误信息 ; nBufSize    pErrMsg对应的SIZE
bool		NETAPI	Startup(char *pErrMsg, int nBufSize);
typedef 
bool		(NETAPI * LPSTARTUP)(char *pErrMsg, int nBufSize);




//系统清除
void		NETAPI	Clearup();
typedef
void		(NETAPI	* LPCLEARUP)();




//objectType 类型列表：
#define SOCKET_TCP	1			//对SOCKET进行TCP封装
#define SOCKET_PEER 2			//对TCP进行PEER协议封装
#define SOCKET_HTTP_CLIENT	3	//HTTP客户端发起类
#define SOCKET_HTTP_SERVER	4	//HTTP服务端侦听类
#define SOCKET_UDP	5			//UDP传输类
#define SOCKET_PROXY	6		//代理传输类

int			NETAPI	CreateServer(int objectType, int nPort, const char *pszAddress = NULL);
//参数说明: objectType   套节字类型,见objectType说明
//			nPort		 套节字绑定端口
//			pszAddress   套节字绑定IP，如为NULL，则绑定所有IP，否则，绑定到指定的IP
//返回值:	-1  系统错误，内存不足
//			-2  创建套节字失败
//			-3  创建侦听服务失败
//			>=0 返回对应套节字句柄
typedef
int			(NETAPI	*LPCREATESERVER)(int objectType, int nPort, const char *pszAddress);


int			NETAPI	CreateClient(int objectType, int nPort, const char *pszAddress = NULL);
//参数说明: objectType   套节字类型,见objectType类型列表
//			nPort		 套节字绑定端口
//			pszAddress   套节字绑定IP，如为NULL，则绑定所有IP，否则，绑定到指定的IP
//返回值:	-1  系统错误，内存不足
//			-2  创建套节字失败
//			>=0 返回对应套节字句柄
typedef
int			(NETAPI	*LPCREATECLIENT)(int objectType, int nPort, const char *pszAddress);

bool		NETAPI	ConnectHost(int sockHandle, LPSOCKADDR pSockAddr);
bool		NETAPI	ConnectHost2(int sockHandle, const char *pszAddress, int nPort);
//参数说明:  sockHandle  套节字句柄
//			 pszAddress, nPort 目标IP和端口
//返回值:		true		成功
//				False		失败
typedef bool (NETAPI *LPCONNECTHOST)(int sockHandle, LPSOCKADDR pSockAddr);
typedef bool (NETAPI *LPCONNECTHOST2)(int sockHandle, const char *pszAddress, int nPort);

void		NETAPI	AttachObject(int sockHandle, HANDLE selfHandle);
//将套节字绑定到对应的目标句柄
typedef void (NETAPI *LPATTACHOBJECT)(int sockHandle, HANDLE selfHandle);

void		NETAPI	DettachObject(int sockHandle);
//解除套节字所绑定目标句柄
typedef void (NETAPI *LPDETTACHOBJECT)(int sockHandle);

bool		NETAPI	SendPackets(int	sockHandle, const void *lpBuf, int nBufLen);
//发送TCP数据报到对应的PEER， 返回值: true 成功；false 失败
typedef	bool (NETAPI *LPSENDPACKETS)(int	sockHandle, const void *lpBuf, int nBufLen);

bool		NETAPI	RecvPackets(int	sockHandle);
//从对应的PEER接收TCP数据， 返回值: true 成功；false 失败
typedef	bool (NETAPI *LPRECVPACKETS)(int	sockHandle);

//for httpclient
bool		NETAPI	DoHttpGet(int sockHandle, const char *pszUrl);
//HTTP客户端发送 GET 数据请求 pszUrl为目标地址
typedef	bool (NETAPI *LPDOHTTPGET)(int sockHandle, const char *pszUrl);

//for httpserver
bool		NETAPI	DoHttpRecv(int sockHandle);
//HTTP服务器端开始接受客户端数据
typedef  bool (NETAPI* LPDOHTTPRECV)(int sockHandle);

//for udp
bool		NETAPI	UdpSendTo(int sockHandle, const void *lpBuf, int nBufLen, LPSOCKADDR pSockAddr);
bool		NETAPI	UdpSendTo2(int sockHandle, const void *lpBuf, int nBufLen, const char *pszAddress, int nPort);
//发送UDP数据到目标地址
//参数说明: sockHandle  对应套节字句柄  lpBuf, nBufLen 发送数据报内容和大小 pszAddress, nPort 目标IP地址和端口
//返回值:	true 成功 ， false 失败
typedef	bool (NETAPI *LPUDPSENDTO)(int sockHandle, const void *lpBuf, int nBufLen, LPSOCKADDR pSockAddr);
typedef	bool (NETAPI *LPUDPSENDTO2)(int sockHandle, const void *lpBuf, int nBufLen, const char *pszAddress, int nPort);

//for udp
bool		NETAPI	UdpRecvFrom(int sockHandle, int nRecvLen);
//接收UDP数据
//参数说明: sockHandle  对应套节字句柄 ； nRecvLen 所接收的最大数据报
typedef	bool (NETAPI* LPUDPRECVFROM)(int sockHandle, int nRecvLen);

void		NETAPI	ReleaseBuffer(const void *lpBuf);
//上层释放所使用的缓冲区
typedef	void (NETAPI* LPRELEASEBUFFER)(const void *lpBuf);

void		NETAPI	CloseIt(int sockHandle);
//关闭对应的套节字服务
typedef	void (NETAPI *LPCLOSEIT)(int sockHandle);

int			NETAPI	GetConnectionNum();
//当前TCP连接数
typedef int	(NETAPI *LPGETCONNECTIONNUM)();

int			NETAPI	GetConnectionPending();
//当前正在连接的TCP数目
typedef	int	(NETAPI *LPGETCONNECTIONPENDING)();

long		NETAPI	GetUploadSpeed();
//上传速度
typedef	long	(NETAPI *LPGETUPLOADSPEED)();

long		NETAPI	GetDownloadSpeed();
//上传速度
typedef	long	(NETAPI *LPGETDOWNLOADSPEED)();

__int64	NETAPI	GetUploadBytes();
//上传总字节
typedef __int64 (NETAPI* LPGETUPLOADBYTES)();

__int64	NETAPI	GetDownloadBytes();
//下载总字节
typedef __int64 (NETAPI* LPGETDOWNLOADBYTES)();

bool	NETAPI	RecvN(int sockHandle, int nRecvLen);
//TCP接收
typedef	bool	(NETAPI* LPRECVN)(int, int);

bool	NETAPI	GetPeerName(int sockHandle, struct sockaddr *pSockAddr, int *nameLen);
//取远程TCP套节字信息
typedef bool (NETAPI *LPGETPEERNAME)(int, struct sockaddr*, int*);

bool		NETAPI	GetLocalName(int sockHandle, struct sockaddr *pSockAddr, int *nameLen);
//取本地套节字信息
typedef bool	(NETAPI	*LPGETLOCALNAME)(int, struct sockaddr*, int*);

long		NETAPI	GetSendingNum(int sockHandle);
//取正在发送的数据报数量
typedef	long	(NETAPI	*LPGETSENDINGNUM)(int);

bool		NETAPI	TCPSend(int sockHandle, const void *lpBuf, int nBufLen);
//发送TCP数据
typedef bool	(NETAPI *LPTCPSEND)(int sockHandle, const void *lpBuf, int nBufLen);

bool		NETAPI	GetProxyPacket(int sockHandle, const void *lpBuf, int nBufLen, LPSOCKADDR pSockAddr);
//取PEER代理数据
typedef bool	(NETAPI *LPGETPROXYPACKET)(int ,const void*, int ,LPSOCKADDR);


bool		NETAPI	GetProxyPacket2(int sockHandle, const void *lpBuf, int nBufLen, 
									const char *pszAddress, int nPort);
//取PEER代理数据
typedef bool	(NETAPI *LPGETPROXYPACKET2)(int ,const void*, int , const char*, int);

void	NETAPI	SetTransferTimeout(int sockHandle, int nTimeout);
//设定通讯超时时间,单位为秒
typedef	void	(NETAPI*	LPSETTRANSFERTIMEOUT)(int, int);

void	NETAPI	SetConnectTimeout(int sockHandle, int nTimeout);
//设定连接超时时间,单位为秒
typedef void	(NETAPI*	LPSETCONNECTTIMEOUT)(int, int);

bool	NETAPI	GetHostIpByName(const char *pszHostName, HOBJ hTargetObject);
//通过域名取IP
typedef bool	(NETAPI*	LPGETHOSTIPBYNAME)(const char*, HOBJ);

bool	NETAPI	JoinGroup(int sockHandle, const char *pszGroupIP, UINT nTTL, bool bLoopBack);
//加入组播属性
typedef bool	(NETAPI*	LPJOINGROUP)(int sockHandle, const char *pszGroupIP, UINT nTTL, bool bLoopBack);

void	NETAPI	LeaveGroup(int sockHandle, const char *pszGroupIP);
//解除组播属性
typedef	void	(NETAPI*	LPLEAVEGROUP)(int sockHandle, const char *pszGroupIP);

void	NETAPI	SetMaxAccept(int sockHandle, WORD nMaxAcceptNum, WORD nFreezeTime, bool bWriteLog);
//WSAAceept防攻击策略
typedef void	(NETAPI*	LPSETMAXACCEPT)(int sockHandle, WORD nMaxAcceptNum, WORD nFreezeTime, bool bWriteLog);

void	NETAPI  SetMaxSendLen(int nMaxSendLen);
//设定最大发送字节数
typedef void	(NETAPI*	LPSETMAXSENDLEN)(int nMaxSendLen);

void	NETAPI  SetMaxRecvLen(int nMaxRecvLen);
//设定最大接受字节数
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

//底层收到通讯消息后，将消息抛给上层所对应的消息ID
//SendObjectMessage( HOBJ Target_obj, HOBJ Sender_obj, UINT Msg,
//					   UINT PARAM_1, UINT PARAM_2, UINT PARAM_3, UINT PARAM_4);
//底层所对应的SendObjectMessage参数中，Sender_obj为对应的套节字句柄，
//Target_obj为通过AttachObject函数，所获得的目标句柄

//MESSAGE ID
#define UM_SOCKET_START		1000

#define UM_SOCKET_ACCEPT_SUCCESS	UM_SOCKET_START + 1	//服务器接受客户端连接成功
//对应参数 param1 : sockHandle	新建连接的套节字句柄
#define UM_SOCKET_ACCEPT_FAIL		UM_SOCKET_START + 2	//服务器接受客户端连接失败
//对应参数 param1 : nErrCode	对应的套节字错误返回值

#define UM_SOCKET_CONNECT_SUCCESS	UM_SOCKET_START + 3 //客户端连接服务器成功
//对应参数 无
#define UM_SOCKET_CONNECT_FAIL		UM_SOCKET_START + 4 //客户端连接服务器失败
//对应参数 param1 : nErrCode	对应的套节字错误返回值

#define UM_SOCKET_RECVPACKETS_SUCCESS UM_SOCKET_START + 5	//接受TCP数据报成功
//对应参数 param1: const void *pszBuf 接受数据报内容
//		   param2: int nBufLen		  接受数据报大小

#define UM_SOCKET_RECVPACKETS_FAIL	  UM_SOCKET_START + 6	//接受TCP数据报失败
//对应参数 param1 : nErrCode	错误返回值 -1:接收报体失败 
//										   -2:接收包头失败 
//										   -3:包头数据非法
//										   -4:接收数据超时
//										   -5:连接已经被另一端关闭

#define UM_SOCKET_RECVFROM_SUCCESS	  UM_SOCKET_START + 7	//接受UDP数据成功
//对应参数 param1: const void *pszBuf 接受数据报内容
//		   param2: int nBufLen		  接受数据报大小
//		   param3: LPSOCKADDR pSockAddr 数据来源IP信息
#define UM_SOCKET_RECVFROM_FAIL		  UM_SOCKET_START + 8	//接受UDP数据失败
//对应参数 param1 : nErrCode	错误返回值 

#define UM_SOCKET_HTTPGET_SUCCESS		UM_SOCKET_START + 9	//http get 成功
//对应参数 param1: const void *pszBuf 接受数据报内容
//		   param2: int nBufLen		  接受数据报大小
#define UM_SOCKET_HTTPGET_FAIL			UM_SOCKET_START + 10	//http get 失败
//对应参数 param1 : nErrCode	错误返回值 -1:连接服务器失败 
//										   -2:发送数据失败 
//										   -3:接收数据失败
//										   -4:致命错误，找不到包头结束位置
//										   -5:系统错误，申请缓冲区失败	  
//										   -6:接收后续包失败
//										   -7:接收数据超时

#define UM_SOCKET_RECVN_SUCCESS		UM_SOCKET_START + 11	//RECVN成功
//对应参数	param1: const void *lpBuf	接收数据报内容
//			param2: int nBufLen			接收数据报大小

#define UM_SOCKET_RECVN_FAIL		UM_SOCKET_START + 12	//RECVN失败
//对应参数	param1:	nErrCode	错误返回值 -1:系统错误，申请缓冲区失败
//										   -2:接收后续报失败
//										   -3:连接已经被另一端关闭
//										   -4:接收数据超时

#define UM_SOCKET_HTTPRECV_SUCCESS		UM_SOCKET_START + 13	//http server recv成功
//对应参数 param1: const void *pszBuf 接受数据报内容
//		   param2: int nBufLen		  接受数据报大小

#define UM_SOCKET_HTTPRECV_FAIL			UM_SOCKET_START + 14	//http server recv失败
//对应参数 param1 : nErrCode	错误返回值 -1:致命错误，找不到包头结束位置
//										   -2:系统错误，申请缓冲区失败
//										   -3:接收后续包失败
//										   -4:连接已经断开
//										   -5:接受数据超时

#define UM_SOCKET_PROXYRECV_SUCCESS		UM_SOCKET_START + 14	//取代理数据成功
//对应参数 param1: const void *pszBuf 接受数据报内容
//		   param2: int nBufLen		  接受数据报大小

#define UM_SOCKET_PROXYRECV_FAIL		UM_SOCKET_START + 15	//取代理数据失败
//对应参数 param1 : nErrCode	错误返回值 -1:连接目标服务器失败
//										   -2:发送PEER协议数据报失败
//										   -3:接收PEER协议数据报失败
//										   -4:对方连接已经断开
//										   -5:接收数据超时

#define UM_SOCKET_GETHOSTBYNAME_SUCCESS	UM_SOCKET_START + 16	//通过域名取IP成功
//对应参数	param1: const char *pszHostName	域名（用户查询）
//			param2:	 DWORD	dwIP			返回IP

#define UM_SOCKET_GETHOSTBYNAME_FAIL	UM_SOCKET_START + 17	//通过域名取IP失败
//对应参数	param1: int	nErrCode			套节字错误代码

#define UM_SOCKET_END		2000


#define NETWORK			2000

#define NETWORK_INFO(message)		LOG(NETWORK, __INFO, message)
#define NETWORK_EVENT(message)		LOG(NETWORK, __EVENT, message)
#define NETWORK_WARN(message)		LOG(NETWORK, __WARN, message)
#define NETWORK_ERROR(message)		LOG(NETWORK, __ERROR, message)
#define NETWORK_DEBUG(message)		LOG(NETWORK, __DEBUG, message)




#endif