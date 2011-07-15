//2005-8-11 11:00 ��С������
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PUB_MODULE_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PUB_MODULE_H_
#include <winsock2.h>
#include <windows.h>

#define NEED_CORE
#define NEED_NET
//#include "Core.h"
#include "ImportFunc.h"


#define MODAPI WINAPI


//////////////////////////////////////////////////////////////////////////
// ��Ϣӳ��궨��
//////////////////////////////////////////////////////////////////////////

#define MESSAGE_MAP_START \
virtual int HandleMSG(MESSAGE& Message) { \
	switch(Message.Msg) {


#define MAP(Msg,Func) \
	case Msg: Func(Message);\
		break;

#define INHERITED(CLASS)\
	default: CLASS::HandleMSG( Message);

#define MESSAGE_MAP_END \
	} \
	return 0; \
}

//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//�������Ϣ����Ŀ�� OBJ_FRAMEWORK
//////////////////////////////////////////////////////////////////////////
#define OBJ_FRAMEWORK	0
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//Framework��Ϣ����
//////////////////////////////////////////////////////////////////////////
#define UM_FRAMEWORK_START 500

#define UM_ECHO UM_FRAMEWORK_START+1
#define UM_CMD	UM_FRAMEWORK_START+2

#define UM_CMD_SOURCEPLAY	UM_FRAMEWORK_START+3
#define UM_CMD_SOURCESTOP	UM_FRAMEWORK_START+4
#define UM_CMD_RENEWMDSANDVIP	UM_FRAMEWORK_START+5

#define UM_FRAMEWORK_END 999


//////////////////////////////////////////////////////////////////////////
//Ӧ�ó�����Ϣ����,������Ϣ�����ɴ����� 5000+
//////////////////////////////////////////////////////////////////////////
#define UM_USER		5000

//////////////////////////////////////////////////////////////////////////
//Ӧ�����Ͷ���
//////////////////////////////////////////////////////////////////////////
#define P2P_LIVE2	1001
#define P2P_LOGON	1002
#define P2P_LIVE1	1003

//////////////////////////////////////////////////////////////////////////
//��־���Ͷ���
//////////////////////////////////////////////////////////////////////////
//Level ���� �Ѿ���Core.h�ж���
//#define __INFO			1000
//#define __EVENT			2000
//#define __WARN			3000
//#define __ERROR			4000
//#define __DEBUG			5000
//#define __NOSEE			6000
//
//
//#define LOG_INFO(type, message)		LOG(type, __INFO, message)
//#define LOG_EVENT(type, message)	LOG(type, __EVENT, message)
//#define LOG_WARN(type, message)		LOG(type, __WARN, message)
//#define LOG_ERROR(type, message)	LOG(type, __ERROR, message)
//#define LOG_DEBUG(type, message)	LOG(type, __DEBUG, message)
//#define LOG_NOSEE(type, message)	LOG(type, __NOSEE, message)


//ModueID ģ��ID
//#define CORE				1000 ��CORE.H�ж���
//#define NETWORK			2000 ��NETWORK.H�ж���

#define FRAMEWORK			3000

#define PEER				4000
#define SOURCE				4100
#define TRACKER				4200
#define IPPOOL				4300
#define UDPDETECT			4400
#define CONNECTION			4500
#define PEERMANAGER			4600
#define MEDIASERVER			4700

#define READER				6000
#define WMFSDKREADER		6001
#define ASFFILEREADER		6002
#define RMFFFILEREADER		6003
#define ASFHTTPREADER		6004
#define ASFMMSHREADER		6005
#define ASFMMSTREADER		6006
#define ASFRTSPREADER		6007
#define RMFFHTTPREADER		6008
#define RMFFRTSPREADER		6009

#define NETWRITER 7000 


//��CORE.H�ж���
//#define CORE_INFO(message)		LOG(CORE, __INFO, message)
//#define CORE_EVENT(message)		LOG(CORE, __EVENT, message)
//#define CORE_WARN(message)		LOG(CORE, __WARN, message)
//#define CORE_ERROR(message)		LOG(CORE, __ERROR, message)
//#define CORE_DEBUG(message)		LOG(CORE, __DEBUG, message)

//��NETWORK.H�ж���
//#define NETWORK_INFO(message)		LOG(NETWORK, __INFO, message)
//#define NETWORK_EVENT(message)		LOG(NETWORK, __EVENT, message)
//#define NETWORK_WARN(message)		LOG(NETWORK, __WARN, message)
//#define NETWORK_ERROR(message)		LOG(NETWORK, __ERROR, message)
//#define NETWORK_DEBUG(message)		LOG(NETWORK, __DEBUG, message)

