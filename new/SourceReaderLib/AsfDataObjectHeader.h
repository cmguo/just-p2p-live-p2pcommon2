
#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_ASFDATA_OBJECT_HEADER_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_ASFDATA_OBJECT_HEADER_H_
#include "AsfObject.h"

namespace Synacast
{
	namespace Format
	{
		class SOURCEREADERLIB_API AsfDataObjectHeader : public AsfObject
		{
		public:
			AsfDataObjectHeader(void);
			~AsfDataObjectHeader(void);

		private:
			GUID				m_FileID;
			UINT64				m_PacketCount;

		public:
			/************************************************************************/
			/* 
			异常：
				FormatException, 格式不对
			*/
			/************************************************************************/
			void			Parse( const char * data, const size_t size );
		};

	}
}

#endif