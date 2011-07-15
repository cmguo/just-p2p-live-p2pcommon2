
#ifndef _LIVE_P2PCOMMON2_NEW_CORE_STREAM_BUFFER_FACTORY_H_
#define _LIVE_P2PCOMMON2_NEW_CORE_STREAM_BUFFER_FACTORY_H_

#include "common/StreamBuffer.h"
#include "common/MediaStorage.h"
#include "common/StreamIndicator.h"
#include "common/StreamBufferStatistics.h"

#include <synacast/protocol/DataSigner.h>
#include <boost/shared_ptr.hpp>


class CMediaServer;
class SourceResource;

struct PEER_MINMAX;
struct PEER_STATUS;


/// streambuffer的实现类
class StreamBufferImpl : public CStreamBuffer, public pool_object
{
public:
	explicit StreamBufferImpl(UINT minBufferTime = 70 * 1000);
	virtual ~StreamBufferImpl();

	virtual Storage& GetStorage()
	{
		return m_storage;
	}

	virtual void SetConfig(const StreamBufferConfig& config) { m_config = config; }

	///启动StreamBuffer
	bool Start();

	///停止StreamBuffer
	bool Stop();

	///复位Buffer的状态
	bool Reset(UINT index);


	/// 获得一段范围之间的Piece个数
	size_t GetRangeCount( UINT rangeMinIndex, UINT rangeMaxIndex ) const;

	virtual const StreamBufferStatistics& GetStatistics() const { return m_statistics; }
	//virtual pair<bool, PPMediaDataPacketPtr> AddSubPiece(SubPieceDataPacketPtr subPiece) { return make_pair(false, PPMediaDataPacketPtr()); }

	virtual void SetPeerInformation( boost::shared_ptr<const PeerInformation> info );

	virtual const StreamIndicator& GetBaseIndicator() const
	{
		return m_base;
	}

	/// 增加一个header片
	virtual bool AddHeaderPiece( MediaHeaderPiecePtr piece );

	/// 增加一个数据片
	virtual bool AddDataPiece( MediaDataPiecePtr piece ) = 0;


public:

	virtual bool NeedHeader(UINT index) const { return !m_storage.HasHeader(index); }

	/// 获取下载的起始位置
	UINT GetDownloadStartIndex() const { return 0; }

	///获得当前缓冲区最小的已下载DataPacket编号
	//UINT GetMinIndex() const { return m_storage.GetMinIndex(); }

	///获得当前缓冲区最大的已下载DataPacket编号
	//UINT GetMaxIndex() const { return m_storage.GetMaxIndex(); }


	UINT GetLocationIndex() const { return 0; }

	/// 获取缓冲时间的估计值－－BufferTime * 缓冲率
	UINT32 GetApproximateBufferTime() const;

	//// 生成完整的资源位图
	BitMap BuildTotalBitmap() const;
	
	virtual void SetPrelocationIndex( UINT PrelocationIndex ) {}
	
	virtual UINT GetPrelocationIndex() const { return 0; }

	virtual void SetMediaServer(CMediaServer* mediaServer) { m_lpMediaServer = mediaServer; }

	virtual void GenerateKey(const GUID& seed);
	// Added by Tady, 090308: 计算当前的下载点与推进点之间的数据长度
	virtual UINT GetPrepaDataLen();
	virtual int GetPrepaDataTime();

	virtual DataSignerPtr GetSigner()
	{
		return m_signer;
	}

protected:
	/// 增加data片
	bool DoAddDataPiece( MediaDataPiecePtr piece );

protected:
	virtual void OnAppTimer(UINT times, UINT downSpeed);

	/// 收缩缓冲区
	virtual void Shrink() = 0;

	/// 推进缓冲区
	virtual void Push() = 0;

	/// 更新统计数据
	virtual void UpdateStatistics(UINT downSpeed) = 0;

	/// 清空
	virtual void Clear();

	void UpdateBase();

	/// 更新基准点之后调用
	void OnUpdateBase();

	/// 实现更新基准点功能
	virtual void DoUpdateBase(const PieceInfo& piece);