#define FRAMEWORK_INFO(message)		LOG(FRAMEWORK, __INFO, message)
#define FRAMEWORK_EVENT(message)		LOG(FRAMEWORK, __EVENT, message)
#define FRAMEWORK_WARN(message)		LOG(FRAMEWORK, __WARN, message)
#define FRAMEWORK_ERROR(message)		LOG(FRAMEWORK, __ERROR, message)
#define FRAMEWORK_DEBUG(message)		LOG(FRAMEWORK, __DEBUG, message)

#define PEER_INFO(message)		LOG(PEER, __INFO, message)
#define PEER_EVENT(message)		LOG(PEER, __EVENT, message)
#define PEER_WARN(message)		LOG(PEER, __WARN, message)
#define PEER_ERROR(message)		LOG(PEER, __ERROR, message)
#define PEER_DEBUG(message)		LOG(PEER, __DEBUG, message)

#define SOURCE_INFO(message)		LOG(SOURCE, __INFO, message)
#define SOURCE_EVENT(message)		LOG(SOURCE, __EVENT, message)
#define SOURCE_WARN(message)		LOG(SOURCE, __WARN, message)
#define SOURCE_ERROR(message)		LOG(SOURCE, __ERROR, message)
#define SOURCE_DEBUG(message)		LOG(SOURCE, __DEBUG, message)

#define TRACKER_INFO(message)		LOG(TRACKER, __INFO, message)
#define TRACKER_EVENT(message)		LOG(TRACKER, __EVENT, message)
#define TRACKER_WARN(message)		LOG(TRACKER, __WARN, message)
#define TRACKER_ERROR(message)		LOG(TRACKER, __ERROR, message)
#define TRACKER_DEBUG(message)		LOG(TRACKER, __DEBUG, message)

#define IPPOOL_INFO(message)		LOG(IPPOOL, __INFO, message)
#define IPPOOL_EVENT(message)		LOG(IPPOOL, __EVENT, message)
#define IPPOOL_WARN(message)		LOG(IPPOOL, __WARN, message)
#define IPPOOL_ERROR(message)		LOG(IPPOOL, __ERROR, message)
#define IPPOOL_DEBUG(message)		LOG(IPPOOL, __DEBUG, message)

#define UDPDETECT_INFO(message)		LOG(UDPDETECT, __INFO, message)
#define UDPDETECT_EVENT(message)		LOG(UDPDETECT, __EVENT, message)
#define UDPDETECT_WARN(message)		LOG(UDPDETECT, __WARN, message)
#define UDPDETECT_ERROR(message)		LOG(UDPDETECT, __ERROR, message)
#define UDPDETECT_DEBUG(message)		LOG(UDPDETECT, __DEBUG, message)

#define CONNECTION_INFO(message)		LOG(CONNECTION, __INFO, message)
#define CONNECTION_EVENT(message)		LOG(CONNECTION, __EVENT, message)
#define CONNECTION_WARN(message)		LOG(CONNECTION, __WARN, message)
#define CONNECTION_ERROR(message)		LOG(CONNECTION, __ERROR, message)
#define CONNECTION_DEBUG(message)		LOG(CONNECTION, __DEBUG, message)

#define PEERMANAGER_INFO(message)		LOG(PEERMANAGER, __INFO, message)
#define PEERMANAGER_EVENT(message)		LOG(PEERMANAGER, __EVENT, message)
#define PEERMANAGER_WARN(message)		LOG(PEERMANAGER, __WARN, message)
#define PEERMANAGER_ERROR(message)		LOG(PEERMANAGER, __ERROR, message)
#define PEERMANAGER_DEBUG(message)		LOG(PEERMANAGER, __DEBUG, message)

#define MEDIASERVER_INFO(message)		LOG(MEDIASERVER, __INFO, message)
#define MEDIASERVER_EVENT(message)		LOG(MEDIASERVER, __EVENT, message)
#define MEDIASERVER_WARN(message)		LOG(MEDIASERVER, __WARN, message)
#define MEDIASERVER_ERROR(message)		LOG(MEDIASERVER, __ERROR, message)
#define MEDIASERVER_DEBUG(message)		LOG(MEDIASERVER, __DEBUG, message)

