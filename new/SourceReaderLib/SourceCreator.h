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
			����url���Դ���Source��
			����ֵ��
			  NULL�����û�к��ʵ�Source֧�ָ�URL�����򴴽��ʺϵ�Source��
			*/
			/************************************************************************/
			Source * CreateSource( const string & url );
			/************************************************************************/
			/* 
			����type������Ӧ��Source��
			����ֵ��
			  NULL�����type��֧�֡�
		    ע�⣺
			  ��ȻĳЩЭ���UDP�����Ѿ����ˣ�����UDPû�������ݰ����������飬�����ʱ��֧��UDP���䡣
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