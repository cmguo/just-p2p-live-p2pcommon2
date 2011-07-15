
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_IO_INPUT_STREAM_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_IO_INPUT_STREAM_H_

#include <stddef.h>


namespace ppl { namespace io { 


/// �������Ľӿ���
class input_stream
{
public:
	input_stream() { }
	virtual ~input_stream() { }

	/// ��ȡȫ��n�ֽڣ������ȡʧ�ܣ���������ȫ����ȡ����������false
	virtual bool read_n( void* buf, size_t size ) = 0;

	/// ��ȡһ���ֽ�
	virtual bool read_byte( unsigned char& val ) = 0;

	/// �ж����ֽڿɹ���ȡ
	virtual size_t available() const = 0;

	/// ����Ƿ���ָ���ֽ��������ݿɹ���ȡ
	virtual bool is_available( size_t size ) const
	{
		return this->available() >= size;
	}

	/// ����n�ֽ�
	virtual bool skip_n( size_t size ) = 0;

	/// ��ȡ��ǰλ��
	virtual size_t position() const = 0;

	/// �ܳ���
	virtual size_t total_size() const = 0;

	/// ��ȡ������ָ��
	virtual const unsigned char* get_buffer() = 0;

};


} }

#endif
