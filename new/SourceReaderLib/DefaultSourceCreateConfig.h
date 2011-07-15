#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_DEFAULT_SOURCE_CREATE_CONFIG_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_DEFAULT_SOURCE_CREATE_CONFIG_H_

#include "SourceCreateConfig.h"

namespace Synacast
{
	namespace StreamSource
	{

		class DefaultSourceCreateConfig :
			public SourceCreateConfig
		{
		public:
			DefaultSourceCreateConfig(void);
			~DefaultSourceCreateConfig(void);

		private:
			vector<SourceType>		m_FileList;
			vector<SourceType>		m_HttpList;
			vector<SourceType>		m_MmsList;
			vector<SourceType>		m_MmshList;
			vector<SourceType>		m_MmstList;
			vector<SourceType>		m_RtspList;

			vector<SourceType>		m_RmRtspList;
			vector<SourceType>		m_MsRtspList;
			
			vector<SourceType>		m_Mp3List;

			//vector<SourceType>		m_RmFileList;
			//vector<SourceType>		m_MsFileList;
			//vector<SourceType>		m_MsHttpList;
			//vector<SourceType>		m_RmHttpList;

			vector<SourceType>		m_EmptyList;

		public:
			const vector<SourceType>  & GetTryList( const string & url ) const;
		};
	}
}

#endif