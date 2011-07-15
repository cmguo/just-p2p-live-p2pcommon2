
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_DATA_MEMORY_POOL_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_DATA_MENORY_POOL_H_

#include <ppl/data/pool_status.h>
#include <ppl/concurrent/lightweight_mutex.h>
#include <ppl/util/macro.h>
#include <ppl/data/intrusive/slist.h>
#include <ppl/diag/trace.h>
#include <ppl/io/data_input_stream.h>
#include <ppl/io/data_output_stream.h>

#include <boost/noncopyable.hpp>
#include <boost/static_assert.hpp>

#include <set>

namespace ppl { namespace data { 

using ppl::concurrent::lightweight_mutex;
typedef lightweight_mutex::scoped_lock AutoLightMutex;

inline void dbgCheckHeap()
{
	//assert( _heapchk() == _HEAPOK );
}




class CoreMemoryPoolInfo : private boost::noncopyable
{
public:
	CoreMemoryPoolInfo()
	{
	}

	void RecordAllocate(UINT bytes, bool reallyAllocated, bool largeAllocated)
	{
		AutoLightMutex lock(m_lock);
		this->Check();
		//TRACE("mempool stats RecordAllocate 1 TotalCount=%d UsedCount=%d SystemAlloc=%d\n", m_Info.TotalCount, m_Info.UsedCount, m_Info.AllocateSystemCount);
		m_status.AllocatePoolCount++;
		if ( largeAllocated )
		{
			m_status.LargeAllocCount++;
			this->IncTotal(bytes);
		}
		else
		{
			if ( reallyAllocated )
			{
				m_status.AllocateSystemCount++;
				this->IncTotal(bytes);
			}
		}
		this->IncUsed(bytes);
		//TRACE("mempool stats RecordAllocate 2 TotalCount=%d UsedCount=%d SystemAlloc=%d\n", m_Info.TotalCount, m_Info.UsedCount, m_Info.AllocateSystemCount);
		this->Check();
	}

	void RecordDeallocate(UINT bytes, bool reallyDeallocated, bool largeDeallocated)
	{
		AutoLightMutex lock(m_lock);
		this->Check();
		//TRACE("mempool stats RecordDeallocate 1 TotalCount=%d UsedCount=%d SystemAlloc=%d\n", m_Info.TotalCount, m_Info.UsedCount, m_Info.AllocateSystemCount);
		this->DecUsed(bytes);
		if ( largeDeallocated )
		{
			this->DecTotal( bytes );
			m_status.LargeFreeCount++;
		}
		else
		{
			if ( reallyDeallocated )
			{
				this->DecTotal( bytes );
				m_status.FreeSystemCount++;
			}
		}
		//TRACE("mempool stats RecordDeallocate 2 TotalCount=%d UsedCount=%d SystemAlloc=%d\n", m_Info.TotalCount, m_Info.UsedCount, m_Info.AllocateSystemCount);
		this->Check2();
	}

	void GetStatus(ppl::data::pool_status& target) const
	{
		target.TotalBytes = m_status.TotalBytes;
		target.UsedBytes = m_status.UsedBytes;
		target.PeakUsedBytes = m_status.PeakUsedBytes;
		target.PeakTotalBytes = m_status.PeakTotalBytes;
		target.AllocatePoolCount = m_status.AllocatePoolCount;
		target.AllocateSystemCount = m_status.AllocateSystemCount;
		target.LargeAllocCount = m_status.LargeAllocCount;
		target.LargeFreeCount = m_status.LargeFreeCount;
		target.FreeSystemCount = m_status.FreeSystemCount;

		target.UsedCount = m_status.UsedCount;
		target.PeakUsedCount = m_status.PeakUsedCount;
		target.TotalCount = m_status.TotalCount;
		target.PeakTotalCount = m_status.PeakTotalCount;
	}

protected:
	void IncTotal(UINT32 bytes)
	{
		m_status.TotalCount++;
		LIMIT_MIN( m_status.PeakTotalCount, m_status.TotalCount );

		m_status.TotalBytes += bytes;
		LIMIT_MIN( m_status.PeakTotalBytes, m_status.TotalBytes );

	}
	void DecTotal( UINT32 bytes )
	{
		assert(m_status.TotalCount > 0);
		m_status.TotalCount--;

		assert(m_status.TotalBytes > bytes);
		m_status.TotalBytes -= bytes;

	}

