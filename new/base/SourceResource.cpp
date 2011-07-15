
#include "StdAfx.h"

#include "SourceResource.h"
#include <assert.h>
#include <ostream>

#include <ppl/data/pool_impl.h>

UINT SourceResource::CalcPieceCount( UINT bufferTime ) const
{
	if ( m_SourceMinMax.IsEmpty() )
	{
		//assert( false );
		return 0;
	}
	assert( m_SourceMinMax.GetLength() < 10000 );
	const double source_buffer_time = 120.0 * 1000.0;
	double pieceCount = 1.0 * m_SourceMinMax.GetLength() * bufferTime / source_buffer_time;
	return static_cast<UINT>( pieceCount );
}

double SourceResource::CalcAveragePieceRate() const
{
	if ( m_SourceMinMax.IsEmpty() )
	{
		//assert( false );
		return 0.0;
	}
	assert( m_SourceMinMax.GetLength() < 10000 );
	const double source_buffer_time = 120.0; // 单位：秒
	double pieceRate = m_SourceMinMax.GetLength() / source_buffer_time;
	return pieceRate;
}

double SourceResource::CalcAveragePieceDuration() const
{
	if ( m_SourceMinMax.IsEmpty() )
	{
		//assert( false );
		return 0.0;
	}
	assert( m_SourceMinMax.GetLength() < 10000 );
	const double source_buffer_time = 120.0 * 1000.0; // 单位：秒
	double pieceDuration = source_buffer_time / m_SourceMinMax.GetLength();
	return pieceDuration;
}

bool SourceResource::CheckPieceIndexValid(UINT pieceIndex) const
{
	//return true;  这句话是用来关闭 Source Min Max 参考的
	const PEER_MINMAX& sourceMinmax = m_SourceMinMax;
	if (sourceMinmax.IsEmpty() || sourceMinmax.GetLength() == 0)
		return true;
	return pieceIndex < (sourceMinmax.MaxIndex + 2*60*60*10) && (pieceIndex + 2000) > sourceMinmax.MinIndex;
}





std::ostream& operator<<( std::ostream& os, const SourceResource& res )
{
	os << res.GetMinMax();
	return os;
}


