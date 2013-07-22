#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_READER_TYPES_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_READER_TYPES_H_

namespace Synacast
{
	namespace SourceReader
	{
		enum ReaderType
		{
			ASF_READER		= 0x01,
			RM_READER		= 0x02,
			//			WMF_SDK_READER	= 0x03
			MKV_READER		= 0x03,
			FLV_READER		= 0x04	// Added by Tady, 06192013.
		};
	}
}

#endif