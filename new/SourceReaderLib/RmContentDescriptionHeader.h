#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_RM_CONTENT_DESCRIPTION_HEADER_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_RM_CONTENT_DESCRIPTION_HEADER_H_
#include "rmobject.h"

namespace Synacast
{
	namespace Format
	{

		class SOURCEREADERLIB_API RmContentDescriptionHeader :
			public RmObject
		{
		public:
			RmContentDescriptionHeader(void);
			~RmContentDescriptionHeader(void);

		private:
			string		m_Title;
			//UINT16    title_len;
			//UINT8[title_len]  title;
			string		m_Author;
			//UINT16    author_len;
			//UINT8[author_len]  author;
			string		m_Copyright;
			//UINT16    copyright_len;
			//UINT8[copyright_len]  copyright;
			string		m_Comment;
			//UINT16    comment_len;
			//UINT8[comment_len]  comment;

		public:
			void SetTitle( const string & title )	{ m_Title = title; };
			const string &  GetTitle()		const	{ return m_Title; };

			void SetAuthor( const string & author ) { m_Author = author; };
			const string &  GetAuthor()		const	{ return m_Author; };

			void SetCopyright( const string & copyright ) { m_Copyright = copyright; };
			const string &  GetCopyright()	const	{ return m_Copyright; };

			void SetComment( const string & comment ) { m_Comment = comment; };
			const string &  GetComment()	const	{ return m_Comment; };

//			void Parse(const byte * data, const size_t size);

		private:
			void		ContentToData();
			UINT32		GetContentSize();
			/************************************************************************/
			/* 
			Exceptions:
				FormatException
			*/
			/************************************************************************/
			void		ParseContent();
		};

	}
}
#endif