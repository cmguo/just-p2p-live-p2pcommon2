#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_ASFFILE_PROPERTIES_OBJECT_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_ASFFILE_PROPERTIES_OBJECT_H_

#include "AsfObject.h"

namespace Synacast
{
	namespace Format
	{
		class AsfFilePropertiesObject :
			public AsfObject
		{
		public:
			AsfFilePropertiesObject(void);
			~AsfFilePropertiesObject(void);

		private:
			GUID		m_FileID;
			UINT64		m_FileSize;
			UINT64		m_CreateTime;
			UINT64		m_PacketCount;
			UINT64		m_PlayDuration;
			UINT64		m_SendDuration;
			UINT64		m_Preroll;
			UINT32		m_Flags;
			UINT32		m_MaxPacketSize;
			UINT32		m_MinPacketSize;
			UINT32		m_MaxBitrate;

		public:
			/************************************************************************/
			/* 
			“Ï≥££∫
				FormatException
			*/
			/************************************************************************/
			void		Parse( const char * data, const size_t size );

		public:
			GUID		GetFileID()			const { return m_FileID; };
			UINT64		GetFileSize()		const { return m_FileSize; };
			UINT64		GetCreateTime()		const { return m_CreateTime; };
			UINT64		GetPacketCount()	const { return m_PacketCount; };
			UINT64		GetPlayDuration()	const { return m_PlayDuration; };
			UINT64		GetSendDuration()	const { return m_SendDuration; };
			UINT64		GetPreroll()		const { return m_Preroll; };
			UINT32		GetFlags()			const { return m_Flags; }; 
			UINT32		GetMaxPacketSize()	const { return m_MaxPacketSize; };
			UINT32		GetMinPacketSize()	const { return m_MinPacketSize; };
			UINT32		GetMaxBitrate()		const { return m_MaxBitrate; };

			bool		IsBroadCast()		const { return ( m_Flags & 0x01 ) == 0x01; };
			bool		IsSeekable()		const { return ( m_Flags & 0x02 ) == 0x02; };
		};
	}
}

#endif