#include "StdAfx.h"
#include "BaseException.h"

using namespace Synacast::Exception;

BaseException::BaseException(void) :
m_Message(),
m_InnerException(NULL)
{
}

BaseException::BaseException( const string & message ) :
m_Message(message),
m_InnerException(NULL)
{
}

BaseException::BaseException( const char * message ) :
m_Message(message),
m_InnerException(NULL)
{
}

BaseException::BaseException( const string & message, const BaseException * innerException ) :
m_Message( message ),
m_InnerException( innerException )
{

}
BaseException::~BaseException(void) throw()
{
}

void Synacast::Exception::BaseException::SetMessage( const char * message )
{
	m_Message = string( message );
}

const char * Synacast::Exception::BaseException::GetMessageCStr() const
{
	return m_Message.c_str();
}