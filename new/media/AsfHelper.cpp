
#include "StdAfx.h"

#include "AsfHelper.h"
#include "AsfBase.h"
#include <assert.h>

// Added for test.
#include <ppl/diag/trace.h>
bool bNeedTrace = false;

const UINT32 replicatedDataOffset = 7;
const UINT32 representTimeOffset = 4;

DWORD AsfHelper::GetTimestamp(const UINT8 * packetBuffer, const size_t maxPacketSize)
{
	size_t timeStampOffset = LocateTimeStampOffset( packetBuffer, maxPacketSize );
	DWORD* sendTime = (DWORD*)(packetBuffer + timeStampOffset);
	return *sendTime;
}

bool AsfHelper::IsKeyFramePacket(const UINT8* packetBuffer, size_t packetSize)
{
	bool single = ( ( packetBuffer[3] & 0x01 ) != 0x01 );
	int payloadOffset = GetPayloadOffset(packetBuffer, packetSize);
	size_t payloadCount = 1;
	if (! single)
	{
		LIVE_ASSERT( ( packetBuffer[payloadOffset] & 0x80 ) == 0x80 );
		payloadOffset += 1;        // multi start with payloads info, 1B
		payloadCount = packetBuffer[payloadOffset - 1] & 0x3F;
	}
	size_t parsedPayload = 0;
	while( parsedPayload < payloadCount )
	{
		bool keyFrame = ((packetBuffer[payloadOffset] & 0x80) == 0x80);
		if (packetBuffer[payloadOffset + 2] == 0x00 &&packetBuffer[payloadOffset + 3] == 0x00 &&
			packetBuffer[payloadOffset + 4] == 0x00 &&packetBuffer[payloadOffset + 5] == 0x00 
			&& keyFrame)
		{
			return true;
		}

		size_t replicatedDataLength = packetBuffer[payloadOffset + 6];
// 		size_t replicatedDataOffset = 7;
// 		size_t representTimeOffset = 4;

		if ( single )
		{
			break;
		}
		parsedPayload += 1;
		size_t payloadLengthOffset = replicatedDataOffset + replicatedDataLength;
		UINT16 payloadLength = *(UINT16*)( packetBuffer + payloadOffset + payloadLengthOffset );
		payloadOffset += (payloadLength + payloadLengthOffset + 2);
	}
	return false;
}

int AsfHelper::GetPayloadOffset( const UINT8* packetBuffer, size_t packetSize )
{

	LengthType packetLengthType = (LengthType)( ( packetBuffer[3] & 0x60 ) >> 5 );
	LengthType paddingLengthType = (LengthType)( ( packetBuffer[3] & 0x18 ) >> 3 );
	LengthType sequenceType = (LengthType)( ( packetBuffer[3] & 0x06 ) >> 1 );

	// data unit
	UINT32 packetInfoOffset = 5;	// 0x820000, 09/08/11, 5D
	return packetInfoOffset + 
		TypeToLength( packetLengthType ) + 
		TypeToLength( sequenceType ) + 
		TypeToLength( paddingLengthType ) + 4 + 2;
}


int AsfHelper::LocateTimeStampOffset( const UINT8* packetBuffer, size_t packetSize )
{
	if ( packetSize < 5 )
		return -1;
	LIVE_ASSERT( ( packetBuffer[3] & 0x80 ) == 0x00 );	// no error correction
	LIVE_ASSERT( packetBuffer[4] == 0x5D );					// payload length type OK

	if(( packetBuffer[3] & 0x80 ) != 0x00 || packetBuffer[4] != 0x5D)
	{
		//OutputDebugString("\r DataError!");
		return -1;
	}
	LengthType packetLengthType = (LengthType)( ( packetBuffer[3] & 0x60 ) >> 5 );
	LengthType paddingLengthType = (LengthType)( ( packetBuffer[3] & 0x18 ) >> 3 );
	LengthType sequenceType = (LengthType)( ( packetBuffer[3] & 0x06 ) >> 1 );

	// data unit
	UINT32 packetInfoOffset = 5;	// 0x820000, 09/08/11, 5D
	return packetInfoOffset + 
		TypeToLength( packetLengthType ) + 
		TypeToLength( sequenceType ) + 
		TypeToLength( paddingLengthType );
}

