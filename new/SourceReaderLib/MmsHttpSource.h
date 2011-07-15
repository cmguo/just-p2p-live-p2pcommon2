#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_MMS_HTTP_SOURCE_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_MMS_HTTP_SOURCE_H_

#include <string>
using namespace std;

#include "NetworkSource.h"

namespace Synacast
{
	namespace StreamSource
	{

		class SOURCEREADERLIB_API MmsHttpSource :
			public NetworkSource
		{
		public:
			MmsHttpSource(void);
			MmsHttpSource( const string & url );
			virtual ~MmsHttpSource(void);

		private:
			string		m_ClientID;

			size_t		m_ContentLength;
			string		m_TransferEncoding;

			// for chunked transfer
			DataBuffer	m_Buffer;
			size_t		m_BufferPointer;

			bool		m_StreamEnd;

			UINT32		m_MaxPacketSize;
			UINT32		m_MinPacketSize;

		public:
			virtual void	SetUrl( const Url & u );
			virtual void	Open();
			virtual void	Close();
			/************************************************************************/
			/* 
			Exceptions:
				SourceException
			*/
			/************************************************************************/
			virtual	size_t	Read( char * buffer, size_t size );
			/************************************************************************/
			/* 
			Exceptions:
				SourceException
			*/
			/************************************************************************/
			virtual INT64	Seek( INT64 offset, SeekOrigin origin );

			// attributes
			virtual bool	CanRead();
			virtual bool	IsSeekable();

		public:
			virtual unsigned short GetDefaultPort();

		protected:
			virtual void SendRequest(void);
			/************************************************************************/
			/* 
			Exceptions:
				SourceException
			*/
			/************************************************************************/
			virtual void ReadResponse(void);
		private:
			/************************************************************************/
			/* 
			Exceptions:
				SourceException
			*/
			/************************************************************************/
			size_t	DoRead( char * buffer, const size_t size );

			void	BufferNextChunk(void);
			void	BufferNextPacket(void);

			/************************************************************************/
			/* 
			异常：
			FormatException, 格式不对
			*/
			/************************************************************************/
			void	ParseHeader( const char * buffer, const size_t size );
			void	PadBuffer( const char * buffer, const size_t size );

			size_t	ReadFromChunks(char * buffer, size_t size);
			size_t	ReadFromRawSocket(char * buffer, size_t size);

			size_t	ReadFromBuffer(char * buffer, size_t size);
		};

	}
}

#endif