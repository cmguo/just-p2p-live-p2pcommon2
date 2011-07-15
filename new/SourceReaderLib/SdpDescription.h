#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_SDP_DESCRIPTION_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_SDP_DESCRIPTION_H_

#include <string>

using namespace std;

namespace Synacast
{
	namespace Protocol
	{
		namespace Rtsp
		{

			class SdpDescription  
			{
			public:
				SdpDescription();
				SdpDescription( const char type, const string & value );
				virtual ~SdpDescription();

			public:
				static SdpDescription Parse( string line );

				void SetValue( string value )	{ m_Value = value; };
				const string & GetValue() const	{ return m_Value; };
				void SetType( char type )		{ m_Type  = type; };
				char GetType() const			{ return m_Type; };


			private:
				char	m_Type;
				string	m_Value;
			};

		}
	}
}

#endif