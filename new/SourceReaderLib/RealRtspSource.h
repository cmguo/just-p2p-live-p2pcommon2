#pragma warning( disable: 4786 )
#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_REAL_RTSP_SOURCE_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_REAL_RTSP_SOURCE_H_

#include "RtspSource.h"
#include "RealRtspStatus.h"
#include "RdtRtspDataPacket.h"

using namespace Synacast::Protocol::Rtsp;

namespace Synacast
{
	namespace StreamSource
	{

		/************************************************************************/
		/* 
		*/
		/************************************************************************/
		class SOURCEREADERLIB_API RealRtspSource :
			public RtspSource
		{
		public:
			RealRtspSource(void);
			RealRtspSource( const string & url );
			virtual ~RealRtspSource(void);

		private:
			RealRtspStatus	m_RtspStatus;

			// heart beat
			size_t			m_SessionTimeout;
			DWORD			m_LastBeatTick;

			// threading
			HANDLE		m_CommandStopEvent;
			HANDLE		m_CommandThread;
			HANDLE		m_FeedStopEvent;
			HANDLE		m_FeedThread;

		public:
			/************************************************************************/
			/* 
			Exceptions:
				SourceException
			*/
			/************************************************************************/
			virtual void	Open();
			virtual void	Close();

			void			ComputeChallenge();

		protected:
			virtual void			SetUrl( const Url & url );

			void DoOptions();
			void DoDescribe();
			/************************************************************************/
			/* 
			Exceptions:
			SourceException
			*/
			/************************************************************************/
			void DoSetups();
			/************************************************************************/
			/* 
			Exceptions:
			SourceException
			*/
			/************************************************************************/
			void DoSetupParameter();
			/************************************************************************/
			/* 
			Exceptions:
			SourceException
			*/
			/************************************************************************/
			void DoPlay();
			void DoHeartBeat();
			void DoTearDown();

			/************************************************************************/
			/* 
			Exceptions:
			SourceException
			*/
			/************************************************************************/
			void ParseOptions( const HttpResponse & response );
			/************************************************************************/
			/* 
			Exceptions:
			SourceException
			*/
			/************************************************************************/
			void ParseDescribe( const HttpResponse & response );

			void InitRequest( HttpRequest & request );

			void CreateFeedThreads();

			static DWORD WINAPI CommandThread( LPVOID realRtspSource );

			void DoCommands();

			/************************************************************************/
			/* 
			Exceptions:
				IllegalArgumentException: size != 4
				IOException
			*/
			/************************************************************************/
			virtual const DataPacket * ReceiveOneDataPacket( const char * data, size_t size );

			DataPacket * Rdt2RmPacket( const RdtRtspDataPacket & dataPacket );

			void DoStreamEnd( UINT16 streamID );

			bool StreamStarted( UINT16 streamID );

			template<class T>
			bool VectorContains( const vector<T> v, T element )
			{
				bool contains = false;

				for ( vector<T>::const_iterator it = v.begin(); it != v.end(); ++it )
				{
					if ( *it == element )
					{
						contains = true;
						break;
					}
				}

				return contains;
			}
		};

	}
}
#endif