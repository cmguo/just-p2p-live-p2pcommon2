
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


/// streambuffer��ʵ����
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

	///����StreamBuffer
	bool Start();

	///ֹͣStreamBuffer
	bool Stop();

	///��λBuffer��״̬
	bool Reset(UINT index);


	/// ���һ�η�Χ֮���Piece����
	size_t GetRangeCount( UINT rangeMinIndex, UINT rangeMaxIndex ) const;

	virtual const StreamBufferStatistics& GetStatistics() const { return m_statistics; }
	//virtual pair<bool, PPMediaDataPacketPtr> AddSubPiece(SubPieceDataPacketPtr subPiece) { return make_pair(false, PPMediaDataPacketPtr()); }

	virtual void SetPeerInformation( boost::shared_ptr<const PeerInformation> info );

	virtual const StreamIndicator& GetBaseIndicator() const
	{
		return m_base;
	}

	/// ����һ��headerƬ
	virtual bool AddHeaderPiece( MediaHeaderPiecePtr piece );

	/// ����һ������Ƭ
	virtual bool AddDataPiece( MediaDataPiecePtr piece ) = 0;


public:

	virtual bool NeedHeader(UINT index) const { return !m_storage.HasHeader(index); }

	/// ��ȡ���ص���ʼλ��
	UINT GetDownloadStartIndex() const { return 0; }

	///��õ�ǰ��������С��������DataPacket���
	//UINT GetMinIndex() const { return m_storage.GetMinIndex(); }

	///��õ�ǰ����������������DataPacket���
	//UINT GetMaxIndex() const { return m_storage.GetMaxIndex(); }


	UINT GetLocationIndex() const { return 0; }

	/// ��ȡ����ʱ��Ĺ���ֵ����BufferTime * ������
	UINT32 GetApproximateBufferTime() const;

	//// ������������Դλͼ
	BitMap BuildTotalBitmap() const;
	
	virtual void SetPrelocationIndex( UINT PrelocationIndex ) {}
	
	virtual UINT GetPrelocationIndex() const { return 0; }

	virtual void SetMediaServer(CMediaServer* mediaServer) { m_lpMediaServer = mediaServer; }

	virtual void GenerateKey(const GUID& seed);
	// Added by Tady, 090308: ���㵱ǰ�����ص����ƽ���֮������ݳ���
	virtual UINT GetPrepaDataLen();
	virtual int GetPrepaDataTime();

	virtual DataSignerPtr GetSigner()
	{
		return m_signer;
	}

protected:
	/// ����dataƬ
	bool DoAddDataPiece( MediaDataPiecePtr piece );

protected:
	virtual void OnAppTimer(UINT times, UINT downSpeed);

	/// ����������
	virtual void Shrink() = 0;

	/// �ƽ�������
	virtual void Push() = 0;

	/// ����ͳ������
	virtual void UpdateStatistics(UINT downSpeed) = 0;

	/// ���
	virtual void Clear();

	void UpdateBase();

	/// ���»�׼��֮�����
	void OnUpdateBase();

	/// ʵ�ָ��»�׼�㹦��
	virtual void DoUpdateBase(const PieceInfo& piece);

	/// �յ���һƬ��������ʱ����ʼ�ƽ�
	void OnFirstPiece(MediaDataPiecePtr lpPacket);

protected:
	/// ͳ������
	StreamBufferStatistics m_statistics;
	StreamBufferConfig m_config;
	boost::shared_ptr<const PeerInformation> m_PeerInformation;
	boost::shared_ptr<const SourceResource> m_SourceResource;

	/// streambuffer������״̬
	enum 
	{
		///δ������״̬����Start��	
		st_none		= 0,
		///�����󣬵���δ���ص��κ�Piece��״̬
		st_waiting	= 1,
		///������SkipTimer���״̬
		st_running	= 2
	} m_State;

	///ָ��MediaServer,Ҫ�����䴴���͸���
	CMediaServer* m_lpMediaServer;

	/// piece����
	Storage m_storage;

	/// ���Ļ�׼λ��
	StreamIndicator m_base;

	/// ��С�����Ļ�������С(MinIndex��SkipIndex��ʱ���)
	UINT m_minBufferTime;

	time_counter m_StartTime;

	DataSignerPtr m_signer;
};





/// ����source��StreamBuffer
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
	/// �ܵ�Ƭ��
	size_t m_totalDownloadedPieceCount;
};




/// peer�˵�StreamBuffer
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

	virtual void Push();			// �ƽ�
	virtual void Shrink();			// ɾ�����ڵ�����
	virtual void UpdateStatistics(UINT downSpeed);
	virtual bool NeedDownload(UINT index) const;

	virtual void DoUpdateBase(const PieceInfo& piece);

	/// ���piece�Ƿ������������reset����
	bool IsValidPieceForReset(MediaDataPiecePtr piece) const;

	/// �Ƿ���Ҫ���û�����
	bool NeedResetBuffer(MediaDataPiecePtr piece) const;

	/// �ƽ�skipIndex
	size_t PushStream(UINT& skipIndex);

	/// �ƽ�playIndex
	//void PushPlay();

	/// �������ƽ�
	///size_t PushContinuously(UINT& skipIndex);

	/// ��ʱ����ƽ�
	///size_t PushTimely(const StreamIndicator& base, UINT& skipIndex);


	virtual void SetPrelocationIndex( UINT PrelocationIndex );

	virtual UINT GetPrelocationIndex() const { return m_PrelocationIndex; }

private:
	/// �ƽ�λ��
	UINT m_skipIndex;
	
	/// ����λ��
	UINT m_playIndex;

	/// Ԥ��λ��λ��,�������Ԥ��λ,��Ԥ��λ֮ǰ�����ݽ���Ϊ�����������ص�����
	UINT m_PrelocationIndex;

	/// ͳ����Ϣ�ռ�
	StreamFeedback m_feedback;

	/// ���ع���ģ��
	Downloader* m_downloader;

};

#endif
