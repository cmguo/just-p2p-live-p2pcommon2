

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0499 */
/* at Mon Sep 24 11:26:12 2007
 */
/* Compiler settings for .\PDRM.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef _LIVE_P2PCOMMON2_NEW_COMMON_PDRM_h_
#define _LIVE_P2PCOMMON2_NEW_COMMON_PDRM_h_

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __ISourcePdrm_FWD_DEFINED__
#define __ISourcePdrm_FWD_DEFINED__
typedef interface ISourcePdrm ISourcePdrm;
#endif 	/* __ISourcePdrm_FWD_DEFINED__ */


#ifndef __IPeerPdrm_FWD_DEFINED__
#define __IPeerPdrm_FWD_DEFINED__
typedef interface IPeerPdrm IPeerPdrm;
#endif 	/* __IPeerPdrm_FWD_DEFINED__ */


#ifndef __IETicket_FWD_DEFINED__
#define __IETicket_FWD_DEFINED__
typedef interface IETicket IETicket;
#endif 	/* __IETicket_FWD_DEFINED__ */


#ifndef __SourcePdrm_FWD_DEFINED__
#define __SourcePdrm_FWD_DEFINED__

#ifdef __cplusplus
typedef class SourcePdrm SourcePdrm;
#else
typedef struct SourcePdrm SourcePdrm;
#endif /* __cplusplus */

#endif 	/* __SourcePdrm_FWD_DEFINED__ */


#ifndef __PeerPdrm_FWD_DEFINED__
#define __PeerPdrm_FWD_DEFINED__

#ifdef __cplusplus
typedef class PeerPdrm PeerPdrm;
#else
typedef struct PeerPdrm PeerPdrm;
#endif /* __cplusplus */

#endif 	/* __PeerPdrm_FWD_DEFINED__ */


#ifndef __ETicket_FWD_DEFINED__
#define __ETicket_FWD_DEFINED__

#ifdef __cplusplus
typedef class ETicket ETicket;
#else
typedef struct ETicket ETicket;
#endif /* __cplusplus */

#endif 	/* __ETicket_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __ISourcePdrm_INTERFACE_DEFINED__
#define __ISourcePdrm_INTERFACE_DEFINED__

/* interface ISourcePdrm */
/* [unique][helpstring][nonextensible][dual][uuid][object] */ 


