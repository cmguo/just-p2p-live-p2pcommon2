
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_NET_ASIO_HTTP_CLIENT_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_NET_ASIO_HTTP_CLIENT_H_

#include <ppl/net/asio/io_service_provider.h>
#include <ppl/net/asio/uri.h>

#include <ppl/data/strings.h>

#include <boost/asio/streambuf.hpp>
#include <boost/asio/deadline_timer.hpp>

#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/shared_ptr.hpp>
using std::string;


class http_client : private boost::noncopyable, public boost::enable_shared_from_this<http_client>
{
public:
	typedef boost::function2<void, unsigned char*, size_t> receive_callback_type;
	typedef boost::function1<void, int> receive_failed_callback_type;

	explicit http_client(boost::asio::io_service& ioservice = io_service_provider::default_service()) : m_socket(ioservice),resolver_(ioservice),  m_detached(false)
	{
		this->request_.reset( new boost::asio::streambuf );
		//	this->response_header_.reset( new boost::asio::streambuf );
		//	this->response_.reset( new boost::asio::streambuf );
	}

	bool create()
	{
		this->close();
		m_detached = false;
		m_socket.open( boost::asio::ip::tcp::v4(), m_last_error );
		return !m_last_error;
	}

	void set_callbacks(receive_callback_type recvCallback, receive_failed_callback_type recvFailedCallback)
	{
		m_receive_callback = recvCallback;
		m_receive_failed_callback = recvFailedCallback;
	}

	bool http_get(const string& url);

	bool close()
	{
		m_detached = true;
		return internal_close();
	}

	static void set_max_http_get_size(size_t size)
	{
		if ( size > 0 )
		{
			ref_max_http_get_size() = size;
		}
	}

protected:
	// void RawRequest( const string & const host, unsigned short port, const string & const rawRequest );
	void RawRequest( unsigned long ip, unsigned short port, const string & rawRequest );

	//virtual void OnTimerElapsed(Timer* sender);

protected:
	static size_t& ref_max_http_get_size()
	{
		static size_t maxGetSize = 64 * 1024;
		return maxGetSize;
	}
	bool internal_close()
	{
		NETWORK_EVENT( "close http client" );
		m_socket.close(m_last_error);
		resolver_.cancel();
		//	http_timer_.cancel();
		return true;
	}

protected:
	void HandleResolve(const boost::system::error_code& err, boost::asio::ip::tcp::resolver::iterator endpoint_iterator);

	void HandleConnect(const boost::system::error_code& err, boost::asio::ip::tcp::resolver::iterator endpoint_iterator);
	void HandleWriteRequest(boost::shared_ptr<boost::asio::streambuf> sbuf, const boost::system::error_code& err);
//	void HandleReadStatusLine(boost::shared_ptr<boost::asio::streambuf> sbuf, const boost::system::error_code& err, size_t bytes_transferred );
//	void HandleReadHeaders(boost::shared_ptr<boost::asio::streambuf> sbuf, const boost::system::error_code& err, size_t bytes_transferred );

	//// by liwei
	//void HandleReadHeader( const boost::asio::error& err );
	void HandleReadContent(boost::shared_ptr<boost::asio::streambuf> sbuf, const boost::system::error_code& err, size_t bytes_transferred );

	void BeginRequest( const string & domain, const unsigned short port );
	bool ParseUrl2Header( const string & url, string & domain, unsigned short & port );
	bool ParseStatusLine( const string & first_line, string & version, unsigned int & code, string & message);

	void OnHttpGet(boost::shared_ptr<boost::asio::streambuf> responseBody);
	void OnHttpGetFail(int nErrCode);

	void BeginConnect( boost::asio::ip::tcp::resolver::iterator endpoint_iterator );
protected:
	// timers
	void HandleResolveTimeout(const boost::system::error_code& err);
	void HandleConnectTimeout( const boost::system::error_code & err );
	void HandleReceiveTimeout(const boost::system::error_code& err);

	template<typename TimeoutHandler>
	void StartTimer( boost::asio::deadline_timer & timer, size_t timeout, TimeoutHandler handler )
	{
		timer.expires_from_now( boost::posix_time::milliseconds(timeout) );
		timer.async_wait( handler );
	}
	void CancelTimer( boost::asio::deadline_timer & timer )
	{
		timer.cancel();
	}

	//virtual void OnConnect(const error_code& error);

protected:
	boost::system::error_code m_last_error;
	boost::asio::ip::tcp::socket m_socket;
	receive_callback_type m_receive_callback;
	receive_failed_callback_type m_receive_failed_callback;

