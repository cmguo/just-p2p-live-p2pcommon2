
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_IO_SERIALIZABLE_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_IO_SERIALIZABLE_H_

#include <ppl/io/data_input_stream.h>
#include <ppl/io/data_output_stream.h>
#include <stddef.h>
#include <assert.h>


namespace ppl { namespace io { 


class data_input_stream;
class data_output_stream;



/// ���л��Ľӿ���
class serializable
{
public:
	virtual ~serializable() { }

	/// ����������ȡ����
	virtual bool read_object( data_input_stream& is ) = 0;
	//{
	//	LIVE_ASSERT( false );
	//	return false;
	//}

	/// �����������������
	virtual void write_object( data_output_stream& os ) const = 0;
	//{
	//	LIVE_ASSERT(false);
	//}

	/// ��ȡ�����С
	virtual size_t get_object_size() const = 0;
	//{
	//	LIVE_ASSERT(false);
	//	return 0;
	//}
};

inline data_input_stream& operator>>( data_input_stream& is, serializable& obj )
{
	if ( false == obj.read_object( is ) )
	{
		// �����ȡʧ�ܣ���is״̬��Ϊbad���Խ�ֹ�����Ķ�ȡ����
		is.set_bad();
	}
	return is;
}

inline data_output_stream& operator<<( data_output_stream& os, const serializable& obj )
{
	obj.write_object( os );
	return os;
}


/// �ն���
class empty_serializable : public serializable
{
public:
	virtual bool read_object( data_input_stream& is )
	{
		return true;
	}

	virtual void write_object( data_output_stream& os ) const
	{
	}

	virtual size_t get_object_size() const
	{
		return 0;
	}
};


/// ���л���ģ��ӿ���
template <typename ObjT>
class serializable_info
{
public:
	/// ����������ȡ����
	// static bool read_object( data_input_stream& is );
	//{
	//	LIVE_ASSERT( false );
	//	return false;
	//}

	/// �����������������
	// static void write_object( data_output_stream& os ) const;
	//{
	//	LIVE_ASSERT(false);
	//}

	/// ��ȡ�����С
	//size_t get_object_size() const = 0;
	//{
	//	LIVE_ASSERT(false);
	//	return 0;
	//}

	/// �����С
	// enum { object_size = 4; }
};



} }

#endif
