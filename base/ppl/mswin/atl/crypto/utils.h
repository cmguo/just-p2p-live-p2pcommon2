#ifndef _LIVE_P2PCOMMON2_BASE_PPL_MSWIN_ATL_CRYPTO_UTILS_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_MSWIN_ATL_CRYPTO_UTILS_H_



#define ON_ERROR_RETURN(cond, val)	\
	ATLASSERT(!(cond));				\
	if (cond) return val;			

#define ON_ERROR_RETURN0(cond)		\
	ATLASSERT(!(cond));				\
	if (cond) return;			

#define ON_ERROR_CONTINUE(cond)		\
	ATLASSERT(!(cond));				\
	if (cond) continue;

#define ON_ERROR_BREAK(cond)		\
	ATLASSERT(!(cond));				\
	if (cond) break;

#define ON_ERROR_GOTO(cond, lable)	\
	ATLASSERT(!(cond));				\
	if (cond) goto lable;

#endif