bool AsfHelper::ChanegeTimeStamps(UINT8* packetBuffer, const size_t maxPacketSize, const UINT32 startTimeStamp, UINT preproll)
{
	bool single = ( ( packetBuffer[3] & 0x01 ) != 0x01 );
	INT32 timeStampOffset = LocateTimeStampOffset( packetBuffer, maxPacketSize );

	if(timeStampOffset == -1)
	{
		return false;
	}

	UINT32* sendTime = (UINT32*)(packetBuffer + timeStampOffset);
	LIVE_ASSERT(*sendTime >= startTimeStamp);
	if (*sendTime < startTimeStamp)
	{
		*sendTime = 0;
		//return false;
	}
	else
		*sendTime = *sendTime - startTimeStamp;

	UINT32 payloadOffset = timeStampOffset + 4 + 2;		// send time, duration
	UINT32 payloadCount = 1;
	if ( ! single )
	{
		LIVE_ASSERT( ( packetBuffer[payloadOffset] & 0x80 ) == 0x80 );
		payloadOffset += 1;								// multi start with payloads info, 1B
		payloadCount = packetBuffer[payloadOffset - 1] & 0x3F;
	}

	if ( payloadOffset > 13 )
	{
		//cout << "payload offset: " << payloadOffset << endl;
	}

	UINT32 parsedPayload = 0;
	while( parsedPayload < payloadCount )
	{
		if ( payloadOffset + 15 >= maxPacketSize )	// 检查下面要访问的内存合法
		{
			break;
		}

		UINT32 replicatedDataLength = packetBuffer[payloadOffset + 6];
// 		UINT32 replicatedDataOffset = 7;
// 		UINT32 representTimeOffset = 4;
		UINT32* presentTime = (UINT32*)( packetBuffer + payloadOffset + replicatedDataOffset + representTimeOffset );
		if( *presentTime  >= startTimeStamp )
		{
			*presentTime = *presentTime - startTimeStamp + preproll;
		}	
		else
		{
			*presentTime = preproll;
		}

		if ( single )
		{
			break;
		}

		parsedPayload += 1;

		UINT32 payloadLengthOffset = replicatedDataOffset + replicatedDataLength;
		UINT16 payloadLength = *(UINT16*)( packetBuffer + payloadOffset + payloadLengthOffset );
		payloadOffset += (payloadLength + payloadLengthOffset + 2);
	}
	return true;
}


bool AsfHelper::ChanegeTimeStampsForVista(UINT8* packetBuffer, const size_t maxPacketSize, const DWORD startTimeStamp)
{
	bool single = ( ( packetBuffer[3] & 0x01 ) != 0x01 );
	size_t timeStampOffset = LocateTimeStampOffset( packetBuffer, maxPacketSize );

	DWORD* sendTime = (DWORD*)(packetBuffer + timeStampOffset);
	LIVE_ASSERT(*sendTime >= startTimeStamp);
	*sendTime = *sendTime - startTimeStamp;

	size_t payloadOffset = timeStampOffset + 4 + 2;		// send time, duration
	size_t payloadCount = 1;
	if ( ! single )
	{
		LIVE_ASSERT( ( packetBuffer[payloadOffset] & 0x80 ) == 0x80 );
		payloadOffset += 1;								// multi start with payloads info, 1B
		payloadCount = packetBuffer[payloadOffset - 1] & 0x3F;
	}

	if ( payloadOffset > 13 )
	{
		//cout << "payload offset: " << payloadOffset << endl;
	}

	size_t parsedPayload = 0;
	while( parsedPayload < payloadCount )
	{
		if ( payloadOffset + 15 >= maxPacketSize )	// 检查下面要访问的内存合法
		{
			break;
		}

		/// is key frame
		// packetBuffer[payloadOffset] & (BYTE)0x80 == (BYTE)0x80;

		/// stream number
		// BYTE streamID = packetBuffer[payloadOffset] & 0x7F;

		/// object ID
		// BYTE objectID = packetBuffer[payloadOffset+1];

		/// offset
		// UINT32 offset = *( (UINT32*)(packetBuffer + payloadOffset + 2) );
		size_t replicatedDataLength = packetBuffer[payloadOffset + 6];
// 		size_t replicatedDataOffset = 7;
// 		size_t representTimeOffset = 4;
		/// object size
		// DWORD objectSize = *( (UINT32*) (packetBuffer + payloadOffset + replicatedDataOffset) );

		DWORD* presentTime = (DWORD*)( packetBuffer + payloadOffset + replicatedDataOffset + representTimeOffset );
		if( *presentTime  > startTimeStamp )
		{
			*presentTime = *presentTime - startTimeStamp;
		}	
		else
		{
			LIVE_ASSERT( false );
			return false;
		}

		if ( single )
		{
			break;
		}

		parsedPayload += 1;

		size_t payloadLengthOffset = replicatedDataOffset + replicatedDataLength;
		if ( payloadOffset + payloadLengthOffset + 2 >= maxPacketSize )	// 检查下面访问payloadLength是否合法
		{
			break;
		}
		UINT16 payloadLength = *(UINT16*)( packetBuffer + payloadOffset + payloadLengthOffset );
		payloadOffset += (payloadLength + payloadLengthOffset + 2);
	}
	return true;
}

