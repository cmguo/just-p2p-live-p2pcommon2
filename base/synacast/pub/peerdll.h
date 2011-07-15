
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PUB_PEERDLL_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PUB_PEERDLL_H_

#include <synacast/pub/CoreStatus.h>

#include <boost/config.hpp>

#ifdef BOOST_HAS_DECLSPEC

#ifdef LIVE_SOURCE
#  define LIVE_DECL __declspec(dllexport)
#else
#  define LIVE_DECL __declspec(dllimport)
#endif  // LIVE_SOURCE

#else
#  define LIVE_DECL __attribute__ ((visibility("default")))
#endif


struct CCoreUPNP;
struct CCoreStatus;
struct CCoreParameter;

#if __cplusplus
extern "C" {
#endif // __cplusplus


typedef int (*FUNC_CallBack)(unsigned int ChannelHandle, unsigned int Msg, unsigned int wParam, unsigned int lParam);

typedef bool (*FUNC_LiveStartup)(int);

typedef void (*FUNC_LiveCleanup)();

typedef void* (*FUNC_LiveStartChannel)(const char*, int, int);

typedef void (*FUNC_LiveStopChannel)(void*);

typedef bool (*FUNC_LiveGetChannelStatus)(void*, CCoreStatus*);
typedef bool (*FUNC_LiveGetChannelParameter)(void*, CCoreParameter*);
typedef bool (*FUNC_LiveSetChannelParameter)(void*, const CCoreParameter*);
typedef bool (*FUNC_LiveSetChannelPlayerStatus)(void*, int);
typedef bool (*FUNC_LiveSetChannelCallback)(void*, FUNC_CallBack, unsigned int);
typedef bool (*FUNC_LiveSetChannelUPNP)(void*, const CCoreUPNP*);

LIVE_DECL bool LiveStartup(int node_type);
LIVE_DECL void LiveCleanup();
LIVE_DECL void* LiveStartChannel(const char* urlstr, int tcpPort, int udpPort);
LIVE_DECL void LiveStopChannel(void* handle);

LIVE_DECL bool LiveGetChannelStatus(void* handle, CCoreStatus* status);
LIVE_DECL bool LiveGetChannelParameter(void* handle, CCoreParameter* param);
LIVE_DECL bool LiveSetChannelParameter(void* handle, const CCoreParameter* param);
LIVE_DECL bool LiveSetChannelPlayerStatus(void* handle, int pstatus);
LIVE_DECL bool LiveSetChannelCallback(void* handle, FUNC_CallBack callback, unsigned int channelHandle);
LIVE_DECL bool LiveSetChannelUPNP(void* handle, const CCoreUPNP* upnpInfo);

#define UM_LIVEMSG_PLAY			(WM_USER + 253)
#define UM_LIVEMSG_PLAYEROFF	(WM_USER + 254)

struct LIVE_CHANNEL_STATUS
{
	unsigned int	MediaType;//媒体类型，MediaFileType
	unsigned int	BufferPercent;//下载缓冲百分比
	unsigned int	BufferTime;//缓冲时间 	

	unsigned int	DownloadSpeed;//下载速度(Byte/sencond)
	unsigned int	UploadSpeed;//上传速度(Byte/sencond)
	unsigned int	ConnectionCountt;//当前连接数(VOD则表示当前使用的peer数)
	//Live
	unsigned int	PendingPeerCount;//发起连接数(Live)
	unsigned int	TotalPeerCount;//备用连接数(Live)

	UINT64	TotalUploadBytes;		// 上传的数据总量(字节数)
	UINT64	TotalDownloadBytes;		// 下载的数据总量(字节数)

	//Live
	unsigned int	ResMinIndex;//(Live)
	unsigned int	ResMaxIndex;//(Live)
	unsigned int	ResCurIndex;//(Live)

};

struct PPLIVE_COMMON_CHANNEL_STATUS
{
	unsigned int	m_MediaType;//媒体类型，MediaFileType
	unsigned int	m_BufferPercent;//下载缓冲百分比
	unsigned int	m_BufferTime;//缓冲时间 	

	unsigned int	m_DownloadSpeed;//下载速度(Byte/sencond)
	unsigned int	m_UploadSpeed;//上传速度(Byte/sencond)
	unsigned int	m_ConnectionCountt;//当前连接数(VOD则表示当前使用的peer数)
	//Live
	unsigned int	m_PendingPeerCount;//发起连接数(Live)
	unsigned int	m_TotalPeerCount;//备用连接数(Live)

	UINT64	m_TotalUploadBytes;		// 上传的数据总量(字节数)
	UINT64	m_TotalDownloadBytes;		// 下载的数据总量(字节数)

	//Live
	unsigned int	m_ResMinIndex;//(Live)
	unsigned int	m_ResMaxIndex;//(Live)
	unsigned int	m_ResCurIndex;//(Live)

	//VOD
	unsigned int	m_Duration;//(VOD影片时间总长(VOD，单位seconds)
	unsigned int	m_StartTime;//(VOD影片开始播放时间(VOD，单位:seconds)
	unsigned int	m_tMediaListenPort;//(VOD，内核媒体数据流端口)
	unsigned int  uCurrentVoDDownLoadTime; //VOD当前下载的时间点，用来UI显示已下载进度条(单位:毫秒)，VOD每隔一秒钟填写，大于0表示有效
	unsigned int  m_VodCacheFileCtrl;//缓存文件管理，0表示手动；大于0表示自动管理
	char	m_VodCacheFilePath[256];//缓冲文件路径,’/0‘结尾
};

#if __cplusplus
}
#endif // __cplusplus

#endif



