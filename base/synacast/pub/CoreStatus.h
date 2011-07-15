#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PUB_CORESTATUS_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PUB_CORESTATUS_H_


/// MediaFileType类型定义如下
enum MediaFileType
{	
	MFT_UNKNOWN	= 0,
	MFT_WMV	= 0x1,
	MFT_RM	= 0x2,
	MFT_MKV	= 0x3,
  	MFT_FLV  	= 0x4,
  	MFT_MP4	= 0x5
};

struct CCoreStatus
{
  	unsigned int	m_MediaType;//媒体类型，MediaFileType
  	unsigned int	m_BufferPercent;//下载缓冲百分比
  	unsigned int	m_BufferTime;//缓冲时间 	  	
  	unsigned int	m_DownloadSpeed;//下载速度(Byte/sencond)
  	unsigned int	m_UploadSpeed;//上传速度(Byte/sencond)
  	unsigned int	m_ConnectionCountt;//当前连接数(VOD则表示当前使用的peer数)
	UINT64	m_TotalUploadBytes;		// 上传的数据总量(字节数)
	UINT64	m_TotalDownloadBytes;	

  	//Live
	unsigned int	m_PendingPeerCount;//发起连接数(Live)
  	unsigned int	m_TotalPeerCount;//备用连接数(Live)		
	//VOD
	unsigned int	m_Duration;//(VOD影片时间总长(VOD，单位seconds)
  	unsigned int	m_StartTime;//(VOD影片开始播放时间(VOD，单位:seconds)
  	unsigned int	m_uMediaListenPort;//(VOD，内核媒体数据流端口)
};

struct CCoreParameter
{
	unsigned short	m_usUdpListenPort;//内核UDP监听端口,Get时内核把真正监听端口写入
	//unsigned short	m_usUdpUPNPPort;	//UDP在UPNP映射的外网端口，客户端填写，非0表示UPNP成功;0表示UPNP没有映射或者映射失败
	unsigned short	m_usTCPListenPort;//内核TCP监听端口Get时内核把真正监听端口写入
	//unsigned short	m_usTCPUPNPPort;	//TCP在UPNP映射的外网端口，客户端填写，非0表示UPNP成功;0表示UPNP没有映射或者映射失败

  	unsigned int	m_LocalNetType;//选择网络类型 ADSL 512K=0，2M=1，小区宽带=2，1M=3，教育网=4，其它=5
  	DWORD		m_dwFileVersionMS;		//内核版本信息，内核填写，The most significant 32 bits of the file's binary version number.（见MSDN：VS_FIXEDFILEINFO结构体）
	DWORD		m_dwFileVersionLS;		//内核版本信息，内核填写，The least significant 32 bits of the file's binary version number.（见MSDN：VS_FIXEDFILEINFO结构体）
  	//live
  	unsigned int	m_MaxConnectionsPerChannel;//每个频道最大连接个数
  	unsigned int	m_MaxCoPendingConnections;//最大同时发出网络连接个数  	
  	unsigned int	m_ConnectionCtrl;//连接数智能控制
  	unsigned int	m_TransferMode;//传输模式 正常模式=0，受运营商限制模式=1，经典模式=2，防止断网模式=3
	unsigned int  m_GraphMode;	//媒体流推送模式 0：系统播放器流 1：MMS流 2：GRAPH流

	//vod		
	//unsigned int	m_MaxUploadSpeed;//限制最大上传速度(Bytes/sencond)
	//unsigned int	m_MaxDownloadSpeed;//限制最大下载速度(Bytes/sencond)
	unsigned int  m_VodCacheFileCtrl;//缓存文件管理，0表示手动；大于0表示自动管理
	char	m_VodCacheFilePath[256];//缓冲文件路径,'/0'结尾		
};

struct CCoreUPNP
{
	unsigned short m_usUdpUpnpIn;		//内网Udp端口，非0表示UPNP成功;0表示UPNP没有映射或者映射失败
	unsigned short m_usUdpUpnpOut;		//外网Udp端口，非0表示UPNP成功;0表示UPNP没有映射或者映射失败
	unsigned short m_usTcpUpnpIn;		//内网Tcp端口，非0表示UPNP成功;0表示UPNP没有映射或者映射失败
	unsigned short m_usTcpUpnpOut;		//外网Tcp端口，非0表示UPNP成功;0表示UPNP没有映射或者映射失败
};


/*
enum ENU_CONTROL_MSG_TYPE
{
    ENU_CMT_START = PREPARE,    //0
    ENU_CMT_STOP = STOP,        //1
    ENU_CMT_BUFFER = BUFFERING, //2  播放器进入了buffering状态
    ENU_CMT_PLAY = PLAYING,        //3，播放器进入了playing状态
	ENU_CMT_RECONNECT = RECONNECT, // 4
    ENU_CMT_STARTUPLOAD,                   //5开启VOD上传数据服务
    ENU_CMT_STOPUPLOAD,                    //6停止VOD上传数据服务
    ENU_VOD_CMD_NPREPARE = VOD_CMD_NPREPARE,
    ENU_CMT_PAUSE = PAUSE,//播放器进入了暂停状态
		
    ENU_CMT_INVALID = 0xFF
};
*/



#endif