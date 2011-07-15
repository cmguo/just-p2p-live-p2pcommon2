
#ifndef _LIVE_P2PCOMMON2_NEW_COMMON_SOURCE_WRAPPER_H_
#define _LIVE_P2PCOMMON2_NEW_COMMON_SOURCE_WRAPPER_H_

#include "wrapper.h"
#include "Source.h"



class SourceModuleWrapperBase : public LiveAppModuleWrapper
{
public:
	explicit SourceModuleWrapperBase( const LIVEPARAM& param ) : LiveAppModuleWrapper( param, SOURCE_PEER )
	{
	}
	~SourceModuleWrapperBase()
	{
		m_sourceList->Stop();
	}

protected:
	//PPL_DECLARE_MESSAGE_MAP()
	MESSAGE_MAP_START
		MAP(UM_CMD_SOURCEPLAY, OnCMDSourcePlay)
		MAP(UM_CMD_SOURCESTOP, OnCMDSourceStop)

		MAP(UM_READER_HEADER, OnReaderHeaderPiece)
		MAP(UM_READER_PIECE, OnReaderDataPiece)
		//MAP(UM_READER_END, OnReaderEndPiece)

		INHERITED (LiveAppModuleWrapper)
	MESSAGE_MAP_END 


	int OnCMDSourcePlay(MESSAGE& Message)
	{
		LPSOURCEPLAYINFO lpInfo= (LPSOURCEPLAYINFO) Message.PARAM_1;

		DWORD maxPieceSize = lpInfo->MaxPieceSize;
#if defined(_DEBUG) && 0
#pragma message("------!!!!!!!!!!!!!replace max size setting")
		maxPieceSize = 8 * 1024;
#endif
		m_sourceList->StartSourcePlay( lpInfo->URL, lpInfo->ID, (HWND)lpInfo->hWnd, maxPieceSize );
		CORE_FREE(lpInfo);
		return 0;
	}
	int OnCMDSourceStop(MESSAGE& Message)
	{
		LPSOURCEPLAYINFO lpInfo= (LPSOURCEPLAYINFO) Message.PARAM_1;
		m_sourceList->StopSourcePlay();
		CORE_FREE(lpInfo);
		return 0;
	}

	/// 收到reader的data piece
	int OnReaderHeaderPiece(MESSAGE& msg)
	{
		MonoMediaHeaderPiece* piece = reinterpret_cast<MonoMediaHeaderPiece*>(msg.PARAM_1);
		APP_DEBUG( "Recv Reader media header Packet " << piece->GetPieceLength());
		m_sourceModule->SafeAddHeaderPiece(piece);
		return 0;
	}

	/// 收到reader的data piece
	int OnReaderDataPiece(MESSAGE& msg)
	{
		MonoMediaDataPiece* piece = reinterpret_cast<MonoMediaDataPiece*>(msg.PARAM_1);
		APP_DEBUG( "Recv Reader media data Packet " << piece->GetPieceLength());
		m_sourceModule->SafeAddDataPiece(piece);
		return 0;
	}

	/// 收到reader的data piece
/*	int OnReaderEndPiece(MESSAGE& msg)
	{
		APP_DEBUG( "Recv Reader media end Packet" );
		PPMediaEndPacket* piece = reinterpret_cast<PPMediaEndPacket*>(msg.PARAM_1);
		SAFE_DELETE(piece);
		return 0;
	}*/

protected:
	boost::scoped_ptr<CSourceList> m_sourceList;
	boost::shared_ptr<SourceModule> m_sourceModule;
};



class SourceModuleWrapper : public SourceModuleWrapperBase
{
public:
	explicit SourceModuleWrapper(const LIVEPARAM& param) : SourceModuleWrapperBase(param)
	{
		m_sourceModule.reset(new SourceModule(m_param));
		m_appModule = m_sourceModule;
		m_sourceList.reset(new CSourceList(m_appModule.get(), this, m_sourceModule->GetDelayTime()));
		m_sourceList->Start();
		m_appModule->Start(m_param);
	}
};

#endif