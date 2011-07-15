
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_PEER_MIN_MAX_IO_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_PEER_MIN_MAX_IO_H_

#include <synacast/protocol/data/PeerMinMax.h>
#include <ppl/io/data_input_stream.h>
#include <ppl/io/data_output_stream.h>


inline ppl::io::data_output_stream& operator<<( ppl::io::data_output_stream& os, const PEER_MINMAX& minmax )
{
	return os 
		<< minmax.MinIndex 
		<< minmax.MaxIndex;
}
inline ppl::io::data_input_stream& operator>>( ppl::io::data_input_stream& is, PEER_MINMAX& minmax )
{
	return is 
		>> minmax.MinIndex 
		>> minmax.MaxIndex;
}


#endif

