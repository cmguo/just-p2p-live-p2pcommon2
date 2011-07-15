
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_PACKET_STREAM_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_PACKET_STREAM_H_

#include <ppl/io/data_output_stream.h>
#include <ppl/io/data_input_stream.h>


/// Э�鱨��ʹ�õ�������������ʹ��little-endian�������Ҫ�ı��ֽ��򣬾͸�����
class PacketInputStream : public ppl::io::memory_data_input_stream
{
public:
	PacketInputStream( const void* data, size_t size ) : ppl::io::memory_data_input_stream( data, size, false )
	{
	}
};

/// Э�鱨��ʹ�õ������������ʹ��little-endian�������Ҫ�ı��ֽ��򣬾͸�����
class PacketOutputStream : public ppl::io::memory_data_output_stream
{
public:
	PacketOutputStream( void* data, size_t size ) : ppl::io::memory_data_output_stream( data, size, false )
	{
	}
};

#endif