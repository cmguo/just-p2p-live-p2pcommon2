
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_IO_OUTPUT_STREAM_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_IO_OUTPUT_STREAM_H_

#include <stddef.h>


namespace ppl { namespace io { 


/// 输出流的接口类
class output_stream
{
public:
	output_stream() { }
	virtual ~output_stream() { }

	/// 写入全部n字节，如果失败（包括不能全部写入），会抛出异常，也就是由上层来保证缓冲区足够大
	virtual void write_n( const void* data, size_t size ) = 0;
	/// 写入一个字节
	virtual void write_byte( unsigned char val ) = 0;

	/// 获取当前位置，也就是已经写了多少字节
	virtual size_t position() const = 0;
};


} }

#endif
