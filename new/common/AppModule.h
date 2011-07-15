
#ifndef _LIVE_P2PCOMMON2_NEW_COMMON_APP_MODULE_H_
#define _LIVE_P2PCOMMON2_NEW_COMMON_APP_MODULE_H_

/**
 * @file
 * @brief AppModule应用模块类的头文件
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



/// 用于视频直播的功能模块基类
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


	/// 是否是source模块
	virtual bool IsSource() const { return false; }

	/// 只读的SysInfo
	const CSysInfo& GetSysInfo() const { return *m_SysInfo; }

	/// 可写的LiveInfo
	CLiveInfo& GetInfo() { return *m_LiveInfo; }

	/// 只读的LiveInfo
	const CLiveInfo& GetInfo() const { return *m_LiveInfo; }

	virtual PeerManager& GetPeerManager() = 0;

	virtual CStreamBuffer& GetStreamBuffer() = 0;

	virtual CIPPool& GetIPPool() = 0;

	virtual TrackerRequester& GetTracker() = 0;

	virtual CUdpDetect& GetUDPDetect() = 0;

	virtual StunModule* GetStunModule() = 0;


	/// 获取当前最大允许的连接数
	virtual size_t GetMaxConnectionCount() const = 0;
	virtual size_t GetMaxConnectionCount2() const = 0;
	virtual size_t GetMinConnectionCount() const = 0; // Added by Tady, 082908. For kick limit.

	virtual bool CheckPieceIndexValid(UINT pieceIndex) const = 0;

	boost::shared_ptr<UDPConnectionlessPacketSender> GetUDPConnectionlessPacketSender() { return m_UDPConnectionlessPacketSender; }
	boost::shared_ptr<UDPConnectionPacketSender> GetUDPConnectionPacketSender() { return m_UDPConnectionPacketSender; }

	boost::shared_ptr<TCPConnectionlessPacketSender> GetTCPConnectionlessPacketSender() { return m_TCPConnectionlessPacketSender; }
	boost::shared_ptr<TCPConnectionPacketSender> GetTCPConnectionPacketSender() { return m_TCPConnectionPacketSender; }

	/// 初始化指定的ini对象
	virtual void InitIni(ini_file& ini) = 0;
	virtual void InitIniFile(ini_file& ini, const TCHAR* filename) = 0;
	virtual void InitConfig(ini_file& ini) = 0;
	virtual void InitConfigFile(ini_file& ini, const TCHAR* filename) = 0;

	/// 记录vip节点的资源范围(可以作为source资源范围的参考值)
	virtual bool SaveVIPMinMax(const PEER_MINMAX& minmax) = 0;



	const std::set<PEER_ADDRESS>& GetMDSPeers() const { return m_mdsPeers; }
	const std::set<PEER_ADDRESS>& GetLANPeers() const { return m_lanPeers; }

	virtual void RecordDownload(size_t size) = 0;


	boost::shared_ptr<PeerInformation> GetPeerInformation() { return m_PeerInformation; }

	virtual const LiveAppModuleCreateParam& GetAppCreateParam() const = 0;

protected:
	/// 全局共享的信息
	const CSysInfo*	m_SysInfo;

	/// 本频道所使用的信息
	CLiveInfo* m_LiveInfo;

	std::set<PEER_ADDRESS> m_mdsPeers;
	std::set<PEER_ADDRESS> m_lanPeers;

	/// 老的udp报文发送器，仅用于peer-tracker通信
	boost::shared_ptr<TrackerPacketSender> m_UDPPacketSender;

	/// udp无连接报文
	boost::shared_ptr<UDPConnectionlessPacketSender> m_UDPConnectionlessPacketSender;

	/// udp连接报文，即以前的session报文
	boost::shared_ptr<UDPConnectionPacketSender> m_UDPConnectionPacketSender;

	/// tcp无连接报文
	boost::shared_ptr<TCPConnectionlessPacketSender> m_TCPConnectionlessPacketSender;

	/// tcp连接报文
	boost::shared_ptr<TCPConnectionPacketSender> m_TCPConnectionPacketSender;

	boost::shared_ptr<PeerInformation> m_PeerInformation;

};


#endif

