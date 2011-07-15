#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_RM_FILE_HEADER_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_RM_FILE_HEADER_H_
#include "rmobject.h"

namespace Synacast
{
	namespace Format
	{

		class SOURCEREADERLIB_API RmFileHeader :
			public RmObject
		{
		public:
			RmFileHeader(void);
			~RmFileHeader(void);

		private:
			UINT32			m_FileVersion;		// host byte order
			UINT32			m_HeaderCount;		// host byte order

		public:
			UINT32	GetFileVersion() const { return m_FileVersion; };
			UINT32	GetHeaderCount() const { return m_HeaderCount; };
			void	SetHeaderCount( UINT32 headerCount ) { m_HeaderCount = headerCount; };

		private:
			void	ContentToData();
			UINT32	GetContentSize();

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