

#ifndef _LIVE_P2PCOMMON2_BASE_PPL_UTIL_TIME_COUNTER_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_UTIL_TIME_COUNTER_H_

#include <ppl/config.h>
#include <ppl/data/int.h>


#if defined(_PPL_PLATFORM_MSWIN)

namespace ppl { namespace util {


namespace detail {

    using ::GetTickCount;

class tick_counter64
{
public:
	tick_counter64()
	{
		m_count32 = ::GetTickCount();
		m_count64 = m_count32;
	}

	/// 获取当前的时间计数
	UINT64 get_count()
	{
		DWORD newCount32 = ::GetTickCount();
		DWORD diff = newCount32 - m_count32;
		m_count32 = newCount32;
		m_count64 += diff;
		return m_count64;
	}


private:
	UINT64 m_count64;
	DWORD m_count32;
};
}


/// 基于GetTickCount的32位计时器(单位：毫秒)
class time_counter32
{
public:
	typedef DWORD count_value_type;

	static DWORD get_system_count()
	{
		return ::GetTickCount();
	}

	/// 获取当前的时间
	static DWORD get_realtime_count()
	{
		return get_system_count();
	}

	time_counter32()
	{
		m_count = get_realtime_count();
	}
	explicit time_counter32(DWORD count) : m_count(count)
	{
	}

	/// 同步计时
	void sync()
	{
		m_count = get_realtime_count();
	}

	/// 获取从上次计时开始的时间
	DWORD elapsed() const
	{
		return get_realtime_count() - m_count;
	}

	/// 获取所记录的时间
	DWORD get_count() const
	{
		return m_count;
	}
	/// 自动转换成DWORD
	operator DWORD() const
	{
		return get_count();
	}

private:
	/// 所记录的时间
	DWORD m_count;
};

/// 基于GetTickCount的计时器(单位：毫秒)
class time_counter
{
public:
	typedef UINT64 count_value_type;

	static UINT64 get_system_count()
	{
		static detail::tick_counter64 tickCounter;
		return tickCounter.get_count();
	}

	static DWORD get_system_count32()
	{
		return ::GetTickCount();
	}

	time_counter()
	{
		m_count = get_realtime_count();
	}
	explicit time_counter(UINT64 count) : m_count(count)
	{
	}

	/// 同步计时
	void sync()
	{
		m_count = get_realtime_count();
	}
	/// 获取从上次计时开始的时间
	UINT64 elapsed() const
	{
		return get_realtime_count() - m_count;
	}
	/// 获取从上次计时开始的时间
	DWORD elapsed32() const
	{
		return static_cast<DWORD>( get_realtime_count() - m_count );
	}

	/// 获取所记录的时间
	UINT64 get_count() const
	{
		return m_count;
	}
	/// 自动转换成DWORD
	operator UINT64() const
	{
		return get_count();
	}

//#if defined(_PPL_USES_TICK_COUNTER64)
#pragma message("------ use simulated 64-bit GetTickCount")
	/// 获取当前的时间
	UINT64 get_realtime_count() const
	{
		return m_tick.get_count();
	}
//#else
	//DWORD GetTimeCount()
	//{
	//	return ::GetTickCount();
	//}
//#endif

private:
	/// 所记录的时间
	UINT64 m_count;
	mutable detail::tick_counter64 m_tick;
};

} }



#elif defined(_PPL_PLATFORM_POSIX)


#include <sys/time.h>
#include <time.h>

#if (defined __MACH__)
#  include <mach/mach_time.h>
#endif

