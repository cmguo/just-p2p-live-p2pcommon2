
#include "StdAfx.h"

#include "util/flow.h"

#include <numeric>
#include <synacast/protocol/DataCollecting.h>


RateMeasure::RateMeasure(size_t maxUnits) : m_CurrentUnit(0)//, m_Rate(0), m_RecentTotal(0), m_Total(0), m_MaxRate( 0 ), m_TotalPackets( 0 )
{
	LIVE_ASSERT(maxUnits >= DEFAULT_UNITS && maxUnits < 60 * 60);
	m_Units.resize(maxUnits + 1);
}


UINT RateMeasure::GetAverageRate() const
{
	UINT usedTime = m_StartTime.elapsed32();
	if (usedTime < 1000)
		usedTime = 1000;
	// error C2520
	// m_total已经是64位的，m_total * 1000溢出的可能性很小
	return static_cast<UINT>(m_Total.Bytes * 1000.0 / usedTime);
}


UINT RateMeasure::GetAveragePacketRate() const
{
	UINT usedTime = m_StartTime.elapsed32();
	if (usedTime < 1000)
		usedTime = 1000;
	return (UINT) ( m_Total.Packets * 1000.0 / usedTime );
}


/*
void RateMeasure::Update(UINT amount)
{
	LIVE_ASSERT(m_CurrentUnit < m_Units.size());
	m_Total.Record(amount);
	m_Units[m_CurrentUnit].Record(amount);

//	m_RecentTotal.Bytes += amount;
//	m_RecentTotal.Packets++;
}
*/

void RateMeasure::Update()
{
	LIVE_ASSERT(m_CurrentUnit < m_Units.size());

#ifdef _DEBUG
	MeasureUnit sum;
	LIVE_ASSERT( std::accumulate( m_Units.begin(), m_Units.end(), sum ) == m_RecentTotal + m_Units[m_CurrentUnit]);
#endif

	size_t lastUnit = m_CurrentUnit;
	m_CurrentUnit++;
	if (m_CurrentUnit >= m_Units.size())
		m_CurrentUnit = 0;

	LIVE_ASSERT(m_RecentTotal.Bytes >= m_Units[m_CurrentUnit].Bytes);
	LIVE_ASSERT(m_RecentTotal.Packets >= m_Units[m_CurrentUnit].Packets);
	m_RecentTotal += m_Units[lastUnit];
	m_RecentTotal -= m_Units[m_CurrentUnit];
	m_Units[m_CurrentUnit].Clear();
	LIVE_ASSERT( m_Units.size() > 1 );
	m_Rate = m_RecentTotal / ( m_Units.size() - 1 );
//#pragma message("------ RateMeasure modification")
	//m_Rate = m_RecentTotal / m_Units.size();

	LIMIT_MIN( m_MaxRate.Bytes, m_Rate.Bytes );
	LIMIT_MIN( m_MaxRate.Packets, m_Rate.Packets );
}

void RateMeasure::SyncTo( RATE_COUNTER& dst ) const
{
	LIVE_ASSERT( sizeof( RATE_COUNTER ) == dst.StructSize );
	LIVE_ASSERT( sizeof( SINGLE_RATE_COUNTER ) == dst.Bytes.StructSize );
	LIVE_ASSERT( sizeof( SINGLE_RATE_COUNTER ) == dst.Packets.StructSize );

	dst.Bytes.Total = m_Total.Bytes;
	dst.Bytes.Rate = m_Rate.Bytes;
	dst.Bytes.AverageRate = this->GetAverageRate();
	dst.Bytes.MaxRate = m_MaxRate.Bytes;

	dst.Packets.Total = m_Total.Packets;
	dst.Packets.Rate = m_Rate.Packets;
	dst.Packets.AverageRate = this->GetAveragePacketRate();
	dst.Packets.MaxRate = m_MaxRate.Packets;
}




void FlowMeasure::SyncTo( TRAFFIC_COUNTER& dst ) const
{
	this->Download.SyncTo( dst.Download );
	this->Upload.SyncTo( dst.Upload );
}


