#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_RM_OBJECT_HEADER_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_RM_OBJECT_HEADER_H_
#include "FormatTypes.h"

namespace Synacast
{
	namespace Format
	{

		class RmObjectHeader
		{
		public:
			RmObjectHeader(RmHeaderID id);
			~RmObjectHeader(void);

		public:
			RmHeaderID	m_ID;			// no byte order
			UINT32		m_Size;			// host byte order
			UINT16		m_Version;		// host byte order
		public:
			static inline UINT32 GetObjectHeaderSize() { return sizeof( RmHeaderID ) + sizeof( UINT32 ) + sizeof( UINT16 ); };

			/************************************************************************/
			/* 
			异常：
				FormatException, 数据长度小于头长度(10)
			*/
			/************************************************************************/
			void			Parse( const char * data, const size_t size );
		};
	}
}

#endif