namespace ppl { namespace util {


namespace detail {
inline UINT64 GetTickCount64()
{
#ifdef __MACH__
            struct mach_timebase_info info;
            ::mach_timebase_info(&info);
            boost::uint64_t t = mach_absolute_time() / 1000 / 1000;
            if (info.numer != info.denom)
                t = t * info.numer / info.denom;
            return t;
#else
	struct timespec t = { 0 };
	int res = clock_gettime(CLOCK_REALTIME, &t);
	LIVE_ASSERT( 0 == res );
    UINT64 val = (UINT64)t.tv_sec * 1000 + (UINT64)t.tv_nsec / 1000000;
	return val;
#endif
}


/*
inline UINT32 GetTickCount()
{
	static UINT64 initialTick64 = GetTickCount64();
	UINT64 currentTick64 = GetTickCount64();
	LIVE_ASSERT(currentTick64 >= initialTick64);
	UINT64 diff = currentTick64 - initialTick64;
	LIVE_ASSERT(diff < INT_MAX);
	return static_cast<UINT32>( diff );
}
*/

inline UINT32 GetTickCount()
{
	UINT64 ticks = GetTickCount64();
	return static_cast<UINT32>( ticks );
}
}

/// 基于GetTickCount的计时器(单位：毫秒)
class time_counter
{
public:
	typedef UINT64 count_value_type;

	time_counter()
	{
		m_count = get_system_count();
	}
	explicit time_counter(count_value_type count) : m_count(count)
	{
	}

	static UINT64 get_system_count()
	{
        return detail::GetTickCount64();
	}
	static UINT32 get_system_count32()
	{
        return static_cast<UINT32>( detail::GetTickCount64() );
	}


	/// 同步计时
	void sync()
	{
		m_count = get_system_count();
	}
	/// 获取从上次计时开始的时间
	count_value_type elapsed() const
	{
		return get_system_count() - m_count;
	}
	DWORD elapsed32() const
	{
		return static_cast<DWORD>( this->elapsed() );
	}

	/// 获取所记录的时间
	count_value_type get_count() const
	{
		return m_count;
	}
	/// 自动转换成DWORD
	operator count_value_type() const
	{
		return get_count();
	}


//#pragma message("------ use simulated 64-bit GetTickCount on posix platform")
	/// 获取当前的时间
	UINT64 get_realtime_count()
	{
		return get_system_count();
	}

private:
	/// 所记录的时间
	count_value_type m_count;
};

} }


#else

#error unsupported platform for TimeCounter

#endif




class time_span
{
public:
	enum 
	{ 
		TICKS_PER_SECOND = 1000, 
		TICKS_PER_MINUTE = 1000 * 60, 
		TICKS_PER_HOUR = 1000 * 60 * 60, 
		TICKS_PER_DAY = 1000 * 60 * 60 * 24, 
	};

	explicit time_span( UINT64 diff = 0 ) : m_diff( diff ) { }

	UINT64 total_ticks() const { return m_diff; }
	UINT64 total_days() const { return m_diff / TICKS_PER_DAY; }
	UINT64 total_hours() const { return m_diff / TICKS_PER_HOUR; }
	UINT64 total_minutes() const { return m_diff / TICKS_PER_MINUTE; }
	UINT64 total_seconds() const { return m_diff / TICKS_PER_SECOND; }
	UINT64 total_milliseconds() const { return m_diff; }

	UINT days() const { return static_cast<UINT>( this->total_days() ); }

	UINT hours() const
	{
		UINT oddTicks = static_cast<UINT>( m_diff % TICKS_PER_DAY );
		return oddTicks / TICKS_PER_HOUR;
	}
	UINT minutes() const
	{
		UINT oddTicks = static_cast<UINT>( m_diff % TICKS_PER_HOUR );
		return oddTicks / TICKS_PER_MINUTE;
	}
	UINT seconds() const
	{
		UINT oddTicks = static_cast<UINT>( m_diff % TICKS_PER_MINUTE );
		return oddTicks / TICKS_PER_SECOND;
	}
	UINT milliseconds() const
	{
		UINT oddTicks = static_cast<UINT>( m_diff % TICKS_PER_SECOND );
		return oddTicks;
	}

private:
	UINT64 m_diff;
};


#endif
