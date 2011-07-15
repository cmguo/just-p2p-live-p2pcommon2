
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_LIVE_URL_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_LIVE_URL_H_

#include <synacast/protocol/data/PeerAddress.h>
#include <synacast/protocol/data/TrackerAddress.h>
#include <synacast/protocol/base.h>
#include <ppl/data/guid.h>
#include <ppl/text/base64.h>
#include <ppl/net/uri.h>
#include <ppl/data/stlutils.h>
#include <ppl/net/inet.h>
#include <ppl/diag/trace.h>
#include <ppl/data/numeric.h>
#include <ppl/text/encoding.h>
#include <ppl/data/tstring.h>

#include <boost/tuple/tuple.hpp>
#include <vector>

const char SynacastUrlCypherKey[] = "pplive";
const TCHAR SynacastUrlDelimiter1 = _T('&');
const TCHAR SynacastUrlDelimiter2 = _T(';');
const TCHAR SynacastUrlDelimiter3 = _T(':');
const TCHAR SynacastUrlUDPTracker[] = _T("udpt");
const TCHAR SynacastUrlTCPTracker[] = _T("tcpt");

const TCHAR SynacastUrlHostAddressInnerDelimiter = _T(':');

const TCHAR SynacastUrlParamInnerDelimiter = SynacastUrlDelimiter2;


//////////////////////////////////////////////////////////////////////////
// parse TrackerAddress and PeerAddress to intermediary form
//////////////////////////////////////////////////////////////////////////


class TrackerAddressInfo
{
public:
	tstring Host;
	tstring Port;
	tstring Type;
};
class PeerAddressInfo
{
public:
	tstring Host;
	tstring TCPPort;
	tstring UDPPort;
};

typedef map<tstring, tstring> tstring_table;


inline PeerAddressInfo ParsePeerAddress(const tstring& param)
{
	PeerAddressInfo info;
	std::vector<tstring> parts;
	strings::split(std::back_inserter(parts), param, _T(':'));
	if (parts.size() == 3)
	{
		info.Host = parts[0];
		info.TCPPort = parts[1];
		info.UDPPort = parts[2];
	}
	return info;
}

inline TrackerAddressInfo ParseTrackerAddress(const tstring& param)
{
	TrackerAddressInfo info;
	ppl::Uri uri(param);
	info.Type = uri.GetScheme();
	boost::tie(info.Host, info.Port) = strings::split_pair(uri.GetPath(), SynacastUrlHostAddressInnerDelimiter);
	return info;
}

inline bool ParseTrackerAddressInfo(const TrackerAddressInfo& trackerInfo, TRACKER_ADDRESS& addr)
{
	addr.IP = ResolveHostName(trackerInfo.Host);
	addr.Port = 8000;
	if (!trackerInfo.Port.empty())
	{
		addr.Port = static_cast<UINT16>( _ttoi(trackerInfo.Port.c_str()) );
	}
	if (trackerInfo.Type == _T("tcpt"))
		addr.Type = PPL_TRACKER_TYPE_TCP;
	else if (trackerInfo.Type == _T("udpt"))
		addr.Type = PPL_TRACKER_TYPE_UDP;
	else
		return false;
	return true;
}


class trackers_output_iterator
{
public:
	explicit trackers_output_iterator(std::vector<TrackerAddressInfo>& trackers) : m_trackers(&trackers) { }

	trackers_output_iterator& operator=(const tstring& queryItem)
	{
		if ( false == queryItem.empty() )
		{
			m_trackers->push_back(ParseTrackerAddress(queryItem));
		}
		return (*this);
	}
	trackers_output_iterator& operator*() { return (*this); }
	trackers_output_iterator& operator++() { return (*this); }
	trackers_output_iterator operator++(int) { return (*this); }
protected:
	std::vector<TrackerAddressInfo>* m_trackers;
};

inline void ParseTrackers(const tstring& param, std::vector<TrackerAddressInfo>& trackers)
{
	trackers.clear();
	trackers_output_iterator outputer(trackers);
	strings::split(outputer, param, SynacastUrlDelimiter2);
}

class peers_output_iterator
{
public:
	explicit peers_output_iterator(std::vector<PeerAddressInfo>& peers) : m_peers(&peers) { }

	peers_output_iterator& operator=(const tstring& queryItem)
	{
		if ( false == queryItem.empty() )
		{
			m_peers->push_back(ParsePeerAddress(queryItem));
		}
		return (*this);
	}
	peers_output_iterator& operator*() { return (*this); }
	peers_output_iterator& operator++() { return (*this); }
	peers_output_iterator operator++(int) { return (*this); }
protected:
	std::vector<PeerAddressInfo>* m_peers;
};

inline void ParsePeers(const tstring& param, std::vector<PeerAddressInfo>& peers)
{
	peers.clear();
	peers_output_iterator outputer(peers);
	strings::split(outputer, param, SynacastUrlDelimiter2);
}




class SynacastEncoding
{
public:
	static string Encode(const string& url)
	{
		Base64Encoding::GetDelimiter() = "";
		const size_t PPL_KEY_LENGTH = strlen(SynacastUrlCypherKey);
		string result;
		result.resize(url.size());
		for (size_t i = 0; i < url.size(); ++i)
		{
			size_t keyIndex = i % PPL_KEY_LENGTH;
			result[i] = url[i] + SynacastUrlCypherKey[keyIndex];
		}
		return Base64Encoding::Encode(result);
	}

