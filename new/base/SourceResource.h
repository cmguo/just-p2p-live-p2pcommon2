
#ifndef _LIVE_P2PCOMMON2_NEW_APP_SOURCE_RESOURCE_H_
#define _LIVE_P2PCOMMON2_NEW_APP_SOURCE_RESOURCE_H_

#include <synacast/protocol/data/PeerMinMax.h>
#include <ppl/util/time_counter.h>
#include <boost/noncopyable.hpp>
#include <iosfwd>


class SourceResource : private boost::noncopyable
{
public:
	SourceResource() : m_UpdateTime( 0 ), m_SourceTimeStamp(0)
	{
		m_SourceMinMax.Clear();
	}

	bool IsEmpty() const { return m_SourceMinMax.IsEmpty(); }
	UINT32 GetLength() const { return m_SourceMinMax.GetLength(); }
	UINT32 GetMinIndex() const { return m_SourceMinMax.MinIndex; }
	UINT32 GetMaxIndex() const { return m_SourceMinMax.MaxIndex; }

	UINT32 GetTimedLength(UINT buftime) const
	{
		double dTimedLen = m_SourceMinMax.GetLength() * 1.0 * buftime / 120000.0;
		return (UINT32)dTimedLen;
	}

	const PEER_MINMAX& GetMinMax() const
	{
		return m_SourceMinMax;
	}

	void Clear()
	{
		m_SourceMinMax.Clear();
	}

	void SaveMinMax( const PEER_MINMAX& minmax )
	{
		m_SourceMinMax = minmax;
		m_UpdateTime.sync();
	}

	void SaveTimeStamp( UINT64 timeStamp )
	{
		if ( timeStamp >= this->GetSourceTimeStamp() )
		{
			m_SourceTimeStamp = timeStamp;
			m_SourceTimeStampTime.sync();
		}
	}

	UINT64 GetSourceTimeStamp() const
	{
		if ( 0 == m_SourceTimeStamp )
			return 0;
		return m_SourceTimeStamp + m_SourceTimeStampTime.elapsed();
	}

	/// 根据SourceMinMax估算指定时间所需的piece数
	UINT CalcPieceCount( UINT bufferTime ) const;

	/// 根据SourceMinMax估算piece速率，单位：piece/s
	double CalcAveragePieceRate() const;

	/// 根据SourceMinMax估算平均每个piece的持续时间
	double CalcAveragePieceDuration() const;

	bool CheckPieceIndexValid(UINT pieceIndex) const;

protected:
	PEER_MINMAX m_SourceMinMax;
	time_counter m_UpdateTime;
	UINT64 m_SourceTimeStamp;
	time_counter m_SourceTimeStampTime;
};


std::ostream& operator<<(std::ostream& os, const SourceResource& res);

#endif