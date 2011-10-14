
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_CRYPTO_SHA_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_CRYPTO_SHA_H_


#include <ppl/config.h>
#include "ppl/crypto/sha1.h"

#ifndef _PPL_WITH_LIB_SSL

class SHA1Hashing : private boost::noncopyable
{
public:
    SHA1Hashing()
    {
    }
    ~SHA1Hashing()
    {
    }

    static const size_t HASH_SIZE = 20;

    bool Init()
    {
        SHA1Init(&m_Context);
        return true;
    }
    void Clear()
    {
    }
    bool AddData(const BYTE* data, size_t size)
    {
        SHA1Update(&m_Context, (BYTE *)data, size);
        return true;
    }
    bool GetResult(BYTE* data)
    {
        SHA1Final(data, &m_Context);
        return true;
    }

private:
    SHA1_CTX m_Context;
};

#elif defined(_PPL_PLATFORM_MSWIN)

#if _MSC_VER > 1300
#include <atlcomcli.h>
#endif
#include <atlcrypt.h>
#include <ppl/mswin/atl/crypto/utils.h>
#include <boost/noncopyable.hpp>

class SHA1Hashing : private boost::noncopyable
{
public:
	SHA1Hashing()
	{
		if (FAILED(m_Context.Initialize(PROV_RSA_FULL, NULL, NULL, 0)))
		{
			m_Context.InitCreateKeySet(PROV_RSA_FULL, NULL, NULL, 0);
		}
	}
	~SHA1Hashing()
	{
	}

	static const size_t HASH_SIZE = 20;

	bool Init()
	{
		HRESULT hr = m_HashAlgorithm.Initialize(m_Context);
		return SUCCEEDED(hr);
	}
	void Clear()
	{
		m_HashAlgorithm.Destroy();
	}
	bool AddData(const BYTE* data, size_t size)
	{
		HRESULT hr = m_HashAlgorithm.AddData(data, size);
		LIVE_ASSERT(SUCCEEDED(hr));
		return SUCCEEDED(hr);
	}
	bool GetResult(BYTE* data)
	{
		DWORD len = HASH_SIZE;
		HRESULT hr = m_HashAlgorithm.GetValue(data, &len);
		LIVE_ASSERT(SUCCEEDED(hr));
		return SUCCEEDED(hr);
	}

private:
	CCryptProv m_Context;
	CCryptSHA1Hash m_HashAlgorithm;
};


#elif defined(_PPL_PLATFORM_LINUX)


#include <openssl/sha.h>

class SHA1Hashing : private boost::noncopyable
{
public:
        SHA1Hashing()
        {
        }
        ~SHA1Hashing()
        {
        }

        static const size_t HASH_SIZE = 20;

        bool Init()
        {
                SHA_Init(&m_Context);
                return true;
        }
        void Clear()
        {
        }
        bool AddData(const BYTE* data, size_t size)
        {
                SHA_Update(&m_Context, (BYTE *)data, size);
                return true;
        }
        bool GetResult(BYTE* data)
        {
                SHA_Final(data, &m_Context);
                return true;
        }

private:
        SHA_CTX m_Context;
};

#endif


#endif
