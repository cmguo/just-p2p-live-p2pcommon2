
#ifndef _LIVE_P2PCOMMON2_NEW_COMMON_WRAPPER_H_
#define _LIVE_P2PCOMMON2_NEW_COMMON_WRAPPER_H_

#include <synacast/net/message.h>
#include <boost/shared_ptr.hpp>

#include "GloalTypes.h"
#include "UDPSender.h"
#include "AppParam.h"





class LiveAppModuleImpl;

class LiveAppModuleWrapper : public CBasicObject
{
public:
	explicit LiveAppModuleWrapper( const LIVEPARAM& param, PeerTypeEnum peerType );
	virtual ~LiveAppModuleWrapper();

	void OnPlayerBuffering(bool isBufferOK);

protected:
	PPL_DECLARE_MESSAGE_MAP()

	/// 消息处理函数：收到新的连接请求
	int OnAcceptConnection(MESSAGE& msg);
	/// 消息处理函数：收到一个udp报文
	int OnUDPRecvFrom(MESSAGE& Message);
	/// 消息处理函数：更新mds和vip列表
	int OnUpdateMDSVIP(MESSAGE& Message);
	/// 消息处理函数：重新加载配置
	int OnReloadConfiguration(MESSAGE& Message);
	/// 消息处理函数：更新tracker列表
	int OnUpdateTrackers(MESSAGE& Message);

	/// 处理接受连接成功的消息
	int OnSocketAccept(MESSAGE& Message);

	/// 处理接受连接失败的消息
	int OnAcceptFailed(MESSAGE& Message);

	bool DispatchUdp(const MESSAGE_UDP_RECVFROM_SUCCESS& msg);

	void OnUDPSenderResponse(BYTE* data, size_t size, const InetSocketAddress& sockAddr, UINT proxyType);

protected:
	LiveAppModuleCreateParam m_param;


	boost::shared_ptr<LiveAppModuleImpl> m_appModule;
};


class LiveAppModuleWrapperFactory
{
public:
	static LiveAppModuleWrapper* CreatePeer(const LIVEPARAM& param, const ExtraLiveParam& extraParam);
	static LiveAppModuleWrapper* CreateSource(const LIVEPARAM& param);
	static LiveAppModuleWrapper* CreateMDS(const LIVEPARAM& param);
	static LiveAppModuleWrapper* CreateMAS(const LIVEPARAM& param);
};
#endif