	void IncUsed(UINT32 bytes)
	{
		m_status.UsedCount++;
		LIMIT_MIN( m_status.PeakUsedCount, m_status.UsedCount );

		m_status.UsedBytes += bytes;
		LIMIT_MIN(m_status.PeakUsedBytes, m_status.UsedBytes);

	}
	void DecUsed(UINT32 bytes)
	{
		assert(m_status.UsedCount > 0);
		m_status.UsedCount--;

		assert(m_status.UsedBytes >= bytes);
		m_status.UsedBytes -= bytes;
		//if ( isFreed )								全局分配就由全局减少
		//{
		//	assert( m_Info.TotalBytes >= bytes );
		//	m_Info.TotalBytes -= bytes;
		//}
	}

	void Check()
	{
		//assert(m_Info.UsedCount >= 0);
		//assert(m_Info.TotalCount >= 0);

		//assert( m_Info.TotalCount >= m_Info.UsedCount );
		//assert( m_Info.TotalBytes >= m_Info.UsedBytes );

		//assert( m_Info.PeakUsedCount >= m_Info.UsedCount );
		//assert( m_Info.PeakUsedBytes >= m_Info.UsedBytes );

		//assert( m_Info.PeakTotalBytes >= m_Info.TotalBytes );
		//assert( m_Info.PeakTotalCount >= m_Info.TotalCount );

		//assert( m_Info.AllocateSystemCount + m_Info.LargeAllocCount >= m_Info.PeakUsedCount );
		//assert( m_Info.AllocateSystemCount + m_Info.LargeAllocCount >= m_Info.UsedCount );
	}

