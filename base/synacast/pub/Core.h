//2005-8-11 11:00 ��С������
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PUB_CORE_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PUB_CORE_H_

#define COREAPI  WINAPI

#ifdef NEED_LOG
  #pragma message( "===LOG===" )
  #include <sstream>
#endif


//��ʹ�ú�POOL_MALLOC����Ŀͻ����򣬱�������˲��ֵĵ���
#ifdef _DEBUG
#define POOL_MALLOC(size) pool_malloc(size,__FILE__,__LINE__)
#define POOL_FREE(pVoid)  pool_free(pVoid,__FILE__,__LINE__)
#define POOL_CLEAR        pool_clear();
#else
#define POOL_MALLOC(size) pool_malloc(size)
#define POOL_FREE(pVoid)  pool_free(pVoid)
#define POOL_CLEAR        pool_clear();
#endif

//////////////////////////////////////////////////////////////////////////
//���ܣ�����һ��ָ����С���ڴ��
//������size		------ �û���Ҫ������ڴ�Ĵ�С
//      lpszFileName------ ���䶯���������Ǹ��ļ�
//      nLine       ------ ���䶯�����ļ��е���λ��
//
extern void * pool_malloc(int size,const char* lpszFileName, int nLine);
typedef void* (*PPOOL_MALLOC) (int size,const char* lpszFileName, int nLine);

//////////////////////////////////////////////////////////////////////////
//���ܣ��ͷ�һ��ָ�����ڴ��
//������lpbuf        ------ �ͷ��ڴ�����ʼλ��
//
extern void pool_free(void * lpbuf,const char* lpszFileName, int nLine);
typedef void (*PPOOL_FREE) (void * lpbuf,const char* lpszFileName, int nLine);

//////////////////////////////////////////////////////////////////////////
//���ܣ��ͷ��ڴ��
//
extern void pool_clear();
typedef void (*PPOOL_CLEAR) ();

//////////////////////////////////////////////////////////////////////////
//��Ϣ������
//

typedef void * HOBJ;
typedef unsigned int UINT;

typedef struct _MESSAGE{
	HOBJ	Target_obj;		//��Ϣ�����ߵľ��|ָ��
	HOBJ	Sender_obj;		//��Ϣ�����ߵľ��|ָ��
	UINT	Msg;			//��Ϣ���
	UINT	PARAM_1;		//�ĸ�����
	UINT	PARAM_2;
	UINT	PARAM_3;
	UINT	PARAM_4;
	UINT	Result;			//��Ϣ������ɱ�־
} MESSAGE, *LPMESSAGE;

//////////////////////////////////////////////////////////////////////////
//���ܣ�����һ����Ϣ��ʵ�����ǽ���Ϣ����������
//
//
extern int	SendObjectMessage( HOBJ Target_obj, HOBJ Sender_obj, UINT Msg,
					   UINT PARAM_1, UINT PARAM_2, UINT PARAM_3, UINT PARAM_4);
typedef int (*PSENDOBJECTMESSAGE)( HOBJ Target_obj, HOBJ Sender_obj, UINT Msg,
					   UINT PARAM_1, UINT PARAM_2, UINT PARAM_3, UINT PARAM_4);

//////////////////////////////////////////////////////////////////////////
//���ܣ���ȡһ����Ϣ
//
//
extern int GetObjectMessage(LPMESSAGE lpMsg);
typedef int (*PGETOBJECTMESSAGE)(LPMESSAGE lpMsg);

//////////////////////////////////////////////////////////////////////////
//���ܣ���������д洢��Ԫ�ؽڵ�
//
//
extern void ClearAllObjectMessage();
typedef void (*PCLEARALLOBJECTMESSAGE) ();

//////////////////////////////////////////////////////////////////////////
//���ܣ������Ϣ�ڴ��
//
//
extern void CleanMemPool();
typedef void (*PCLEANMEMPOOL)();

//////////////////////////////////////////////////////////////////////////
//���ܣ������������ָ��Ŀ�������ص���Ϣ
//
//
extern void ClearTargetObjectMessage( HOBJ Target_obj);
typedef void (*PCLEARTARGETOBJECTMESSAGE) ( HOBJ Target_obj);

//////////////////////////////////////////////////////////////////////////
//������������Ϣ����Ŀ
//
//
extern int GetMessageCount();
typedef int (*PGETMESSAGECOUNT)();

extern int WaitEvent(HANDLE hEvent, DWORD dwMilliseconds);
typedef int (*PWAITEVENT)(HANDLE hEvent, DWORD dwMilliseconds);

extern int SetQuitFlag();
typedef int (*PSETQUITFALG)();

extern void log(DWORD level, DWORD type, char *cText);
typedef void (*PLOG)(DWORD level, DWORD type, char *cText);

extern void vlog(DWORD level, DWORD type, char* format, ...);
typedef void (*PVLOG)(DWORD level, DWORD type, char* format, ...);

extern void SetLogLevel(DWORD Loglevel);
typedef void (*PSetLogLevel)(DWORD Loglevel);

extern void LogOn(DWORD LogType);
typedef void (*PLogOn)(DWORD LogType);

extern void LogOff(DWORD LogType);
typedef void (*PLogOff)(DWORD LogType);


extern UINT InsertTimer(UINT uElapse, UINT Flag, HOBJ Target_obj, HOBJ Sender_obj, UINT Msg,
					   UINT PARAM_1, UINT PARAM_2, UINT PARAM_3, UINT PARAM_4);
typedef UINT (*PInsertTimer)(UINT uElapse, UINT Flag, HOBJ Target_obj, HOBJ Sender_obj, UINT Msg,
					   UINT PARAM_1, UINT PARAM_2, UINT PARAM_3, UINT PARAM_4);

