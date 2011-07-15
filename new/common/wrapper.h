
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

	/// ��Ϣ���������յ��µ���������
	int OnAcceptConnection(MESSAGE& msg);
	/// ��Ϣ���������յ�һ��udp����
	int OnUDPRecvFrom(MESSAGE& Message);
	/// ��Ϣ������������mds��vip�б�
	int OnUpdateMDSVIP(MESSAGE& Message);
	/// ��Ϣ�����������¼�������
	int OnReloadConfiguration(MESSAGE& Message);
	/// ��Ϣ������������tracker�б�
	int OnUpdateTrackers(MESSAGE& Message);

	/// ����������ӳɹ�����Ϣ
	int OnSocketAccept(MESSAGE& Message);

	/// �����������ʧ�ܵ���Ϣ
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