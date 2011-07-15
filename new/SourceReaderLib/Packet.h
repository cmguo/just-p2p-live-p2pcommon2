#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_PACKET_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_PACKET_H_

#include "CommandPacket.h"
#include "DataPacket.h"

namespace Synacast
{
	namespace Protocol
	{
		class Packet
		{
		public:
			Packet(void);
			virtual ~Packet(void);

		private:
			const DataPacket	* m_Data;
			const CommandPacket	* m_Command;
			bool				m_HasData;
			bool				m_HasCommand;

		public:
			bool			HasData() const { return m_HasData; };
			bool			HasCommand() const { return m_HasCommand; };

			void			SetData( const DataPacket * data ) 
			{ 
				m_HasCommand = false;
				m_Data = data; 
				m_HasData = true;
			};
			void			SetCommand( const CommandPacket * command ) 
			{ 
				m_HasData = false;
				m_Command = command; 
				m_HasCommand = true;
			};

			const DataPacket	*	GetData() const { return m_Data; };
			const CommandPacket *	GetCommand() const{ return m_Command; };
		};
	}
}
#endif