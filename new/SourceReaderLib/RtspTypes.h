#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_RTSP_TYPES_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_RTSP_TYPES_H_

namespace Synacast
{
	namespace Protocol
	{
		namespace Rtsp
		{
//#if _MSC_VER < 1400
			typedef UINT16 PacketType;
			const static PacketType	RDT_ACK					= 0xFF02;
			const static PacketType	RDT_RTT_RESPONSE		= 0xFF04;
			const static PacketType	RDT_STREAM_END			= 0xFF06;
			const static PacketType	RDT_LATENCY_REPORT		= 0xFF08;
//#else
//			enum PacketType : UINT16
//			{
//				RDT_ACK					= 0xFF02,
//				RDT_RTT_RESPONSE		= 0xFF04,
//				RDT_STREAM_END			= 0xFF06,
//				RDT_LATENCY_REPORT		= 0xFF08
//			};
//#endif
		}
	}
}

#endif