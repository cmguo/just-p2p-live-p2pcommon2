
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_PACKET_HEADIO_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_PACKET_HEADIO_H_

#include <synacast/protocol/PacketHead.h>
#include <synacast/protocol/data/NetInfoIO.h>
#include <synacast/protocol/data/PeerAddressIO.h>
#include <synacast/protocol/data/DegreeInfoIO.h>
#include <synacast/protocol/DataIO.h>
#include <ppl/data/guid_io.h>




inline data_output_stream& operator<<( data_output_stream& os, const OLD_UDP_PACKET_HEAD& info )
{
	return os 
		<< info.AppType 
		<< info.Action 
		<< info.ActionType 
		<< info.Magic 
		<< info.ProtocolVersion 
		<< info.TransactionID;
}
inline data_input_stream& operator>>( data_input_stream& is, OLD_UDP_PACKET_HEAD& info )
{
	return is 
		>> info.AppType 
		>> info.Action 
		>> info.ActionType 
		>> info.Magic 
		>> info.ProtocolVersion 
		>> info.TransactionID;
}


inline data_output_stream& operator<<( data_output_stream& os, const TCP_PACKET_HEAD& info )
{
	return os 
		<< info.Action 
		<< info.ReservedActionType 
		<< info.ProtocolVersion;
}
inline data_input_stream& operator>>( data_input_stream& is, TCP_PACKET_HEAD& info )
{
	return is 
		>> info.Action 
		>> info.ReservedActionType 
		>> info.ProtocolVersion;
}


inline data_output_stream& operator<<( data_output_stream& os, const NEW_UDP_PACKET_HEAD& info )
{
	return os 
		<< info.Action 
		<< info.ReservedActionType 
		<< info.ProtocolVersion 
		<< info.TransactionID;
}
inline data_input_stream& operator>>( data_input_stream& is, NEW_UDP_PACKET_HEAD& info )
{
	return is 
		>> info.Action 
		>> info.ReservedActionType 
		>> info.ProtocolVersion 
		>> info.TransactionID;
}




inline data_output_stream& operator<<( data_output_stream& os, const UDP_SESSION_INFO& info )
{
	return os 
		<< info.SequenceID 
		<< info.SessionKey;
}
inline data_input_stream& operator>>( data_input_stream& is, UDP_SESSION_INFO& info )
{
	return is 
		>> info.SequenceID 
		>> info.SessionKey;
}



inline data_output_stream& operator<<( data_output_stream& os, const PACKET_PEER_INFO& info )
{
	return os 
		<< info.ChannelGUID 
		<< info.PeerGUID 
		<< info.Address 
		<< info.OuterAddress 
		<< info.DetectedRemoteAddress 
		<< info.Degrees 
		<< info.AppVersion 
		<< info.IsEmbedded 
		<< info.CoreInfo;
}
inline data_input_stream& operator>>( data_input_stream& is, PACKET_PEER_INFO& info )
{
	return is 
		>> info.ChannelGUID 
		>> info.PeerGUID 
		>> info.Address 
		>> info.OuterAddress 
		>> info.DetectedRemoteAddress 
		>> info.Degrees 
		>> info.AppVersion 
		>> info.IsEmbedded 
		>> info.CoreInfo;
}


#endif
