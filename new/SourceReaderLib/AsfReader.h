#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_ASFREADER_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_ASFHEADER_H_

#include "Reader.h"
#include "HeaderChunk.h"
#include "PacketChunk.h"
#include "IndexChunk.h"
#include "Source.h"
using namespace Synacast::StreamSource;

namespace Synacast
{
	namespace SourceReader
	{

		class SOURCEREADERLIB_API AsfReader :
			public Reader, private noncopyable
		{
		public:
			AsfReader( Source & source );
			virtual ~AsfReader(void);

		private:
			Source &	m_AsfSource;

			UINT32		m_MaxPacketSize;
			UINT32		m_MinPacketSize;
			UINT64		m_PacketCount;
			UINT64		m_PacketRead;

		public:
			/************************************************************************/
			/* 
			Exceptions:
				SourceException
			*/
			/************************************************************************/
			virtual HeaderChunk * ReadHeader();
			/************************************************************************/
			/* 
			Exceptions:
				SourceException
			*/
			/************************************************************************/
			virtual PacketChunk * ReadDataPacket();
			virtual IndexChunk * ReadIndex();
			virtual void Close();

		private:
			void		Reset();
		};

	}
}
#endif