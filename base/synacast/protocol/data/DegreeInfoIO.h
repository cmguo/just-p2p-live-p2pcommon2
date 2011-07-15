
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_DEGREE_INFOIO_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_DEGREE_INFOIO_H_

#include <synacast/protocol/data/DegreeInfo.h>
#include <ppl/io/data_input_stream.h>
#include <ppl/io/data_output_stream.h>


inline ppl::io::data_output_stream& operator<<( ppl::io::data_output_stream& os, const DEGREE_PAIR& info )
{
	// DEGREE_PAIR里面都是字节，不需要处理字节序
	os.write_n( &info, DEGREE_PAIR::object_size );
	return os;
}
inline ppl::io::data_input_stream& operator>>( ppl::io::data_input_stream& is, DEGREE_PAIR& info )
{
	is.read_n( &info, DEGREE_PAIR::object_size );
	return is;
}

inline ppl::io::data_output_stream& operator<<( ppl::io::data_output_stream& os, const DEGREE_INFO& info )
{
	// DEGREE_INFO里面都是字节，不需要处理字节序
	os.write_n( &info, DEGREE_INFO::object_size );
	return os;
}
inline ppl::io::data_input_stream& operator>>( ppl::io::data_input_stream& is, DEGREE_INFO& info )
{
	is.read_n( &info, DEGREE_INFO::object_size );
	return is;
}



#endif


