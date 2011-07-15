#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_MMS_CLIENT_COMMAND01_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_MMS_CLIENT_COMMAND01_H_
#include "mmsclientcommand.h"

namespace Synacast
{
	namespace Protocol
	{
		namespace Mms
		{
			class MmsClientCommand01 :
				public MmsClientCommand
			{
			public:
				MmsClientCommand01(void);
				~MmsClientCommand01(void);

			public:
				void ParseData();

			private:
				wstring	m_ServerVersion;
				wstring m_ToolVersion;
				wstring m_DownloadUrl;
				wstring m_EncryptionType;

			public:
				const wstring & GetServerVersioin()		const { return m_ServerVersion; };
				const wstring & GetToolVersioin()		const { return m_ToolVersion; };
				const wstring & GetDownloadUrl()		const { return m_DownloadUrl; };
				const wstring & GetEncryptionType()		const { return m_EncryptionType; };
			};

		}
	}
}

#endif