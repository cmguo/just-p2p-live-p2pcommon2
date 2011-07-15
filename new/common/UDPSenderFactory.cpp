
#include "StdAfx.h"

#include "UDPSenderFactory.h"
#include "framework/socket.h"
#include "framework/timer.h"
#include "framework/log.h"
#include "framework/memory.h"

#include <synacast/protocol/base.h>
#include <ppl/text/base64.h>
#include <ppl/net/http.h>
#include <ppl/data/stlutils.h>
#include <ppl/net/asio/message_tcp_socket.h>
#include <ppl/net/asio/http_client.h>

#include <boost/noncopyable.hpp>
#include <map>


#ifdef _PPL_USE_ASIO

#include <ppl/net/asio/udp_socket.h>

class NormalUDPSender : public UDPSender
{
public:
	explicit NormalUDPSender(boost::shared_ptr<udp_socket> udp) : m_udp(udp)
	{
	}

	virtual bool Send(const void* data, size_t size, const InetSocketAddress& sockAddr)
	{
		assert(size > 0);
		//assert(READ_MEMORY(data, UINT16) == PPL_P2P_LIVE2);
		//assert(m_udp >= 0);
		//return false; // 屏蔽udp发包，用于检测udp不通时的运行状况
		const UINT8* buf = static_cast<const UINT8*>(data);
		(void)buf;
                APP_DEBUG("NormalUDPSender Send " << sockAddr << " " << buf[2]);
		bool res = m_udp->send(data, (int)size, sockAddr);
		if (!res)
		{
			int errcode = m_udp->last_error();
		        (void)errcode;
                	VIEW_ERROR("UDP send failed. " << errcode << " " << sockAddr << ", " << make_buffer_pair(data, size));
		}
		return res;
	}

private:
	boost::shared_ptr<udp_socket> m_udp;
};





#else

class NormalUDPSender : public UDPSender
{
public:
	explicit NormalUDPSender(int udp) : m_udp(udp) { }
	virtual bool Send(const void* data, size_t size, const InetSocketAddress& sockAddr)
	{
		assert(size > 0);
		//assert(READ_MEMORY(data, UINT16) == PPL_P2P_LIVE2);
		assert(m_udp >= 0);
		//return false; // 屏蔽udp发包，用于检测udp不通时的运行状况
		const UINT8* buf = static_cast<const UINT8*>(data);
		APP_DEBUG("NormalUDPSender Send " << sockAddr << " " << buf[2]);
		bool res = NET_UDP_SendTo(m_udp, data, (int)size, const_cast<sockaddr*>(sockAddr.GetAddress()));
		if (!res)
		{
			int errcode = ::WSAGetLastError();
			VIEW_ERROR("UDP send failed. " << errcode << " " << sockAddr << ", " << make_buffer_pair(data, size));
		}
		return res;
	}

private:
	int m_udp;
};


#endif


class TCPProxyUDPSender;

class TCPProxyClient : private boost::noncopyable, public pool_object, public tcp_socket_listener
{
public:
	explicit TCPProxyClient(TCPProxyUDPSender& owner, const InetSocketAddress& serverAddr) : m_Owner( owner ), m_ServerAddress( serverAddr )
	{
		m_TimeoutTimer.set_callback(boost::bind(&TCPProxyClient::OnTimeout, this));
		m_Socket.reset(new message_tcp_socket);
		m_Socket->set_listener( this );
	}

	const InetSocketAddress& GetServerAddress() const { return m_ServerAddress;}

	bool Start( const void* data, size_t size, UINT timeout )
	{
		m_buffer.resize(size);
		memcpy(m_buffer.data(), data, size);
		if ( false == m_Socket->connect( m_ServerAddress ) )
			return false;
		m_TimeoutTimer.start( timeout );
		return true;
	}

	virtual void on_socket_connect(tcp_socket* sender);
	virtual void on_socket_connect_failed(tcp_socket* sender, int errcode);
	virtual void on_socket_receive(tcp_socket* sender, BYTE* data, size_t size);
	virtual void on_socket_receive_failed(tcp_socket* sender, int errcode);

protected:
	void OnTimeout();


protected:
	TCPProxyUDPSender& m_Owner;
	boost::shared_ptr<message_tcp_socket> m_Socket;
	once_timer m_TimeoutTimer;
	const InetSocketAddress m_ServerAddress;
	char_buffer m_buffer;
};

