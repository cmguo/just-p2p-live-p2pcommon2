
#ifndef _LIVE_P2PCOMMON2_NEW_UTIL_FLOW_H_
#define _LIVE_P2PCOMMON2_NEW_UTIL_FLOW_H_

#include <ppl/util/time_counter.h>
#include <boost/noncopyable.hpp>
#include <vector>
#include <assert.h>

/// 默认的测试时间记录长度，1分钟
const UINT DEFAULT_RATE_MEASURE_PERIOD = 60 * 1000;

struct RATE_COUNTER;
struct TRAFFIC_COUNTER;


template <typename ValueT>
struct MeasureUnitT
{
	ValueT Bytes;
	ValueT Packets;

	typedef MeasureUnitT<ValueT> this_type;

	MeasureUnitT()
	{
		this->Clear();
	}
	MeasureUnitT(ValueT bytes, ValueT packets) : Bytes(bytes), Packets(packets)
	{
	}

	void Clear()
	{
		this->Bytes = 0;
		this->Packets = 0;
	}

	void Record(ValueT amount)
	{
		this->Bytes += amount;
		this->Packets++;
	}


	void operator+=(const this_type& mu)
	{
		this->Bytes += mu.Bytes;
		this->Packets += mu.Packets;
	}
	void operator-=(const this_type& mu)
	{
		assert(this->Bytes >= mu.Bytes);
		assert(this->Packets >= mu.Packets);
		this->Bytes -= mu.Bytes;
		this->Packets -= mu.Packets;
	}
	bool operator==(const this_type& mu) const
	{
		return this->Bytes == mu.Bytes && this->Packets == mu.Packets;
	}
	this_type operator/(size_t count) const
	{
		assert(count > 0);
		return MeasureUnitT(this->Bytes / count, this->Packets / count);
	}
	this_type operator+(const this_type& mu) const
	{
		return MeasureUnitT(this->Bytes + mu.Bytes, this->Packets + mu.Packets);
	}

};



typedef MeasureUnitT<UINT> MeasureUnit;
typedef MeasureUnitT<UINT64> MeasureUnit64;


//typedef UINT MeasureUnit;
//typedef INT64 MeasureUnit64;



/// 老方式的速度计算器(误差较小)
class RateMeasure : private boost::noncopyable
{
public:
	explicit RateMeasure(size_t maxUnits = DEFAULT_UNITS);

	/// 记录流量
	//void Update(UINT amount)
	//{
	//	this->Record( amount );
	//}
	void Record(UINT amount)
	{
		assert(m_CurrentUnit < m_Units.size());
		m_Total.Record( amount );
		m_Units[m_CurrentUnit].Record( amount );
	}

	/// 重置最大速度（重新统计）
	void ResetMax()
	{
		m_MaxRate.Clear();
	}

	UINT GetMaxRate() const { return m_MaxRate.Bytes; }
	UINT GetMaxPacketRate() const { return m_MaxRate.Packets; }

	/// 更新流量(推进记录位置)
	void Update();

	/// 获取即时速率(最近若干秒的速率)，单位(B/s)
	UINT GetRate() const { return m_Rate.Bytes; }

	/// 获取平均速率，单位(B/s)
	UINT GetAverageRate() const;

	UINT64 GetTotalBytes() const { return m_Total.Bytes; }
	UINT64 GetTotalPackets() const { return m_Total.Packets; }

	void SyncTo( RATE_COUNTER& dst ) const;

	/// 获取即时的报文速率(最近若干秒的速率)，单位(个/10s)
	UINT GetPacketRate() const { return m_Rate.Packets; }


	/// 获取平均报文速率，单位(个/10s)
	UINT GetAveragePacketRate() const;

	UINT GetRecentBytes() const { return m_RecentTotal.Bytes; }
	UINT GetRecentPackets() const { return m_RecentTotal.Packets; }

private:
	enum { DEFAULT_UNITS = 5 };

	typedef std::vector<MeasureUnit> MeasureUnitArray;

	MeasureUnitArray m_Units;

	MeasureUnit64 m_Total;

 	MeasureUnit m_RecentTotal;
	MeasureUnit m_Rate;

	/// 历史上最大的速度
	MeasureUnit m_MaxRate;

	size_t m_CurrentUnit;
	ppl::util::time_counter m_StartTime;
};








/// 流量测量器
class FlowMeasure
{
public:
	FlowMeasure() { }
	FlowMeasure(size_t maxUnits) : Upload(maxUnits), Download(maxUnits)
	{ }

	/// 上传的流量
	RateMeasure Upload;
	/// 下载的流量
	RateMeasure Download;

	void Update()
	{
		Upload.Update();
		Download.Update();
	}

	/// 获取总的平均速率
	UINT GetAverageRate() const
	{
		return Download.GetAverageRate() + Upload.GetAverageRate();
	}

	/// 获取总的瞬时速率
	UINT GetRate() const
	{
		return Download.GetRate() + Upload.GetRate();
	}
	UINT GetRateMax() const
	{
		return max(Download.GetRate(), Upload.GetRate());
	}
	void SyncTo( TRAFFIC_COUNTER& dst ) const;
};


#endif