	void Check2()
	{
		//assert(m_Info.UsedCount > 0);
		//assert(m_Info.TotalCount > 0);

		//assert( m_Info.TotalCount > m_Info.UsedCount );
		//assert( m_Info.TotalBytes > m_Info.UsedBytes );

		//assert( m_Info.PeakUsedCount >= m_Info.UsedCount );
		//assert( m_Info.PeakUsedBytes >= m_Info.UsedBytes );

		//assert( m_Info.PeakTotalBytes >= m_Info.TotalBytes );
		//assert( m_Info.PeakTotalCount >= m_Info.TotalCount );

		//assert( m_Info.AllocateSystemCount + m_Info.LargeAllocCount >= m_Info.PeakUsedCount );
		//assert( m_Info.AllocateSystemCount + m_Info.LargeAllocCount > m_Info.UsedCount );
	}

private:
	ppl::data::pool_status m_status;
	lightweight_mutex m_lock;
};




#ifdef _PPL_PLATFORM_MSWIN
#pragma message("------ use pooled malloc and free")
#endif

//buf结构定义

#define MEM_TAG			0x12345678

const UINT32 MEM_ALLOCATED_TAG	= 0xA10CACDA;
const UINT32 MEM_IN_USE_TAG		= 0xDEADACDA;
const UINT32 MEM_AVAILABLE_TAG	= 0x900DACDA;

#define MEM_CLEAR_TAG	0x9ABCDEF0
//#define POOL_SIZE (128+1024+1024)
// 实际好像应该为128 + 8 + 254，不过可以稍微多留一些
const size_t FIRST_LEVEL_MAX_SIZE		= 2 * 1024;
const size_t SECOND_LEVEL_MAX_SIZE		= 20 * 1024;

const size_t SECOND_LEVEL_BLOCK_SIZE	= 1024;

const size_t POOL_SMALL_COUNT = 6;
const size_t POOL_MEDIUM_COUNT = 18;

BOOST_STATIC_ASSERT( ( SECOND_LEVEL_MAX_SIZE - FIRST_LEVEL_MAX_SIZE ) % SECOND_LEVEL_BLOCK_SIZE == 0 );
BOOST_STATIC_ASSERT( POOL_MEDIUM_COUNT == ( SECOND_LEVEL_MAX_SIZE - FIRST_LEVEL_MAX_SIZE ) / SECOND_LEVEL_BLOCK_SIZE );

const size_t FIRST_LEVEL_MAX_POOL		= 1000;
const size_t SECOND_LEVEL_MAX_POOL		= 100;

const size_t POOL_SIZE = POOL_SMALL_COUNT + POOL_MEDIUM_COUNT;




//#define GetBlockSize(blockSize,size,BLOCK_SIZE) { blockSize = ((size-1)/BLOCK_SIZE+1)*BLOCK_SIZE; }




BOOST_STATIC_ASSERT( POOL_SMALL_COUNT == 6 );
BOOST_STATIC_ASSERT( POOL_MEDIUM_COUNT == 18 );
BOOST_STATIC_ASSERT( POOL_SIZE == POOL_SMALL_COUNT + POOL_MEDIUM_COUNT );
BOOST_STATIC_ASSERT( POOL_SIZE == 6 + 18 );
BOOST_STATIC_ASSERT( POOL_SIZE == 24 );

//CriticalSection mem_APB_Lock;

//static CoreMemoryPoolInfo s_MemoryPoolInfo;

//#pragma warning(disable : 4200)

const size_t POOL_SMALL_START_SIZE = 64;



inline bool GetPosBlockSize(int size, int &pos, int &blockSize)
{
	assert( size > 0 );
	if ((size_t) size > SECOND_LEVEL_MAX_SIZE )
	{
		assert( false );
		return false;
	}
	if ( (size_t)size > FIRST_LEVEL_MAX_SIZE )
	{
		assert((size_t) size >= FIRST_LEVEL_MAX_SIZE + 1 );
		pos = ( size - 1 - FIRST_LEVEL_MAX_SIZE ) / SECOND_LEVEL_BLOCK_SIZE;
		assert( pos >= 0 &&(size_t) pos < POOL_MEDIUM_COUNT );
		blockSize = (pos + 1) * SECOND_LEVEL_BLOCK_SIZE + FIRST_LEVEL_MAX_SIZE;
		pos += POOL_SMALL_COUNT;
		assert( (size_t)pos >= POOL_SMALL_COUNT &&(size_t)pos < POOL_SMALL_COUNT + POOL_MEDIUM_COUNT );	// 64-70
		assert(blockSize>=size &&(size_t) blockSize > FIRST_LEVEL_MAX_SIZE && (size_t)blockSize <= SECOND_LEVEL_MAX_SIZE );
		return true;
	}
	int smallSize = POOL_SMALL_START_SIZE;
	for ( int smallIndex = 0; (size_t)smallIndex < POOL_SMALL_COUNT; ++smallIndex, smallSize *= 2 )
	{
		if ( size <= smallSize )
		{
			pos = smallIndex;
			blockSize = smallSize;
			return true;
		}
	}
	assert( false );
	return false;
}


/*
int GetSizeFromPos(int pos)
{
assert( pos >= 0 );
if (pos<POOL_SMALL_COUNT)
{
return (pos)*FIRST_LEVEL_BLOCK_SIZE;
}
else if (pos<POOL_SMALL_COUNT + POOL_MEDIUM_COUNT)
{
return (pos-POOL_SMALL_COUNT)*SECOND_LEVEL_BLOCK_SIZE;
}
else if (pos<POOL_SMALL_COUNT + POOL_MEDIUM_COUNT + THIRD_LEVEL_COUNT)
{
return (pos-POOL_SMALL_COUNT - POOL_MEDIUM_COUNT)*THIRD_LEVEL_BLOCK_SIZE;
}
else
{
assert(0);
//位置>388是错误的
return 0;
}
}*/


//#define DebugMemory(msg,size,BlockSize)
//
#if 0
#pragma message("------ trace memory pool for release version")
inline void DebugMemory( const char * msg, const size_t size, const size_t BlockSizeInPool )
{
	//ostringstream oss;
	//oss << msg << size << "\t BlockSizeInPool: " << BlockSizeInPool<<". \t" ;
	//oss << "Total Memory: " << s_MemoryPoolInfo.TotalBytes << "  Used Memory: " << s_MemoryPoolInfo.UsedBytes;
	//oss << "  Pool Alloc: " << s_MemoryPoolInfo.AllocatePoolCount << " System Alloc: " << s_MemoryPoolInfo.AllocateSystemCount;
	//OutputDebugStringA( oss.str().c_str() );
}
#else
inline void DebugMemory( const char * msg, const size_t size, const size_t BlockSizeInPool )
{}
#endif


struct block_header : public slist_node_base
{
	/// 块标志
	UINT32 tag;
};


class block_pool : private boost::noncopyable
{
public:
	typedef sslist<block_header> block_list;

