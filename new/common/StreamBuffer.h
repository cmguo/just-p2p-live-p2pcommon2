/********************************************************************
Copyright(c) 2004-2005 PPLive.com All rights reserved.                                                 
	filename: 	StreamBuffer.h
	created:	2005-4-21   19:40
	author:		
	purpose:	
*********************************************************************/

#ifndef _LIVE_P2PCOMMON2_NEW_COMMON_STREAMBUFFER_H_
#define _LIVE_P2PCOMMON2_NEW_COMMON_STREAMBUFFER_H_


struct PEER_MINMAX;
class BitMap;
class StreamBufferStatistics;
class StreamBufferConfig;
class Downloader;
class Storage;
class CMediaServer;

class StreamIndicator;
class PeerInformation;


#include "piecefwd.h"


class DataSigner;
typedef boost::shared_ptr<DataSigner> DataSignerPtr;


/// 负责数据缓冲的处理(接口类)
class CStreamBuffer
{
public:
	CStreamBuffer() { }

	virtual ~CStreamBuffer() { }

	virtual void SetConfig(const StreamBufferConfig& config) = 0;

	///启动StreamBuffer
	virtual bool Start() = 0;

	///停止StreamBuffer
	virtual bool Stop() = 0;

	///复位Buffer的状态
	virtual bool Reset(UINT index) = 0;

	/// 主定时器
	virtual void OnAppTimer(UINT times, UINT downSpeed) = 0;

	/// 设置下载管理模块
	virtual void SetDownloader(Downloader* downloader) { }

	virtual void SetMediaServer(CMediaServer* mediaServer) = 0;

	/// 增加一个header片
	virtual bool AddHeaderPiece(MediaHeaderPiecePtr piece) = 0;

	/// 增加一个数据片
	virtual bool AddDataPiece(MediaDataPiecePtr piece) = 0;


//	virtual pair<bool, PPMediaDataPacketPtr> AddSubPiece(SubMediaPiecePtr subPiece) = 0;

	/// 获取数据片管理器
	virtual Storage& GetStorage() = 0;

	/// 获取统计数据
	virtual const StreamBufferStatistics& GetStatistics() const = 0;

	virtual const StreamIndicator& GetBaseIndicator() const = 0;


	/// 获得当前缓冲区最小的已下载DataPacket编号
	//virtual UINT GetMinIndex() const = 0;

	/// 获得当前缓冲区最大的已下载DataPacket编号
	//virtual UINT GetMaxIndex() const = 0;

	/// 获取当前推进的位置
	virtual UINT GetSkipIndex() const = 0;
	
	virtual UINT64 GetSkipTimestamp() const = 0;

	/// 获得一段范围之间的Piece个数
	virtual UINT GetRangeCount( UINT rangeMinIndex, UINT rangeMaxIndex ) const = 0;

	/// 获取下载的起始位置
	virtual UINT GetDownloadStartIndex() const = 0;

	virtual UINT GetLocationIndex() const = 0;

	virtual UINT GetPrelocationIndex() const = 0;


	//// 生成完整的资源位图
	virtual BitMap BuildTotalBitmap() const = 0;


	///对应的index是否需要下载
	virtual bool NeedDownload(UINT index) const = 0;

	///对应的index是否需要下载
	virtual bool NeedHeader(UINT index) const = 0;

	
	virtual void SetPrelocationIndex( UINT PrelocationIndex ) = 0;
	
	virtual void SetPeerInformation(boost::shared_ptr<const PeerInformation> info) = 0;

	virtual void GenerateKey(const GUID& seed) = 0;
	// Added by Tady, 090308: 计算当前的下载点与推进点之间的数据长度
	virtual UINT GetPrepaDataLen() = 0;
	virtual int GetPrepaDataTime() = 0;

	virtual DataSignerPtr GetSigner() = 0;
};

typedef CStreamBuffer StreamBuffer;

/// streambuffer的类工厂
class StreamBufferFactory
{
public:
	static StreamBuffer* PeerCreate();
	static StreamBuffer* MCCCreate();
	static StreamBuffer* MDSCreate();
	static StreamBuffer* SimpleMDSCreate();
};


#endif
