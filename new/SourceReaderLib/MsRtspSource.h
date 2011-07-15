#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_MS_RTSP_SOURCE_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_MS_RTSP_SOURCE_H_

#include "RtspSource.h"

namespace Synacast
{
	namespace StreamSource
	{
		/************************************************************************/
		/* 
		由于对MS RTSP UDP传输机制并不清楚，UDP传输暂不支持
		*/
		/************************************************************************/
		class SOURCEREADERLIB_API MsRtspSource :
			public RtspSource
		{
		public:
			MsRtspSource(void);
			MsRtspSource( const string & u );
			virtual ~MsRtspSource(void);
		private:
			size_t	m_Sequence;
			size_t	m_HeaderSize;
			string	m_PlaylistID;
			string	m_Session;
			DWORD	m_LastBeatTick;
			size_t	m_SessionTimeout;
			string	m_Transport;

			string	m_UserAgent;

			UINT32		m_MaxPacketSize;
			UINT32		m_MinPacketSize;

		protected:
			virtual void			SetUrl( const Url & url );

			//virtual void	DoOptions();
			/************************************************************************/
			/* 
			Exceptions:
			SourceException
			*/
			/************************************************************************/
			virtual void	DoDescribe();
			/************************************************************************/
			/* 
			Exceptions:
			SourceException
			*/
			/************************************************************************/
			virtual void	DoSetups();
			/************************************************************************/
			/* 
			Exceptions:
			SourceException
			*/
			/************************************************************************/
			virtual void	DoPlay();
			/************************************************************************/
			/* 
			Exceptions:
			SourceException
			*/
			/************************************************************************/
			virtual void	DoPacketPair();
			virtual void	DoHeartBeat();
			virtual void	DoTearDown();

			virtual void	Reply( const HttpRequest & request );

			// threading
			HANDLE		m_CommandStopEvent;
			HANDLE		m_CommandThread;
			HANDLE		m_FeedStopEvent;
			HANDLE		m_FeedThread;

		private:
			virtual void	CreateFeedThreads();
			static DWORD WINAPI FeedThread( LPVOID msRtspSource );
			static DWORD WINAPI CommandThread( LPVOID msRtspSource );

			virtual void	DoCommands();
			virtual void	DoFeed();

			virtual void	InitRequest(HttpRequest & request);

			/************************************************************************/
			/* 
			异常：
				FormatException, 格式不对
				SourceException
			*/
			/************************************************************************/
			virtual void	ParseDescribe( const HttpResponse & response );
			virtual void	ParseSession( const HttpResponse & response );

		public:
			/************************************************************************/
			/* 
			Exceptions:
			SourceException
			*/
			/************************************************************************/
			virtual void	Open();
			virtual void	Close();
		protected:
			/************************************************************************/
			/* 
			异常：
				IllegalArgumentException: size > 20 || ( size > 0 && start == NULL )
				IOException
				SourceException
			*/
			/************************************************************************/
			virtual const	DataPacket * ReceiveOneDataPacket( const char * start, size_t size );
		private:
			void StopFeedThread(void);
		};

	}
}

#endif