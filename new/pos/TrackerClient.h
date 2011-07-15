
#ifndef _LIVE_P2PCOMMON2_NEW_POS_TRACKER_CLIENT_H_
#define _LIVE_P2PCOMMON2_NEW_POS_TRACKER_CLIENT_H_

#include <synacast/protocol/DataIO.h>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

/**
 * @file
 * @brief ����tracker�ͻ��˵������
 */


struct OLD_UDP_PACKET_HEAD;
struct NEW_UDP_PACKET_HEAD;
struct TRACKER_ADDRESS;
struct TRACKER_LOGIN_ADDRESS;
class TrackerClientListener;
class RateMeasure;
class PeerInformation;
class TrackerPacketSender;
class SecureTrackerRequestPacketSender;
struct SECURE_TRACKER_PACKET_HEAD;



/// tracker�ͻ��ˣ���װ��һ��trackerͨѶ��ϸ��
class TrackerClient : private boost::noncopyable
{
public:
	TrackerClient() { }
	virtual ~TrackerClient() { }

	virtual void Init( TrackerClientListener& listener, const TRACKER_LOGIN_ADDRESS& addr, boost::shared_ptr<PeerInformation> peerInformation, boost::shared_ptr<TrackerPacketSender> packetSender ) = 0;

	/// ��ȡ�������ĵ�ַ
	virtual const TRACKER_ADDRESS& GetServerAddress() const = 0;

	/// �������Ƿ���UDP��ʽ��
	virtual bool IsUDP() const = 0;

	/// �Ƿ��Ѿ���¼��������
	virtual bool IsOnline() const = 0;

	/// �Ƿ�
	virtual bool IsActive() const = 0;

	/// �����ͻ���
	virtual void Start() = 0;

	/// ֹͣ�ͻ���
	virtual void Stop() = 0;
	virtual void Restart() = 0;

	/// ����������һ�α���
	virtual void KeepAlive() = 0;

	/// ���������ϻ�ȡ�ɹ����ʵ�peer�б�
	virtual void ListPeers() = 0;

	/// ��¼������
	virtual void Login() = 0;
	/// �˳�������
	virtual void Logout() = 0;


	/// �����Ӧ���ģ�����true��ʾ������
	virtual bool HandleResponse( data_input_stream& is, const OLD_UDP_PACKET_HEAD& head ) = 0;
	/// ����ȫ��Ӧ���ģ�����true��ʾ������
	virtual bool HandleSecureResponse( data_input_stream& is, const NEW_UDP_PACKET_HEAD& head, bool isPeerProxy ) = 0;

	/// ��ȡ״̬����(1/0/������)
	virtual long GetStatusCode() const = 0;

	virtual void SaveDHTTransactionID(UINT32 transactionID, UINT8 action) = 0;

public:
	/// ʧ�ܴ���
	UINT16		FailedTimes;
	/// ��List�����Ĵ���
	UINT8		ListTimes;
	/// List�ĸ���
	UINT8		ListCount;
};



/// TrackerClient�๤��
class TrackerClientFactory
{
public:
	static TrackerClient* PeerCreate();

	static TrackerClient* MCCCreate();

	static TrackerClient* SimpleMDSCreate();

};


#endif