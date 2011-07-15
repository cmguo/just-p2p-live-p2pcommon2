#include "StdAfx.h"
#include "FormatException.h"
#include "AsfFilePropertiesObject.h"

using namespace Synacast::Format;

AsfFilePropertiesObject::AsfFilePropertiesObject(void) :
AsfObject(),

m_FileID(),
m_FileSize(0),
m_CreateTime(0),
m_PacketCount(0),
m_PlayDuration(0),
m_SendDuration(0),
m_Preroll(0),
m_Flags(0),
m_MaxPacketSize(0),
m_MinPacketSize(0),
m_MaxBitrate(0)
{
}

AsfFilePropertiesObject::~AsfFilePropertiesObject(void)
{
}

void AsfFilePropertiesObject::Parse( const char * data, const size_t size )
{
	//m_RawData.Append( data, size );
//	const byte * data = m_RawData.GetData();
	if ( size < 104 )
	{
		throw FormatException();
	}

	GUID guid = *( (GUID*)( data ) );
	if ( guid != ASF_FILE_PROPERTIES_OBJECT )
	{
		throw FormatException();
	}

	m_Guid			= guid;
	m_Size			= *( (UINT64*)( data + 16 ) );

	m_FileID		= *( (GUID*)(data + 24) );  
	m_FileSize		= *( (UINT64*)(data + 40) );
	m_CreateTime	= *( (UINT64*)(data + 48) );
	m_PacketCount	= *( (UINT64*)(data + 56) );
	m_PlayDuration	= *( (UINT64*)(data + 64) );
	m_SendDuration	= *( (UINT64*)(data + 72) );
	m_Preroll		= *( (UINT64*)(data + 80) );
	m_Flags			= *( (UINT32*)(data + 88) );
	m_MaxPacketSize	= *( (UINT32*)(data + 92) );
	m_MinPacketSize	= *( (UINT32*)(data + 96) );
	m_MaxBitrate	= *( (UINT32*)(data + 100) );
}