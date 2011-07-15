
#include "StdAfx.h"

#include "VODProxy.h"
#include "common/BootModule.h"
#include "util/StunModule.h"
#include <ppl/net/inet.h>

static inline boost::uint16_t little_endian_to_host_short(
    boost::uint16_t v)
{
    char * ch = (char *)&v;
    return (boost::uint16_t)ch[1] << 8
        | (boost::uint16_t)ch[0];
}


bool VODProxy::HandlePacket( BYTE* data, size_t size, const InetSocketAddress& remoteAddr, BootModule* bootModule, StunModule* stunModule )
{

	//const size_t total_header_size = sizeof(CNetHeader) + sizeof(ShortP2PProtocolHeader);
	//if ( size < total_header_size )
	//{
	//	return result;
	//}

	if ( size < sizeof(CNetHeader) )
		return false;

	if(0 != VODPacketBuilder::CheckSum((UINT16*)data, size))
	{// 校验和失败，释放传输结构
		return false;
	}

	CNetHeader* pstNetHeader = (CNetHeader*)data;
	if ( NHTYPE_CONFUSE_PFS_SHORT_P2PHEADER == (little_endian_to_host_short)(pstNetHeader->m_usNetType) )
	{
		if ( stunModule )
		{
			return stunModule->HandlePacket( data, size, remoteAddr );
		}
	}
	else if ( NHTYPE_CONFUSE_PFS == little_endian_to_host_short(pstNetHeader->m_usNetType ))
	{
		return bootModule->HandlePacket( data, size, remoteAddr );
	}
	return false;
}
