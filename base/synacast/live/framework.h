
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_LIVE_FRAMEWORK_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_LIVE_FRAMEWORK_H_

#include <ppl/os/module.h>
#include <ppl/mswin/crash.h>


//BOOST_STATIC_ASSERT(sizeof(LIVEPARAM) == 2100);

//static WinsockLib s_winsockLib;

/*
void log_impl(unsigned long type, unsigned long level, const char* text)
{
cout << text << endl;
}*/

/*
TS_0001=Framework_Startup
TS_0002=Framework_Cleanup
TS_0003=ExecCommand
TS_0004=CreateAppComponent
TS_0005=DeleteAppComponent
TS_0006=ExCreateSharedMemory
TS_0007=ExOpenSharedMemory
TS_0008=ExCloseSharedMemory
TS_0009=Framework_LoadModule

*/

#define FRWKAPI WINAPI

/// ��������ڲ���
#define CID_SENDSTRING		1001

/// source��ʼ����
#define CID_SOURCEPLAY		2001

/// sourceֹͣ����
#define CID_SOURCESTOP		2002

/// ֪ͨ�ײ����mds��vip�б�
#define CID_UPDATE_MDS_VIP	2010

/// ֪ͨ�ײ����¶�ȡ������Ϣ
#define CID_RELOAD_CONFIGURATION 2011

/// ֪ͨ�ײ����tracker��Ϣ
#define CID_UPDATE_TRACKERS 2012


typedef int (FRWKAPI *FUNC_Framework_Startup)(int& TCPPORT, int& UDPPORT);
typedef int (FRWKAPI *FUNC_Framework_Cleanup)();
typedef int (FRWKAPI *FUNC_ExecCommand)(DWORD nCmdID, void * ParamBuf, DWORD BufSize);
typedef int (FRWKAPI *FUNC_CreateAppComponent)(LPVARPARAM lpParam);
typedef int (FRWKAPI *FUNC_DeleteAppComponent)(GUID ResourceGuid);
typedef int (FRWKAPI *FUNC_ExCreateSharedMemory)(MAPITEM &MapItem);
typedef int (FRWKAPI *FUNC_ExOpenSharedMemory)(MAPITEM &MapItem);
typedef int (FRWKAPI *FUNC_ExCloseSharedMemory)(MAPITEM &MapItem);
typedef int (FRWKAPI *FUNC_Framework_LoadModule)(const char* moduleName);




class LiveFrameworkModule
{
public:
	typedef void (__stdcall *FUNC_SetCrashHandlerMode)(UINT mode);

	static void SetQuietCrashHandlerMode()
	{
		ppl::os::module module(GetModuleHandle(_T("eroc.dll")));
		FUNC_SetCrashHandlerMode fun = reinterpret_cast<FUNC_SetCrashHandlerMode>(module.get_export_item("SetCrashHandlerMode"));
		if (fun)
		{
			fun(CRASH_HANDLER_VERY_QUIET);
		}
	}


	LiveFrameworkModule()
	{
		m_FuncStartup = NULL;
		m_FuncCleanup = NULL;
		m_FuncExecCommand = NULL;
		m_FuncCreateAppComponent = NULL;
		m_FuncDeleteAppComponent = NULL;
		m_FuncLoadModule = NULL;
	}

	ppl::os::loadable_module& GetModule() { return m_Module; }

	bool Init(const tstring& baseDir)
	{
		tstring dllPath = ppl::os::paths::combine(baseDir, _T("kom.dll"));
		if (!m_Module.load(dllPath.c_str()))
		{
			int errcode = ::GetLastError();
			APP_ERROR("failed to load kom.dll, error code is " << errcode << " " << dllPath);
			return false;
		}
		m_FuncStartup = reinterpret_cast<FUNC_Framework_Startup>(m_Module.get_export_item("TS_0001"));
		m_FuncCleanup = reinterpret_cast<FUNC_Framework_Cleanup>(m_Module.get_export_item("TS_0002"));
		m_FuncExecCommand = reinterpret_cast<FUNC_ExecCommand>(m_Module.get_export_item("TS_0003"));
		m_FuncCreateAppComponent = reinterpret_cast<FUNC_CreateAppComponent>(m_Module.get_export_item("TS_0004"));
		m_FuncDeleteAppComponent = reinterpret_cast<FUNC_DeleteAppComponent>(m_Module.get_export_item("TS_0005"));
		m_FuncLoadModule = reinterpret_cast<FUNC_Framework_LoadModule>(m_Module.get_export_item("TS_0009"));
		if (m_FuncStartup == NULL || m_FuncCleanup == NULL || m_FuncExecCommand == NULL || m_FuncCreateAppComponent == NULL || m_FuncDeleteAppComponent == NULL || m_FuncLoadModule == NULL)
		{
			APP_ERROR("failed to retrieve kom API from kom.dll");
			return false;
		}
		SetQuietCrashHandlerMode();
		return true;
	}

