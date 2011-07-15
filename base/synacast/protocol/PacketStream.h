
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_PACKET_STREAM_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_PACKET_STREAM_H_

#include <ppl/io/data_output_stream.h>
#include <ppl/io/data_input_stream.h>


/// 协议报文使用的数据输入流，使用little-endian，如果需要改变字节序，就改这里
class PacketInputStream : public ppl::io::memory_data_input_stream
{
public:
	PacketInputStream( const void* data, size_t size ) : ppl::io::memory_data_input_stream( data, size, false )
	{
	}
};

/// 协议报文使用的数据输出流，使用little-endian，如果需要改变字节序，就改这里
class PacketOutputStream : public ppl::io::memory_data_output_stream
{
public:
	PacketOutputStream( void* data, size_t size ) : ppl::io::memory_data_output_stream( data, size, false )
	{
	}
};

#endif