	boost::asio::ip::tcp::endpoint endpoint_;

	boost::shared_ptr<boost::asio::streambuf> request_;
//	boost::shared_ptr<boost::asio::streambuf> response_header_;
//	boost::shared_ptr<boost::asio::streambuf> response_;
	//	boost::asio::streambuf total_respose_;

	//string header_;

	boost::asio::ip::tcp::resolver resolver_;
	//	boost::asio::deadline_timer http_timer_;

	bool m_detached;


	//std::vector<byte>	response_header_;

	//std::vector<byte>	response_data_;
	//std::size_t			received_pos_;

	//string	domain_;
	//unsigned short port_;

};





inline bool http_client::http_get(const string& url)
{
	string domain;
	unsigned short port = 80;

	if( !ParseUrl2Header(url, domain, port ) )
	{
		return false;
	}
	BeginRequest( domain, port );
	return true;
}
//
//void HTTPClientSocket::RawRequest( const string & const host, unsigned short port, const string & const rawRequest  )
//{
//	if( is_closed_ == true ) return;
//
//	std::ostream request_stream(&request_);
//	request_stream << rawRequest;
//
//	BeginRequest( host, port );
//}

inline void http_client::RawRequest( unsigned long ip, unsigned short port, const string & rawRequest )
{
	NETWORK_EVENT( "HTTPClientSocket::RawRequest" );


	//	std::ostream request_stream(request_.get());
	//	request_stream << rawRequest;

	endpoint_ = boost::asio::ip::tcp::endpoint( boost::asio::ip::address_v4(ntohl(ip)), port );


	BeginConnect( boost::asio::ip::tcp::resolver::iterator() );
}

