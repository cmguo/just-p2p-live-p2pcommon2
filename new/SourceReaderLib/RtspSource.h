#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_RTSP_SOURCE_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_RTSP_SOURCE_H_
#include <WinSock2.h>
#include "BufferedNetworkSource.h"

#include "Packet.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

using namespace Synacast::Protocol::Http;

namespace Synacast
{
	namespace StreamSource
	{

		class SOURCEREADERLIB_API RtspSource :
			public BufferedNetworkSource
		{
		public:
			RtspSource( SourceType type, const wstring & name );
			RtspSource( const string & u, SourceType type, const wstring & name );
			virtual ~RtspSource(void);

		protected:
			TransportNetwork	m_TransportNetwork;

			unsigned short		m_UdpListenPort;
			sockaddr_in			m_ServerAddress;
			int					m_AddressSize;
			SOCKET				m_UdpSocket;

		public:
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
			virtual unsigned short	GetDefaultPort();

			virtual void			SetTransport( TransportNetwork transport ) { m_TransportNetwork = transport; };
			virtual TransportNetwork GetTransport() const { return m_TransportNetwork; };

		protected:
			virtual void			SendRequest( HttpRequest & request );
			/************************************************************************/
			/* 
			Exceptions:
				IOException
				FormatException
			*/
			/************************************************************************/
			virtual HttpResponse 	Request( HttpRequest & request );
			/************************************************************************/
			/* 
			Exceptions:
				IOException
			*/
			/************************************************************************/
			virtual void			OpenSocket();

			/************************************************************************/
			/* 
			Exceptions:
				SourceException
			*/
			/************************************************************************/
			virtual const Packet *	ReceiveOnePacket(void);
			/************************************************************************/
			/* 
			“Ï≥££∫
				IOException
			*/
			/************************************************************************/
			virtual			void	PadHttpMessage( HttpMessage & message );

			virtual const DataPacket * ReceiveOneDataPacket( const char * data, size_t size ) = 0;
		};
	}
}
#endif