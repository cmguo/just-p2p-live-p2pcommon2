#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_HEADER_CHUNK_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_HEADER_CHUNK_H_


#include "DataBuffer.h"
using namespace Synacast::Common;

#include "SourceTypes.h"
using namespace Synacast::StreamSource;

namespace Synacast
{
	namespace SourceReader
	{
		class SOURCEREADERLIB_API HeaderChunk
		{
		public:
			HeaderChunk(FileFormat	format);
			virtual ~HeaderChunk(void);

		protected:
			DataBuffer	m_RawData;

			FileFormat	m_Format;

		public:
			const DataBuffer & GetRawData() const { return m_RawData; };

			virtual void SetRawData( const char * data, const size_t size ) = 0;
//			virtual void AppendRawData( byte * data, size_t size );

			FileFormat GetFormat() const { return m_Format; };

			virtual size_t	GetMaxPacketSize() const = 0;
			virtual size_t	GetMaxBitrate() const = 0;

		//private:
		//	virtual void Parse() = 0;
		};
	}
}

#endif