//2005-8-11 11:00 张小兵定标
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PUB_CORE_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PUB_CORE_H_

#define COREAPI  WINAPI

#ifdef NEED_LOG
  #pragma message( "===LOG===" )
  #include <sstream>
#endif


//在使用宏POOL_MALLOC必须的客户程序，必须包含此部分的调用
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
//功能：分配一个指定大小的内存块
//参数：size		------ 用户需要分配的内存的大小
//      lpszFileName------ 分配动作发生在那个文件
//      nLine       ------ 分配动作在文件中的行位置
//
extern void * pool_malloc(int size,const char* lpszFileName, int nLine);
typedef void* (*PPOOL_MALLOC) (int size,const char* lpszFileName, int nLine);

//////////////////////////////////////////////////////////////////////////
//功能：释放一个指定的内存块
//参数：lpbuf        ------ 释放内存块的起始位置
//
extern void pool_free(void * lpbuf,const char* lpszFileName, int nLine);
typedef void (*PPOOL_FREE) (void * lpbuf,const char* lpszFileName, int nLine);

//////////////////////////////////////////////////////////////////////////
//功能：释放内存池
//
extern void pool_clear();
typedef void (*PPOOL_CLEAR) ();

//////////////////////////////////////////////////////////////////////////
//消息处理部分
//

typedef void * HOBJ;
typedef unsigned int UINT;

typedef struct _MESSAGE{
	HOBJ	Target_obj;		//消息接收者的句柄|指针
	HOBJ	Sender_obj;		//消息发送者的句柄|指针
	UINT	Msg;			//消息编号
	UINT	PARAM_1;		//四个参数
	UINT	PARAM_2;
	UINT	PARAM_3;
	UINT	PARAM_4;
	UINT	Result;			//消息处理完成标志
} MESSAGE, *LPMESSAGE;

//////////////////////////////////////////////////////////////////////////
//功能：发送一个消息，实际上是将消息放入容器中
//
//
extern int	SendObjectMessage( HOBJ Target_obj, HOBJ Sender_obj, UINT Msg,
					   UINT PARAM_1, UINT PARAM_2, UINT PARAM_3, UINT PARAM_4);
typedef int (*PSENDOBJECTMESSAGE)( HOBJ Target_obj, HOBJ Sender_obj, UINT Msg,
					   UINT PARAM_1, UINT PARAM_2, UINT PARAM_3, UINT PARAM_4);

//////////////////////////////////////////////////////////////////////////
//功能：读取一个消息
//
//
extern int GetObjectMessage(LPMESSAGE lpMsg);
typedef int (*PGETOBJECTMESSAGE)(LPMESSAGE lpMsg);

//////////////////////////////////////////////////////////////////////////
//功能：清空容器中存储的元素节点
//
//
extern void ClearAllObjectMessage();
typedef void (*PCLEARALLOBJECTMESSAGE) ();

//////////////////////////////////////////////////////////////////////////
//功能：清空消息内存池
//
//
extern void CleanMemPool();
typedef void (*PCLEANMEMPOOL)();

//////////////////////////////////////////////////////////////////////////
//功能：清除容器中与指定目标对象相关的消息
//
//
extern void ClearTargetObjectMessage( HOBJ Target_obj);
typedef void (*PCLEARTARGETOBJECTMESSAGE) ( HOBJ Target_obj);

//////////////////////////////////////////////////////////////////////////
//返回容器中消息的数目
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
	
	//版本控制部分
	DWORD           Version;

	//内存核心部分
	PPOOL_MALLOC    Pool_malloc;
	PPOOL_FREE      Pool_free;
	PPOOL_CLEAR     Pool_clear;

	//消息核心部分
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
//功能：在当前执行模块所在目录下寻找某个文件并加载
//参数：lpFileName ------- 需要加载的文件名称
//注意：由于此函数需要在LoadLibrary之前被调用，所以需要在头文件中定义为类的静态成员函数
//调用方式：HMODULE hmod=CMoudleManager::LoadLibraryInSameDir("test.dll");
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



//LOG宏定义
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
//日志类型定义
//////////////////////////////////////////////////////////////////////////
//Level 级别
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


//CORE中使用的调试定义
#define CORE				1000 

#define CORE_INFO(message)		LOG(CORE, __INFO, message)
#define CORE_EVENT(message)		LOG(CORE, __EVENT, message)
#define CORE_WARN(message)		LOG(CORE, __WARN, message)
#define CORE_ERROR(message)		LOG(CORE, __ERROR, message)
#define CORE_DEBUG(message)		LOG(CORE, __DEBUG, message)



//////////////////////////////////////////////////////////////////////////
//功能：内存分配核心模块的导出函数
//参数：pPoolInterface ------- 内存分配函数结构体的指针
//
extern void COREAPI TS_XXXX(LPCOREINTERFACE pPoolInterface);

#endif