	block_pool() : _used_count( 0 ), _block_size( 0 ), _max_free_count( 0 )
	{
	}

	void init( size_t block_size, size_t max_free_count )
	{
		//AutoLightMutex lock(_lock);

		assert( 0 == this->_block_size );
		assert( 0 == this->_max_free_count );
		this->_block_size = block_size;
		this->_max_free_count = max_free_count;
	}

	void* allocate( size_t size, bool& reallyAllocated )
	{
		AutoLightMutex lock(_lock);
		//ScopedTracer _tracer(strings::format("mempool scope block_alloc.allocate %d %s %d %d", size, filename, line, this->_blocks.size()));

		assert( this->_block_size > 0 );
		assert( size == this->_block_size );

		block_header* p = NULL;
		if ( _blocks.empty() )
		{
			size_t realSize = sizeof( block_header) + size;
			p = static_cast<block_header*>( malloc( realSize ) );	// 没有初始化 MEMORYBLOCK 信息
			p->tag = MEM_ALLOCATED_TAG;	// first time allocated
			p->next = NULL;				// 最后一个节点一定不要指到别的地方
			reallyAllocated = true;
			//TRACE("mempool malloc 0x%p %s %d\n", p, filename, line);
		}
		else
		{
			p = this->_blocks.pop();
			assert( MEM_AVAILABLE_TAG == p->tag );
			p->tag = MEM_IN_USE_TAG;	// available to in use
			reallyAllocated = false;
		}
		this->_used_count++;
		this->check();
		//TRACE("mempool block.allocate 0x%p %s %d %d %d\n", p + 1, filename, line, this->_block_size, reallyAllocated);
		return p + 1;
	}

	bool deallocate( void* p )
	{
		AutoLightMutex lock(_lock);
		//ScopedTracer _tracer(strings::format("mempool scope block_alloc.deallocate 0x%p %d %s %d %d", p, this->_block_size, filename, line, this->_blocks.size()));

		assert( p );
		block_header* header = static_cast<block_header*>( p );
		header--;
		assert( MEM_IN_USE_TAG == header->tag || MEM_ALLOCATED_TAG == header->tag );

		//TRACE("mempool block.deallocate 0x%p %s %d %d\n", p, filename, line, this->_block_size);

		bool reallyFreed = false;
		//if ( this->FreeCount > this->UsedCount )	// 对于大块内存，比例可以更低一点，甚至为0。对于小块内存，预留少量空闲内存即可
		if ((size_t) _blocks.size() >(size_t) this->_max_free_count )
		{
			// 真正释放
			free( header );
			reallyFreed = true;
		}
		else
		{
			// 加入到空闲队列
			header->tag = MEM_AVAILABLE_TAG;	// 标记可用
			assert( _blocks.empty() || FALSE == ::IsBadReadPtr( _blocks.top(), sizeof( block_header ) ) );
			//assert( _blocks.empty() || _CrtIsValidHeapPointer( _blocks.top() ) );
			_blocks.push(header);
			assert( false == _blocks.empty() && FALSE == ::IsBadReadPtr( _blocks.top(), sizeof( block_header ) ) );
			//assert( false == _blocks.empty() && _CrtIsValidHeapPointer( _blocks.top() ) );
		}
		assert( this->_used_count > 0 );
		this->_used_count--;
		this->check();
		return reallyFreed;
	}

	void clear()
	{
		AutoLightMutex lock(_lock);

		this->_used_count = 0;
		this->_block_size = 0;
		while ( false == _blocks.empty() )
		{
			block_header* p = _blocks.pop();
			assert( MEM_AVAILABLE_TAG == p->tag );
			assert( FALSE == ::IsBadReadPtr( p, sizeof( block_header ) ) );
			//assert( _CrtIsValidHeapPointer( p ) );
			free( p );
		}
		assert( _blocks.empty() );
		this->_blocks.clear();
	}
	void check() const
	{
#if defined(_DEBUG) && 0
		STL_FOR_EACH_CONST(block_list, _blocks, iter)
		{
			const block_header* p = &(*iter);
			assert( MEM_IN_USE_TAG == p->tag || MEM_ALLOCATED_TAG == p->tag || MEM_AVAILABLE_TAG == p->tag );
			assert( FALSE == ::IsBadReadPtr( p, sizeof( block_header ) ) );
			//assert( _CrtIsValidHeapPointer( p ) );
		}
#endif
	}

private:
	//block_header* _header;
	block_list _blocks;
	int _used_count;
	size_t _block_size;
	size_t _max_free_count;
	lightweight_mutex _lock;
};






template <typename BlockAllocT>
class memory_pool_impl : private boost::noncopyable
{
public:
	struct pool_block_header
	{
		UINT32 tag;
		int pos;
		int alloc_size;
		int data_size;
	};

