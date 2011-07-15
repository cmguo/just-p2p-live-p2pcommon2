#ifdef PDRM_ENABLED

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


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


#ifdef __cplusplus
extern "C"{
#endif 


#include <rpc.h>
#include <rpcndr.h>

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#else // !_MIDL_USE_GUIDDEF_

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#endif !_MIDL_USE_GUIDDEF_

MIDL_DEFINE_GUID(IID, IID_ISourcePdrm,0x99740BF5,0x2715,0x4D80,0x87,0xF0,0xF1,0x1D,0xDF,0xEA,0x86,0x30);


MIDL_DEFINE_GUID(IID, IID_IPeerPdrm,0x234A8462,0xE971,0x44D6,0xAD,0xB5,0xC6,0x3F,0xC7,0x21,0xFC,0x30);


MIDL_DEFINE_GUID(IID, IID_IETicket,0x9CE55F59,0xEC3D,0x47DE,0xA4,0x5B,0xF1,0xA7,0x94,0x49,0xDF,0x17);


MIDL_DEFINE_GUID(IID, LIBID_PDRMLib,0xCC7D98E8,0x2EE4,0x4C93,0x82,0xBA,0x59,0xAD,0x81,0x33,0xE2,0xC3);


MIDL_DEFINE_GUID(CLSID, CLSID_SourcePdrm,0xBA40CA5A,0x6218,0x4EE2,0xB9,0xA7,0x7E,0x40,0xA4,0xEC,0xB1,0x31);


MIDL_DEFINE_GUID(CLSID, CLSID_PeerPdrm,0xE7F436C8,0x81E8,0x45C9,0x9B,0xE5,0xB0,0xF5,0xEC,0xDF,0x29,0x83);


MIDL_DEFINE_GUID(CLSID, CLSID_ETicket,0xC643E27A,0xAE6D,0x43FF,0x94,0x63,0x00,0xCF,0x89,0x2E,0x07,0x82);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif
#endif