bool AsfHelper::ChanegeTimeStamps(UINT8* packetBuffer, const size_t maxPacketSize, UINT32& startTimeStamp, UINT32& startTimeStampPL, UINT32 preroll, bool bIsStartRound, UINT32& lastSendTS, UINT32& lastPresentTS)
{
	bool single = ( ( packetBuffer[3] & 0x01 ) != 0x01 );
	INT32 timeStampOffset = LocateTimeStampOffset( packetBuffer, maxPacketSize );

	if(timeStampOffset == -1)
	{
		return false;
	}

	UINT32* sendTime = (UINT32*)(packetBuffer + timeStampOffset);
	LIVE_ASSERT(*sendTime >= startTimeStamp);
	if (*sendTime < startTimeStamp)
		*sendTime = (UINT32)((UINT64)0x100000000ULL + *sendTime - startTimeStamp);
	else
		*sendTime = *sendTime - startTimeStamp;
	
	//////////////////////////////////////////////////////////////////////////
//	static UINT32 uLastSendTime(0);
	UINT32 sendTimeDelta;
	sendTimeDelta = *sendTime - lastSendTS;
	
	if (sendTimeDelta > 50000/* || sendTimeDelta < -5000*/)
	{
		*sendTime = lastSendTS + 30;
		startTimeStamp = (UINT32)(sendTimeDelta + 30 + startTimeStamp);
		TRACEOUT_T("Tady =====> +++++ newStartTS [%d]", startTimeStamp);
	}
	lastSendTS = *sendTime;

	TRACEOUT_T("Tady =====> sendTS [%d] --- SendTimeDelta [%d]", *sendTime, sendTimeDelta);
	//////////////////////////////////////////////////////////////////////////
#if(1)
	UINT32 payloadOffset = timeStampOffset + 4 + 2; // send time, duration
	UINT32 payloadCount = 1;
	if ( ! single )
	{
		LIVE_ASSERT( ( packetBuffer[payloadOffset] & 0x80 ) == 0x80 );
		payloadOffset += 1; // multi start with payloads info, 1B
		payloadCount = packetBuffer[payloadOffset - 1] & 0x3F;
	}

	if ( payloadOffset > 13 )
	{
		//cout << "payload offset: " << payloadOffset << endl;
	}

	UINT32 parsedPayload = 0;
	while( parsedPayload < payloadCount )
	{
		if ( payloadOffset + 15 >= maxPacketSize )	// 检查下面要访问的内存合法
		{ 
			break;
		}

		UINT32 replicatedDataLength = packetBuffer[payloadOffset + 6];
// 		UINT32 replicatedDataOffset = 7;
// 		UINT32 representTimeOffset = 4;
		UINT32* presentTime = (UINT32*)( packetBuffer + payloadOffset + replicatedDataOffset + representTimeOffset );
		//UINT32 preT = *presentTime;
// 		if( *presentTime >= startTimeStampPL )
// 		{
// 			*presentTime = *presentTime - startTimeStampPL + preroll + 1;
// 		}
// 		else
// 		{
// 			if (bIsStartRound)
// 			{
// 				*presentTime = preroll - 1;
// 			}
// 			else
// 			{
// 				*presentTime = (UINT32)((UINT64)0x100000000 + *presentTime - startTimeStampPL + preroll + 1);
// 				UINT32 preT2 = *presentTime;
// 			}
// 		}
		if (bIsStartRound && *presentTime < startTimeStampPL)
		{
			*presentTime = preroll + 1;
		}
		else
		{
			*presentTime = *presentTime - startTimeStampPL + preroll + 31;
		}
		//////////////////////////////////////////////////////////////////////////
		/*
		static UINT32 uLastPT(preroll);
		INT64 PTDelta;
		if (((uLastPT & 0x80000000) == 0x80000000) 
			&& ((*presentTime & 0x80000000) == 0))
		{
			PTDelta = ((INT64)0x100000000 + *presentTime - uLastPT);
		}
		else
		{
			PTDelta = *presentTime - uLastPT;
		}

		if (PTDelta > 5000 || PTDelta < -5000)
		{
			*presentTime = uLastPT + 30;
			startTimeStampPL = (PTDelta + 30 + startTimeStampPL);
		}
		uLastPT = *presentTime;
		*/
		INT32 PTDelta; 
		PTDelta = *presentTime - lastPresentTS;
		if (PTDelta > 50000 || PTDelta < -50000)
		{
			*presentTime = lastPresentTS + 40;
			startTimeStampPL = (PTDelta + 40 + startTimeStampPL);
//			TRACEOUT_T("Tady -> +++ new StartPTS [%d]", startTimeStampPL);
		}
		lastPresentTS = *presentTime;
//		TRACEOUT_T("Tady -> PT [%d] --- PTDelta [%d]", *presentTime, PTDelta);
		//////////////////////////////////////////////////////////////////////////

		if ( single )
		{
			break;
		}

		parsedPayload += 1;

		UINT32 payloadLengthOffset = replicatedDataOffset + replicatedDataLength;
		UINT16 payloadLength = *(UINT16*)( packetBuffer + payloadOffset + payloadLengthOffset );
		payloadOffset += (payloadLength + payloadLengthOffset + 2);
	}
#endif
	return true;
} 

