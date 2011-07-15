#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_SOURCE_CREATOR_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_SOURCE_CREATOR_H_
#include "Source.h"

#include "SourceCreateConfig.h"

namespace Synacast
{
	namespace StreamSource
	{

		class SOURCEREADERLIB_API SourceCreator
		{
		public:
			SourceCreator(void);
			~SourceCreator(void);

		private:
			const SourceCreateConfig * 	m_Config;

		public:
			/************************************************************************/
			/* 
			根据url尝试创建Source。
			返回值：
			  NULL，如果没有合适的Source支持该URL，否则创建适合的Source。
			*/
			/************************************************************************/
			Source * CreateSource( const string & url );
			/************************************************************************/
			/* 
			根据type创建相应的Source。
			返回值：
			  NULL，如果type不支持。
		    注意：
			  虽然某些协议的UDP传输已经做了，但是UDP没有做数据包的乱序重组，因此暂时不支持UDP传输。
			*/
			/************************************************************************/
			Source * CreateSource( const string & url, SourceType type );

			Source * CreateSource(const char * url);
			Source * CreateSource(const char * url, SourceType type);

		public:
			// Thread unsafe
			const SourceCreateConfig & GetConfig();
			void SetSourceCreateConfig( const SourceCreateConfig * config );
		};

	}
}
#endif