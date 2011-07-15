#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_REAL_RTSP_STATUS_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_REAL_RTSP_STATUS_H_
#include "DataBuffer.h"
using namespace Synacast::Common;

#include <vector>
using namespace std;

namespace Synacast
{
	namespace StreamSource
	{

		class RealRtspStatus
		{
		public:
			RealRtspStatus(void);
			~RealRtspStatus(void);
			
			bool			m_IsLive;
			
			DataBuffer		m_HeaderData;

			size_t			m_Sequence;
			string			m_Session;
			vector<UINT16>	m_StreamID;
			vector<UINT16>	m_EndStreamID;
			vector<UINT16>	m_StartedStreams;
			string			m_Subscribe;

			UINT32			m_PrevTimeStamp;
			UINT16			m_PrevStreamID;

			// challenge
			string			m_ServerChallenge;
			string			m_Challenge;
			string			m_Challenge2;
			string			m_CheckSum;

		public:
			void			Reset();
		};

	}
}

#endif