
#include "StdAfx.h"
#include "AsfObject.h"
#include <assert.h>

using namespace Synacast::Format;

AsfObject::AsfObject(void) :
m_Guid(),
m_Size(0)
//m_RawData(),
{
}

AsfObject::~AsfObject(void)
{
}

void AsfObject::Parse( const char * data, size_t size )
{
	assert( size > 24 );
	m_Guid			= *( (GUID*)( data ) );
	m_Size			= *( (UINT64*)( data + 16 ) );
}