#define READER_INFO(message)		LOG(READER, __INFO, message)
#define READER_EVENT(message)		LOG(READER, __EVENT, message)
#define READER_WARN(message)		LOG(READER, __WARN, message)
#define READER_ERROR(message)		LOG(READER, __ERROR, message)
#define READER_DEBUG(message)		LOG(READER, __DEBUG, message)

#define WMFSDK_INFO(message)		LOG(WMFSDKREADER, __INFO, message)
#define WMFSDK_EVENT(message)		LOG(WMFSDKREADER, __EVENT, message)
#define WMFSDK_WARN(message)		LOG(WMFSDKREADER, __WARN, message)
#define WMFSDK_ERROR(message)		LOG(WMFSDKREADER, __ERROR, message)
#define WMFSDK_DEBUG(message)		LOG(WMFSDKREADER, __DEBUG, message)

#define ASFFILE_INFO(message)		LOG(ASFFILEREADER, __INFO, message)
#define ASFFILE_EVENT(message)		LOG(ASFFILEREADER, __EVENT, message)
#define ASFFILE_WARN(message)		LOG(ASFFILEREADER, __WARN, message)
#define ASFFILE_ERROR(message)		LOG(ASFFILEREADER, __ERROR, message)
#define ASFFILE_DEBUG(message)		LOG(ASFFILEREADER, __DEBUG, message)

#define RMFFFILE_INFO(message)		LOG(RMFFFILEREADER, __INFO, message)
#define RMFFFILE_EVENT(message)		LOG(RMFFFILEREADER, __EVENT, message)
#define RMFFFILE_WARN(message)		LOG(RMFFFILEREADER, __WARN, message)
#define RMFFFILE_ERROR(message)		LOG(RMFFFILEREADER, __ERROR, message)
#define RMFFFILE_DEBUG(message)		LOG(RMFFFILEREADER, __DEBUG, message)

#define ASFHTTP_INFO(message)		LOG(ASFHTTPREADER, __INFO, message)
#define ASFHTTP_EVENT(message)		LOG(ASFHTTPREADER, __EVENT, message)
#define ASFHTTP_WARN(message)		LOG(ASFHTTPREADER, __WARN, message)
#define ASFHTTP_ERROR(message)		LOG(ASFHTTPREADER, __ERROR, message)
#define ASFHTTP_DEBUG(message)		LOG(ASFHTTPREADER, __DEBUG, message)

#define ASFMMSH_INFO(message)		LOG(ASFMMSHREADER, __INFO, message)
#define ASFMMSH_EVENT(message)		LOG(ASFMMSHREADER, __EVENT, message)
#define ASFMMSH_WARN(message)		LOG(ASFMMSHREADER, __WARN, message)
#define ASFMMSH_ERROR(message)		LOG(ASFMMSHREADER, __ERROR, message)
#define ASFMMSH_DEBUG(message)		LOG(ASFMMSHREADER, __DEBUG, message)

#define ASFMMST_INFO(message)		LOG(ASFMMSTREADER, __INFO, message)
#define ASFMMST_EVENT(message)		LOG(ASFMMSTREADER, __EVENT, message)
#define ASFMMST_WARN(message)		LOG(ASFMMSTREADER, __WARN, message)
#define ASFMMST_ERROR(message)		LOG(ASFMMSTREADER, __ERROR, message)
#define ASFMMST_DEBUG(message)		LOG(ASFMMSTREADER, __DEBUG, message)

#define ASFRTSP_INFO(message)		LOG(ASFRTSPREADER, __INFO, message)
#define ASFRTSP_EVENT(message)		LOG(ASFRTSPREADER, __EVENT, message)
#define ASFRTSP_WARN(message)		LOG(ASFRTSPREADER, __WARN, message)
#define ASFRTSP_ERROR(message)		LOG(ASFRTSPREADER, __ERROR, message)
#define ASFRTSP_DEBUG(message)		LOG(ASFRTSPREADER, __DEBUG, message)

#define RMFFHTTP_INFO(message)		LOG(RMFFHTTPREADER, __INFO, message)
#define RMFFHTTP_EVENT(message)		LOG(RMFFHTTPREADER, __EVENT, message)
#define RMFFHTTP_WARN(message)		LOG(RMFFHTTPREADER, __WARN, message)
#define RMFFHTTP_ERROR(message)		LOG(RMFFHTTPREADER, __ERROR, message)
#define RMFFHTTP_DEBUG(message)		LOG(RMFFHTTPREADER, __DEBUG, message)

