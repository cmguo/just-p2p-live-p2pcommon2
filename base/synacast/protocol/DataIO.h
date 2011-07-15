
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATAIO_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATAIO_H_

#include <iosfwd>

namespace ppl { namespace io {

class serializable;
class data_input_stream;
class data_output_stream;

} }

using ppl::io::serializable;
using ppl::io::data_input_stream;
using ppl::io::data_output_stream;


std::ostream& operator<<( std::ostream& os, const data_input_stream& is );

#endif
