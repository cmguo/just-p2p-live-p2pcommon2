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
			HTTP���ݿ飬HTTP��chuncked��������ģ�͡�
			[length]\r\n
			[chunck]\r\n
			����length�����ݿ�Ĵ�С��16����ASCII�ַ���
			���������ݴ�����֮�����һ��chunk��length��0���ܹ�5���ֽڡ�
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