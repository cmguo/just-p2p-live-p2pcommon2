
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_CONCURRENT_SCOPED_LOCK_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_CONCURRENT_SCOPED_LOCK_H_

#include <boost/noncopyable.hpp>


namespace ppl { namespace concurrent { 


/// 局部的自动锁
template <typename LockT>
class scoped_lock : private boost::noncopyable
{
public:
	explicit scoped_lock(LockT& lock) : m_lock(lock)
	{
		m_lock.lock();
	}
	~scoped_lock()
	{
		m_lock.unlock();
	}
private:
	/// 同步对象
	LockT& m_lock;
};


template <typename ReadWriteLockT>
class scoped_read_lock : private boost::noncopyable
{
public:
	explicit scoped_read_lock(ReadWriteLockT& lock) : m_lock(lock) { m_lock.lock_read(); }
	~scoped_read_lock() { m_lock.unlock_read(); }

private:
	ReadWriteLockT& m_lock;
};

template <typename ReadWriteLockT>
class scoped_write_lock : private boost::noncopyable
{
public:
	explicit scoped_write_lock(ReadWriteLockT& lock) : m_lock(lock) { m_lock.lock_write(); }
	~scoped_write_lock() { m_lock.unlock_write(); }

private:
	ReadWriteLockT& m_lock;
};


} }

#endif
