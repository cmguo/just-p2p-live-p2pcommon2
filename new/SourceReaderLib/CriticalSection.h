
#ifndef _LOCK_H
#define _LOCK_H
#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_CRITICAL_SECTION_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_CRITICAL_SECTION_H_


class CriticalSection
{
public:
	CriticalSection() { InitializeCriticalSection(&_critical); }
	~CriticalSection() { DeleteCriticalSection(&_critical); }
	inline void Enter() { EnterCriticalSection(&_critical); }
	inline void Leave() { LeaveCriticalSection(&_critical); }
private:
	CRITICAL_SECTION _critical;
};

#endif
#endif