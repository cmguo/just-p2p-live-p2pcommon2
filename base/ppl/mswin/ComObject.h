
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_MSWIN_COMOBJECT_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_MAWIN_COMOBJECT_H_

/**
 * @file
 * @brief com对象的实现
 */

//#include "interlocked.h"



#define PPL_COM_INTERFACE_MAP_BEGIN()	\
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void __RPC_FAR *__RPC_FAR *ppvObject) { LIVE_ASSERT(riid != __uuidof(IUnknown));


#define PPL_COM_INTERFACE_ENTRY(theInterface)	if (riid == __uuidof(theInterface)) { theInterface* pTempInterface = this; *ppvObject = pTempInterface; this->AddRef(); return S_OK; }

#define PPL_COM_INTERFACE_MAP_END()		return E_NOINTERFACE; }


/*
#define PPL_COM_IMPL_REF_COUNT()													\
	public: \
		ULONG STDMETHODCALLTYPE AddRef() { return m_refCount.Increment(); } \
		ULONG STDMETHODCALLTYPE Release() { \
			ULONG refCount = m_refCount.Decrement(); \
			if (refCount == 0) delete this; \
			return refCount; \
		} \
	private: \
		InterlockedInteger m_refCount;
*/


#define PPL_COM_IMPL_FAKE_REF_COUNT() \
		ULONG STDMETHODCALLTYPE AddRef() { return 1; } \
		ULONG STDMETHODCALLTYPE Release() { return 1; }



/*
/// 负责实现IUnkown接口
template <typename InterfaceT>
class ComObject : public InterfaceT
{
public:
	typedef InterfaceT interface_type;

	ComObject() { }
	virtual ~ComObject() { }

	PPL_COM_INTERFACE_MAP_BEGIN()
		PPL_COM_INTERFACE_ENTRY(InterfaceT)
	PPL_COM_INTERFACE_MAP_END()

	PPL_COM_IMPL_REF_COUNT()

};
*/



/// 负责实现IUnkown接口
template <typename InterfaceT>
class FakeComObject : public InterfaceT
{
public:
	typedef InterfaceT interface_type;

	FakeComObject() { }
	virtual ~FakeComObject() { }

	PPL_COM_INTERFACE_MAP_BEGIN()
		PPL_COM_INTERFACE_ENTRY(InterfaceT)
	PPL_COM_INTERFACE_MAP_END()

	PPL_COM_IMPL_FAKE_REF_COUNT()

};


#endif