class TCPProxyUDPSender : public UDPSender
{
protected:
	typedef std::map<TCPProxyClient*, boost::shared_ptr<TCPProxyClient> > TCPProxyClientCollection;
public:
	explicit TCPProxyUDPSender(DWORD timeout, UDPSenderCallbackType callback) 
		: m_timeout(timeout), m_callback(callback) { }

	virtual bool Send(const void* data, size_t size, const InetSocketAddress& sockAddr)
	{
		assert(size > 0);
		boost::shared_ptr<TCPProxyClient> client(new TCPProxyClient( *this, sockAddr ) );
		if ( false == client->Start(data, size, m_timeout) )
			return false;
		m_Clients[client.get()] = client;
		return true;
	}

	void OnResponseOK(TCPProxyClient* client, BYTE* data, size_t size)
	{
		TCPProxyClientCollection::iterator iter = m_Clients.find( client );
		if ( iter == m_Clients.end() )
			return;

		m_callback(data, size, client->GetServerAddress(), PROXY_TCP);
		//BYTE* buf = reinterpret_cast<BYTE*>( CORE_MALLOC( size + sizeof( sockaddr_in ) ) );
		//memcpy( buf, data, size );
		//sockaddr_in* sa = reinterpret_cast<sockaddr_in*>( buf + size );
		//*sa = client->GetServerAddress().GetRawAddress();
		//CORE_SendObjectMessage(&m_listener, this, UM_SOCKET_RECVFROM_SUCCESS, (DWORD)buf, size, (DWORD)sa, PROXY_HTTP);
	}
	void OnResponseError(TCPProxyClient* client, int errcode)
	{
		TCPProxyClientCollection::iterator iter = m_Clients.find( client );
		if ( iter == m_Clients.end() )
			return;
		m_Clients.erase( iter );
	}

private:
	DWORD m_timeout;
	UDPSenderCallbackType m_callback;
	TCPProxyClientCollection m_Clients;
};

void TCPProxyClient::on_socket_connect(tcp_socket* sender)
{
	m_Socket->send(m_buffer.data(), m_buffer.size());
	m_Socket->receive();
}
void TCPProxyClient::on_socket_connect_failed(tcp_socket* sender, int errcode)
{
	m_TimeoutTimer.stop();
	m_Owner.OnResponseError(this, errcode);
}
void TCPProxyClient::on_socket_receive(tcp_socket* sender, BYTE* data, size_t size)
{
	m_TimeoutTimer.stop();
	m_Owner.OnResponseOK(this, data, size);
}

void TCPProxyClient::on_socket_receive_failed(tcp_socket* sender, int errcode)
{
	m_TimeoutTimer.stop();
	this->m_Owner.OnResponseError(this, errcode);
}

void TCPProxyClient::OnTimeout()
{
	this->m_Owner.OnResponseError(this, 1);
}



class HTTPProxyUDPSender;

class HTTPProxyClient : private boost::noncopyable, public pool_object
{
public:
	explicit HTTPProxyClient(HTTPProxyUDPSender& owner, const InetSocketAddress& serverAddr) : m_Owner( owner ), m_ServerAddress( serverAddr )
	{
		m_TimeoutTimer.set_callback(boost::bind(&HTTPProxyClient::OnTimeout, this));
		m_Socket.reset(new http_client);
		m_Socket->set_callbacks(
			boost::bind(&HTTPProxyClient::on_socket_receive, this, _1, _2), 
			boost::bind(&HTTPProxyClient::on_socket_receive_failed, this, _1));
	}
	~HTTPProxyClient()
	{
		m_Socket->close();
	}

	const InetSocketAddress& GetServerAddress() const { return m_ServerAddress;}

	bool Start( const char* urlstr, UINT timeout )
	{
		if ( false == m_Socket->create() )
			return false;
		if ( false == m_Socket->http_get( urlstr ) )
			return false;
		m_TimeoutTimer.start( timeout );
		return true;
	}

protected:
	void on_socket_receive(BYTE* data, size_t size);
	void on_socket_receive_failed(int errcode);
	void OnTimeout();


protected:
	HTTPProxyUDPSender& m_Owner;
	boost::shared_ptr<http_client> m_Socket;
	once_timer m_TimeoutTimer;
	const InetSocketAddress m_ServerAddress;
};

