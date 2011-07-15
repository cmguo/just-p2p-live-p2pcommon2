
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_IO_INPUT_STREAM_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_IO_INPUT_STREAM_H_

#include <stddef.h>


namespace ppl { namespace io { 


/// 输入流的接口类
class input_stream
{
public:
	input_stream() { }
	virtual ~input_stream() { }

	/// 读取全部n字节，如果读取失败（包括不能全部读取到），返回false
	virtual bool read_n( void* buf, size_t size ) = 0;

	/// 读取一个字节
	virtual bool read_byte( unsigned char& val ) = 0;

	/// 有多少字节可供读取
	virtual size_t available() const = 0;

	/// 检查是否有指定字节数的数据可供读取
	virtual bool is_available( size_t size ) const
	{
		return this->available() >= size;
	}

	/// 跳过n字节
	virtual bool skip_n( size_t size ) = 0;

	/// 获取当前位置
	virtual size_t position() const = 0;

	/// 总长度
	virtual size_t total_size() const = 0;

	/// 获取缓冲区指针
	virtual const unsigned char* get_buffer() = 0;

};


} }

#endif
