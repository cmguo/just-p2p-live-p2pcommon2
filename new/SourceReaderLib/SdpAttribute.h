#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_SDP_ATTRIBUTE_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_SDP_ATTRIBUTE_H_

#include "SdpDescription.h"

namespace Synacast
{
	namespace Protocol
	{
		namespace Rtsp
		{

			//enum SessionAttributeValueType
			//{
			//	NONE			= 0,
			//	INTEGER			= 1,
			//	STRING			= 2,
			//	BUFFER			= 3
			//};

			class SdpAttribute :
				public SdpDescription
			{
			public:
				SdpAttribute( const char type, const string & value );
				~SdpAttribute(void);

			private:
				string			m_Name;
				string			m_ValueType;
				string			m_AttributeValue;

				string			m_StringValue;
				string			m_BufferStringValue;

			public:
				const string &				GetAttributeName()	const { return m_Name; };
//				SessionAttributeValueType	GetValueType()		const { return m_ValueType; };
				const string &				GetValueType()		const { return m_ValueType; };

				const string &	GetRawValue()		const { return m_AttributeValue; };
				const string &	GetStringValue()	;
				const string &	GetBufferValue()	;
				int				GetIntegerValue()	const;

			private:
				void			ParseValue();
			};

		}
	}
}

#endif