	/// 收到第一片，启动定时器开始推进
	void OnFirstPiece(MediaDataPiecePtr lpPacket);

protected:
	/// 统计数据
	StreamBufferStatistics m_statistics;
	StreamBufferConfig m_config;
	boost::shared_ptr<const PeerInformation> m_PeerInformation;
	boost::shared_ptr<const SourceResource> m_SourceResource;

	/// streambuffer的运行状态
	enum 
	{
		///未启动的状态，除Start外	
		st_none		= 0,
		///启动后，但是未下载到任何Piece的状态
		st_waiting	= 1,
		///启动了SkipTimer后的状态
		st_running	= 2
	} m_State;

	///指向MediaServer,要负责其创建和更新
	CMediaServer* m_lpMediaServer;

	/// piece集合
	Storage m_storage;

	/// 流的基准位置
	StreamIndicator m_base;

	/// 最小保留的缓冲区大小(MinIndex到SkipIndex的时间差)
	UINT m_minBufferTime;

	time_counter m_StartTime;

	DataSignerPtr m_signer;
};





/// 用于source的StreamBuffer
class SourceStreamBuffer : public StreamBufferImpl
{
public:
	explicit SourceStreamBuffer();

	virtual UINT GetSkipIndex() const { return m_storage.GetMaxIndex(); }

	virtual UINT64 GetSkipTimestamp() const;

	virtual bool AddDataPiece( MediaDataPiecePtr piece );

protected:
	virtual void Push();
	virtual void Shrink();
	virtual void UpdateStatistics(UINT downSpeed);

	virtual void Clear();

	bool NeedDownload(UINT index) const { return false; }

private:
	/// 总的片数
	size_t m_totalDownloadedPieceCount;
};




/// peer端的StreamBuffer
class PeerStreamBuffer : public StreamBufferImpl
{
public:
	explicit PeerStreamBuffer();

	virtual bool AddDataPiece( MediaDataPiecePtr piece );

	virtual UINT GetSkipIndex() const { return m_skipIndex; }
	
	virtual UINT64 GetSkipTimestamp() const;

	UINT GetDownloadStartIndex() const { return max(m_skipIndex + 1,m_PrelocationIndex); }

	virtual bool Reset(UINT index);

	virtual void SetDownloader(Downloader* downloader)
	{
		assert(downloader != NULL);
		m_downloader = downloader;
	}
	virtual BitMap BuildTotalBitmap() const;

	virtual DataSignerPtr GetSigner();

// 	virtual pair<bool, PPMediaDataPacketPtr> AddSubPiece(SubPieceDataPacketPtr subPiece)
// 	{
// 		return m_storage.GetUnfinished().AddSubPiece(subPiece);
// 	}

protected:
	virtual void Clear();

	virtual void Push();			// 推进
	virtual void Shrink();			// 删除过期的数据
	virtual void UpdateStatistics(UINT downSpeed);
	virtual bool NeedDownload(UINT index) const;

	virtual void DoUpdateBase(const PieceInfo& piece);

	/// 检查piece是否可以用来进行reset操作
	bool IsValidPieceForReset(MediaDataPiecePtr piece) const;

	/// 是否需要重置缓冲区
	bool NeedResetBuffer(MediaDataPiecePtr piece) const;

	/// 推进skipIndex
	size_t PushStream(UINT& skipIndex);

	/// 推进playIndex
	//void PushPlay();

	/// 连续的推进
	///size_t PushContinuously(UINT& skipIndex);

	/// 按时间的推进
	///size_t PushTimely(const StreamIndicator& base, UINT& skipIndex);


	virtual void SetPrelocationIndex( UINT PrelocationIndex );

	virtual UINT GetPrelocationIndex() const { return m_PrelocationIndex; }

private:
	/// 推进位置
	UINT m_skipIndex;
	
	/// 播放位置
	UINT m_playIndex;

	/// 预定位的位置,如果发生预定位,则预定位之前的数据将视为不可能在下载的数据
	UINT m_PrelocationIndex;

	/// 统计信息收集
	StreamFeedback m_feedback;

	/// 下载管理模块
	Downloader* m_downloader;

};

#endif