	memory_pool_impl()
	{
		size_t blockSize = POOL_SMALL_START_SIZE;
		for ( size_t smallIndex = 0; smallIndex < POOL_SMALL_COUNT; ++smallIndex )
		{
			m_allocs[smallIndex].init(blockSize, FIRST_LEVEL_MAX_POOL);
			blockSize *= 2;
		}
		for ( size_t mediumIndex = 0; mediumIndex < POOL_MEDIUM_COUNT; ++mediumIndex )
		{
			m_allocs[ POOL_SMALL_COUNT + mediumIndex ].init( (mediumIndex+1) * 1024 + FIRST_LEVEL_MAX_SIZE, SECOND_LEVEL_MAX_POOL );
		}
	}

	enum { MAX_BLOCK_ALLOC_SIZE = SECOND_LEVEL_MAX_SIZE };

#ifdef _DEBUG
	void* allocate(int size, const char* filename, int line)
#else
	void* allocate(int size)
#endif
	{
		dbgCheckHeap();

		if ( size < 0 )
		{
			assert( !"negative size for pool allocate" );
			return NULL;
		}

		if ( size <= 0 )
		{
			size = 1;
		}
		size_t realSize = size + sizeof( pool_block_header );

		int pos;
		int blockSize;
		void* p;
		bool reallyAllocated = false;
		bool largeAllocated = false;

		if (realSize>MAX_BLOCK_ALLOC_SIZE)
		{
			pos = -1;
			blockSize = realSize;
			p = malloc( realSize );
			reallyAllocated = true;
			largeAllocated = true;
			// 			s_MemoryPoolInfo.LargeAllocCount++;
			// 			s_MemoryPoolInfo.IncTotal(blockSize);
			//TRACE("malloc large block %d %s %d\n", realSize, filename, line);
		}
		else
		{
			GetPosBlockSize(realSize, pos, blockSize);
			assert( pos >= 0 && (size_t)pos < POOL_SIZE );
			//判断是否有空闲的内存块可用，如果没有分配之
			//AutoLightMutex lock(m_lock);
#ifdef _DEBUG
			p = m_allocs[pos].allocate( blockSize, reallyAllocated, filename, line );
#else
			p = m_allocs[pos].allocate( blockSize, reallyAllocated );
#endif
			// 			if ( reallyAllocated )
			// 			{
			// 				s_MemoryPoolInfo.AllocateSystemCount++;
			// 				s_MemoryPoolInfo.IncTotal(blockSize);
			// 			}
		}

		assert( p );
		pool_block_header* header = static_cast<pool_block_header*>( p );
		header->pos = pos;
		header->tag = MEM_TAG;
		header->alloc_size = blockSize;
		header->data_size = size;
		//s_MemoryPoolInfo.IncUsed(blockSize);
		//s_MemoryPoolInfo.SyncTo(s_MemorySyncTime, g_Statistics->MemoryPool);
		//s_MemoryPoolInfo.AllocatePoolCount++;
		s_MemoryPoolInfo.RecordAllocate(blockSize, reallyAllocated, largeAllocated);

		DebugMemory( "malloc ", size, blockSize );
		dbgCheckHeap();

		return header + 1;
	}


