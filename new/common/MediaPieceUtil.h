
#ifndef _LIVE_P2PCOMMON2_NEW_COMMON_MEDIA_PIECE_UTIL_H_
#define _LIVE_P2PCOMMON2_NEW_COMMON_MEDIA_PIECE_UTIL_H_

#include "piecefwd.h"
#include "framework/memory.h"
#include <synacast/protocol/DataIO.h>
#include <synacast/protocol/piecefwd.h>
#include <boost/shared_ptr.hpp>

class DataSigner;
typedef boost::shared_ptr<DataSigner> DataSignerPtr;


class MediaPieceUtil
{
public:
	static const pool_byte_buffer& GetBuffer() { return GetBufferRef(); }

	static bool Verify( const MediaPiece& piece, DataSignerPtr signer );

	static bool Sign( const MonoMediaDataPiece& dataPiece, DataSignerPtr signer );

	static bool Sign( const MonoMediaHeaderPiece& headerPiece, DataSignerPtr signer );

	static MonoMediaPiecePtr MakeMonoPiece(const MediaPiece& piece);

protected:
	static void PrepareSignBuffer( const MonoMediaPiece& piece );

	static bool SignBuffer( data_output_stream& os, const MonoMediaPiece& piece, DataSignerPtr signer );

private:
	/// 共用缓冲区，优化性能，减少piece缓冲区操作时的内存申请（非线程安全）
	static pool_byte_buffer& GetBufferRef()
	{
		static pool_byte_buffer buffer(16 * 1024);
		return buffer;
	}
};

#endif

