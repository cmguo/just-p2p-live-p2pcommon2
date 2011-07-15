#pragma warning( disable: 4786 )

#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_RM_HEADER_CHUNK_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_RM_HEADER_CHUNK_H_
#include "RmContentDescriptionHeader.h"
#include "RmDataChunkHeader.h"
#include "RmFileHeader.h"
#include "RmMediaPropertiesHeader.h"
#include "RmPropertiesHeader.h"
using namespace Synacast::Format;

#include <vector>
#include <set>
using namespace std;

#include "HeaderChunk.h"

namespace Synacast
{
	namespace SourceReader
	{

		class SOURCEREADERLIB_API RmHeaderChunk :
			public HeaderChunk
		{
		public:
			RmHeaderChunk(void);
			virtual ~RmHeaderChunk(void);

		private:
			RmFileHeader						m_FileHeader;
			RmPropertiesHeader					m_PropertiesHeader;
			vector<RmMediaPropertiesHeader>		m_MediaHeaders;
			RmContentDescriptionHeader			m_ContentHeader;
			RmDataChunkHeader					m_DataChunkHeader;

			set<UINT16>							m_VideoStreamID;
			set<UINT16>							m_AudioStreamID;

			DataBuffer							m_PureHeader;

		public:
			/************************************************************************/
			/* 
			“Ï≥££∫
			FormatException
			*/
			/************************************************************************/
			virtual void SetRawData( const char * data, const size_t size );

			virtual DataBuffer & BuildPureHeaderChunk();

			const RmFileHeader & GetFileHeader() const { return m_FileHeader; };
			const RmPropertiesHeader& GetPropertiesHeader() const { return m_PropertiesHeader; };
			const RmContentDescriptionHeader & GetContentDescriptionHeader() const { return m_ContentHeader; };
			const RmDataChunkHeader & GetDataChunkHeader() const { return m_DataChunkHeader; };

			const vector<RmMediaPropertiesHeader> & GetMediaHeaders() const { return m_MediaHeaders; };

			size_t	GetMaxBitrate()		const { return m_PropertiesHeader.GetMaxBitrate(); };
			size_t	GetAvgBitrate()		const { return m_PropertiesHeader.GetAvgBitrate(); };
			size_t	GetMaxPacketSize()	const { return m_PropertiesHeader.GetMaxPacketSize(); };

			const set<UINT16> & GetVideoStreamID() const { return m_VideoStreamID; };
			const set<UINT16> & GetAudioStreamID() const { return m_AudioStreamID; };

			size_t GetVideoStreamCount() const;
			size_t GetAudioStreamCount() const;

			size_t GetVideoStreamID( UINT16 * buffer, const size_t size );
			size_t GetAudioStreamID( UINT16 * buffer, const size_t size );

		private:
			/************************************************************************/
			/* 
			Exceptions:
				FormatException
			*/
			/************************************************************************/
			void ParseHeaders();
		};

	}
}

#endif