	//////////////////////////////////////////////////////////////////////////
	//功能：释放一个指定的内存块的发行版本
	//
#ifdef _DEBUG
	bool deallocate(void * lpbuf, const char* filename, int line)
#else
	bool deallocate(void * lpbuf)
#endif
	{
		dbgCheckHeap();

		assert( lpbuf != NULL );
		pool_block_header* header = static_cast<pool_block_header*>( lpbuf );
		header--;

		bool reallyFreed = false;
		bool success = true;
		//__try
		//{
			if ( MEM_TAG == header->tag ) //检查是否是真实的内存块
			{
				header->tag=MEM_CLEAR_TAG; //清除标记

				int pos = header->pos;
				size_t blockSize = header->alloc_size;
				bool largeFree = false;
				if ( -1 == pos )
				{
					assert( blockSize >= MAX_BLOCK_ALLOC_SIZE - 1000 );
					free( header );
					reallyFreed = true;
					largeFree = true;
					// 					s_MemoryPoolInfo.DecUsed(blockSize);
					// 					s_MemoryPoolInfo.DecTotal( blockSize );
					// 					s_MemoryPoolInfo.LargeFreeCount++;
				}
				else if ( pos >= 0 && (size_t)pos < POOL_SIZE )
				{
#ifdef _DEBUG
					reallyFreed = block_deallocate( pos, header, filename, line );
#else
					reallyFreed = block_deallocate( pos, header );
#endif
					//					s_MemoryPoolInfo.DecUsed(blockSize);
					// 					if (reallyFreed)
					// 					{
					// 						s_MemoryPoolInfo.DecTotal( blockSize );
					// 						s_MemoryPoolInfo.FreeSystemCount++;
					// 					}
				}
				else
				{
					TRACEOUT("pool_free exception: pos %d\n", pos);
					assert(false);
				}
				//				s_MemoryPoolInfo.SyncTo(s_MemorySyncTime, g_Statistics->MemoryPool);
				s_MemoryPoolInfo.RecordDeallocate(blockSize, reallyFreed, largeFree);

				DebugMemory( "free: ", blockSize, 0 );
			}
			else
			{
				//如果不是真实的内存块，是不能加入空闲列表的
				assert(0);
			}
		//}
		//__except ( MemExcepFilter(lpbuf))
		//{
		//	//由于m_allocs[pos]的改变是在最后做的，而错误是在其之前可能发生，所以
		//	//m_allocs[pos]的值无需保存和恢复，所以这里什么都不作也是可以的
		//	success = false;
		//	TRACEOUT("pool_free exception: 0x%p\n", lpbuf);
		//	assert(false);
		//}
		assert( success );
		dbgCheckHeap();

		return reallyFreed;
	}


	//////////////////////////////////////////////////////////////////////////
	//功能：释放内存池中所分配的内存空间
	//

	void clear()
	{
		m_lock.lock();
		//LPMEMORYBLOCK lpBlock;
		for(int i=0;i<POOL_SIZE;++i)
		{
			//__try
			{
				m_allocs[i].clear();
			}
// 			__except (EXCEPTION_EXECUTE_HANDLER)
// 			{
// 				TRACEOUT("pool_clear exception: %d\n", i);
// 				assert(false);
// 			}
		}
		m_lock.unlock();
	}

protected:
#ifdef _DEBUG
	bool block_deallocate(int pos, void* p, const char* filename = NULL, int line = 0)
#else
	bool block_deallocate(int pos, void* p)
#endif
	{
		//AutoLightMutex lock(m_lock);
#ifdef _DEBUG
		return m_allocs[pos].deallocate(p, filename, line);
#else
		return m_allocs[pos].deallocate(p);
#endif
	}

	static DWORD MemExcepFilter(void * lpbuf)
	{
		//pool_free触发了异常，能作什么处理呢？
		::OutputDebugStringA("error occurred when free memory\n");
		return 1;//EXCEPTION_EXECUTE_HANDLER;
	}

private:
	BlockAllocT m_allocs[POOL_SIZE];
	lightweight_mutex m_lock;
	CoreMemoryPoolInfo s_MemoryPoolInfo;
};

//const unsigned long MEM_TAG = 0X12345678;
//const unsigned long MEM_CLEAR_TAG = 0X9ABCDEF0;

const unsigned long MEM_GUARD = 0xFAFAFAFA;
const unsigned char MEM_UNINITIALIZED = 0xFB;
const unsigned char MEM_FREED = 0xF9;


template <typename AllocT>
class debug_pool : private boost::noncopyable
{
public:
	struct debug_header
	{
		size_t size;
		unsigned long tag;
		unsigned long alloc_time;
		const char* filename;
		int line;
		unsigned long front_guard;
	};
	typedef std::set<debug_header*> debug_header_collection;

