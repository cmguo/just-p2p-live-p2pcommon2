#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_RM_DATA_CHUNK_HEADER_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_RM_DATA_CHUNK_HEADER_H_
#include "rmobject.h"

namespace Synacast
{
	namespace Format
	{

		class SOURCEREADERLIB_API RmDataChunkHeader :
			public RmObject
		{
		public:
			RmDataChunkHeader(void);
			~RmDataChunkHeader(void);

		private:
			UINT32		m_PacketCount;		//    num_packets; 
			UINT32		m_NextDataOffset;	//    next_data_header;

		public:
			UINT32		GetPacketCount()	const { return m_PacketCount; };
			UINT32		GetNextDataOffset()	const { return m_NextDataOffset; };

//			void Parse(const byte * data, const size_t size);
		private:
			void		ContentToData();
			UINT32		GetContentSize();

			/************************************************************************/
			/* 
			Exceptions:
				FormatException
			*/
			/************************************************************************/
			void		ParseContent();
		};

	}
}

#endif