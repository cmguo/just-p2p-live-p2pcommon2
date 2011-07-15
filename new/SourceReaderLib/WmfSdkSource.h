#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_WMF_SDK_SOURCE_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_WMF_SDK_SOURCE_H_
#include "BufferedNetworkSource.h"

#include "wmsdk.h"
#pragma comment(lib, "wmvcore.lib")

namespace Synacast
{
	namespace StreamSource
	{
#ifndef SAFE_ARRAYDELETE
#define SAFE_ARRAYDELETE( x )	\
	if( NULL != x )				\
		{							\
		delete[] x;				\
		x = NULL;				\
	}
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE( x )	\
	if( NULL != x ) {		\
	x->Release( );		\
	x = NULL;			\
	}
#endif

		class SOURCEREADERLIB_API WmfSdkSource :
			public BufferedNetworkSource,
			public IWMWriterSink,
			public IWMReaderCallback, 
			public IWMReaderCallbackAdvanced
		{
		public:
			WmfSdkSource();
			WmfSdkSource( const string & url );
			~WmfSdkSource(void);

		private:
			IWMWriterAdvanced*		m_AdvancedWriter;
			IWMReaderAdvanced*		m_AdvancedReader;
			IWMReader*				m_Reader;
			IWMWriter*				m_Writer;
			IWMHeaderInfo*			m_ReaderHeaderInfo;
			IWMHeaderInfo*			m_WriterHeaderInfo;

			QWORD					m_DeliverTime;
			DWORD					m_StartTime;
			DWORD					m_Bitrate;
			QWORD					m_Duration;

			HANDLE					m_ReaderEvent;
			HRESULT					m_AsyncResult;

		public:
			/************************************************************************/
			/* 
			Exceptions:
				SourceException
			*/
			/************************************************************************/
			virtual void	Open();
			virtual void	Close();

			/************************************************************************/
			/* 
			Exceptions:
				SourceException
			*/
			/************************************************************************/
			virtual INT64	Seek( INT64 offset, SeekOrigin origin );

			// attributes
			virtual bool	CanRead();
			virtual bool	IsSeekable() { return false; };

		private:	
			HRESULT Init();
			HRESULT GetAttribute( LPCWSTR  pszName, unsigned char * value, size_t size );
			HRESULT CopyCodecInfo();
			HRESULT CopyAttribute();

			void Stop();

			void ResetStatus();

			virtual void	SetUrl( const Url & url );

		private:
			void CacheBuffer( INSSBuffer * pBuffer );

		public:
			/* IWMWriterSink */
			virtual HRESULT STDMETHODCALLTYPE OnHeader( 
				/* [in] */ INSSBuffer *pHeader);

			virtual HRESULT STDMETHODCALLTYPE IsRealTime( 
				/* [out] */ BOOL *pfRealTime);

			virtual HRESULT STDMETHODCALLTYPE AllocateDataUnit( 
				/* [in] */ DWORD cbDataUnit,
				/* [out] */ INSSBuffer **ppDataUnit);

			virtual HRESULT STDMETHODCALLTYPE OnDataUnit( 
				/* [in] */ INSSBuffer *pDataUnit);

			virtual HRESULT STDMETHODCALLTYPE OnEndWriting( void );

			/* IWMReaderCallback */
			HRESULT STDMETHODCALLTYPE OnSample( /* [in] */ DWORD dwOutputNum,
				/* [in] */ QWORD cnsSampleTime,
				/* [in] */ QWORD cnsSampleDuration,
				/* [in] */ DWORD dwFlags,
				/* [in] */ INSSBuffer __RPC_FAR *pSample,
				/* [in] */ void __RPC_FAR *pvContext);

			HRESULT STDMETHODCALLTYPE OnStatus( /* [in] */ WMT_STATUS Status,
				/* [in] */ HRESULT hr,
				/* [in] */ WMT_ATTR_DATATYPE dwType,
				/* [in] */ BYTE __RPC_FAR *pValue,
				/* [in] */ void __RPC_FAR *pvContext);

			/* IWMReaderCallbackAdvanced */
			HRESULT STDMETHODCALLTYPE AllocateForOutput( /* [in] */ DWORD dwOutputNum,
				/* [in] */ DWORD cbBuffer,
				/* [out] */ INSSBuffer __RPC_FAR *__RPC_FAR *ppBuffer,
				/* [in] */ void __RPC_FAR *pvContext);

			HRESULT STDMETHODCALLTYPE AllocateForStream( /* [in] */ WORD wStreamNum,
				/* [in] */ DWORD cbBuffer,
				/* [out] */ INSSBuffer __RPC_FAR *__RPC_FAR *ppBuffer,
				/* [in] */ void __RPC_FAR *pvContext);

			HRESULT STDMETHODCALLTYPE OnOutputPropsChanged( /* [in] */ DWORD dwOutputNum,
				/* [in] */ WM_MEDIA_TYPE __RPC_FAR *pMediaType,
				/* [in] */ void __RPC_FAR *pvContext);

			HRESULT STDMETHODCALLTYPE OnStreamSample( /* [in] */ WORD wStreamNum,
				/* [in] */ QWORD cnsSampleTime,
				/* [in] */ QWORD cnsSampleDuration,
				/* [in] */ DWORD dwFlags,
				/* [in] */ INSSBuffer __RPC_FAR *pSample,
				/* [in] */ void __RPC_FAR *pvContext);

			HRESULT STDMETHODCALLTYPE OnStreamSelection( /* [in] */ WORD wStreamCount,
				/* [in] */ WORD __RPC_FAR *pStreamNumbers,
				/* [in] */ WMT_STREAM_SELECTION __RPC_FAR *pSelections,
				/* [in] */ void __RPC_FAR *pvContext);

			HRESULT STDMETHODCALLTYPE OnTime( /* [in] */ QWORD cnsCurrentTime,
				/* [in] */ void __RPC_FAR *pvContext);

			/*	IUnknown */
			HRESULT STDMETHODCALLTYPE QueryInterface( REFIID riid,
				void __RPC_FAR *__RPC_FAR *ppvObject);

			ULONG STDMETHODCALLTYPE AddRef( void ) { return 1; }

			ULONG STDMETHODCALLTYPE Release( void ) { return 1; }

		};

	}
}

#endif