
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_BOOSTLIB_ENDPOINT_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_BOOSTLIB_ENDPOINT_H_

#include <ppl/net/inet.h>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>


typedef boost::asio::ip::udp::endpoint udp_endpoint;
typedef boost::asio::ip::tcp::endpoint tcp_endpoint;


namespace ppl { namespace boostlib {

inline udp_endpoint make_udp_endpoint( const InetSocketAddress& addr )
{
	udp_endpoint ep( boost::asio::ip::address_v4(addr.GetIPHostByteOrder()), addr.GetPort() );
	return ep;
}

inline tcp_endpoint make_tcp_endpoint( const InetSocketAddress& addr )
{
	tcp_endpoint ep( boost::asio::ip::address_v4(addr.GetIPHostByteOrder()), addr.GetPort() );
	return ep;
}

inline InetSocketAddress from_endpoint( const udp_endpoint& ep )
{
	InetSocketAddress addr( htonl( ep.address().to_v4().to_ulong() ), ep.port() );
	return addr;
}

inline InetSocketAddress from_endpoint( const tcp_endpoint& ep )
{
	InetSocketAddress addr( htonl( ep.address().to_v4().to_ulong() ), ep.port() );
	return addr;
}

} }

#endif
