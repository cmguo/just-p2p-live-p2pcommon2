#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_REAL_RTSP_CHALLENGE_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_REAL_RTSP_CHALLENGE_H_

namespace Synacast
{
	namespace Protocol
	{
		namespace Rtsp
		{

#define uint32_t unsigned long

#define le2me_16(x) (x)
#define le2me_32(x) (x)

#define be2me_16(x) (((x) & 0x00ff) << 8 | ((x) & 0xff00) >> 8)
#define be2me_32(x) ((((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >>  8) | (((x) & 0x0000ff00) <<  8) | (((x) & 0x000000ff) << 24))

#define BE_32C(x,y) (*((uint32_t*)(x))=be2me_32(y))
#define BE_16(x)  be2me_16(*(UINT16*)(x))
#define BE_32(x)  be2me_32(*(uint32_t*)(x))

#ifndef MAX
#	define MAX(x,y) ((x>y) ? x : y)
#endif

			class RmRtspChallenge  
			{
			public:
				static void calc_response_and_checksum (char *response, char *chksum, char *challenge);
				static void calc_response_string (char *result, char *challenge);
				static void calc_response (char *result, char *field);
				static void call_hash (char *key, char *challenge, size_t len);
				static void hash(char *field, char *param);
				RmRtspChallenge();
				~RmRtspChallenge();
			};

		}
	}
}

#endif