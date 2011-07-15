#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_DATA_PACKET_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_DATA_PACKET_H_

#include "DataBuffer.h"
using namespace Synacast::Common;

namespace Synacast
{
	namespace Protocol
	{
		class DataPacket
		{
		public:
			DataPacket(void);
			virtual ~DataPacket(void);

		protected:
			DataBuffer	m_Data;
			size_t		m_Sequence;

		public:
			void				SetData( const char * data, size_t size );
			void				SetData( const DataBuffer & data );
			const DataBuffer &	GetData()		const { return m_Data; };

			size_t				GetSequence()	const { return m_Sequence; };
			virtual void SetSequence( size_t sequence ) { m_Sequence = sequence; };
		};

	}
}
#endif