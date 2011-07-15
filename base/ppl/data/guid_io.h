
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_DATA_GUID_IO_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_DATA_GUID_IO_H_

/**
 * @file
 * @brief 包含guid的less<>特化，供以GUID为key的map和set使用
 */


#include <ppl/data/guid.h>

#include <ppl/io/data_input_stream.h>
#include <ppl/io/data_output_stream.h>



inline ppl::io::data_output_stream& operator<<( ppl::io::data_output_stream& os, const GUID& x )
{
	os << x.Data1 << x.Data2 << x.Data3;
	os.write_n( x.Data4, 8 );
	return os;
}
inline ppl::io::data_input_stream& operator>>( ppl::io::data_input_stream& is, GUID& x )
{
	is >> x.Data1 >> x.Data2 >> x.Data3;
	is.read_n( x.Data4, 8 );
	return is;
}


#endif