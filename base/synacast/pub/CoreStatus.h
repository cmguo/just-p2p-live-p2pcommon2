#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PUB_CORESTATUS_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PUB_CORESTATUS_H_


/// MediaFileType���Ͷ�������
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
  	unsigned int	m_MediaType;//ý�����ͣ�MediaFileType
  	unsigned int	m_BufferPercent;//���ػ���ٷֱ�
  	unsigned int	m_BufferTime;//����ʱ�� 	  	
  	unsigned int	m_DownloadSpeed;//�����ٶ�(Byte/sencond)
  	unsigned int	m_UploadSpeed;//�ϴ��ٶ�(Byte/sencond)
  	unsigned int	m_ConnectionCountt;//��ǰ������(VOD���ʾ��ǰʹ�õ�peer��)
	UINT64	m_TotalUploadBytes;		// �ϴ�����������(�ֽ���)
	UINT64	m_TotalDownloadBytes;	

  	//Live
	unsigned int	m_PendingPeerCount;//����������(Live)
  	unsigned int	m_TotalPeerCount;//����������(Live)		
	//VOD
	unsigned int	m_Duration;//(VODӰƬʱ���ܳ�(VOD����λseconds)
  	unsigned int	m_StartTime;//(VODӰƬ��ʼ����ʱ��(VOD����λ:seconds)
  	unsigned int	m_uMediaListenPort;//(VOD���ں�ý���������˿�)
};

struct CCoreParameter
{
	unsigned short	m_usUdpListenPort;//�ں�UDP�����˿�,Getʱ�ں˰����������˿�д��
	//unsigned short	m_usUdpUPNPPort;	//UDP��UPNPӳ��������˿ڣ��ͻ�����д����0��ʾUPNP�ɹ�;0��ʾUPNPû��ӳ�����ӳ��ʧ��
	unsigned short	m_usTCPListenPort;//�ں�TCP�����˿�Getʱ�ں˰����������˿�д��
	//unsigned short	m_usTCPUPNPPort;	//TCP��UPNPӳ��������˿ڣ��ͻ�����д����0��ʾUPNP�ɹ�;0��ʾUPNPû��ӳ�����ӳ��ʧ��

  	unsigned int	m_LocalNetType;//ѡ���������� ADSL 512K=0��2M=1��С�����=2��1M=3��������=4������=5
  	DWORD		m_dwFileVersionMS;		//�ں˰汾��Ϣ���ں���д��The most significant 32 bits of the file's binary version number.����MSDN��VS_FIXEDFILEINFO�ṹ�壩
	DWORD		m_dwFileVersionLS;		//�ں˰汾��Ϣ���ں���д��The least significant 32 bits of the file's binary version number.����MSDN��VS_FIXEDFILEINFO�ṹ�壩
  	//live
  	unsigned int	m_MaxConnectionsPerChannel;//ÿ��Ƶ��������Ӹ���
  	unsigned int	m_MaxCoPendingConnections;//���ͬʱ�����������Ӹ���  	
  	unsigned int	m_ConnectionCtrl;//���������ܿ���
  	unsigned int	m_TransferMode;//����ģʽ ����ģʽ=0������Ӫ������ģʽ=1������ģʽ=2����ֹ����ģʽ=3
	unsigned int  m_GraphMode;	//ý��������ģʽ 0��ϵͳ�������� 1��MMS�� 2��GRAPH��

	//vod		
	//unsigned int	m_MaxUploadSpeed;//��������ϴ��ٶ�(Bytes/sencond)
	//unsigned int	m_MaxDownloadSpeed;//������������ٶ�(Bytes/sencond)
	unsigned int  m_VodCacheFileCtrl;//�����ļ�����0��ʾ�ֶ�������0��ʾ�Զ�����
	char	m_VodCacheFilePath[256];//�����ļ�·��,'/0'��β		
};

struct CCoreUPNP
{
	unsigned short m_usUdpUpnpIn;		//����Udp�˿ڣ���0��ʾUPNP�ɹ�;0��ʾUPNPû��ӳ�����ӳ��ʧ��
	unsigned short m_usUdpUpnpOut;		//����Udp�˿ڣ���0��ʾUPNP�ɹ�;0��ʾUPNPû��ӳ�����ӳ��ʧ��
	unsigned short m_usTcpUpnpIn;		//����Tcp�˿ڣ���0��ʾUPNP�ɹ�;0��ʾUPNPû��ӳ�����ӳ��ʧ��
	unsigned short m_usTcpUpnpOut;		//����Tcp�˿ڣ���0��ʾUPNP�ɹ�;0��ʾUPNPû��ӳ�����ӳ��ʧ��
};


/*
enum ENU_CONTROL_MSG_TYPE
{
    ENU_CMT_START = PREPARE,    //0
    ENU_CMT_STOP = STOP,        //1
    ENU_CMT_BUFFER = BUFFERING, //2  ������������buffering״̬
    ENU_CMT_PLAY = PLAYING,        //3��������������playing״̬
	ENU_CMT_RECONNECT = RECONNECT, // 4
    ENU_CMT_STARTUPLOAD,                   //5����VOD�ϴ����ݷ���
    ENU_CMT_STOPUPLOAD,                    //6ֹͣVOD�ϴ����ݷ���
    ENU_VOD_CMD_NPREPARE = VOD_CMD_NPREPARE,
    ENU_CMT_PAUSE = PAUSE,//��������������ͣ״̬
		
    ENU_CMT_INVALID = 0xFF
};
*/



#endif