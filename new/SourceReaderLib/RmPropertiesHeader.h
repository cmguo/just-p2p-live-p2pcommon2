#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_RM_PROPERTIES_HEADER_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_RM_PROPERTIES_HEADER_H_
#include "RmObject.h"

namespace Synacast
{
	namespace Format
	{

		class SOURCEREADERLIB_API RmPropertiesHeader : public RmObject
		{
		public:
			RmPropertiesHeader(void);
			~RmPropertiesHeader(void);

		private:
			UINT32		m_MaxBitrate;		// max_bit_rate;
			UINT32		m_AvgBitrate;		// avg_bit_rate;
			UINT32		m_MaxPacketSize;	// max_packet_size;
			UINT32		m_AvgPacketSize;	// avg_packet_size;
			UINT32		m_PacketCount;		// num_packets;
			UINT32		m_Duration;			// duration;
			UINT32		m_Preroll;			// preroll;
			UINT32		m_IndexOffset;		// index_offset;
			UINT32		m_DataOffset;		// data_offset;
			UINT16		m_StreamCount;		// num_streams;
			UINT16		m_Flags;			// flags;				// host byte order

		public:
			UINT32			GetMaxBitrate() 	const { return m_MaxBitrate; };	
			UINT32			GetAvgBitrate() 	const { return m_AvgBitrate; };	
			UINT32			GetMaxPacketSize() 	const { return m_MaxPacketSize; };
			UINT32			GetAvgPacketSize() 	const { return m_AvgPacketSize; };
			UINT32			GetPacketCount() 	const { return m_PacketCount; };	
			UINT32			GetDuration() 		const { return m_Duration; };		
			UINT32			GetPreroll() 		const { return m_Preroll; };		
			UINT32			GetIndexOffset() 	const { return m_IndexOffset; };	
			UINT32			GetDataOffset() 	const { return m_DataOffset; };	
			UINT16			GetStreamCount() 	const { return m_StreamCount; };	
			UINT16			GetFlags() 			const { return m_Flags; };
			
			void SetMaxBitrate		(	const UINT32  maxBitrate	) { m_MaxBitrate	= maxBitrate; };		
			void SetAvgBitrate		(	const UINT32  avgBitrate	) { m_AvgBitrate	= avgBitrate; };		
			void SetMaxPacketSize	(	const UINT32  maxPacketSize	) { m_MaxPacketSize	= maxPacketSize; };	
			void SetAvgPacketSize	(	const UINT32  avgPacketSize	) { m_AvgPacketSize	= avgPacketSize; };	
			void SetPacketCount		(	const UINT32  packetCount	) { m_PacketCount	= packetCount; };	
			void SetDuration		(	const UINT32  duration		) { m_Duration		= duration; };		
			void SetPreroll			(	const UINT32  preroll		) { m_Preroll		= preroll; };		
			void SetIndexOffset		(	const UINT32  indexOffset	) { m_IndexOffset	= indexOffset; };	
			void SetDataOffset		(	const UINT32  dataOffset	) { m_DataOffset	= dataOffset; };		
			void SetStreamCount		(	const UINT16  streamCount	) { m_StreamCount	= streamCount; };	
			void SetFlags			(	const UINT16  flags			) { m_Flags			= flags; };

		private:
			void		ContentToData();
			UINT32		GetContentSize();

			/************************************************************************/
			/* 
			Exceptions:
				FormatException
			*/
			/************************************************************************/
			void		ParseContent();
		};
	}
}

#endif