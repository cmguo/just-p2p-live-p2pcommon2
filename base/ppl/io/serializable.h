
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_IO_SERIALIZABLE_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_IO_SERIALIZABLE_H_

#include <ppl/io/data_input_stream.h>
#include <ppl/io/data_output_stream.h>
#include <stddef.h>
#include <assert.h>


namespace ppl { namespace io { 


class data_input_stream;
class data_output_stream;



/// 序列化的接口类
class serializable
{
public:
	virtual ~serializable() { }

	/// 从输入流读取对象
	virtual bool read_object( data_input_stream& is ) = 0;
	//{
	//	LIVE_ASSERT( false );
	//	return false;
	//}

	/// 将对象数据输出到流
	virtual void write_object( data_output_stream& os ) const = 0;
	//{
	//	LIVE_ASSERT(false);
	//}

	/// 获取对象大小
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
		// 如果读取失败，将is状态置为bad，以禁止后续的读取操作
		is.set_bad();
	}
	return is;
}

inline data_output_stream& operator<<( data_output_stream& os, const serializable& obj )
{
	obj.write_object( os );
	return os;
}


/// 空对象
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


/// 序列化的模板接口类
template <typename ObjT>
class serializable_info
{
public:
	/// 从输入流读取对象
	// static bool read_object( data_input_stream& is );
	//{
	//	LIVE_ASSERT( false );
	//	return false;
	//}

	/// 将对象数据输出到流
	// static void write_object( data_output_stream& os ) const;
	//{
	//	LIVE_ASSERT(false);
	//}

	/// 获取对象大小
	//size_t get_object_size() const = 0;
	//{
	//	LIVE_ASSERT(false);
	//	return 0;
	//}

	/// 对象大小
	// enum { object_size = 4; }
};



} }

#endif