	static string Decode(const string& url)
	{
		const size_t PPL_KEY_LENGTH = strlen(SynacastUrlCypherKey);

		string url2 = Base64Encoding::Decode(url);
#ifdef _DEBUG
		Base64Encoding::GetDelimiter() = "";
		string url3 = Base64Encoding::Encode(url2);
		assert(url3 == url);
#endif
		string result;
		result.resize(url2.size());
		for (size_t i = 0; i < url2.size(); ++i)
		{
			size_t keyIndex = i % PPL_KEY_LENGTH;
			result[i] = url2[i] - SynacastUrlCypherKey[keyIndex];
		}
		return result;
	}

};


class synacast_url
{
public:
	synacast_url() : m_AppType( 0 ), m_TestVersionNumber( 0 ), m_TestUpdateWaitTime(5)
	{
	}
	~synacast_url()
	{
	}

	const tstring& channel_guid() const { return m_ChannelGUID; }
	const std::vector<TrackerAddressInfo>& trackers() const { return m_Trackers; }
	const std::vector<PeerAddressInfo>& mdss() const { return m_MDSs; }

	int app_type() const { return m_AppType; }
	int test_version_number() const { return m_TestVersionNumber; }
	const tstring& test_version_string() const { return m_TestVersion; }

	bool is_test_channel() const { return !m_TestVersion.empty(); }

	/// 获取测试内核升级的最大等待时间
	int test_update_wait_time() const
	{
		return m_TestUpdateWaitTime;
	}

	void switch_to_1012()
	{
		m_TestVersion = _T("");
		m_TestVersionNumber = 0;
		assert(m_AppType == 1012);
		m_AppType = 1012;
	}

	bool parse(const string& urlstr)
	{
		tstring body;
		boost::tie(m_Scheme, body) = strings::split_pair(urlstr, _T("://"));
		string decodedUrl = SynacastEncoding::Decode(body);

		tstring_table params;
		ppl::parse_dictionary(decodedUrl, params, SynacastUrlDelimiter1, _T('='));
		STL_FOR_EACH_CONST(tstring_table, params, iter)
		{
			TRACE(_T("synacast url param %s : %s\n"), iter->first.c_str(), iter->second.c_str());
		}

		m_ChannelGUID = string_dicts::get_value_ignore_case(params, _T("channel"));
		if (m_ChannelGUID.empty())
		{
			assert(false);
			return false;
		}

		m_TestUpdateWaitTime = numeric<UINT>::parse(string_dicts::get_value_ignore_case(params, _T("tt")).c_str(), m_TestUpdateWaitTime);
		m_AppType = _ttoi(string_dicts::get_value_ignore_case(params, _T("at")).c_str());
		m_TestVersion = string_dicts::get_value_ignore_case(params, _T("tv"));
		if ( false == m_TestVersion.empty() )
		{
			std::vector<tstring> vers;
			strings::split( std::back_inserter( vers ), m_TestVersion, _T('.') );
			if ( vers.size() == 4 )
			{
				m_TestVersionNumber = _ttoi( vers[3].c_str() );
			}
		}

		tstring param = string_dicts::get_value_ignore_case(params, _T("ko"));
		if (param.empty())
		{
			assert(false);
			return false;
		}
		ParseTrackers(param, m_Trackers);
		if (m_Trackers.empty())
		{
			assert(false);
			return false;
		}

		param = string_dicts::get_value_ignore_case(params, _T("name"));
		ppl::parse_dictionary(param, m_Names, SynacastUrlDelimiter2, SynacastUrlDelimiter3);

		param = string_dicts::get_value_ignore_case(params, _T("m"));
		ParsePeers(param, m_MDSs);

		// Added by Tady, 011811: Spark!
		param = string_dicts::get_value_ignore_case(params, _T("s"));
		ParsePeers(param, m_sparks);

        m_sparkTranstype = strings::lower(string_dicts::get_value_ignore_case(params, _T("st")));
		if (m_sparkTranstype == TEXT(""))
			m_sparkTranstype = TEXT("udp");

		m_sparkLen = _ttoi(string_dicts::get_value_ignore_case(params, _T("sl")).c_str());
		if (m_sparkLen <= 0)
			m_sparkLen = 512;

		return true;
	}

	// Added by Tady, 011811: Spark.
	const std::vector<PeerAddressInfo>& sparks() const { return m_sparks; }
	const tstring& spark_trans_type() const { return m_sparkTranstype; }
	int	  spark_len() const { return m_sparkLen; }
protected:
	tstring m_Scheme;
	tstring m_ChannelGUID;
	tstring_table m_Names;
	std::vector<TrackerAddressInfo> m_Trackers;
	std::vector<PeerAddressInfo> m_MDSs;
	tstring m_TestVersion;
	int m_AppType;
	int m_TestVersionNumber;

	/// 测试内核升级的最大等待时间，单位：秒
	UINT m_TestUpdateWaitTime;

	/// Added by Tady, 011811: Spark.
	std::vector<PeerAddressInfo> m_sparks;
	tstring m_sparkTranstype;
	int m_sparkLen;
};








inline bool ParsePeerIP(const tstring& str, PEER_ADDRESS& addr)
{
	if (str.empty())
		return false;
	addr.IP = ResolveHostName(str);
	addr.TcpPort = 0;
	addr.UdpPort = 0;
	if (addr.IP == INADDR_NONE || addr.IP == INADDR_ANY)
		return false;
	return true;
}



/*
inline void ParseFrameworkPeerAddresses(const tstring& str, std::vector<PEER_ADDRESS>& peers)
{
	peers.clear();
	framework_peer_address_output_iterator outputer(peers, ParsePeerAddress);
	strings::split(outputer, str, SynacastUrlParamInnerDelimiter);
}*/
#endif