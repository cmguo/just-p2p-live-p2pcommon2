#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_NETWORK_UTIL_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_NETWORK_UTIL_H_
#include "DataBuffer.h"

namespace Synacast
{
	namespace Common
	{

		class NetworkUtil
		{
		private:
			NetworkUtil(void);
			~NetworkUtil(void);
		public:
			// TCP
			/************************************************************************/
			/* 
			创建Socket，并连接到服务器端
			返回值：
				创建的Socket
				INVALID_SOCKET, 如果创建不成功
			异常：
				IOException, 如果连接出错，抛出
			*/
			/************************************************************************/
			static SOCKET CreateClientSocket( const string & host, const unsigned short port);
			/************************************************************************/
			/* 
			异常：
				IOException, 发送出错
			*/
			/************************************************************************/
			static void SendToServer( const SOCKET server, const DataBuffer & data);
			/************************************************************************/
			/* 
			读出错，抛出IOException
			*/
			/************************************************************************/
			static size_t ReadFromServer( const SOCKET server, char * buffer, const size_t size);
			/************************************************************************/
			/* 
			读出错，抛出IOException
			*/
			/************************************************************************/
			static size_t SkipFromServer( const SOCKET server, const size_t size);

			//// UDP
			//static SOCKET CreateUdpSocket();
			//static void SendToServer( SOCKET theSocket, sockaddr_in address, const DataBuffer & data );
			//static size_t ReadFromServer( SOCKET theSocket, sockaddr_in address, int * addressSize, byte * buffer, size_t size );
			//static size_t SkipFromServer( SOCKET server, sockaddr_in address, int * addressSize, size_t size );
		};

	}
}

#endif