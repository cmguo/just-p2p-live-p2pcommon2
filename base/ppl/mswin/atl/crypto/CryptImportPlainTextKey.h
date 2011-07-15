#ifndef _LIVE_P2PCOMMON2_BASE_PPL_MSWIN_ATL_CRYPTO_CRYPT_IMPORT_PLAIN_TEXT_KEY_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_MSWIN_ATL_CRYPTO_CRYPT_IMPORT_PLAIN_TEXT_KEY_H_

#include "stdafx.h"

namespace ATL
{
	class CCryptImportPlainTextKey :
		public CCryptImportKey
	{
	public:
		CCryptImportPlainTextKey(void) : CCryptImportKey() {}
		virtual ~CCryptImportPlainTextKey(void) {}

		HRESULT ImportPlainText(
			CCryptProv &Prov,
			const BYTE * btKey,
			DWORD dwKeyLen,
			ALG_ID nAlgId) throw()
		{
			CAtlArray<BYTE> arrMem;
			arrMem.SetCount(sizeof(BLOBHEADER) + sizeof(DWORD) + dwKeyLen);
			BLOBHEADER *pbh = (BLOBHEADER*) arrMem.GetData();
			pbh->aiKeyAlg = nAlgId;
			pbh->bType = PLAINTEXTKEYBLOB;
			pbh->bVersion = CUR_BLOB_VERSION;
			pbh->reserved = 0;
			DWORD *pdwKeySize = (DWORD*) (arrMem.GetData() + sizeof(BLOBHEADER));
			*pdwKeySize = dwKeyLen;
			memcpy(arrMem.GetData() + sizeof(BLOBHEADER) + sizeof(DWORD), btKey, dwKeyLen);

			return Initialize(Prov, arrMem.GetData(), sizeof(BLOBHEADER) + sizeof(DWORD) + dwKeyLen, EmptyKey, 0);
		}
	};
}

#endif