#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_NETWORK_SOURCE_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_NETWORK_SOURCE_H_
#include "source.h"

#include <WinSock2.h>

#include "Url.h"
#include "DataBuffer.h"
using namespace Synacast::Common;

#include "DataPacket.h"
using namespace Synacast::Protocol;

namespace Synacast
{
	namespace StreamSource
	{
#define HTTP_DEFAULT_PORT		80
#define MMS_HTTP_DEFAULT_PORT	8080
#define MMS_DEFAULT_PORT		1755
#define RTSP_DEFAULT_PORT		554

		class SOURCEREADERLIB_API NetworkSource :
			public Source
		{
		protected:
			NetworkSource( SourceType type, const wstring & name );
			NetworkSource( const string & url, SourceType type, const wstring & name );
			virtual ~NetworkSource(void);

		protected:
			SOCKET			m_Socket;
			int				m_SocketReadTimeout;

		private:
			void InitWSA(void);
		protected:
			/************************************************************************/
			/* 
			����Socket�������UDP���ݴ��䣨MMS��RTSP����Ҳ�ᴴ��UDP Socket
			�쳣��
			����Socket�����׳�IOException
			�UDP Socket���˿ڳ����׳�IOException
			*/
			/************************************************************************/
			virtual void	OpenSocket(void);
			virtual void	Close();
			/************************************************************************/
			/* 
			�쳣��
			�������׳�IOException
			*/
			/************************************************************************/
			virtual size_t	ReadFromServer(char * buffer, size_t size);
			/************************************************************************/
			/* 
			�쳣��
			���ͳ����׳�IOException
			*/
			/************************************************************************/
			virtual void	SendToServer(const DataBuffer & data);
			/************************************************************************/
			/* 
			�쳣��
			�������׳�IOException
			*/
			/************************************************************************/
			virtual	size_t	SkipFromServer(size_t size);

			virtual void	SetUrl(const Url & u);
			virtual unsigned short GetDefaultPort();

			virtual const DataPacket * FixDataPacket( const DataPacket * packet, size_t fixPacketSize );
		};

	}
}
#endif