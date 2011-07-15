
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_DATA_GUID_LESS_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_DATA_GUID_LESS_H_

/**
 * @file
 * @brief 包含guid的less<>特化，供以GUID为key的map和set使用
 */


#include <ppl/data/guid.h>



namespace std {

template<>
struct less<GUID> : public binary_function<GUID, GUID, bool>
{
	bool operator()(const GUID& x, const GUID& y) const
	{
		return memcmp(&x, &y, sizeof(GUID)) < 0;
	}
};

}

#endif
