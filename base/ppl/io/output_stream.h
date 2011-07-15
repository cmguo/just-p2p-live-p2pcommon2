
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_IO_OUTPUT_STREAM_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_IO_OUTPUT_STREAM_H_

#include <stddef.h>


namespace ppl { namespace io { 


/// ������Ľӿ���
class output_stream
{
public:
	output_stream() { }
	virtual ~output_stream() { }

	/// д��ȫ��n�ֽڣ����ʧ�ܣ���������ȫ��д�룩�����׳��쳣��Ҳ�������ϲ�����֤�������㹻��
	virtual void write_n( const void* data, size_t size ) = 0;
	/// д��һ���ֽ�
	virtual void write_byte( unsigned char val ) = 0;

	/// ��ȡ��ǰλ�ã�Ҳ�����Ѿ�д�˶����ֽ�
	virtual size_t position() const = 0;
};


} }

#endif
