#pragma warning( disable: 4786 )

#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_RING_QUEUE_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_RING_QUEUE_H_

#include "IllegalArgumentException.h"
using namespace Synacast::Exception;

#include <queue>
using namespace std;

namespace Synacast
{
	namespace Common
	{
		/************************************************************************/
		/* 
		ѭ�����У���������ʱ����������ӵ����ݡ�
		*/
		/************************************************************************/
		template<class T>
		class RingQueue
		{
		public:
			RingQueue(void) : m_Queue(), m_MaxSize( 0xFFFF ){};
			RingQueue( size_t maxSize ) : m_Queue(), m_MaxSize( maxSize ){};
			virtual ~RingQueue(void){};

		private:
			queue<T>	m_Queue;
			size_t		m_MaxSize;

		public:
			/************************************************************************/
			/* 
			������ʱ��������ӣ����Բ���
			*/
			/************************************************************************/
			virtual void Push( const T & data )
			{
				if ( m_Queue.size() < m_MaxSize )
				{
					m_Queue.push( data );
				}
			};
			/************************************************************************/
			/* 
			�쳣��
				IllegalArgumentException, û��Ԫ��
			*/
			/************************************************************************/
			virtual T Pop()
			{
				if( m_Queue.empty() )
				{
					throw IllegalArgumentException();
				}

				T data = m_Queue.front();
				m_Queue.pop();
				return data;
			};

			virtual size_t GetSize() const
			{
				return m_Queue.size();
			}

			virtual size_t GetCapacity() { return m_MaxSize; };
		};

	}
}

#endif