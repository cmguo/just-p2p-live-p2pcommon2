
#ifndef _LIVE_P2PCOMMON2_NEW_COMMON_APP_MODULE_H_
#define _LIVE_P2PCOMMON2_NEW_COMMON_APP_MODULE_H_

/**
 * @file
 * @brief AppModuleӦ��ģ�����ͷ�ļ�
 */


#include "appcore.h"
#include "framework/log.h"
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <set>

class LiveAppModuleCreateParam;

class ini_file;
class InetSocketAddress;
class CStreamBuffer;
class PeerManager;
class CIPPool;
class TrackerRequester;

class PeerNetInfo;
class PeerStatusInfo;
class PeerInformation;

class PacketBase;

class CLiveInfo;
class CSysInfo;

class IPTable;

struct PEER_ADDRESS;
struct PEER_MINMAX;
struct FLOW_INFO;
struct FLOW_STATISTICS;
struct LOCAL_STATISTICS;
struct UDPT_HEAD_INFO;
class CUdpDetect;

class StunModule;

class UDPConnectionlessPacketSender;
class UDPConnectionPacketSender;
class TCPConnectionlessPacketSender;
class TCPConnectionPacketSender;
class TrackerPacketSender;



/// ������Ƶֱ���Ĺ���ģ�����
class AppModule : private boost::noncopyable
{
public:
	AppModule() : m_SysInfo(NULL), m_LiveInfo(NULL)
	{
		APP_EVENT("construct AppModule " << this);
	}
	virtual ~AppModule()
	{
		APP_EVENT("destruct AppModule " << this);
	}

	virtual GUID GetResourceGUID() const = 0;


	/// �Ƿ���sourceģ��
	virtual bool IsSource() const { return false; }

	/// ֻ����SysInfo
	const CSysInfo& GetSysInfo() const { return *m_SysInfo; }

	/// ��д��LiveInfo
	CLiveInfo& GetInfo() { return *m_LiveInfo; }

	/// ֻ����LiveInfo
	const CLiveInfo& GetInfo() const { return *m_LiveInfo; }

	virtual PeerManager& GetPeerManager() = 0;

	virtual CStreamBuffer& GetStreamBuffer() = 0;

	virtual CIPPool& GetIPPool() = 0;

	virtual TrackerRequester& GetTracker() = 0;

	virtual CUdpDetect& GetUDPDetect() = 0;

	virtual StunModule* GetStunModule() = 0;


	/// ��ȡ��ǰ��������������
	virtual size_t GetMaxConnectionCount() const = 0;
	virtual size_t GetMaxConnectionCount2() const = 0;
	virtual size_t GetMinConnectionCount() const = 0; // Added by Tady, 082908. For kick limit.

	virtual bool CheckPieceIndexValid(UINT pieceIndex) const = 0;

	boost::shared_ptr<UDPConnectionlessPacketSender> GetUDPConnectionlessPacketSender() { return m_UDPConnectionlessPacketSender; }
	boost::shared_ptr<UDPConnectionPacketSender> GetUDPConnectionPacketSender() { return m_UDPConnectionPacketSender; }

	boost::shared_ptr<TCPConnectionlessPacketSender> GetTCPConnectionlessPacketSender() { return m_TCPConnectionlessPacketSender; }
	boost::shared_ptr<TCPConnectionPacketSender> GetTCPConnectionPacketSender() { return m_TCPConnectionPacketSender; }

	/// ��ʼ��ָ����ini����
	virtual void InitIni(ini_file& ini) = 0;
	virtual void InitIniFile(ini_file& ini, const TCHAR* filename) = 0;
	virtual void InitConfig(ini_file& ini) = 0;
	virtual void InitConfigFile(ini_file& ini, const TCHAR* filename) = 0;

	/// ��¼vip�ڵ����Դ��Χ(������Ϊsource��Դ��Χ�Ĳο�ֵ)
	virtual bool SaveVIPMinMax(const PEER_MINMAX& minmax) = 0;



	const std::set<PEER_ADDRESS>& GetMDSPeers() const { return m_mdsPeers; }
	const std::set<PEER_ADDRESS>& GetLANPeers() const { return m_lanPeers; }

	virtual void RecordDownload(size_t size) = 0;


	boost::shared_ptr<PeerInformation> GetPeerInformation() { return m_PeerInformation; }

	virtual const LiveAppModuleCreateParam& GetAppCreateParam() const = 0;

protected:
	/// ȫ�ֹ������Ϣ
	const CSysInfo*	m_SysInfo;

	/// ��Ƶ����ʹ�õ���Ϣ
	CLiveInfo* m_LiveInfo;

	std::set<PEER_ADDRESS> m_mdsPeers;
	std::set<PEER_ADDRESS> m_lanPeers;

	/// �ϵ�udp���ķ�������������peer-trackerͨ��
	boost::shared_ptr<TrackerPacketSender> m_UDPPacketSender;

	/// udp�����ӱ���
	boost::shared_ptr<UDPConnectionlessPacketSender> m_UDPConnectionlessPacketSender;

	/// udp���ӱ��ģ�����ǰ��session����
	boost::shared_ptr<UDPConnectionPacketSender> m_UDPConnectionPacketSender;

	/// tcp�����ӱ���
	boost::shared_ptr<TCPConnectionlessPacketSender> m_TCPConnectionlessPacketSender;

	/// tcp���ӱ���
	boost::shared_ptr<TCPConnectionPacketSender> m_TCPConnectionPacketSender;

	boost::shared_ptr<PeerInformation> m_PeerInformation;

};


#endif