UINT32 AsfHelper::GetPresentTime(const UINT8* packetBuffer, size_t packetSize)
{
	const UINT8* buffer = packetBuffer;

	INT32 timeStampOffset = LocateTimeStampOffset( buffer, packetSize );

	bool single = ( ( buffer[3] & 0x01 ) != 0x01 );

	UINT32 payloadOffset = timeStampOffset + 4 + 2; // send time, duration
	UINT32 payloadCount = 1;
	if ( ! single )
	{
		LIVE_ASSERT( ( buffer[payloadOffset] & 0x80 ) == 0x80 );
		payloadOffset += 1; // multi start with payloads info, 1B
		payloadCount = buffer[payloadOffset - 1] & 0x3F;
	}

	if ( payloadOffset > 13 )
	{
		//cout << "payload offset: " << payloadOffset << endl;
	}
	UINT32 uPresentTime = 0;
	UINT32 parsedPayload = 0;
	while( parsedPayload < payloadCount )
	{
		UINT32 replicatedDataLength = buffer[payloadOffset + 6];
// 		UINT32 replicatedDataOffset = 7;
// 		UINT32 representTimeOffset = 4;
		UINT32* presentTime = (UINT32*)( buffer + payloadOffset + replicatedDataOffset + representTimeOffset );

		bool keyFrame = ( (buffer[payloadOffset] & 0x80 ) == 0x80);
		//INT8 streamID = buffer[payloadOffset] & 0x7f;
		if (buffer[payloadOffset + 2] == 0x00 &&buffer[payloadOffset + 3] == 0x00 &&
			buffer[payloadOffset + 4] == 0x00 &&buffer[payloadOffset + 5] == 0x00 
			&& keyFrame )//&& streamID == 2)
			// if (keyFrame)
		{
			uPresentTime = *presentTime;
			break;
		}

		if (*presentTime < uPresentTime || uPresentTime == 0)
		{
			uPresentTime = *presentTime;
		}
		if ( single )
		{
			break;
		}

		parsedPayload += 1;

		UINT32 payloadLengthOffset = replicatedDataOffset + replicatedDataLength;
		UINT16 payloadLength = *(UINT16*)( buffer + payloadOffset + payloadLengthOffset );
		payloadOffset += (payloadLength + payloadLengthOffset + 2);
	}
	return uPresentTime;
}

size_t AsfHelper::GetDataUnitSize(const UINT8* packetBuffer)
{
	if(( packetBuffer[3] & 0x80 ) != 0x00 || packetBuffer[4] != 0x5D)
	{
		//OutputDebugString("\r DataError!");
		return 0;
	}

	size_t typeLen = TypeToLength((LengthType)( ( packetBuffer[3] & 0x60 ) >> 5 ));
	if (typeLen == 0)
		return 0;

	DWORD dwDataUnitSize = 0;
	memcpy((void *)dwDataUnitSize, packetBuffer + 4, typeLen);
	return dwDataUnitSize;
}

size_t AsfHelper::GetDataUnitSizeFromHeader(const UINT8* headerPacketBuffer, const size_t buffersize)
{
	//分析ASF文件剩下的字节,关键是把 ASF_File_Properties_Object 的重要信息解析出来
	DWORD FilePropObjGuidIndex = 0;
	for( ; FilePropObjGuidIndex < buffersize - 24 - 104; FilePropObjGuidIndex++)
	{
		if( GUID_ASF_FILE_PROPERTIES_OBJECT == *((GUID*)(headerPacketBuffer + FilePropObjGuidIndex)) ) 
			break;
	}

	if( FilePropObjGuidIndex >= buffersize - 24 - 104 )
	{
		return 0;
	}	

	// ASF文件的切片数目
//	DWORD m_PacketNumber = *(DWORD*)(buffer+FilePropObjGuidIndex+56);
	// ASF文件的每个切片的长度
	//DWORD packetLength = *(DWORD*)(headerPacketBuffer + FilePropObjGuidIndex + 92);
	//DWORD bakPacketLength = *(DWORD*)(headerPacketBuffer + FilePropObjGuidIndex + 96);
        DWORD packetLength[2];
        memcpy(packetLength, headerPacketBuffer + FilePropObjGuidIndex + 92, sizeof(packetLength));

	if( packetLength[1] != packetLength[0] )
	{
		return 0;
	}

	return packetLength[1];
}
