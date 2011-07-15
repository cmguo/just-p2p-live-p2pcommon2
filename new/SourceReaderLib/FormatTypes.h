#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_FORMAT_TYPES_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_FORMAT_TYPES_H_

namespace Synacast
{
	namespace Format
	{
#if _MSC_VER < 1400
		typedef int AsfLengthType;

		const static AsfLengthType LENGTH_NOT_EXISTS		= 0;
		const static AsfLengthType LENGTH_IN_BYTE			= 1;
		const static AsfLengthType LENGTH_IN_WORD			= 2;
		const static AsfLengthType LENGTH_IN_DWORD			= 3;

		typedef UINT32 RmHeaderID;

		const static RmHeaderID			RM_FILE = 0x464d522e;
		const static RmHeaderID			RM_PROPERTIES = 0x504F5250;
		const static RmHeaderID			RM_MEDIA_PROPERTIES = 0x5250444D;
		const static RmHeaderID			RM_CONTENT_DESCRIPTION = 0x544E4F43;
		const static RmHeaderID			RM_DATA_CHUNK = 0x41544144;
		const static RmHeaderID			RM_INDEX = 0x58444E49;
#else
		enum AsfLengthType
		{
			LENGTH_NOT_EXISTS		= 0,
			LENGTH_IN_BYTE			= 1,
			LENGTH_IN_WORD			= 2,
			LENGTH_IN_DWORD			= 3
		};

		enum RmHeaderID
		{
			RM_FILE = 0x464d522e,
			RM_PROPERTIES = 0x504F5250,
			RM_MEDIA_PROPERTIES = 0x5250444D,
			RM_CONTENT_DESCRIPTION = 0x544E4F43,
			RM_DATA_CHUNK = 0x41544144,
			RM_INDEX = 0x58444E49
		};
#endif
		enum RmStreamType
		{
			AUDIO_STREAM			= 0,
			VIDEO_STREAM			= 1,
			OTHER_STREAM			= 10
		};
	}
}


#endif