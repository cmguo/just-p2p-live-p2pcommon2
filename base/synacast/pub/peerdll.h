
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
	unsigned int	MediaType;//ý�����ͣ�MediaFileType
	unsigned int	BufferPercent;//���ػ���ٷֱ�
	unsigned int	BufferTime;//����ʱ�� 	

	unsigned int	DownloadSpeed;//�����ٶ�(Byte/sencond)
	unsigned int	UploadSpeed;//�ϴ��ٶ�(Byte/sencond)
	unsigned int	ConnectionCountt;//��ǰ������(VOD���ʾ��ǰʹ�õ�peer��)
	//Live
	unsigned int	PendingPeerCount;//����������(Live)
	unsigned int	TotalPeerCount;//����������(Live)

	UINT64	TotalUploadBytes;		// �ϴ�����������(�ֽ���)
	UINT64	TotalDownloadBytes;		// ���ص���������(�ֽ���)

	//Live
	unsigned int	ResMinIndex;//(Live)
	unsigned int	ResMaxIndex;//(Live)
	unsigned int	ResCurIndex;//(Live)

};

struct PPLIVE_COMMON_CHANNEL_STATUS
{
	unsigned int	m_MediaType;//ý�����ͣ�MediaFileType
	unsigned int	m_BufferPercent;//���ػ���ٷֱ�
	unsigned int	m_BufferTime;//����ʱ�� 	

	unsigned int	m_DownloadSpeed;//�����ٶ�(Byte/sencond)
	unsigned int	m_UploadSpeed;//�ϴ��ٶ�(Byte/sencond)
	unsigned int	m_ConnectionCountt;//��ǰ������(VOD���ʾ��ǰʹ�õ�peer��)
	//Live
	unsigned int	m_PendingPeerCount;//����������(Live)
	unsigned int	m_TotalPeerCount;//����������(Live)

	UINT64	m_TotalUploadBytes;		// �ϴ�����������(�ֽ���)
	UINT64	m_TotalDownloadBytes;		// ���ص���������(�ֽ���)

	//Live
	unsigned int	m_ResMinIndex;//(Live)
	unsigned int	m_ResMaxIndex;//(Live)
	unsigned int	m_ResCurIndex;//(Live)

	//VOD
	unsigned int	m_Duration;//(VODӰƬʱ���ܳ�(VOD����λseconds)
	unsigned int	m_StartTime;//(VODӰƬ��ʼ����ʱ��(VOD����λ:seconds)
	unsigned int	m_tMediaListenPort;//(VOD���ں�ý���������˿�)
	unsigned int  uCurrentVoDDownLoadTime; //VOD��ǰ���ص�ʱ��㣬����UI��ʾ�����ؽ�����(��λ:����)��VODÿ��һ������д������0��ʾ��Ч
	unsigned int  m_VodCacheFileCtrl;//�����ļ�����0��ʾ�ֶ�������0��ʾ�Զ�����
	char	m_VodCacheFilePath[256];//�����ļ�·��,��/0����β
};

#if __cplusplus
}
#endif // __cplusplus

#endif



