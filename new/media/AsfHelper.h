
#ifndef _LIVE_P2PCOMMON2_NEW_MEDIA_ASF_HELPER_H_
#define _LIVE_P2PCOMMON2_NEW_MEDIA_ASF_HELPER_H_

const GUID GUID_ASF_FILE_PROPERTIES_OBJECT =
{0x8CABDCA1, 0xA947, 0x11CF, {0x8E,0xE4,0x00,0xC0,0x0C,0x20,0x53,0x65} };

class AsfHelper
{
public:
	static DWORD GetTimestamp(const UINT8 * packetBuffer, const size_t maxPacketSize);

	static bool IsKeyFramePacket(const UINT8* packetBuffer, size_t packetSize);
	static int GetPayloadOffset(const UINT8* packetBuffer, size_t packetSize);
	static int LocateTimeStampOffset( const UINT8* packetBuffer, size_t maxPacketSize );

	static UINT32 GetPresentTime(const UINT8* packetBuffer, size_t packetSize);

	static bool ChanegeTimeStamps(UINT8* packetBuffer, const size_t maxPacketSize, const UINT32 startTimeStamp, UINT preproll);

	static bool ChanegeTimeStampsForVista(UINT8* packetBuffer, const size_t maxPacketSize, const DWORD startTimeStamp);
	static bool ChanegeTimeStamps(UINT8* packetBuffer, const size_t maxPacketSize, UINT32& startTimeStamp, UINT32& startTimeStampPL, UINT32 preroll, bool bIsStartRound, UINT32& lastSendTS, UINT32& lastPresentTS);
	
	static size_t GetDataUnitSize(const UINT8* packetBuffer);
	static size_t GetDataUnitSizeFromHeader(const UINT8* headerPacketBuffer, const size_t buffersize);

	static const UINT32 MAX_TIMESTAMP = 0xffffffff;
};


#endif