#define RMFFRTSP_INFO(message)		LOG(RMFFRTSPREADER, __INFO, message)
#define RMFFRTSP_EVENT(message)		LOG(RMFFRTSPREADER, __EVENT, message)
#define RMFFRTSP_WARN(message)		LOG(RMFFRTSPREADER, __WARN, message)
#define RMFFRTSP_ERROR(message)		LOG(RMFFRTSPREADER, __ERROR, message)
#define RMFFRTSP_DEBUG(message)		LOG(RMFFRTSPREADER, __DEBUG, message)

#define NETWRITER_INFO(message) LOG(NETWRITER, __INFO, message) 
#define NETWRITER_EVENT(message) LOG(NETWRITER, __EVENT, message) 
#define NETWRITER_WARN(message) LOG(NETWRITER, __WARN, message) 
#define NETWRITER_ERROR(message) LOG(NETWRITER, __ERROR, message) 
#define NETWRITER_DEBUG(message) LOG(NETWRITER, __DEBUG, message) 

//////////////////////////////////////////////////////////////////////////
//����ʹ�õ�EVENT��װ
//////////////////////////////////////////////////////////////////////////
class CEvent
{
public:
	CEvent(BOOL bInitiallyState = FALSE, BOOL bManualReset = FALSE,
		LPCTSTR lpszNAme = NULL, LPSECURITY_ATTRIBUTES lpsaAttribute = NULL)
	{
		m_Handle = ::CreateEvent( lpsaAttribute, bManualReset, bInitiallyState, lpszNAme );
	}
	
	~CEvent()
	{
		::CloseHandle(m_Handle);
	}

	void SetEvent()
	{
		::SetEvent(m_Handle);
	}
	
	void ResetEvent()
	{
		::ResetEvent(m_Handle);
	}
	HANDLE m_Handle;
protected:
private:
};


//////////////////////////////////////////////////////////////////////////
//����ʹ�õ�CriticalSection��װ
//////////////////////////////////////////////////////////////////////////
class CCriticalSection
{
protected:
private:
	CRITICAL_SECTION m_section;

public:
	CCriticalSection()
	{
		InitializeCriticalSection(&m_section);
	}

	~CCriticalSection()
	{
		DeleteCriticalSection(&m_section);
	}
	
	void Lock()
	{
		EnterCriticalSection(&m_section);
	}

	void Unlock()
	{
		LeaveCriticalSection(&m_section);
	}
	
};

const DWORD VALID_OBJECT_TAG	= 0xfafbfcfd;
const DWORD INVALID_OBJECT_TAG	= 0xfafafafa;

class CBasicObjectInterface
{
	DWORD m_ObjectTag;
private:
	int OnEcho(MESSAGE& Message){return 0;};

public:
	CBasicObjectInterface() : m_ObjectTag(VALID_OBJECT_TAG)
	{}
	virtual ~CBasicObjectInterface()
	{
		CORE_ClearTargetObjectMessage(this);
		m_ObjectTag = INVALID_OBJECT_TAG;
	}
	bool IsObjectValid() const
	{
		return m_ObjectTag == VALID_OBJECT_TAG;
	}
	DWORD GetObjectTag() const
	{
		return m_ObjectTag;
	}

	MESSAGE_MAP_START
		MAP(UM_ECHO, OnEcho);
	MESSAGE_MAP_END 
};


#pragma warning(disable : 4200)

typedef struct _PEERADDR{
	DWORD	IP;
	WORD	TCPPort;
	WORD	UDPPort;
}PEERADDR, *LPPEERADDR;

typedef struct _SOURCEPLAYINFO {
	GUID	ChannelGUID;
	char	URL[512];
	DWORD	ID;
	DWORD	hWnd;
	DWORD	MaxPieceSize;
} SOURCEPLAYINFO, *LPSOURCEPLAYINFO;


typedef struct _MDSANDVIPINFO {
	GUID	ChannelGUID;

	DWORD	MDSCount;	//MDS�б��С
	PEERADDR MDSList[64]; //MDS�б�

	DWORD	VIPCount;	//VIP�б��С
	PEERADDR VIPList[64]; //VIP�б�
} MDSANDVIPINFO, *LPMDSANDVIPINFO;


typedef struct _NETTYPE {
	WORD	Region;
	WORD	ISP;
	WORD	Area;
	WORD	Unit;
}NETTYPE, *LPNETTYPE;

typedef struct _APPSHOWITEM{
	GUID	ResourceGuid;
	DWORD	AppType;
	DWORD	Param[3];
}APPSHOWITEM, *LPAPPSHOWITEM;


