#include "StdAfx.h"
#include "FormatException.h"

using namespace Synacast::Format;

FormatException::FormatException(void)
{
}

FormatException::~FormatException(void) throw()
{
}

FormatException::FormatException( const string & message ) :
BaseException( message )
{

}