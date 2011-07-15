#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_ASFHEADER_CHUNK_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_ASFHEADER_CHUNK_H_

#include "HeaderChunk.h"
#include "AsfDataObjectHeader.h"
#include "AsfHeaderObject.h"
using namespace Synacast::Format;

namespace Synacast
{
	namespace SourceReader
	{
		class SOURCEREADERLIB_API AsfHeaderChunk :
			public HeaderChunk
		{
		public:
			AsfHeaderChunk(void);
			virtual ~AsfHeaderChunk(void);

		private:
			AsfHeaderObject		m_HeaderObject;
			AsfDataObjectHeader	m_DataObjectHeader;

		public:
			/************************************************************************/
			/* 
			异常：
			FormatException, 格式不对
			*/
			/************************************************************************/
			virtual void SetRawData( const char * data, const size_t size );

			const AsfHeaderObject & GetAsfHeaderObject() const { return m_HeaderObject; };
			const AsfDataObjectHeader & GetAsfDataObjectHeader() const { return m_DataObjectHeader; };

			size_t	GetMaxPacketSize() const { return m_HeaderObject.GetFilePropertiesObject()->GetMaxPacketSize(); };
			size_t	GetMaxBitrate() const { return m_HeaderObject.GetFilePropertiesObject()->GetMaxBitrate(); };
		};

	}
}
#endif