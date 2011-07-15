
#ifndef _LIVE_P2PCOMMON2_NEW_MEDIA_ASF_BASE_H_
#define _LIVE_P2PCOMMON2_NEW_MEDIA_ASF_BASE_H_



enum LengthType
{
	LENGTH_NOT_EXISTS	= 0,
	LENGTH_IN_BYTE		= 1,
	LENGTH_IN_WORD		= 2,
	LENGTH_IN_DWORD		= 3
};


#define  LENGTH_IN

inline size_t TypeToLength( LengthType lengthType )
{
	switch( lengthType )
	{
	case LENGTH_IN_BYTE:
		return 1;

	case LENGTH_IN_WORD:
		return 2;

	case LENGTH_IN_DWORD:
		return 4;

	case LENGTH_NOT_EXISTS:
		return 0;
	}

	return 0;
}
#endif