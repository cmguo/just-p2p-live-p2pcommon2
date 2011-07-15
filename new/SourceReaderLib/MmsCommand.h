#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_MMS_COMMAND_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_MMS_COMMAND_H_

#include "DataBuffer.h"
using namespace Synacast::Common;

#include "CommandPacket.h"
using namespace Synacast::Protocol;

//#if _MSC_VER < 1400
			typedef UINT16 CommandDirection;
			const static CommandDirection TO_SERVER = 0x03;
			const static CommandDirection TO_CLIENT = 0x04;
//#else
//			enum CommandDirection : UINT16
//			{
//				TO_SERVER		= 0x03,
//				TO_CLIENT		= 0x04
//			};
//#endif

namespace Synacast
{
	namespace Protocol
	{
		namespace Mms
		{
			class MmsCommand : public CommandPacket
			{
			public:
				MmsCommand();
				MmsCommand( UINT16 command, CommandDirection direction, UINT32 sequence, double timeStamp, INT32 prefix1, INT32 prefix2 );
				virtual ~MmsCommand(void);

			protected:
				INT32		m_Version;
				INT32		m_ID;
				UINT32		m_Length;
				INT32		m_Protocol;
				UINT32		m_LengthInByte;
				UINT32		m_Sequence;				// *
				double		m_TimeStamp;			// *
				UINT32		m_LengthInByte2;

				UINT16		m_Command;				// *
				CommandDirection	m_Direction;	// *

				DataBuffer	m_Header;
				DataBuffer	m_Data;

				INT32		m_Prefix1;
				INT32		m_Prefix2;

			public:
				virtual UINT32 GetSequence()		const	{ return m_Sequence; };
				virtual double GetTimeStamp()		const	{ return m_TimeStamp; };
				virtual UINT16 GetCommand()			const	{ return m_Command; };
				virtual CommandDirection GetDirection() const { return m_Direction; };

				virtual void SetSequence( UINT32 sequence )		{ m_Sequence = sequence; };
				virtual void SetTimeStamp( double timeStamp )	{ m_TimeStamp = timeStamp; };
				virtual void SetCommand( UINT16 command )		{ m_Command = command; };
				virtual void SetDirection( CommandDirection direction ) { m_Direction = direction; };

				static bool ValidateCommandID( const char * data );

				// raw data
				virtual const DataBuffer & GetHeader()	const	{ return m_Header; };
				virtual const DataBuffer & GetData()	const	{ return m_Data; };
			protected:
				virtual bool ValidateCommand( const char * data, size_t size );
				virtual bool ValidateCommand( const DataBuffer & data );
			};

		}
	}
}
#endif