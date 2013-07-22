#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_SOURCE_TYPES_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_SOURCE_TYPES_H_

namespace Synacast 
{
	namespace StreamSource 
	{

		// enums

		/************************************************************************/
		/* 
		0 1 2 3  4 5 6 7        8 9 10 11  12 13 14 15
		+ + + -  + - - +        + + ++ ++  ++ ++ ++ ++
		| | |    |              | | |  |   |  |  |  |
		+ + + -  + - - +        + + ++ ++  ++ ++ ++ ++
		^ ^ ^    ^     ^        ^ ^ ^  ^   ^  ^  ^  ^ 
		| | |    |     |        | | |  |   |  |  |  UDP    
		| | |    |     |        | | |  |   |  |  TCP      
		| | |    |     |        | | |  |   |  REAL RTSP
		| | |    |     |        | | |  |   MS RTSP
		| | |    |     |        | | |  RTSP
		| | |    |     |        | | MMS   
		| | |    |     WM SDK   | MMSH    
		| | |    source         HTTP      
		| | buffer
		| network
		file    


		SOURCE:		0000 1000 0000 0000

		FILE:		1000 1000 0000 0000 *
		
		Network:	0100 1000 0000 0000
		HTTP:		0100 1000 1000 0000 *
		MMSH:		0100 1000 0100 0000 *

		Buffer:     0110 1000 0000 0000
		
		MMS:		0110 1000 0010 0000 *
		MMST:		0110 1000 0010 0010
		MMSU:		0110 1000 0010 0001

		RTSP:		0110 1000 0001 0000
		MS RTSP:	0110 1000 0001 1000 *
		MS RTSP TCP:0110 1000 0001 1010
		REAL RTSP:	0110 1000 0001 0100 *
		RM RTSP TCP:0110 1000 0001 0110

		WM SDK:		0110 1001 0000 0000 *
		*/
		/************************************************************************/
		enum SourceType 
		{
//			SOURCE				= 0x0800,

			FILE_SOURCE			= 0x8800,		// *

			NETWORK_SOURCE		= 0x4800,
			
			HTTP_SOURCE			= 0x4880,		// *

			MMS_HTTP_SOURCE		= 0x4840,		// *

			BUFFERRED_SOURCE	= 0x6800,

			MMS_SOURCE			= 0x6820,		// *
			MMS_TCP_SOURCE		= 0x6822,
			MMS_UDP_SOURCE		= 0x6821,

			RTSP_SOURCE			= 0x6810,
			MS_RTSP_SOURCE		= 0x6818,		// *
			MS_RTSP_TCP_SOURCE	= 0x681A,

			REAL_RTSP_SOURCE	= 0x6814,		// *
			REAL_RTSP_TCP_SOURCE= 0x6816,

			WMF_SDK_SOURCE		= 0x6900

/*
			MS_RTSP_UDP_SOURCE	= 0x0122,

			REAL_RTSP_UDP_SOURCE= 0x0125,
			REAL_RTSP_TCP_SOURCE= 0x0126
*/
		};

		enum FileFormat 
		{
			RAW					= 0x00,
			ASF					= 0x01,
			RMFF				= 0x02,
            MKV					= 0x03, 
            FLV                 = 0x04
		};

		enum SeekOrigin 
		{
			BEGIN				= FILE_BEGIN,
			CURRENT				= FILE_CURRENT,
			END					= FILE_END
		};

		enum TransportNetwork {
			TCP_TRANSPORT		= 0x01,
			UDP_TRANSPORT		= 0x02
		};


	}
}
#endif