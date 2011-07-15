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
			����Socket�������ӵ���������
			����ֵ��
				������Socket
				INVALID_SOCKET, ����������ɹ�
			�쳣��
				IOException, ������ӳ����׳�
			*/
			/************************************************************************/
			static SOCKET CreateClientSocket( const string & host, const unsigned short port);
			/************************************************************************/
			/* 
			�쳣��
				IOException, ���ͳ���
			*/
			/************************************************************************/
			static void SendToServer( const SOCKET server, const DataBuffer & data);
			/************************************************************************/
			/* 
			�������׳�IOException
			*/
			/************************************************************************/
			static size_t ReadFromServer( const SOCKET server, char * buffer, const size_t size);
			/************************************************************************/
			/* 
			�������׳�IOException
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