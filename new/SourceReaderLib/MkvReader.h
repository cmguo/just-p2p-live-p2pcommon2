#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_MKV_READER_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_MKV_READER_H_

#include "Reader.h"

#include "HeaderChunk.h"
#include "PacketChunk.h"
#include "IndexChunk.h"
#include "Source.h"
#include "MKVReaderFace.h"

#define MAXCLUSTERSIZE (1024 * 64)

using namespace Synacast::StreamSource;

namespace Synacast
{
	namespace SourceReader
	{
		class SOURCEREADERLIB_API MKVReader :
			public Reader
		{
		public:
			MKVReader( const char * const url, const size_t urlLength );
			virtual ~MKVReader(void);

		private:
			char			m_SouceUrl[1024];
			CMKVReaderFace	m_MKVReadFace;
			void		*	m_MKVReader;

		//	CMKVParser	m_MKVParser;

			UINT32		m_MaxPacketSize;
			UINT32		m_MinPacketSize;
			UINT64		m_PacketCount;
			UINT64		m_PacketRead;

			BYTE		m_Buf[MAXCLUSTERSIZE];

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