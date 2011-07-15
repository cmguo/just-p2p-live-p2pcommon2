
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_MSWIN_SHELL_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_MAWIN_SHELL_H_

#include <ppl/data/tstring.h>

#include <shlobj.h>
#pragma comment(lib, "shell32.lib")


class ShellFolders
{
public:
	static tstring CommonAppDataDirectory()
	{
		TCHAR szPath[MAX_PATH * 4 + 1] = { 0 };
		if(SUCCEEDED(SHGetFolderPath(NULL, 
			CSIDL_COMMON_APPDATA | CSIDL_FLAG_CREATE, 
			NULL, 
			0, 
			szPath))) 
		{
			return tstring( szPath );
		}
		return tstring();
	}

};

#endif
