
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_PEER_STATUSIO_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_PEER_STATUSIO_H_

#include <synacast/protocol/data/PeerStatus.h>
#include <synacast/protocol/data/PeerMinMaxIO.h>
#include <synacast/protocol/data/DegreeInfoIO.h>
#include <ppl/io/data_input_stream.h>
#include <ppl/io/data_output_stream.h>


inline ppl::io::data_output_stream& operator<<( ppl::io::data_output_stream& os, const PEER_STATUS& status )
{
	return os 
		<< status.UploadBWLeft 
		<< status.Qos 
		<< status.DegreeLeft 
		<< status.OutDegree 
		<< status.InDegree 
		<< status.SkipPercent;
}

inline ppl::io::data_input_stream& operator>>( ppl::io::data_input_stream& is, PEER_STATUS& status )
{
	return is 
		>> status.UploadBWLeft 
		>> status.Qos 
		>> status.DegreeLeft 
		>> status.OutDegree 
		>> status.InDegree 
		>> status.SkipPercent;
}



inline ppl::io::data_output_stream& operator<<( ppl::io::data_output_stream& os, const PEER_STATUS_INFO& info )
{
	return os 
		<< info.Status 
		<< info.MinMax;
}
inline ppl::io::data_input_stream& operator>>( ppl::io::data_input_stream& is, PEER_STATUS_INFO& info )
{
	return is 
		>> info.Status 
		>> info.MinMax;
}




inline ppl::io::data_output_stream& operator<<( ppl::io::data_output_stream& os, const PeerStatusEx& info )
{
	return os 
		<< info.UploadBWLeft 
		<< info.Qos 
		<< info.Degrees 
		<< info.SkipPercent;
}
inline ppl::io::data_input_stream& operator>>( ppl::io::data_input_stream& is, PeerStatusEx& info )
{
	return is 
		>> info.UploadBWLeft 
		>> info.Qos 
		>> info.Degrees 
		>> info.SkipPercent;
}


#endif