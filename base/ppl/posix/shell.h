
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_POSIX_SHELL_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_POSIX_SHELL_H_

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>


class ShellFolders
{
public:
	static tstring CurrentUserHome()
	{
		struct passwd* userInfo = getpwuid(getuid());
		if ( NULL == userInfo )
			return tstring();
		return tstring( userInfo->pw_dir );
	}

};

#endif