inline void http_client::HandleResolve(const boost::system::error_code& err, boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
{
	//	http_timer_.cancel();

	if (!err)
	{
		endpoint_ = *endpoint_iterator;
		BeginConnect(endpoint_iterator);
	}
	else if( err == boost::asio::error::operation_aborted )
	{
		OnHttpGetFail(-101);
		NETWORK_ERROR( "HTTPClientSocket::HandleResolve Timeout ");
	}
	else
	{	
		// 以错误的方式结束
		OnHttpGetFail(-1);
		NETWORK_ERROR( "HTTPClientSocket::HandleResolve "<<err);
	}
}

inline void http_client::HandleConnect(const boost::system::error_code& err, boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
{


	if (!err)
	{
		//boost::asio::async_write(
		//	m_impl, 
		//	request_,
		//	boost::bind(
		//		&HTTPClientSocket::HandleWriteRequest, 
		//		this,
		//		boost::asio::placeholders::error
		//	)
		//);

		boost::asio::async_write(
			m_socket,
			*request_, 
			boost::bind( 
				&http_client::HandleWriteRequest, 
				this->shared_from_this(), 
				request_, 
				boost::asio::placeholders::error
				)
			);

		// 写不需要超时吧？
		//		http_timer_.expires_from_now(boost::posix_time::milliseconds(20*1000));
		//http_timer_.async_wait(
		//	boost::bind(
		//		&HTTPClientSocket::HandleTimeout, 
		//		boost::static_pointer_cast<HTTPClientSocket>( shared_from_this() ), 
		//		boost::asio::placeholders::error
		//	)
		//		);
	}
	else if( err == boost::asio::error::operation_aborted )
	{
		OnHttpGetFail(-102);
		NETWORK_ERROR( "HTTPClientSocket::HandleConnect Timeout ");
	}
	else if (endpoint_iterator != boost::asio::ip::tcp::resolver::iterator())
	{
		endpoint_ = *endpoint_iterator;
		BeginConnect( ++endpoint_iterator );
	}
	else
	{
		// 以错误的方式结束
		OnHttpGetFail(-2);
		NETWORK_ERROR( "HTTPClientSocket::HandleConnect "<<err);
	}
}

inline void http_client::HandleWriteRequest(boost::shared_ptr<boost::asio::streambuf> request, const boost::system::error_code& err)
{
	NETWORK_EVENT( "HTTPClientSocket::HandleWriteRequest" );

	boost::shared_ptr<boost::asio::streambuf> responseBody(new boost::asio::streambuf);
	if (!err)
	{
		// Read the response status line.
		boost::asio::async_read(
			m_socket, 
			// response_, 
			*responseBody,
			boost::asio::transfer_at_least(1),
			boost::bind( 
				&http_client::HandleReadContent, 
				this->shared_from_this(), 
				responseBody, 
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred
				)
			);

	}
	else if( err == boost::asio::error::operation_aborted )
	{
		OnHttpGetFail(-103);
		NETWORK_ERROR( "HTTPClientSocket::HandleWriteRequest Timeout ");
	}
	else
	{
		// 以错误的方式结束
		OnHttpGetFail(-3);
		NETWORK_ERROR( "HTTPClientSocket::HandleWriteRequest "<<err);
	}
}

//void HTTPClientSocket::HandleReadStatusLine(boost::shared_ptr<boost::asio::streambuf> responseHead, const boost::system::error_code& err, size_t bytes_transferred )
//{
//	NETWORK_EVENT( "HTTPClientSocket::HandleReadStatusLine" );
////	http_timer_.cancel();
//	
//	if (!err)
//	{
//		boost::scoped_array<char> buf( new char[bytes_transferred] );
//		istream is( responseHead.get() );
//		is.read( buf.get(), bytes_transferred);
//		header_.assign( buf.get(), bytes_transferred );
//
//		std::string http_version;
//		unsigned int status_code = 0;
//		std::string status_message;
//
//		bool parsed = ParseStatusLine( header_, http_version, status_code, status_message );
//		if( ! parsed )	// 不是HTTP响应头部
//		{
//			OnHttpGetFail(-4);
//			NETWORK_ERROR( "HTTPClientSocket::HandleReadStatusLine not HTTP ");
//			return;
//		}
//
//		if( ! boost::algorithm::starts_with( http_version, "HTTP") )
//		{
//			OnHttpGetFail(-4);
//			NETWORK_ERROR( "HTTPClientSocket::HandleReadStatusLine not HTTP ");
//			return;
//		}
//		if (status_code != 200)
//		{
//			// 不是以 200 OK 结束，所以退出
//			OnHttpGetFail(-5);
//			NETWORK_ERROR( "HTTPClientSocket::HandleReadStatusLine Response returned with status code"<<status_code);
//			return;
//		}
//
//		// Read the response headers, which are terminated by a blank line.
//		boost::asio::async_read_until(
//			m_Socket->GetSocket(), 
//			*responseHead, 
//			boost::regex("\r\n\r\n"),
//			boost::bind(
//				&TCPSocketImpl::HandleLengthEvent, 
//				m_Socket, 
//				boost::asio::placeholders::error,
//				boost::asio::placeholders::bytes_transferred, 
//				TCPSocketImpl::LengthEventHandler(boost::bind(&HTTPClientSocket::HandleReadHeaders, this, responseHead, _1, _2))
//			)
//		);
//	}
//	else if( err == boost::asio::error::operation_aborted )
//	{
//		OnHttpGetFail(-106);
//		NETWORK_ERROR( "HTTPClientSocket::HandleReadStatusLine Timeout ");
//	}
//	else
//	{
//		// 以错误的方式结束
//		OnHttpGetFail(-6);
//		NETWORK_ERROR( "HTTPClientSocket::HandleReadStatusLine "<<err);
//	}
//
//}
//
//void HTTPClientSocket::HandleReadHeaders(boost::shared_ptr<boost::asio::streambuf> responseHeader, const boost::system::error_code& err, size_t bytes_transferred )
//{
//	NETWORK_EVENT( "HTTPClientSocket::HandleReadHeaders: " << responseHeader->size() );
////	http_timer_.cancel();
//
//	if (!err)
//	{
//		boost::scoped_array<char> buf( new char[bytes_transferred] );
//		istream is( responseHeader.get() );
//		is.read( buf.get(), bytes_transferred );
//		header_ += string( buf.get(), bytes_transferred );
//
//		// other bytes write to response_
//		// 把剩余字节写入
//		//size_t length = response_header_.size();
//		//boost::scoped_array<char> buf2( new char[length] );
//		//is.read( buf2.get(), length );
//		//response_.sputn( buf2.get(), length );
//		// is.read( &response_, response_header_.size() );
//
//		boost::shared_ptr<boost::asio::streambuf> responseBody(new boost::asio::streambuf);
//		boost::asio::async_read(
//			m_Socket->GetSocket(), 
//			*responseBody,
//			boost::asio::transfer_at_least(1),
//			boost::bind(
//				&TCPSocketImpl::HandleLengthEvent, 
//				m_Socket, 
//				boost::asio::placeholders::error,
//				boost::asio::placeholders::bytes_transferred, 
//				TCPSocketImpl::LengthEventHandler(boost::bind(&HTTPClientSocket::HandleReadContent, this, responseBody, _1, _2))
//			)
//		);
//	}
//	else if( err == boost::asio::error::operation_aborted )
//	{
//		OnHttpGetFail(-107);
//		NETWORK_ERROR( "HTTPClientSocket::HandleReadHeaders Timeout ");
//	}
//	else
//	{
//		// 以错误的方式结束
//		OnHttpGetFail(-7);
//		NETWORK_ERROR( "HTTPClientSocket::HandleResolve "<<err);
//	}
//}

inline void http_client::HandleReadContent(boost::shared_ptr<boost::asio::streambuf> responseBody, const boost::system::error_code& err, size_t bytes_transferred )
{
	NETWORK_EVENT( "HTTPClientSocket::HandleReadContent: " << responseBody->size() );
	//	http_timer_.cancel();

	if (!err)
	{
		//boost::asio::async_read(
		//	m_impl, 
		//	response_,
		//	boost::asio::transfer_at_least(1),
		//	boost::bind(
		//	&HTTPClientSocket::HandleReadContent, 
		//	boost::static_pointer_cast<HTTPClientSocket>( shared_from_this() ),
		//	boost::asio::placeholders::error
		//	)
		//	);
		if ( responseBody->size() > ref_max_http_get_size() )
		{
			OnHttpGetFail(-109);
			return;
		}
		boost::asio::async_read(
			m_socket, 
			*responseBody,
			boost::asio::transfer_at_least(1),
			boost::bind(
				&http_client::HandleReadContent, 
				this->shared_from_this(), 
				responseBody, 
				boost::asio::placeholders::error, 
				boost::asio::placeholders::bytes_transferred
				)
			);

		//http_timer_.expires_from_now(boost::posix_time::milliseconds(10*1000));
		//http_timer_.async_wait(
		//	boost::bind(
		//		&HTTPClientSocket::HandleTimeout, 
		//		boost::static_pointer_cast<HTTPClientSocket>( shared_from_this() ), 
		//		boost::asio::placeholders::error
		//	)
		//);
	}
	else if( err == boost::asio::error::operation_aborted )
	{
		OnHttpGetFail(-108);
		NETWORK_ERROR( "HTTPClientSocket::HandleResolve Timeout ");
	}
	else if (err == boost::asio::error::eof)
	{	// 以错误的方式结束
		OnHttpGet(responseBody);
	}
	else
	{	// 接受数据完成
		OnHttpGetFail(-8);
		NETWORK_ERROR( "HTTPClientSocket::HandleResolve "<<err);
	}
}

inline void http_client::HandleReceiveTimeout(const boost::system::error_code& err)
{
	if( err != boost::asio::error::operation_aborted )
	{
		NETWORK_EVENT("HTTPClientSocket::HandleReceiveTimeout "<<endpoint_);
		this->internal_close();
	}
}

inline void http_client::HandleResolveTimeout( const boost::system::error_code& err )
{
	if( err != boost::asio::error::operation_aborted )
	{
		NETWORK_EVENT("HTTPClientSocket::HandleResolveTimeout");
		this->internal_close();
	}
}

inline void http_client::HandleConnectTimeout( const boost::system::error_code & err )
{
	if( err != boost::asio::error::operation_aborted )
	{
		NETWORK_EVENT("HTTPClientSocket::HandleConnectTimeout");
		this->internal_close();
	}
}

inline void http_client::OnHttpGet(boost::shared_ptr<boost::asio::streambuf> responseBody)
{
	// 回应全部HTTP响应
	//size_t bytes_transferred = response_.size();
	size_t bytes_transferred = responseBody->size();

	if( bytes_transferred == 0)
	{
		OnHttpGetFail(-9);
		return;
	}
	/*
	LPBUFFER lpBuffer = BufferManager::PopBuffer(bytes_transferred);
	if ( ! lpBuffer)
	{
	OnHttpGetFail(-10);
	return;
	}

	// std::copy( header_.begin(), header_.end(), lpBuffer->pData );
	memcpy_s( lpBuffer->pData, lpBuffer->nDataSize, header_.c_str(), header_.length() );
	std::istream request_stream(&response_);
	request_stream.read( (char*)(lpBuffer->pData + header_.length() ), response_.size());
	lpBuffer->nUsed = bytes_transferred;

	CORE_SendObjectMessage(
	(HOBJ)obj_, 
	(HOBJ)handle_, 
	UM_SOCKET_HTTPGET_SUCCESS,
	(UINT)lpBuffer->pData, 
	(UINT)lpBuffer->nUsed,
	0, 0);
	*/
	byte_buffer buf;
	buf.resize( bytes_transferred );
	std::istream request_stream(responseBody.get());
	request_stream.read( (char*)( buf.data() ), responseBody->size());
	internal_close();
	if (!m_detached)
	{
		m_receive_callback(buf.data(), buf.size());
	}
}

inline void http_client::OnHttpGetFail(int nErrCode)
{
	/*	CORE_SendObjectMessage(
	(HOBJ)obj_, 
	(HOBJ)handle_, 
	UM_SOCKET_HTTPGET_FAIL,
	(UINT)nErrCode, 
	0, 0, 0);
	*/
	internal_close();
	if (!m_detached)
	{
		m_receive_failed_callback(nErrCode);
	}
}

inline bool http_client::ParseUrl2Header( const string & url, string & domain, unsigned short & port )
{
	Uri uri(url);

	domain = uri.getdomain();
	if( domain.size() == 0 )
	{	// 
		OnHttpGetFail(-11);
		return false;
	}

	port = static_cast<unsigned short>(atoi(uri.getport().c_str()));
	if ( port <= 0 )
		port = 80;

	string path = uri.getrequest();
	if( path.size() == 0 ) path = "/";

	//	urlparser paser(url);
	//domain = paser.getdomain();
	//string port_s = paser.getport();
	//string path = paser.getpath();

	//port = 80;
	//if( port_s.size() != 0 )
	//{
	//	try
	//	{
	//		port = boost::lexical_cast<u_short>(port_s);
	//	}
	//	catch( boost::bad_lexical_cast &)
	//	{
	//		//OnHttpGetFail(-12);
	//		//return false;
	//	}
	//}

	std::ostream request_stream(request_.get());
	request_stream << "GET " << path << " HTTP/1.0\r\n";
	request_stream << "Host: " << domain << "\r\n";
	request_stream << "Accept: */*\r\n";
	request_stream << "Connection: close\r\n\r\n";

	return true;

}

inline void http_client::BeginRequest( const string & domain, const unsigned short port )
{
	boost::asio::ip::tcp::resolver::query query(domain, strings::format("%hu", port));
	//	resolver_.resolve( query );
	resolver_.async_resolve(
		query,
		boost::bind(
		&http_client::HandleResolve, 
		this->shared_from_this(),
		boost::asio::placeholders::error,
		boost::asio::placeholders::iterator
		)
		);
}

inline void http_client::BeginConnect( boost::asio::ip::tcp::resolver::iterator endpoint_iterator )
{
	//m_impl.async_connect(
	//	endpoint_,
	//	boost::bind(
	//	&HTTPClientSocket::HandleConnect, 
	//	this,
	//	boost::asio::placeholders::error, 
	//	endpoint_iterator
	//	)
	//	);

	//	this->Connect(from_endpoint(*endpoint_iterator));

	m_socket.async_connect(
		endpoint_,
		boost::bind( 
			&http_client::HandleConnect,
			this->shared_from_this(), 
			boost::asio::placeholders::error, 
			endpoint_iterator
			)
		);


	//connect_timer_.expires_from_now( boost::posix_time::milliseconds( connect_timeout_ ) );
	//connect_timer_.async_wait(
	//		boost::bind(
	//		&HTTPClientSocket::HandleConnectTimeout,
	//		boost::static_pointer_cast<HTTPClientSocket>( shared_from_this() ),
	//		boost::asio::placeholders::error
	//		)
	//	);

	//StartTimer( connect_timer_, 20 * 1000, 
	//	boost::bind(
	//	&HTTPClientSocket::HandleConnectTimeout,
	//	boost::static_pointer_cast<HTTPClientSocket>( shared_from_this() ),
	//	_1
	//	)
	//	);
}

inline bool http_client::ParseStatusLine( const string & first_line, string & version, unsigned int & code, string & message )
{
	std::vector<string> str_s;
	boost::algorithm::split(str_s, first_line, boost::algorithm::is_any_of(" "));
	if( str_s.size() < 3 )
	{
		return false;
	}
	boost::algorithm::trim(str_s[0]);
	boost::algorithm::to_upper(str_s[0]);
	version = str_s[0];
	code = atoi(str_s[1].c_str());

	for(size_t i = 2; i < str_s.size(); i ++)
	{
		message.append(str_s[i] + " ");
	}
	boost::algorithm::trim(message);

	return true;
}

//void HTTPClientSocket::OnTimerElapsed( Timer* sender )
//{
//	
//}


//void HTTPClientSocket::OnConnect( const error_code& error )
//{
//	if ( err )
//	{
//		this->Close();
//	}
//}


#endif
