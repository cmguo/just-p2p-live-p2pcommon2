
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_MSWIN_COM_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_MAWIN_COM_H_

#include <assert.h>

/**
 * @file
 * @brief com相关的函数和类
 */


/// 负责自动初始化COM库
class ComInitializer
{
public:
	ComInitializer()
	{
		HRESULT hr = ::CoInitialize(NULL);
		assert(hr == S_OK);
	}
	~ComInitializer()
	{
		::CoUninitialize();
	}
};

#endif