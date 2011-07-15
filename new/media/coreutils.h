
#ifndef _LIVE_P2PCOMMON2_NEW_MEDIA_COREUTILS_H_
#define _LIVE_P2PCOMMON2_NEW_MEDIA_COREUTILS_H_



//#include "util/coreutils.h"




/************************************************************************/
/*                             �������ֽ���ת��                       */
/************************************************************************/
#define UINT32_PARSE(buf)  ( ( (UINT32)(*(UINT8*)(buf)) << 24 ) | ( (UINT32)(*(UINT8*)(buf+1)) << 16 ) | ( (UINT32)(*(UINT8*)(buf+2)) << 8 ) | ( (UINT32)(*(UINT8*)(buf+3)) ) )
#define UINT16_PARSE(buf) ( ( (UINT16)(*(UINT8*)(buf)) << 8 ) | ( (UINT16)(*(UINT8*)(buf+1)) ) )
#define UINT8_PARSE(buf) ( (UINT8)(*(UINT8*)(buf)) )

#endif

