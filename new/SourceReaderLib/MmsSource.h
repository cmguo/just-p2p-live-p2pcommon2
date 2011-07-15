#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_MMS_SOURCE_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_MMS_SOURCE_H_

#include <WinSock2.h>

#include "CriticalSection.h"

#include "BufferedNetworkSource.h"

#include "Packet.h"
#include "MmsDataPacket.h"

#include "MmsCommand.h"

#include "MmsClientCommand.h"
#include "MmsServerCommand.h"

using namespace Synacast::Protocol::Mms;

namespace Synacast
{
	namespace StreamSource
	{
		/************************************************************************/
		/* 
		先不考虑UDP传输条件下，数据包丢失，重复，失序等情况。
		所有失序，重复的数据包都丢弃处理。
		*/
		/************************************************************************/
		class SOURCEREADERLIB_API MmsSource :
			public BufferedNetworkSource
		{
		public:
			MmsSource(void);
			MmsSource( const string & u );
			virtual ~MmsSource(void);

		private:
			TransportNetwork	m_TransportNetwork;

			unsigned short	m_UdpListenPort;
			sockaddr_in		m_ServerAddress;
			int				m_AddressSize;
			SOCKET			m_UdpSocket;

			// protocol status
			UINT32		m_Sequence;
			size_t		m_HeaderSize;
			size_t		m_PacketSize;
			INT32		m_ClientID;

			// threading
			HANDLE		m_CommandThread;
			HANDLE		m_FeedThread;
			HANDLE		m_CommandStopEvent;
			HANDLE		m_FeedStopEvent;


		public:
			virtual void		SetUrl( const Url & u );
			virtual void		SetTransport( TransportNetwork transport ) { m_TransportNetwork = transport; };
			virtual TransportNetwork GetTransport() const { return m_TransportNetwork; };

		public:
			/************************************************************************/
			/* 
			Exceptions:
				SourceException
			*/
			/************************************************************************/
			virtual void	Open();
			virtual void	Close();
//			virtual size_t	Read( byte * buffer, size_t size );
			/************************************************************************/
			/* 
			Exceptions:
				SourceException
			*/
			/************************************************************************/
			virtual INT64	Seek( INT64 offset, SeekOrigin origin );

			virtual bool	CanRead();
			virtual bool	IsSeekable();

		public:
			virtual unsigned short GetDefaultPort();

		private:
			Packet * ReceiveOnePacket();
			/************************************************************************/
			/* 
			TCP only
			usually, size == 8
			Notice: while reading command, size should not greater than 40!
			异常：
			FormatException: Command格式错误
			IOException: 读取数据出错
			IllegalArgumentException: size > 40, 或者 start == NULL && size > 0
			*/
			/************************************************************************/
			const MmsClientCommand *	ReadOneCommand( const char * start, size_t size );
			/************************************************************************/
			/* 
			UDP & TCP
			usually, size == 8
			异常：
				IOException: 读出错
				IllegalArgumentException: size > 0 && start == NULL
			*/
			/************************************************************************/
			const MmsDataPacket *		ReadOneDataPacket( const char * start, const size_t size );

//			const MmsDataPacket *		FixDataPacket( const MmsDataPacket * packet );
			/************************************************************************/
			/* 
			UDP & TCP
			异常：
			IOException: 读出错
			*/
			/************************************************************************/
			void		ReceiveHeader();

			/************************************************************************/
			/* 
			异常：
			IOException: 发送出错
			*/
			/************************************************************************/
			void	DoCommand( const MmsCommand & command );

			/************************************************************************/
			/* 
			TCP only
			异常：
			IOException: 发送出错
			*/
			/************************************************************************/
			void	SendCommand( MmsServerCommand & command );
			/************************************************************************/
			/* 
			向Server发送一个Command，并获取Server的响应Command

			异常：
			IOException: 发送或者读取数据出错
			*/
			/************************************************************************/
			void	RequestServerCommand( MmsServerCommand & serverCommand, MmsClientCommand & clientCommand );

			static DWORD WINAPI FeedThread( LPVOID mmsSource );
			static DWORD WINAPI CommandThread(LPVOID param);

			/************************************************************************/
			/* 
			异常：
			IOExceptioin: 读取数据失败
			*/
			/************************************************************************/
			void	DoFeed();
			/************************************************************************/
			/* 
			异常：
			IOException: 读取或者发送数据失败
			*/
			/************************************************************************/
			void	DoCommands();

			const wstring GetLocalIP();

			virtual void CreateFeedThreads(void);
			virtual void ResetStatus(void);

			/************************************************************************/
			/* 
			Exceptions:
				IOException
			*/
			/************************************************************************/
			virtual void OpenSocket();
			void StopFeedThread(void);
		};
	}
}

#endif