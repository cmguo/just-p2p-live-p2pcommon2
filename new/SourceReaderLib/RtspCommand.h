#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_RTSP_COMMAND_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_RTSP_COMMAND_H_

#include "HttpRequest.h"
#include "HttpResponse.h"

#include "CommandPacket.h"
using namespace Synacast::Protocol;
using namespace Synacast::Protocol::Http;

namespace Synacast
{
	namespace Protocol
	{
		namespace Rtsp
		{

			class RtspCommand : public CommandPacket
			{
			public:
				RtspCommand(void);
				virtual ~RtspCommand(void);

			private:
				bool					m_IsRequest;

				const HttpRequest  *	m_Request;
				const HttpResponse *	m_Response;

			public:
				bool	IsRequest() const { return m_IsRequest; };
				const HttpResponse *	GetResponse()	const { return m_Response; };
				const HttpRequest *		GetRequest()	const { return m_Request; };

				void	SetRequest( const HttpRequest * request ) 
				{
					m_Request = request;
					if ( m_Request != NULL )
					{
						m_IsRequest = true;
					}
				}
				void	SetResponse( const HttpResponse * response )
				{
					m_Response = response;
					if ( m_Response != NULL )
					{
						m_IsRequest = false;
					}
				}
			};

		}
	}
}
#endif