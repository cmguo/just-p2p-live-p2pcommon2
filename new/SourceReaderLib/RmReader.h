#pragma warning( disable: 4786 )

#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_RM_READER_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_RM_READER_H_
#include "Reader.h"

#include "Source.h"
using namespace Synacast::StreamSource;

#include <set>

namespace Synacast
{
	namespace SourceReader
	{

		class SOURCEREADERLIB_API RmReader :
			public Reader, private noncopyable
		{
		public:
			RmReader( Source & source );
			~RmReader(void);

		private:
			Source &	m_RmSource;
			UINT32		m_PacketCount;
			UINT32		m_PacketRead;

			set<UINT16>	m_VideoStreamID;
			set<UINT16>	m_AudioStreamID;

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
			void	Close();

		private:
			void	Reset();
		};
	}
}

#endif
