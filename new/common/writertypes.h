
#ifndef _LIVE_P2PCOMMON2_NEW_COMMON_WRITER_TYPES_H_
#define _LIVE_P2PCOMMON2_NEW_COMMON_WRITER_TYPES_H_


// ������ʽ
enum START_METHOD		
{
	SM_NONE	= 0,		// ��ʼֵ	
	SM_FORWORD = 1,		// ����ǰ��������ʽ
	SM_MIDWORD = 2,		// ��������������ʽ
	SM_1_0_7_4 = 3		// ����1.0.7.4��������ʽ
};


/// ���ŵ�ý������
enum PLAYER_TYPE
{
	NULL_PLAYER = 0,
	MEDIA_PLAYER = 1,
	REAL_PLAYER = 2,
	MKV_PLAYER = 3
};

const char HTTP_HEADER[] = 
"HTTP/1.1 200 OK\r\n"
"Server: Synacast Media Server/1.0\r\n"
"Connection: Close\r\n"
"\r\n";

//
//const char HTTP_HEADER[] = "HTTP/1.1 200 OK\r\n"
//"Content-Type: video/x-ms-wmv\r\n"
//"Accept-Ranges: bytes\r\n"
//"Server: Microsoft-IIS/6.0\r\n"
//"Connection: close\r\n"
//"\r\n";

const char MMSH_HTTP_FIRST_HEADER[] = 
"HTTP/1.0 200 OK\r\n"
"Server: Rex/10.0.0.3700\r\n"
"Cache-Control: no-cache\r\n"
"Pragma: no-cache\r\n"
"Pragma: client-id=959175126\r\n"
"Pragma: features=\"broadcast,playlist\"\r\n"
"Content-Type: application/vnd.ms.wms-hdr.asfv1\r\n"
"Content-Length: %d\r\n"
"Connection: Close\r\n"
"\r\n";

const char MMSH_HTTP_SECOND_HEADER[] = 
"HTTP/1.0 200 OK\r\n"
"Server: Rex/10.0.0.3700\r\n"
"Cache-Control: no-cache\r\n"
"Pragma: no-cache\r\n"
"Pragma: client-id=959175126\r\n"
"Pragma: features=\"broadcast,playlist\"\r\n"
"Content-Type: application/x-mms-framed\r\n"
"\r\n";


#endif