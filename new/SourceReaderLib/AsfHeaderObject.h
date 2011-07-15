#ifdef _PPL_PLATFORM_MSWIN
#pragma warning(disable: 4251)
#endif
#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_ASFHEADER_OBJECT_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_ASFHEADER_OBJECE_H_
#include "SourceReaderLib.h"

#include "AsfObject.h"
#include "AsfFilePropertiesObject.h"

#include <vector>
using namespace std;

namespace Synacast
{
	namespace Format
	{
		class SOURCEREADERLIB_API AsfHeaderObject : public AsfObject
		{
		public:
			AsfHeaderObject(void);
			~AsfHeaderObject(void);

		private:
			UINT32						m_HeaderCount;
			vector<AsfObject*>			m_Headers;

			AsfFilePropertiesObject *	m_FilePropertiesObject;

			void ClearHeaders();

		public:
			const vector<AsfObject*> &	GetHeaders() const { return m_Headers; };
			/************************************************************************/
			/* 
			“Ï≥££∫
				FormatException
			*/
			/************************************************************************/
			void						Parse( const char * data, const size_t size );
			const AsfFilePropertiesObject * GetFilePropertiesObject() const { return m_FilePropertiesObject; };
		//	void						AppendRawData( byte * data, size_t size );
		};
	}
}

#endif
