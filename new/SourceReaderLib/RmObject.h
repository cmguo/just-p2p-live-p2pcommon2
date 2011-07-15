#pragma warning(disable: 4251)
#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_RM_OBJECT_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_RM_OBJECT_H_
#include "SourceReaderLib.h"

#include "DataBuffer.h"
using namespace Synacast::Common;

#include "RmObjectHeader.h"

namespace Synacast
{
	namespace Format
	{

		class SOURCEREADERLIB_API RmObject
		{
		public:
			RmObject( RmObjectHeader headerType );
			virtual ~RmObject(void);

		protected:
			RmObjectHeader		m_Header;

			DataBuffer			m_RawData;

		public:
			RmHeaderID		GetID()		const { return m_Header.m_ID; };
			UINT32			GetSize();
			UINT16			GetVersion()const { return m_Header.m_Version; };

		public:
			virtual const DataBuffer &	ToData();
			/************************************************************************/
			/* 
			“Ï≥££∫
				FormatException
			*/
			/************************************************************************/
			virtual void SetRawData( const char * data, const size_t size );

		protected:
			virtual void	Parse();
			virtual void	ParseContent() = 0;

			virtual	void	BuildRawData();
			virtual	void	HeaderToData();
			virtual void	ContentToData()	= 0;
			virtual UINT32	GetContentSize()= 0;
		};
	}
}

#endif