//ϵͳȫ����Ϣ
typedef struct _SYSINFO {
	DWORD	OSVersion;				//����ϵͳ
	DWORD	SoftwareVersion;		//����汾
	DWORD	ProtocolVersion;		//Э��汾

	WORD	FrameWork_STATE;		//��Ϣ״̬����FrameWork����ʱ����ΪFFFF������ģ�鷢�ָ�ֵ��ΪFFFFʱ��Ҳ��������

	WORD	TcpPort;				//����������TCP�����˿�
	WORD	UdpPort;				//����������UDP�����˿�
	DWORD	ExtenalIP;				//���ɼ���IP��ַ
	WORD	ExtenalTcpPort;			//���ɼ���TCP�����˿ڣ�Mapӳ��˿ڻ�UPNP�˿�
	WORD	ExtenalUdpPort;			//���ɼ���UDP�����˿ڣ�Mapӳ��˿ڻ�UPNP�˿�
	DWORD	CommHwnd;				//��ϢͨѶ���ھ��
	WORD	MaxUpBandwidth;			//����ϴ�������ʱ�ƺ�������
	WORD	MaxDownBandwidth;		//������ش�����ʱ�ƺ�������
	WORD	MaxConnectPendingCount;	//���ͬʱ����Connect�ĸ���,XP��2003����
	WORD	MaxAppPeerCount;		//ÿ��Ӧ�ã�Ƶ����������Ӹ���
	GUID	PeerGUID;				//�û�GUID���ɱ������
	DWORD	PeerID;					//�û����
	char	PeerPass[32];			//�û���¼������
	NETTYPE	NetType;				//��������
	DWORD	NetID;					//������
	WORD	NetState;				//����������
	WORD	LocalIPCount;
	DWORD	LocalIPs[32];
	

	int			AppCount;				//�������ٸ�����
	APPSHOWITEM AppItems[32];			//ÿ���������Ϣ

}SYSINFO, *LPSYSINFO;

//TCP����UDP���������ģ��UDP��Ϣ��ʽ����
//
//lpTarget:�ɹ���ʧ�ܵ���Ϣ���͵�Ŀ�����
//IPADDR,TCPPORT:���ӶԷ���IP��ַ��˿�
//TIMEOUT:��ʱ
//Buf ,BufSize:���͵����ݵĵ�ַ�볤��
typedef void (*LPTCPProxyUDP) (CBasicObjectInterface *lpTarget, DWORD IPAddr, DWORD TCPPort, DWORD TimeOut, void *Buf, int BufSize);

//���������ṹ
typedef struct _VARPARAM{
	int		nSize;			//�����ṹ�Ĵ�С

	DWORD	hWnd;			//ͨѶ��Window���������˴���д������Ϣ�Ĵ��ھ��

	char	Comment[128];	//˵����Ϣ
	int		AppType;		//Ӧ������
	GUID	ResourceGuid;	//��ԴGUID

	union{
		DWORD Reserved[32]; //UI���Բ�����Щ��Ϣ
		struct { //��FrameWork��д
			int		CommUDP;		//ͨѶ��UDP�׽���
			LPSYSINFO lpSysInfo;	//ϵͳ��Ϣ�ṹָ��
			LPTCPProxyUDP TCPProxyUDP; //TCP��������ַ
		}Framework_Info;
	};
	/////////////////////////////////////////////////����Ϊ��׼����

	char	Padding[0];		//����ṹ��������������й�
}VARPARAM, *LPVARPARAM;


#define TRACKER_TYPE_UDP 0
#define TRACKER_TYPE_TCP 1

typedef struct _TRACKERADDR{
	DWORD	IP;
	WORD	Port;
	BYTE	Type;	//TRACKER_TYPE_UDP or TRACKER_TYPE_TCP
	BYTE	Reserved;
	DWORD	PeerIP;
	WORD	PeerTcpPort;
	WORD	PeerUdpPort;
}TRACKERADDR, *LPTRACKERADDR;