	debug_pool()
	{

	}
	~debug_pool()
	{
		this->clear();
	}

	void init( size_t blockSize, size_t max_free_count )
	{
		m_impl.init( blockSize + sizeof(debug_header) + sizeof(unsigned long), max_free_count );
	}

	void* allocate(size_t size, bool& reallyAllocated, const char* filename, int line)
	{
		AutoLightMutex lock(m_lock);
		//ScopedTracer _tracer(strings::format("mempool scope debug_alloc.allocate %d %s %d", size, filename, line));
		assert( size > 0 );
		void* p = m_impl.allocate( sizeof(debug_header) + size + sizeof(unsigned long), reallyAllocated );
		assert( p );
		debug_header* header = static_cast<debug_header*>( p );
		header->size = size;
		header->alloc_time = ppl::util::detail::GetTickCount();
		header->filename = filename;
		header->line = line;
		assert(header->tag != MEM_TAG);
		header->tag = MEM_TAG;
		header->front_guard = MEM_GUARD;
		char* buf = reinterpret_cast<char*>(header + 1);
		memset(buf, MEM_UNINITIALIZED, size);
		WRITE_MEMORY(buf + size, MEM_GUARD, unsigned long);
		{
			//TRACE("mempool m_UsedBlocks.insert 0x%p %s %d %d\n", p, header->filename, header->line, header->size);
			bool success = m_UsedBlocks.insert( header ).second;
			if ( false == success )
			{
				//TRACE("mempool invalid used block 0x%p\n", p);
			}
			assert(success);
		}
		return buf;
	}
	bool deallocate(void* p, const char* filename, int line)
	{
		AutoLightMutex lock(m_lock);
		//ScopedTracer _tracer(strings::format("mempool scope debug_alloc.deallocate 0x%p %d %s %d", p, 0, filename, line));
		assert( p != NULL );
		debug_header* header = static_cast<debug_header*>(p);
		--header;
		if (header->tag != MEM_TAG)
		{
//			CORE_EVENT("debug_alloc: deallcate error " << p);
			TRACEOUT("debug_alloc: deallcate error %s" , p);
			assert(false);
			return false;
		}
		assert(header->tag == MEM_TAG);
		assert(header->front_guard == MEM_GUARD);
		const char* buf = reinterpret_cast<const char*>(p);
		assert(READ_MEMORY(buf + header->size, unsigned long) == MEM_GUARD);
		header->tag = MEM_CLEAR_TAG;
		memset(p, MEM_FREED, header->size);
		bool res = m_impl.deallocate(header);
		{
			//TRACE("mempool m_UsedBlocks.erase 0x%p\n", header);
			size_t count = m_UsedBlocks.erase( header );
			assert( 1 == count );
		}
		return res;
	}
	void clear()
	{
		AutoLightMutex lock(m_lock);
		m_impl.clear();

		if ( m_UsedBlocks.empty() )
			return;
		LOGCORE_ERROR( "memory leak blocks " << m_UsedBlocks.size() );
		STL_FOR_EACH_CONST(typename debug_header_collection, m_UsedBlocks, iter )
		{
			debug_header* header = *iter;
			assert(header->tag == MEM_TAG);
			assert(header->front_guard == MEM_GUARD);
			const char* buf = reinterpret_cast<const char*>(header + 1);
			assert(READ_MEMORY(buf + header->size, unsigned long) == MEM_GUARD);

			TRACEOUT("memory leak: size=%u,AllocTime=%u,line=%d,file=", header->size, header->alloc_time, header->line);
			::OutputDebugString(header->filename);
			::OutputDebugString("\n");
		}
		m_UsedBlocks.clear();
	}

	void print_status()
	{
		//m_impl.print_status();
	}

private:
	AllocT m_impl;
	debug_header_collection m_UsedBlocks;
	lightweight_mutex m_lock;
};

typedef debug_pool<block_pool> debug_block_pool;


#if defined(_DEBUG)
//#if 0

typedef memory_pool_impl<debug_block_pool> memory_pool;

#else

typedef memory_pool_impl<block_pool> memory_pool;

#endif


} }


#endif
