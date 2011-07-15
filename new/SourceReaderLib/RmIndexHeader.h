#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_RM_INDEX_HEADER_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_RM_INDEX_HEADER_H_
#include "rmobject.h"

namespace Synacast
{
	namespace Format
	{

		class SOURCEREADERLIB_API RmIndexHeader :
			public RmObject
		{
		public:
			RmIndexHeader(void);
			~RmIndexHeader(void);

		private:
			UINT32		m_IndexCount;		//     num_indices;
			UINT16		m_StreamID;			//     stream_number;
			UINT32		m_NextIndexOffset;	//     next_index_header;

		public:
			UINT32			GetIndexCount()			const	  { return m_IndexCount; };
			UINT16			GetStreamID()			const	  { return m_StreamID; };
			UINT32			GetNextIndexOffset()	const     { return m_NextIndexOffset; };

		private:
			void			ContentToData();
			UINT32			GetContentSize();

			/************************************************************************/
			/* 
			Exceptions:
				FormatException
			*/
			/************************************************************************/
			void			ParseContent();
		};
	}
}

#endif