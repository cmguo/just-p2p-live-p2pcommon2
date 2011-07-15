#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_HTTP_READER_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_HTTP_READER_H_
#include <WinSock2.h>
#include "DataBuffer.h"
using namespace Synacast::Common;

namespace Synacast
{
	namespace Protocol
	{
		namespace Http
		{
			class HttpReader
			{
			public:
				HttpReader(void);
				virtual ~HttpReader(void);

			protected:
				DataBuffer m_Header;

			public:
				/************************************************************************/
				/* 
				¶Á³ö´í£¬Å×³öIOException
				*/
				/************************************************************************/
				virtual void ReadHeader(SOCKET server);

				const DataBuffer & GetHeader() const { return m_Header; };
			};

		}
	}
}
#endif