class HTTPProxyUDPSender : public UDPSender
{
protected:
	typedef std::map<HTTPProxyClient*, boost::shared_ptr<HTTPProxyClient> > HTTPProxyClientCollection;
public:

	explicit HTTPProxyUDPSender(DWORD timeout, UDPSenderCallbackType callback) : m_timeout(timeout), m_callback(callback) { }

	void SetCallback(UDPSenderCallbackType callback)
	{
		m_callback = callback;
	}

	virtual bool Send(const void* data, size_t size, const InetSocketAddress& sockAddr)
	{
		boost::shared_ptr<HTTPProxyClient> client(new HTTPProxyClient( *this, sockAddr ) );
		string urlstrHeader = strings::format("http://%s:%d/?q=", AnsiFormatIPAddress(sockAddr.GetIP()).c_str(), sockAddr.GetPort());
		string body = Base64Encoding::Encode( static_cast<const char*>( data ), size );
		string urlstr = urlstrHeader + body;
		if ( false == client->Start( urlstr.c_str(), m_timeout ) )
			return false;
		m_Clients[client.get()] = client;
		return true;
	}

	void OnResponseOK(HTTPProxyClient* client, BYTE* data, size_t size)
	{
		HTTPProxyClientCollection::iterator iter = m_Clients.find( client );
		if ( iter == m_Clients.end() )
			return;

		m_callback(data, size, client->GetServerAddress(), PROXY_HTTP);
		//BYTE* buf = reinterpret_cast<BYTE*>( CORE_MALLOC( size + sizeof( sockaddr_in ) ) );
		//memcpy( buf, data, size );
		//sockaddr_in* sa = reinterpret_cast<sockaddr_in*>( buf + size );
		//*sa = client->GetServerAddress().GetRawAddress();
		//CORE_SendObjectMessage(&m_listener, this, UM_SOCKET_RECVFROM_SUCCESS, (DWORD)buf, size, (DWORD)sa, PROXY_HTTP);
	}
	void OnResponseError(HTTPProxyClient* client, int errcode)
	{
		HTTPProxyClientCollection::iterator iter = m_Clients.find( client );
		if ( iter == m_Clients.end() )
			return;
		m_Clients.erase( iter );
	}

private:
	DWORD m_timeout;
	UDPSenderCallbackType m_callback;
	HTTPProxyClientCollection m_Clients;
};

void HTTPProxyClient::on_socket_receive(BYTE* data, size_t size)
{
	m_TimeoutTimer.stop();
	HTTPResponse resp( reinterpret_cast<const char*>( data ), size );
	int bodyPos = resp.FindBody();
	if ( bodyPos < 0 )
	{
		this->m_Owner.OnResponseError(this, 2);
	}
	else
	{
		size_t pos = static_cast<size_t>( bodyPos );
		assert( pos > 0 && pos < size );
		m_Owner.OnResponseOK(this, data + pos, size - pos);
	}
}

void HTTPProxyClient::on_socket_receive_failed(int errcode)
{
	m_TimeoutTimer.stop();
	this->m_Owner.OnResponseError(this, errcode);
}

void HTTPProxyClient::OnTimeout()
{
	this->m_Owner.OnResponseError(this, 1);
}





#ifndef _PPL_USE_ASIO

UDPSender* UDPSenderFactory::CreateNormal(int udp)
{
	return new NormalUDPSender(udp);
}





#else

UDPSender* UDPSenderFactory::CreateNormal(boost::shared_ptr<udp_socket> udp)
{
	return new NormalUDPSender(udp);
}


#endif


UDPSender* UDPSenderFactory::CreateTCPProxy(UINT timeout, UDPSenderCallbackType callback)
{
	return new TCPProxyUDPSender(timeout, callback);
}

UDPSender* UDPSenderFactory::CreateHTTPProxy(UINT timeout, UDPSenderCallbackType callback)
{
	return new HTTPProxyUDPSender(timeout, callback);
}


