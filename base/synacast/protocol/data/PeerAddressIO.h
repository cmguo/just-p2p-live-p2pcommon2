#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_PEER_ADDRESSIO_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_PEER_ADDRESSIO_H_

#include <synacast/protocol/data/PeerAddress.h>
#include <ppl/io/data_input_stream.h>
#include <ppl/io/data_output_stream.h>


inline ppl::io::data_output_stream& operator<<( ppl::io::data_output_stream& os, const PEER_ADDRESS& addr )
{
    os.write_n(&addr.IP, sizeof(addr.IP ));
	return os 
		<< addr.TcpPort 
		<< addr.UdpPort;
}
inline ppl::io::data_input_stream& operator>>( ppl::io::data_input_stream& is, PEER_ADDRESS& addr )
{
    is.read_n(&addr.IP, sizeof(addr.IP ));
	return is 
		>> addr.TcpPort 
		>> addr.UdpPort;
}


#endif
