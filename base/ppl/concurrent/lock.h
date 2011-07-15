
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_CONCURRENT_LOCK_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_CONCURRENT_LOCK_H_

#include <ppl/concurrent/lightweight_mutex.h>
#include <boost/noncopyable.hpp>


namespace ppl { namespace concurrent { 


class read_write_lock : private boost::noncopyable
{
public:
	read_write_lock() : _readcount( 0 )
	{
	}
	void lock_read()
	{
		lightweight_mutex::scoped_lock writeLock(_writer);
		lightweight_mutex::scoped_lock mutexLock(_mutex);
		if( ++_readcount == 1 )
			_reader.lock();
	}
	void unlock_read()
	{
		lightweight_mutex::scoped_lock lock(_mutex);
		if( --_readcount == 0 )
			_reader.unlock();
	}
	void lock_write()
	{
		_writer.lock();
		_reader.lock();
	}
	void unlock_write()
	{
		_reader.unlock();
		_writer.unlock();
	}	
private:
	lightweight_mutex _reader;
	lightweight_mutex _writer;
	lightweight_mutex _mutex;
	int _readcount;
};


} }

#endif
