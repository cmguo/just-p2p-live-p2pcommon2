
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_NET_INFOIO_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_NET_INFOIO_H_

#include <synacast/protocol/data/NetInfo.h>
#include <ppl/io/data_input_stream.h>
#include <ppl/io/data_output_stream.h>


inline ppl::io::data_output_stream& operator<<( ppl::io::data_output_stream& os, const PEER_CORE_INFO& coreInfo )
{
	// PEER_CORE_INFO里面都是字节，不需要处理字节序
    char buf[PEER_CORE_INFO::object_size];
    buf[0] = (coreInfo.PeerType) | (coreInfo.PeerNetType << 4) | (coreInfo.Reserved << 6);
    buf[1] = (coreInfo.NATType) | (coreInfo.Reserved2 << 4);
    memcpy(buf + 2, coreInfo.Reserved1, sizeof(coreInfo.Reserved1));
	os.write_n( buf, PEER_CORE_INFO::object_size );
	return os;
}
inline ppl::io::data_input_stream& operator>>( ppl::io::data_input_stream& is, PEER_CORE_INFO& coreInfo )
{
    char buf[PEER_CORE_INFO::object_size];
	is.read_n( buf, PEER_CORE_INFO::object_size );
    coreInfo.PeerType = buf[0] & 15;
    coreInfo.PeerNetType = (buf[0] >> 4) & 3;
    coreInfo.Reserved = (buf[0] >> 6) & 3;
    coreInfo.NATType = buf[1] & 15;
    coreInfo.Reserved2 = (buf[1] >> 4) & 15;
    memcpy(coreInfo.Reserved1, buf + 2, sizeof(coreInfo.Reserved1));
	return is;
}

#endif