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
			创建Socket，如果有UDP数据传输（MMS，RTSP），也会创建UDP Socket
			异常：
			创建Socket出错，抛出IOException
			邦定UDP Socket到端口出错，抛出IOException
			*/
			/************************************************************************/
			virtual void	OpenSocket(void);
			virtual void	Close();
			/************************************************************************/
			/* 
			异常：
			读出错，抛出IOException
			*/
			/************************************************************************/
			virtual size_t	ReadFromServer(char * buffer, size_t size);
			/************************************************************************/
			/* 
			异常：
			发送出错，抛出IOException
			*/
			/************************************************************************/
			virtual void	SendToServer(const DataBuffer & data);
			/************************************************************************/
			/* 
			异常：
			读出错，抛出IOException
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