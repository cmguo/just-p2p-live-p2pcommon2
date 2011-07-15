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
			��ȡ��Chunk�����ɵ����߸������١�
			�쳣��
				SourceException����ȡ����
			*/
			/************************************************************************/
			virtual HeaderChunk * ReadHeader() = 0;
			/************************************************************************/
			/* 
			����ֵ��
				NULL���Ѿ���������Packet
			�쳣��
				SourceException����ȡ����
			*/
			/************************************************************************/
			virtual PacketChunk * ReadDataPacket() = 0;
			virtual IndexChunk * ReadIndex() = 0;
			virtual void Close() = 0;
		};

	}
}

#endif