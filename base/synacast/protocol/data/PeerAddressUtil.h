
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_PEER_ADDRESS_UTIL_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_PEER_ADDRESS_UTIL_H_

#include <synacast/protocol/data/PeerAddress.h>
#include <ppl/data/tstring.h>
#include <ppl/data/int.h>
#include <set>


class PeerAddressUtil
{
public:
	static bool ParseAddress( PEER_ADDRESS& addr, const string& s );
	static void ParseAddressList( std::set<PEER_ADDRESS>& addrs, const tstring& s );
	static void ParseIPList( std::set<UINT32>& ips, const tstring& s );
};


#endif
