#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_BASE64_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_BASE64_H_

#define	BAD		-1

static const char base64digits[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char base64val[] =	{
	BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD,
	BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD,
	BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD, 62, BAD,BAD,BAD,	63,
	52, 53, 54, 55,  56, 57, 58, 59,  60, 61,BAD,BAD, BAD,BAD,BAD,BAD,
	BAD,  0,  1,  2,   3,  4,  5,  6,	7,	8,	9, 10,	11,	12,	13,	14,
	15, 16, 17, 18,  19, 20, 21, 22,  23, 24, 25,BAD, BAD,BAD,BAD,BAD,
	BAD, 26, 27, 28,  29, 30, 31, 32,  33, 34, 35, 36,	37,	38,	39,	40,
	41, 42, 43, 44,  45, 46, 47, 48,  49, 50, 51,BAD, BAD,BAD,BAD,BAD
};

#define	DECODE64(c)	 (isascii(c) ? base64val[c]	: BAD)

namespace Synacast
{
	namespace Common
	{

		class Base64  
		{
		public:
			/************************************************************************/
			/* 
			返回值：
			解码后的字符长度
			0xFFFFFFFF - 输入字符串不是合法的BASE64编码串
			*/
			/************************************************************************/
			static size_t Decode( char * out, const char * in );
			static void Encode( unsigned char * out, const unsigned char * in, size_t  length );
		};
	}
}
#endif