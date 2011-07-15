#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_MMS_CLIENT_COMMAND_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_MMS_CLIENT_COMMAND_H_

#include "MmsCommand.h"

namespace Synacast
{
	namespace Protocol
	{
		namespace Mms
		{
			class MmsClientCommand :
				public MmsCommand
			{
			public:
				MmsClientCommand();
				virtual ~MmsClientCommand(void);

			public:
				/************************************************************************/
				/* 
				Exceptions:
					FormatException
				*/
				/************************************************************************/
				virtual void SetHeader( const DataBuffer & header );
				/************************************************************************/
				/* 
				Exceptions:
					FormatException
				*/
				/************************************************************************/
				virtual void SetHeader( const char * header, size_t size );
				virtual void SetData( const DataBuffer & data );
				virtual void SetData( const char * data, size_t size );

				/************************************************************************/
				/* 
				Exceptions:
					FormatException
				*/
				/************************************************************************/
				virtual void Parse( const DataBuffer & data );

				virtual UINT32 GetLength() const { return m_Length; };
				virtual UINT32 GetDataLength() const { return m_Length - 24; };
			protected:
				/************************************************************************/
				/* 
				Exceptions:
					FormatException
				*/
				/************************************************************************/
				virtual void ParseHeader();
				virtual void ParseData();
				virtual void ParsePrefix();
			};

		}
	}
}
#endif