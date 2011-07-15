#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_LOGSERVER_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_LOGSERVER_H_

#define CUR_PROTOCOL_ID 0x01

typedef enum
{
	EPI_ECHO = 0x03,
	EPI_SEND = 0x11,
	EPI_POST = 0x12, 
} EPINetCMD;

typedef enum
{
	EPI_FIND = 0x20,
	EPI_GET  = 0x21,
	EPI_LISTEN  = 0x22,
} EPINetCMDEx;

typedef struct  
{
	UINT8	CMDCode;
	UINT8	Ver;
	UINT16  SessionID;
	UINT32	AID;
	UINT8   Reserved[3];
	UINT8   DataType;
	UINT32	DataLen;
} EPINetHeader;


#if( !defined(__cdecl) )
#define __cdecl
#endif
typedef int (__cdecl *FUNC_MSendMessageCallback)(void*, const char*, size_t);


const int EPI_ERROR_INVALID_LOGGER_HANDLE = -1;

const int EPI_ERROR_LARGE_PACKET_TO_SEND = -2;

const int EPI_ERROR_UDP_SEND_FAILED = -3;

const int EPI_ERROR_TRACE_FAILED = -4;

const int EPI_ERROR_DLL_NO_FUNC = -31;


enum EPIDataTypeEnum
{
	EPI_DATA_NORMAL_STRING = 1, 
	EPI_DATA_JSON = 2, 
	EPI_DATA_XML = 3, 
};



const size_t EPI_MAX_UDP_PACKET_SIZE = 32 * 1024;

const size_t EPI_MAX_REQUEST_BODY_SIZE = 1 * 1024 * 1024;

struct EPINetRequest
{
	EPINetHeader Header;
	char Body[EPI_MAX_REQUEST_BODY_SIZE];
};



enum EPILogSendingTypeEnum
{
	EPI_DATA_POST = 1, 
	EPI_DATA_SEND = 2, 
};


int PostMSG(UINT32 appID, const char* data, size_t size, UINT8 dataType);
int SendMSG(UINT32 appID, const char* data, size_t size, UINT8 dataType);

//int TraceMSG(EPILogSendingTypeEnum type, UINT32 appID, UINT8 dataType, const char* formatString, ...);
int UploadMSG(UINT32 appID, const char* filename, UINT8 dataType);


typedef int (__cdecl *FUNC_PostMSG)(UINT32, const char*, size_t, UINT8);
typedef int (__cdecl *FUNC_SendMSG)(UINT32, const char*, size_t, UINT8);

//typedef int (__cdecl *FUNC_TraceMSG)(EPILogSendingTypeEnum, UINT32, UINT8, const char*, size_t, ...);
typedef int (__cdecl *FUNC_UploadMSG)(UINT32, const char*, UINT8);

#endif