	/// ����Framework_Startup
	int Startup(int& tcpPort, int& udpPort)
	{
		if (m_FuncStartup == NULL)
			return -1;
		return m_FuncStartup(tcpPort, udpPort);
	}
	int Cleanup()
	{
		if (m_FuncCleanup == NULL)
			return -1;
		return m_FuncCleanup();
	}
	int ExecCommand(DWORD nCmdID, void * ParamBuf, DWORD BufSize)
	{
		if (m_FuncExecCommand == NULL)
			return -1;
		return m_FuncExecCommand(nCmdID, ParamBuf, BufSize);
	}
	int CreateAppComponent(LPVARPARAM lpParam)
	{
		if (m_FuncCreateAppComponent == NULL)
			return -1;
		return m_FuncCreateAppComponent(lpParam);
	}
	int DeleteAppComponent(GUID ResourceGuid)
	{
		if (m_FuncDeleteAppComponent == NULL)
			return -1;
		return m_FuncDeleteAppComponent(ResourceGuid);
	}
	int LoadModule(const char* moduleName)
	{
		if (m_FuncLoadModule == NULL)
			return -1;
		return m_FuncLoadModule(moduleName);
	}

private:
	ppl::os::loadable_module m_Module;
	FUNC_Framework_Startup m_FuncStartup;
	FUNC_Framework_Cleanup m_FuncCleanup;
	FUNC_ExecCommand m_FuncExecCommand;
	FUNC_CreateAppComponent m_FuncCreateAppComponent;
	FUNC_DeleteAppComponent m_FuncDeleteAppComponent;
	FUNC_Framework_LoadModule m_FuncLoadModule;
};



// daemon messages

/// ������Ϣ
const UINT16 DM_STOP				= 0x0001;

/// ��̬����Tracker ��Ϣ
const UINT16 DM_REFRESH_TRACKER		= 0x0002;

/// ��̬����MDS�б� �� VIP ��Ϣ
const UINT16 DM_REFRESH_MDS_VIP		= 0x0003;

/// ������Ŀ���� ��Ϣ
const UINT16 DM_START_SOURCE_PLAY	= 0x0301;

/// ֹͣ��Ŀ���� ��Ϣ
const UINT16 DM_STOP_SOURCE_PLAY	= 0x0302;

/// Sourceý��ģ������淢����Ϣ
const UINT16 DM_SOURCE_MESSAGE		= 0x0303;

//#define FRWKAPI WINAPI

/// ��������ڲ���
#define CID_SENDSTRING		1001

/// source��ʼ����
#define CID_SOURCEPLAY		2001

/// sourceֹͣ����
#define CID_SOURCESTOP		2002

/// ֪ͨ�ײ����mds��vip�б�
#define CID_UPDATE_MDS_VIP	2010

/// ֪ͨ�ײ����¶�ȡ������Ϣ
#define CID_RELOAD_CONFIGURATION 2011

/// ֪ͨ�ײ����tracker��Ϣ
#define CID_UPDATE_TRACKERS 2012



/// daemon message structs

struct DMS_TRACKER_INFO
{
	UINT32 IP;
	UINT16 Port;
	UINT8 Type;
	UINT8 Reserved;
};

struct DMS_REFRESH_TRACKER
{
	UINT32 TrackerCount;
	DMS_TRACKER_INFO Trackers[1];
};

struct DMS_REFRESH_MDS_VIP
{
	UINT32 MDSCount;
	PEER_ADDRESS MDSList[64];
	UINT32 VIPCount;
	PEER_ADDRESS VIPList[64];
};

struct DMS_START_SOURCE_PLAY
{
	char Url[512];
	UINT32 SourceID;
	UINT32 MaxPieceSize;
};


struct DMS_SOURCE_MESSAGE
{
	UINT16 CommandID;
	UINT32 StatusCode;
	UINT32 SourceID;
	UINT32 StreamTypeID;
};

/// ���ڸ���tracker�б�
struct DMS_PARAM_UPDATE_TRACKERS
{
	GUID ChannelGUID;
	DWORD TrackerCount;
	TRACKERADDR Trackers[100];
};



/************************************************************************/
/*                 ������ReaderҪ�ͽ��淢�͵���Ϣ�Ķ���                 */
/************************************************************************/
#define WM_READER	WM_USER+1 
#define WM_READER_INIT_SUCCED WM_READER+1
#define WM_READER_INIT_FAIDED WM_READER+2	//GUIӦ��ѡ�񲥷���һ��
#define WM_READER_STARTED WM_READER+3
#define WM_READER_STOPED WM_READER+4		//GUIӦ��ѡ�񲥷���һ��
#define WM_READER_PIECE_ERROR WM_READER+5
#define WM_READER_PIECE_WARN WM_READER+6
#define WM_READER_PIECE_END WM_READER+7




#endif