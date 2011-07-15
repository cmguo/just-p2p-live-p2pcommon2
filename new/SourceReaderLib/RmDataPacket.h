#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_RM_DATA_PACKET_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_RM_DATA_PACKET_H_
#include "DataBuffer.h"
using namespace Synacast::Common;

#include "FormatTypes.h"

namespace Synacast
{
	namespace Format
	{

		class SOURCEREADERLIB_API RmDataPacket
		{
		public:
			RmDataPacket(void);
			~RmDataPacket(void);

		private:
			UINT16		m_Version;		//	object_version;

			UINT16		m_Length;		//	length;
			UINT16		m_StreamID;		//	stream_number;
			UINT32		m_TimeStamp;	//	timestamp;
			UINT8		m_Reserved;		//	reserved; 
			UINT8		m_Flags;		//	flags; 

			DataBuffer	m_Data;

			DataBuffer	m_RawData;

			RmStreamType	m_StreamType;
		//	UINT8[length]  data; 

		public:
			UINT16			GetVersion()	const		{ return m_Version; };	        

			UINT16			GetLength()		const		{ return m_Length; };	        

			void SetStreamID( const UINT16 streamID ) { m_StreamID = streamID; };
			UINT16			GetStreamID()	const		{ return m_StreamID; };

			void SetTimeStamp( const UINT32 time ) { m_TimeStamp = time; };
			UINT32			GetTimeStamp()	const		{ return m_TimeStamp; };        

			UINT8			GetReserved()	const		{ return m_Reserved; };	        

			void SetFlag( const UINT8 flags ) { m_Flags = flags; };
			UINT8			GetFlags()		const		{ return m_Flags; };	        

			void SetData( const char * data, const UINT16 size );
			const DataBuffer &	GetData()	const		{ return m_Data; };

			void SetRawData( const char * data, const size_t size );
			const DataBuffer & ToRawData();

			void	SetStreamType( const RmStreamType type ) { m_StreamType = type; };
			const RmStreamType GetStreamType() const { return m_StreamType; };

		private:
			void Parse();
		};
	}
}

#endif