EXTERN_C const IID IID_ISourcePdrm;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("99740BF5-2715-4D80-87F0-F11DDFEA8630")
    ISourcePdrm : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE PutHeader( 
            /* [in] */ const BYTE *btHeader,
            /* [in] */ ULONG nHeaderLen) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ProfileLength( 
            /* [retval][out] */ ULONG *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetProfile( 
            /* [in] */ ULONG nBufferLen,
            /* [retval][out] */ BYTE *btProfile) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_EncryptedHeaderLength( 
            /* [retval][out] */ ULONG *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetEncryptedHeader( 
            /* [in] */ ULONG nBufferLen,
            /* [retval][out] */ BYTE *btHeader) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ChannelGuid( 
            /* [retval][out] */ GUID *pVal) = 0;
        
        virtual /* [helpstring][id][propputref] */ HRESULT STDMETHODCALLTYPE putref_ChannelGuid( 
            /* [in] */ GUID newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ProgramId( 
            /* [retval][out] */ ULONG *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ProgramId( 
            /* [in] */ ULONG newVal) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISourcePdrmVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISourcePdrm * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISourcePdrm * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISourcePdrm * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISourcePdrm * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISourcePdrm * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISourcePdrm * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISourcePdrm * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *PutHeader )( 
            ISourcePdrm * This,
            /* [in] */ const BYTE *btHeader,
            /* [in] */ ULONG nHeaderLen);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ProfileLength )( 
            ISourcePdrm * This,
            /* [retval][out] */ ULONG *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetProfile )( 
            ISourcePdrm * This,
            /* [in] */ ULONG nBufferLen,
            /* [retval][out] */ BYTE *btProfile);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_EncryptedHeaderLength )( 
            ISourcePdrm * This,
            /* [retval][out] */ ULONG *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetEncryptedHeader )( 
            ISourcePdrm * This,
            /* [in] */ ULONG nBufferLen,
            /* [retval][out] */ BYTE *btHeader);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ChannelGuid )( 
            ISourcePdrm * This,
            /* [retval][out] */ GUID *pVal);
        
        /* [helpstring][id][propputref] */ HRESULT ( STDMETHODCALLTYPE *putref_ChannelGuid )( 
            ISourcePdrm * This,
            /* [in] */ GUID newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ProgramId )( 
            ISourcePdrm * This,
            /* [retval][out] */ ULONG *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ProgramId )( 
            ISourcePdrm * This,
            /* [in] */ ULONG newVal);
        
        END_INTERFACE
    } ISourcePdrmVtbl;

    interface ISourcePdrm
    {
        CONST_VTBL struct ISourcePdrmVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISourcePdrm_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ISourcePdrm_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ISourcePdrm_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ISourcePdrm_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define ISourcePdrm_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define ISourcePdrm_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define ISourcePdrm_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define ISourcePdrm_PutHeader(This,btHeader,nHeaderLen)	\
    ( (This)->lpVtbl -> PutHeader(This,btHeader,nHeaderLen) ) 

#define ISourcePdrm_get_ProfileLength(This,pVal)	\
    ( (This)->lpVtbl -> get_ProfileLength(This,pVal) ) 

#define ISourcePdrm_GetProfile(This,nBufferLen,btProfile)	\
    ( (This)->lpVtbl -> GetProfile(This,nBufferLen,btProfile) ) 

#define ISourcePdrm_get_EncryptedHeaderLength(This,pVal)	\
    ( (This)->lpVtbl -> get_EncryptedHeaderLength(This,pVal) ) 

#define ISourcePdrm_GetEncryptedHeader(This,nBufferLen,btHeader)	\
    ( (This)->lpVtbl -> GetEncryptedHeader(This,nBufferLen,btHeader) ) 

#define ISourcePdrm_get_ChannelGuid(This,pVal)	\
    ( (This)->lpVtbl -> get_ChannelGuid(This,pVal) ) 

#define ISourcePdrm_putref_ChannelGuid(This,newVal)	\
    ( (This)->lpVtbl -> putref_ChannelGuid(This,newVal) ) 

#define ISourcePdrm_get_ProgramId(This,pVal)	\
    ( (This)->lpVtbl -> get_ProgramId(This,pVal) ) 

#define ISourcePdrm_put_ProgramId(This,newVal)	\
    ( (This)->lpVtbl -> put_ProgramId(This,newVal) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ISourcePdrm_INTERFACE_DEFINED__ */


#ifndef __IPeerPdrm_INTERFACE_DEFINED__
#define __IPeerPdrm_INTERFACE_DEFINED__

/* interface IPeerPdrm */
/* [unique][helpstring][nonextensible][dual][uuid][object] */ 


EXTERN_C const IID IID_IPeerPdrm;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("234A8462-E971-44D6-ADB5-C63FC721FC30")
    IPeerPdrm : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Start( 
            /* [in] */ int pObj) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Stop( void) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BasicObjectPointer( 
            /* [retval][out] */ int *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SendMessageToPdrm( 
            /* [in] */ UINT uMsg,
            /* [in] */ UINT nParam1,
            /* [in] */ UINT nParam2,
            /* [in] */ UINT nParam3,
            /* [in] */ UINT nParam4) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE FreeMemory( 
            /* [in] */ LONG pMem) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPeerPdrmVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPeerPdrm * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPeerPdrm * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPeerPdrm * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IPeerPdrm * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IPeerPdrm * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IPeerPdrm * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IPeerPdrm * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Start )( 
            IPeerPdrm * This,
            /* [in] */ int pObj);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Stop )( 
            IPeerPdrm * This);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BasicObjectPointer )( 
            IPeerPdrm * This,
            /* [retval][out] */ int *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SendMessageToPdrm )( 
            IPeerPdrm * This,
            /* [in] */ UINT uMsg,
            /* [in] */ UINT nParam1,
            /* [in] */ UINT nParam2,
            /* [in] */ UINT nParam3,
            /* [in] */ UINT nParam4);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *FreeMemory )( 
            IPeerPdrm * This,
            /* [in] */ LONG pMem);
        
        END_INTERFACE
    } IPeerPdrmVtbl;

    interface IPeerPdrm
    {
        CONST_VTBL struct IPeerPdrmVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPeerPdrm_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IPeerPdrm_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IPeerPdrm_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IPeerPdrm_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IPeerPdrm_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IPeerPdrm_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IPeerPdrm_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IPeerPdrm_Start(This,pObj)	\
    ( (This)->lpVtbl -> Start(This,pObj) ) 

#define IPeerPdrm_Stop(This)	\
    ( (This)->lpVtbl -> Stop(This) ) 

#define IPeerPdrm_get_BasicObjectPointer(This,pVal)	\
    ( (This)->lpVtbl -> get_BasicObjectPointer(This,pVal) ) 

#define IPeerPdrm_SendMessageToPdrm(This,uMsg,nParam1,nParam2,nParam3,nParam4)	\
    ( (This)->lpVtbl -> SendMessageToPdrm(This,uMsg,nParam1,nParam2,nParam3,nParam4) ) 

#define IPeerPdrm_FreeMemory(This,pMem)	\
    ( (This)->lpVtbl -> FreeMemory(This,pMem) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IPeerPdrm_INTERFACE_DEFINED__ */


#ifndef __IETicket_INTERFACE_DEFINED__
#define __IETicket_INTERFACE_DEFINED__

/* interface IETicket */
/* [unique][helpstring][nonextensible][dual][uuid][object] */ 


EXTERN_C const IID IID_IETicket;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9CE55F59-EC3D-47DE-A45B-F1A79449DF17")
    IETicket : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_KeyId( 
            /* [retval][out] */ ULONGLONG *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_IsValid( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_HasExpired( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Clear( void) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_KeyProgram( 
            /* [retval][out] */ BYTE *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ChannelGuid( 
            /* [retval][out] */ GUID *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ProgramId( 
            /* [retval][out] */ LONGLONG *pVal) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IETicketVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IETicket * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IETicket * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IETicket * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IETicket * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IETicket * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IETicket * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IETicket * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_KeyId )( 
            IETicket * This,
            /* [retval][out] */ ULONGLONG *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_IsValid )( 
            IETicket * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HasExpired )( 
            IETicket * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Clear )( 
            IETicket * This);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_KeyProgram )( 
            IETicket * This,
            /* [retval][out] */ BYTE *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ChannelGuid )( 
            IETicket * This,
            /* [retval][out] */ GUID *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ProgramId )( 
            IETicket * This,
            /* [retval][out] */ LONGLONG *pVal);
        
        END_INTERFACE
    } IETicketVtbl;

    interface IETicket
    {
        CONST_VTBL struct IETicketVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IETicket_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IETicket_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IETicket_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IETicket_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IETicket_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IETicket_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IETicket_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IETicket_get_KeyId(This,pVal)	\
    ( (This)->lpVtbl -> get_KeyId(This,pVal) ) 

#define IETicket_get_IsValid(This,pVal)	\
    ( (This)->lpVtbl -> get_IsValid(This,pVal) ) 

#define IETicket_get_HasExpired(This,pVal)	\
    ( (This)->lpVtbl -> get_HasExpired(This,pVal) ) 

#define IETicket_Clear(This)	\
    ( (This)->lpVtbl -> Clear(This) ) 

#define IETicket_get_KeyProgram(This,pVal)	\
    ( (This)->lpVtbl -> get_KeyProgram(This,pVal) ) 

#define IETicket_get_ChannelGuid(This,pVal)	\
    ( (This)->lpVtbl -> get_ChannelGuid(This,pVal) ) 

#define IETicket_get_ProgramId(This,pVal)	\
    ( (This)->lpVtbl -> get_ProgramId(This,pVal) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IETicket_INTERFACE_DEFINED__ */



#ifndef __PDRMLib_LIBRARY_DEFINED__
#define __PDRMLib_LIBRARY_DEFINED__

/* library PDRMLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_PDRMLib;

EXTERN_C const CLSID CLSID_SourcePdrm;

#ifdef __cplusplus

class DECLSPEC_UUID("BA40CA5A-6218-4EE2-B9A7-7E40A4ECB131")
SourcePdrm;
#endif

EXTERN_C const CLSID CLSID_PeerPdrm;

#ifdef __cplusplus

class DECLSPEC_UUID("E7F436C8-81E8-45C9-9BE5-B0F5ECDF2983")
PeerPdrm;
#endif

EXTERN_C const CLSID CLSID_ETicket;

#ifdef __cplusplus

class DECLSPEC_UUID("C643E27A-AE6D-43FF-9463-00CF892E0782")
ETicket;
#endif
#endif /* __PDRMLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif

#endif
