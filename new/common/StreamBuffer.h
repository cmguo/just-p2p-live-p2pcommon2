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


/// �������ݻ���Ĵ���(�ӿ���)
class CStreamBuffer
{
public:
	CStreamBuffer() { }

	virtual ~CStreamBuffer() { }

	virtual void SetConfig(const StreamBufferConfig& config) = 0;

	///����StreamBuffer
	virtual bool Start() = 0;

	///ֹͣStreamBuffer
	virtual bool Stop() = 0;

	///��λBuffer��״̬
	virtual bool Reset(UINT index) = 0;

	/// ����ʱ��
	virtual void OnAppTimer(UINT times, UINT downSpeed) = 0;

	/// �������ع���ģ��
	virtual void SetDownloader(Downloader* downloader) { }

	virtual void SetMediaServer(CMediaServer* mediaServer) = 0;

	/// ����һ��headerƬ
	virtual bool AddHeaderPiece(MediaHeaderPiecePtr piece) = 0;

	/// ����һ������Ƭ
	virtual bool AddDataPiece(MediaDataPiecePtr piece) = 0;


//	virtual pair<bool, PPMediaDataPacketPtr> AddSubPiece(SubMediaPiecePtr subPiece) = 0;

	/// ��ȡ����Ƭ������
	virtual Storage& GetStorage() = 0;

	/// ��ȡͳ������
	virtual const StreamBufferStatistics& GetStatistics() const = 0;

	virtual const StreamIndicator& GetBaseIndicator() const = 0;


	/// ��õ�ǰ��������С��������DataPacket���
	//virtual UINT GetMinIndex() const = 0;

	/// ��õ�ǰ����������������DataPacket���
	//virtual UINT GetMaxIndex() const = 0;

	/// ��ȡ��ǰ�ƽ���λ��
	virtual UINT GetSkipIndex() const = 0;
	
	virtual UINT64 GetSkipTimestamp() const = 0;

	/// ���һ�η�Χ֮���Piece����
	virtual UINT GetRangeCount( UINT rangeMinIndex, UINT rangeMaxIndex ) const = 0;

	/// ��ȡ���ص���ʼλ��
	virtual UINT GetDownloadStartIndex() const = 0;

	virtual UINT GetLocationIndex() const = 0;

	virtual UINT GetPrelocationIndex() const = 0;


	//// ������������Դλͼ
	virtual BitMap BuildTotalBitmap() const = 0;


	///��Ӧ��index�Ƿ���Ҫ����
	virtual bool NeedDownload(UINT index) const = 0;

	///��Ӧ��index�Ƿ���Ҫ����
	virtual bool NeedHeader(UINT index) const = 0;

	
	virtual void SetPrelocationIndex( UINT PrelocationIndex ) = 0;
	
	virtual void SetPeerInformation(boost::shared_ptr<const PeerInformation> info) = 0;

	virtual void GenerateKey(const GUID& seed) = 0;
	// Added by Tady, 090308: ���㵱ǰ�����ص����ƽ���֮������ݳ���
	virtual UINT GetPrepaDataLen() = 0;
	virtual int GetPrepaDataTime() = 0;

	virtual DataSignerPtr GetSigner() = 0;
};

typedef CStreamBuffer StreamBuffer;

/// streambuffer���๤��
class StreamBufferFactory
{
public:
	static StreamBuffer* PeerCreate();
	static StreamBuffer* MCCCreate();
	static StreamBuffer* MDSCreate();
	static StreamBuffer* SimpleMDSCreate();
};


#endif
