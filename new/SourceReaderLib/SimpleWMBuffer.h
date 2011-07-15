#ifndef _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_SIMPLE_WMBUFFER_H_
#define _LIVE_P2PCOMMON2_NEW_SOURCEREADERLIB_SIMPLE_WMBUFFER_H_

#include "wmsdk.h"

namespace Synacast
{
	namespace StreamSource
	{

class SimpleWMBuffer : public INSSBuffer  
{
private:
	LONG	m_Ref;
	BYTE *	m_Buffer;
	DWORD	m_BufSize;
	DWORD	m_DataSize;

public:
	SimpleWMBuffer( DWORD inSize );
	~SimpleWMBuffer();

	/* INSSBuffer */
	HRESULT STDMETHODCALLTYPE GetLength( 
		/* [out] */ DWORD *pdwLength)
	{
		*pdwLength = m_DataSize;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE SetLength( 
		/* [in] */ DWORD dwLength)
	{
		m_DataSize = dwLength;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE GetMaxLength( 
		/* [out] */ DWORD *pdwLength)
	{
		*pdwLength = m_BufSize;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE GetBuffer( 
		/* [out] */ BYTE **ppdwBuffer)
	{
		*ppdwBuffer = m_Buffer;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE GetBufferAndLength( 
		/* [out] */ BYTE **ppdwBuffer,
		/* [out] */ DWORD *pdwLength)
	{
		*ppdwBuffer = m_Buffer;
		*pdwLength = m_DataSize;
		return S_OK;
	}

	/* IUnknown */
	HRESULT STDMETHODCALLTYPE QueryInterface( 
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject)
	{
		if( IID_INSSBuffer == riid || IID_IUnknown == riid )
		{
			*ppvObject = static_cast<INSSBuffer*>(this);
			AddRef();
			return S_OK;
		}
		*ppvObject = NULL;
		return E_NOINTERFACE;
	}

	ULONG STDMETHODCALLTYPE AddRef( void)
	{
		return InterlockedIncrement( &m_Ref );
	}

	ULONG STDMETHODCALLTYPE Release( void)
	{
		if( 0 == InterlockedDecrement(&m_Ref) )
		{
			delete this;
			return 0;
		}
		return m_Ref;
	}
};
	}
}

#endif