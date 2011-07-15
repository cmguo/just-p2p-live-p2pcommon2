#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_RM_MEDIA_PROPERTIES_HEADER_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_RM_MEDIA_PROPERTIES_HEADER_H_
#include "rmobject.h"

namespace Synacast
{
	namespace Format
	{

		class SOURCEREADERLIB_API RmMediaPropertiesHeader :
			public RmObject
		{
		public:
			RmMediaPropertiesHeader(void);
			~RmMediaPropertiesHeader(void);

		private:
			UINT16			m_StreamID;		//             stream_number;
			UINT32			m_MaxBitrate;	//             max_bit_rate;
			UINT32			m_AvgBitrate;	//             avg_bit_rate;
			UINT32			m_MaxPacketSize;	//             max_packet_size;
			UINT32			m_AvgPacketSize;	//             avg_packet_size;
			UINT32			m_StartTime;		//             start_time;
			UINT32			m_Preroll;		//             preroll;
			UINT32			m_Duration;		//             duration;

			string			m_StreamName;
		//	UINT8			m_StreamNameSize;             stream_name_size;
		//	UINT8[stream_name_size]     stream_name;
			string			m_MimeType;
			//UINT8                       mime_type_size;
			//UINT8[mime_type_size]       mime_type;
			string			m_SpecificData;
			//UINT32                      type_specific_len;
			//UINT8[type_specific_len]    type_specific_data;

		public:
			void SetStreamID( const UINT16 streamID )		{ m_StreamID = streamID; };
			void SetMaxBitrate( const UINT32 maxBitrate )	{ m_MaxBitrate = maxBitrate; };
			void SetAvgBitrate( const UINT32 avgBitrate )	{ m_AvgBitrate = avgBitrate; };
			void SetMaxPacketSize( const UINT32 packetSize ){ m_MaxPacketSize = packetSize; };
			void SetAvgPacketSize( const UINT32 packetSize ){ m_AvgPacketSize = packetSize; };
			void SetStartTime( const UINT32 startTime )		{ m_StartTime = startTime; };
			void SetPreroll( const UINT32 preroll )			{ m_Preroll = preroll; };
			void SetDuration( const UINT32 duration )		{ m_Duration = duration; };
			void SetStreamName( const string & name )		{ m_StreamName = name; };
			void SetMimeType( const string & mime )			{ m_MimeType = mime; };
			void SetSpecificData( const string & data )		{ m_SpecificData = data; }; 

			UINT16			GetStreamID 	()		const { return m_StreamID; };
			UINT32			GetMaxBitrate 	()		const { return m_MaxBitrate; };
			UINT32			GetAvgBitrate 	()		const { return m_AvgBitrate; };
			UINT32			GetMaxPacketSize()		const { return m_MaxPacketSize; };
			UINT32			GetAvgPacketSize()		const { return m_AvgPacketSize; };
			UINT32			GetStartTime 	()		const { return m_StartTime; };
			UINT32			GetPreroll 		()		const { return m_Preroll; };
			UINT32			GetDuration 	()		const { return m_Duration; };

			const string &	GetStreamName 	()		const { return m_StreamName; };

			const string &	GetMimeType 	()		const { return m_MimeType; };

			const string &	GetSpecificData ()		const { return m_SpecificData; };

		private:
			void			ContentToData();
			UINT32			GetContentSize();

			/************************************************************************/
			/* 
			Exceptions:
				FormatException
			*/
			/************************************************************************/
			void			ParseContent();
		};
	}
}

#endif