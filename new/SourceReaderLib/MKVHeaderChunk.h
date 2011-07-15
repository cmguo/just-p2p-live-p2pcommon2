#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_MKV_HEADER_CHUNK_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_MKV_HEADER_CHUNK_H_
#include "HeaderChunk.h"

namespace Synacast
{
	namespace SourceReader
	{
		class SOURCEREADERLIB_API MKVHeaderChunk :
			public HeaderChunk
		{
		public:
			MKVHeaderChunk(void);
			virtual ~MKVHeaderChunk(void);

		private:

		public:
			/************************************************************************/
			/* 
			异常：
			FormatException, 格式不对
			*/
			/************************************************************************/
			virtual void SetRawData( const char * data, const size_t size );

			size_t	GetMaxPacketSize() const {return (1024 * 8);};
			size_t	GetMaxBitrate() const {return 500;};
		};

	}
}
#endif