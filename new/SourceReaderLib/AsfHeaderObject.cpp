#include "StdAfx.h"
#include "FormatException.h"
#include "AsfHeaderObject.h"


using namespace Synacast::Format;

AsfHeaderObject::AsfHeaderObject(void) :
AsfObject(),
m_HeaderCount(0),
m_Headers(),
m_FilePropertiesObject(NULL)
{
}

AsfHeaderObject::~AsfHeaderObject(void)
{
	ClearHeaders();
}

void AsfHeaderObject::Parse( const char * data, const size_t size )
{
	GUID guid = *( (GUID*)( data ) );
	if ( guid != ASF_HEADER_OBJECT )
	{
		throw FormatException();
	}

	m_Guid			= guid;
	m_Size			= *( (UINT64*)( data + 16 ) );
	m_HeaderCount	= *( (UINT32*)( data + 24 ) );

	size_t offset = 30;

	ClearHeaders();

	// parse headers
	size_t headerCount = 0;
	while ( offset <= ( size - 24 ) )									// stop by size
	{
		GUID guid			= *( (GUID*)( data + offset ) );
		UINT64 objectSize	= *( (UINT64*)( data + offset + 16 ) );
		AsfObject * object = NULL;
		if ( guid == ASF_FILE_PROPERTIES_OBJECT )
		{
			m_FilePropertiesObject = new AsfFilePropertiesObject();	// DESTROY in ClearHeaders()
			object = m_FilePropertiesObject;
		}
		else
		{
			object	= new AsfObject();								// 
		}
		object->Parse( data + offset, (size_t)objectSize );
		m_Headers.push_back( object );

		offset += (size_t)objectSize;

		headerCount += 1;
		if ( headerCount >= m_HeaderCount )								// stop by header count
		{
			break;
		}
	}
}

void Synacast::Format::AsfHeaderObject::ClearHeaders()
{
	//	m_Headers;
	for ( vector<AsfObject*>::const_iterator it = m_Headers.begin(); it != m_Headers.end(); ++it )
	{
		delete *it;
	}
	m_Headers.clear();

	m_FilePropertiesObject = NULL;
}
