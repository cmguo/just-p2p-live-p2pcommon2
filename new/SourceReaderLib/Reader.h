#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_READER_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_READER_H_
#include "ReaderTypes.h"
#include "HeaderChunk.h"
#include "PacketChunk.h"
#include "IndexChunk.h"

namespace Synacast
{
	namespace SourceReader
	{

		class SOURCEREADERLIB_API Reader
		{
		public:
			Reader(ReaderType);
			virtual ~Reader(void);

		private:
			ReaderType		m_Type;

		public:
			ReaderType GetReaderType() const { return m_Type; };

		public:
			/************************************************************************/
			/* 
			读取的Chunk对象由调用者负责销毁。
			异常：
				SourceException，读取出错
			*/
			/************************************************************************/
			virtual HeaderChunk * ReadHeader() = 0;
			/************************************************************************/
			/* 
			返回值：
				NULL，已经读完所有Packet
			异常：
				SourceException，读取出错
			*/
			/************************************************************************/
			virtual PacketChunk * ReadDataPacket() = 0;
			virtual IndexChunk * ReadIndex() = 0;
			virtual void Close() = 0;
		};

	}
}

#endif