typedef struct _LIVEPARAM{
	int		nSize;			//�����ṹ�Ĵ�С

	DWORD	hWnd;			//ͨѶ��Window���������˴���д������Ϣ�Ĵ��ھ��

	char	Comment[128];	//˵����Ϣ
	int		AppType;		//Ӧ������
	GUID	ResourceGuid;	//��ԴGUID

	union{
		DWORD Reserved[32]; //UI���Բ�����Щ��Ϣ
		struct { //��FrameWork��д
			int		CommUDP;		//ͨѶ��UDP�׽���
			LPSYSINFO lpSysInfo;	//ϵͳ��Ϣ�ṹָ��
			LPTCPProxyUDP TCPProxyUDP; //TCP��������ַ
			int RealAppType;
			int TestVersion;

		}Framework_Info;
	};
	/////////////////////////////////////////////////����Ϊ��׼����

	WORD	Channel_ID;		//Ƶ��ID�ţ�����ԴGUID��Ӧ
	WORD	ICP_ID;			//ICP��ţ���������������
	char	ICPUSER_ID[32];	//�û���ICP�����û���
	WORD	ICPUSER_STATE;	//�û�״̬��ICP����
	char	Cookie[32];		//cooikeֵ

	char	Username[64];
	char	Password[64];

	DWORD	Crypt_TYPE;		//��������: 0 δ���� 1 ������COOKIE ....
	char	SEKEY[64];		//��Կ
	char	AUTH_Info[256];	//��֤��Ϣ

	WORD	TrackerCount;	//Tracker�б��С
	TRACKERADDR TrackerList[16]; //Tracker�б�

	DWORD	MDSCount;	//MDS�б��С
	PEERADDR MDSList[64]; //MDS�б�

	DWORD	VIPCount;	//VIP�б��С
	PEERADDR VIPList[64]; //VIP�б�

}LIVEPARAM, *LPLIVEPARAM;


#define ROOT_TYPE_UDP 0
#define ROOT_TYPE_TCP 1

#define ROOT_TYPE_KEEPALIVE	0
#define ROOT_TYPE_REGISTER	1

typedef struct _ROOTADDR{
	DWORD	IP;
	WORD	Port;
	BYTE	SocketType;	//ROOT_TYPE_UDP or ROOT_TYPE_TCP
	BYTE	ActionType; //ROOT_TYPE_KEEPALIVE or ROOT_TYPE_REGISTER
}ROOTADDR, *LPROOTADDR;


typedef struct _LOGONPARAM{
	int		nSize;			//�����ṹ�Ĵ�С

	DWORD	hWnd;			//ͨѶ��Window���������˴���д������Ϣ�Ĵ��ھ��

	char	Comment[128];	//˵����Ϣ
	int		AppType;		//Ӧ������
	GUID	ResourceGuid;	//��ԴGUID

	union{
		DWORD Reserved[32]; //UI���Բ�����Щ��Ϣ
		struct { //��FrameWork��д
			int		CommUDP;		//ͨѶ��UDP�׽���
			LPSYSINFO lpSysInfo;	//ϵͳ��Ϣ�ṹָ��
			LPTCPProxyUDP TCPProxyUDP; //TCP��������ַ
		}Framework_Info;
	};
	/////////////////////////////////////////////////����Ϊ��׼����

	WORD	RootCount;	//Root�б��С
	ROOTADDR RootList[0]; //Root�б�
}LOGONPARAM, *LPLOGONPARAM;



//Ӧ�û�����Ϣ
//typedef struct _APPSTDINFO {
//	WORD	APP_TYPE;				//Ӧ������
//
//	WORD	APP_STATE;				//��Ϣ״̬����AppӦ�ý���ʱ����ΪFFFF������ģ�鷢�ָ�ֵ��ΪFFFFʱ��Ҳ��������
//
//	GUID	ResourceGuid;			//��ԴGUID
//	char	COMMENT[128];			//Ӧ��˵����Ϣ
//}APPSTDINFO, *LPAPPSTDINFO;


#pragma warning(default : 4200)

#define SYSINFO_TAG "OS_SYS_INFO"
#define SOURCE_LIVEINFO_TAG "OS_SYS_INFO"
#define PEER_LIVEINFO_TAG "OS_SYS_META"


CBasicObjectInterface *	MODAPI	CreateComponent(LPVARPARAM lpParam);
int MODAPI DeleteComponent(CBasicObjectInterface * obj);

typedef CBasicObjectInterface * (MODAPI *LPCREATECOMPONENT)(LPVARPARAM Param);
typedef int (MODAPI *LPDELETECOMPONENT)(CBasicObjectInterface * obj);

typedef struct _MODULEINTERFACE{
	DWORD Version;
	int AppType;
	LPCREATECOMPONENT CreateComponent;
	LPDELETECOMPONENT DeleteComponent;
} MODULEINTERFACE, *LPMODULEINTERFACE;

typedef	void	(MODAPI *LPGETMODULEINTERFACE)(LPMODULEINTERFACE pModuleInterface);




#endif