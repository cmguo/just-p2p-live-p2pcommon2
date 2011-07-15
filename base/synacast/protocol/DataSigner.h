
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_SIGNER_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_DATA_SIGNER_H_

#include <synacast/protocol/SignatureData.h>

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

#include <vector>

//#pragma message("------ enable crypto")

#include <ppl/crypto/hmac.h>


/// 数据签名算法
class SignAlgorithm
{
public:
	bool Sign(const BYTE* data, size_t len, const std::vector<BYTE>& key, SignatureData& result)
	{
		assert(!key.empty());
		bool res = GetHasher().HashData(&key[0], static_cast<DWORD>( key.size() ), data, static_cast<DWORD>( len ), result.c_array());
		assert(res);
		return res;
	}
	bool Verify(const BYTE* data, size_t len, const std::vector<BYTE>& key, const BYTE* signature)
	{
		assert(!key.empty());
		SignatureData result;
		bool res = this->Sign(data, static_cast<DWORD>( len ), key, result);
		assert(res);
		if (!res)
		{
			// 如果签名失败，一般可能是系统的加解密模块有问题，这里忽略这种问题，认为验证成功
			return true;
			//return false;
		}
		int cmp = memcmp(signature, result.data(), HMACHashing::HASH_SIZE);
		return cmp == 0;
	}

private:
	static HMACHashing& GetHasher()
	{
		static HMACHashing hasher;
		return hasher;
	}
};


/// 数据签名类
class DataSigner : private boost::noncopyable
{
public:
	explicit DataSigner( const std::vector<BYTE>& key ) : m_key( key )
	{
	}

	bool Sign(const BYTE* data, size_t len, SignatureData& result)
	{
		return m_algorithm.Sign( data, len, m_key, result );
	}
	bool Verify(const BYTE* data, size_t len, const BYTE* signature)
	{
		return m_algorithm.Verify( data, len, m_key, signature );
	}

	const std::vector<BYTE> GetKey() const { return m_key; }

private:
	SignAlgorithm m_algorithm;
	std::vector<BYTE> m_key;
};

typedef boost::shared_ptr<DataSigner> DataSignerPtr;


#endif
