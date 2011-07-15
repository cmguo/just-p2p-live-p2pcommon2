#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_HTTP_CHUNK_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_HTTP_CHUNK_H_

#include "DataBuffer.h"
using namespace Synacast::Common;

namespace Synacast
{
	namespace Protocol
	{
		namespace Http
		{

			/************************************************************************/
			/* 
			HTTP数据块，HTTP中chuncked传输编码的模型。
			[length]\r\n
			[chunck]\r\n
			其中length是数据块的大小，16进制ASCII字符串
			在所有数据传输完之后，最后一个chunk的length是0，总共5个字节。
			*/
			/************************************************************************/
			class HttpChunk
			{
			public:
				HttpChunk(void);
				virtual ~HttpChunk(void);

			private:
				size_t m_Size;
				DataBuffer m_Data;

				virtual void SetRawData( const DataBuffer & raw );
				
			public:
				void SetSize( size_t size );
				size_t GetSize() const;
				void SetData( const DataBuffer & data );
				void SetData( const char * data, size_t size );
				const DataBuffer & GetData() const;
			};

		}
	}
}
#endif