extern void RemoveTimer(UINT TimerID);
typedef void (*PRemoveTimer)(UINT TimerID);


typedef struct _MAPITEM {
	char	MemName[256];
	HANDLE	m_hMapping;
	void *	m_lpData;
	DWORD	size;
} MAPITEM, *LPMAPITEM;

extern int CreateSharedMemory(MAPITEM &item);
typedef int (*PCreateSharedMemory)(MAPITEM &item);

extern int OpenSharedMemory(MAPITEM &item);
typedef int (*POpenSharedMemory)(MAPITEM &item);

extern void CloseSharedMemory(MAPITEM &item);
typedef void (*PCloseSharedMemory)(MAPITEM &item);

extern void RemoveTimerByTarget(HOBJ Taget_obj);
typedef void (*PRemoveTimerByTarget)(HOBJ Taget_obj);


typedef struct _COREINTERFACE{
	
	//�汾���Ʋ���
	DWORD           Version;

	//�ڴ���Ĳ���
	PPOOL_MALLOC    Pool_malloc;
	PPOOL_FREE      Pool_free;
	PPOOL_CLEAR     Pool_clear;

	//��Ϣ���Ĳ���
	PSENDOBJECTMESSAGE SendObjectMessage;
	PGETOBJECTMESSAGE  GetObjectMessage;
	PCLEARALLOBJECTMESSAGE ClearAllObjectMessage;
	PCLEANMEMPOOL      CleanMemPool;
	PCLEARTARGETOBJECTMESSAGE ClearTargetObjectMessage;
	PGETMESSAGECOUNT GetMessageCount;
	PWAITEVENT WaitEvent;
	PSETQUITFALG SetQuitFlag;

	PLOG log;
	PVLOG vlog;
	PSetLogLevel SetLogLevel;
	PLogOn LogOn;
	PLogOff LogOff;

	PInsertTimer InsertTimer;
	PRemoveTimer RemoveTimer;

	PCreateSharedMemory CreateSharedMemory;
	POpenSharedMemory OpenSharedMemory;
	PCloseSharedMemory CloseSharedMemory;

	PRemoveTimerByTarget RemoveTimerByTarget;
}COREINTERFACE,*LPCOREINTERFACE;


//////////////////////////////////////////////////////////////////////////
//���ܣ��ڵ�ǰִ��ģ������Ŀ¼��Ѱ��ĳ���ļ�������
//������lpFileName ------- ��Ҫ���ص��ļ�����
//ע�⣺���ڴ˺�����Ҫ��LoadLibrary֮ǰ�����ã�������Ҫ��ͷ�ļ��ж���Ϊ��ľ�̬��Ա����
//���÷�ʽ��HMODULE hmod=CMoudleManager::LoadLibraryInSameDir("test.dll");
class CMoudleManager
{
public:
	static HMODULE CMoudleManager::LoadLibraryInSameDir( LPCTSTR lpFileName )
	{ 
		MEMORY_BASIC_INFORMATION mbi;
		HMODULE self;
		TCHAR buffer[1024];
		int n,i;

		VirtualQuery(&LoadLibraryInSameDir, &mbi, sizeof(mbi));
		if (mbi.State != MEM_COMMIT)
			return 0;
		else
			self = HMODULE(mbi.AllocationBase);

		n=GetModuleFileName(self, buffer, 1024);
		if (n==0) return 0;

		for (i=n-1;i>0;i--)
			if (buffer[i]=='\\' ) break;

		if (i<0) return 0;

		buffer[i+1]=0;

		_tcscat(buffer, lpFileName);

		return LoadLibrary(buffer);
	}
};



//LOG�궨��
#ifdef NEED_CORE
		extern COREINTERFACE CoreInterface;
		#define fact_log CoreInterface.log
#else
		#define fact_log log
#endif


#ifdef NEED_LOG
#define LOG(level, type, message) do{\
	std::ostringstream oss; \
	oss << message; \
	fact_log(level, type, (char *)oss.str().c_str()); } while(false)
#else

#define LOG(level, type, message)

#endif


//////////////////////////////////////////////////////////////////////////
//��־���Ͷ���
//////////////////////////////////////////////////////////////////////////
//Level ����
#define __NOSEE			500
#define __DEBUG			1000
#define __INFO			2000
#define __WARN			3000
#define __EVENT			4000
#define __ERROR			5000


#define LOG_INFO(type, message)		LOG(type, __INFO, message)
#define LOG_EVENT(type, message)	LOG(type, __EVENT, message)
#define LOG_WARN(type, message)		LOG(type, __WARN, message)
#define LOG_ERROR(type, message)	LOG(type, __ERROR, message)
#define LOG_DEBUG(type, message)	LOG(type, __DEBUG, message)
#define LOG_NOSEE(type, message)	LOG(type, __NOSEE, message)


//CORE��ʹ�õĵ��Զ���
#define CORE				1000 

#define CORE_INFO(message)		LOG(CORE, __INFO, message)
#define CORE_EVENT(message)		LOG(CORE, __EVENT, message)
#define CORE_WARN(message)		LOG(CORE, __WARN, message)
#define CORE_ERROR(message)		LOG(CORE, __ERROR, message)
#define CORE_DEBUG(message)		LOG(CORE, __DEBUG, message)



//////////////////////////////////////////////////////////////////////////
//���ܣ��ڴ�������ģ��ĵ�������
//������pPoolInterface ------- �ڴ���亯���ṹ���ָ��
//
extern void COREAPI TS_XXXX(LPCOREINTERFACE pPoolInterface);

#endif