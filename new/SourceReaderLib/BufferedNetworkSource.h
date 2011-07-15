#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_BUFFERED_NETWORK_SOURCE_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_BUFFERED_NETWORK_SOURCE_H_

#include "NetworkSource.h"
#include "RingQueue.h"
#include "CriticalSection.h"

#include "DataPacket.h"

using namespace Synacast::Protocol;

namespace Synacast
{
	namespace StreamSource
	{

		class SOURCEREADERLIB_API BufferedNetworkSource :
			public NetworkSource
		{
		public:
			BufferedNetworkSource( SourceType type, const wstring & name );
			BufferedNetworkSource( const string & url, SourceType type, const wstring & name );
			virtual ~BufferedNetworkSource(void);

			const static size_t				INVALID_SPEED;

		private:
			size_t							m_BufferMinSequence;
			size_t							m_BufferMaxSequence;
			RingQueue<const DataPacket*> *	m_Buffer;
			bool							m_NoMoreData;				// stream end
			HANDLE							m_DataSemaphore;
			CriticalSection					m_Lock;

			const DataPacket *				m_CurrentPacket;
			const DataBuffer *				m_CurrentBuffer;
			size_t							m_BufferPointer;

		public:
			virtual void Open();
			/************************************************************************/
			/* 
			Exceptions:
				IOException
				SourceException
			*/
			/************************************************************************/
			virtual size_t	Read( char * buffer, size_t size );
			virtual void Close();


		protected:
			/************************************************************************/
			/* 
			Buffer operations
			*/
			/************************************************************************/
			void	PushInBuffer( const DataPacket * packet );
			/************************************************************************/
			/* 
			如果缓冲区中没有数据包，返回空，否则返回第一个数据包的指针。
			*/
			/************************************************************************/
			const DataPacket * PopFromBuffer();

			const DataPacket * PopFromBufferInTime( DWORD milliseconds );

			virtual void EndStream();

		private:
			void ClearBuffer();
		};

	}
}
#endif