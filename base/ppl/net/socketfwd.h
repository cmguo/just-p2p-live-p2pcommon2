
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_NET_SOCKETFWD_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_NET_SOCKETFWD_H_

#include <boost/shared_ptr.hpp>
#include <assert.h>
#include <stddef.h>
#include <iosfwd>

class SocketAddress;
class InetSocketAddress;

class udp_socket;
class tcp_acceptor;
class tcp_socket;

typedef boost::shared_ptr<tcp_socket> tcp_socket_ptr;


/// udp socket���¼�
class udp_socket_listener
{
public:
	virtual ~udp_socket_listener() { }

	/// data received
	virtual void on_socket_receive(udp_socket* sender, const InetSocketAddress& addr, unsigned char* data, size_t size) = 0;

	/// failed to receive data
	virtual void on_socket_receive_failed(udp_socket* sender, int errcode) = 0;
};


/// tcp_acceptor���¼��ӿ�
class tcp_acceptor_listener
{
public:
	virtual ~tcp_acceptor_listener() { }

	/// connection accepted
	virtual void on_socket_accept(tcp_acceptor* sender, tcp_socket* client, const InetSocketAddress& addr) = 0;

	/// failed to accept connection
	virtual void on_socket_accept_failed(tcp_acceptor* sender, int errcode) { }	// can be ignored
};


/// TCPClientSocket���¼��ӿ�
class tcp_socket_listener
{
public:
	virtual ~tcp_socket_listener() { }

	/// ���ӳɹ�
	virtual void on_socket_connect(tcp_socket* sender) { LIVE_ASSERT(false); }
	/// ����ʧ��
	virtual void on_socket_connect_failed(tcp_socket* sender, int errcode) { LIVE_ASSERT(false); }

	/// ���յ�����
	virtual void on_socket_receive(tcp_socket* sender, unsigned char* data, size_t size)  = 0;
	/// ��������ʧ��
	virtual void on_socket_receive_failed(tcp_socket* sender, int errcode) = 0;

	/// �����������
	virtual void on_socket_send(tcp_socket* sender) { }	// can be ignored
	/// ��������ʧ��
	virtual void on_socket_send_failed(tcp_socket* sender, int errcode) { this->on_socket_receive_failed(sender, errcode); }
};





class trivial_tcp_acceptor_listener : public tcp_acceptor_listener
{
public:
	virtual void on_socket_accept(tcp_acceptor* sender, tcp_socket* client, const InetSocketAddress& addr)
	{
		LIVE_ASSERT(false);
	}
};



class trivial_udp_socket_listener : public udp_socket_listener
{
public:
	virtual void on_socket_receive(udp_socket* sender, const InetSocketAddress& addr, unsigned char* data, size_t size) { LIVE_ASSERT(false); }
	virtual void on_socket_receive_failed(udp_socket* sender, int errcode) { LIVE_ASSERT(false); }
};

class trivial_tcp_socket_listener : public tcp_socket_listener
{
public:
	/// ���ӳɹ�
	virtual void on_socket_connect(tcp_socket* sender) { LIVE_ASSERT(false); }
	/// ����ʧ��
	virtual void on_socket_connect_failed(tcp_socket* sender, int errcode) { LIVE_ASSERT(false); }

	/// ���յ�����
	virtual void on_socket_receive(tcp_socket* sender, unsigned char* data, size_t size) { LIVE_ASSERT(false); }
	/// ��������ʧ��
	virtual void on_socket_receive_failed(tcp_socket* sender, int errcode) { LIVE_ASSERT(false); }

	/// �����������
	virtual void on_socket_send(tcp_socket* sender) { }	// can be ignored
	/// ��������ʧ��
	virtual void on_socket_send_failed(tcp_socket* sender, int errcode) { LIVE_ASSERT(false); }
};



std::ostream& operator<<(std::ostream& os, const tcp_socket& sock);

#endif