
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_SYNACASTURL_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_SYNACASTURL_H_


#include <ppl/text/base64.h>
//#include <ppl/stl/iterator.h>
#include <ppl/net/uri.h>
#include <synacast/protocol/types.h>

const TCHAR SynacastUrlCypherKey[] = TEXT("trinity");
const TCHAR SynacastUrlParamInnerDelimiter = TEXT(';');
const TCHAR SynacastUrlUDPTracker[] = TEXT("udpt");
const TCHAR SynacastUrlTCPTracker[] = TEXT("tcpt");

const TCHAR SynacastUrlHostAddressInnerDelimiter = TEXT(':');



class TrackerAddressInfo
{
public:
	tstring Host;
	tstring Port;
	tstring Type;
};
typedef vector<TrackerAddressInfo> TrackerAddressInfoCollection;


inline TrackerAddressInfo ParseTrackerAddress(const tstring& param)
{
	TrackerAddressInfo info;
	Uri uri(param);
	info.Type = uri.GetScheme();
	tie(info.Host, info.Port) = strings::split_pair(uri.GetPath(), SynacastUrlHostAddressInnerDelimiter);
	return info;
}

inline bool ParseTrackerAddressInfo(const TrackerAddressInfo& trackerInfo, TRACKER_ADDRESS& addr)
{
	addr.IP = ResolveHostName(trackerInfo.Host.c_str());
	addr.Port = 8000;
	if (!trackerInfo.Port.empty())
	{
		addr.Port = _ttoi(trackerInfo.Port.c_str());
	}
	if (trackerInfo.Type == TEXT("tcpt"))
		addr.Type = PPL_TRACKER_TYPE_TCP;
	else if (trackerInfo.Type == TEXT("udpt"))
		addr.Type = PPL_TRACKER_TYPE_UDP;
	else
		return false;
	return true;
}



class trackers_output_iterator
{
public:
	explicit trackers_output_iterator(TrackerAddressInfoCollection& trackers) : m_trackers(trackers) { }

	trackers_output_iterator& operator=(const tstring& queryItem)
	{
		m_trackers.push_back(ParseTrackerAddress(queryItem));
		return (*this);
	}
	trackers_output_iterator& operator*() { return (*this); }
	trackers_output_iterator& operator++() { return (*this); }
	trackers_output_iterator operator++(int) { return (*this); }
protected:
	TrackerAddressInfoCollection& m_trackers;
};


class SynacastUrl
{
public:
	SynacastUrl(const tstring& urlstr) : m_isTestChannel(false)
	{
		Parse(urlstr);
	}
	~SynacastUrl()
	{
	}

	const GUID& GetChannelGUID() const { return m_channelGUID; }
	const TrackerAddressInfoCollection& GetTrackers() const { return m_trackers; }

	bool IsTestChannel() const
	{
		return m_isTestChannel;
	}

	tstring Encode(const tstring& url)
	{
		Base64Encoding::GetDelimiter() = "";
		const size_t PPL_KEY_LENGTH = _tcslen(SynacastUrlCypherKey);
		tstring result;
		result.resize(url.size());
		for (size_t i = 0; i < url.size(); ++i)
		{
			size_t keyIndex = i % PPL_KEY_LENGTH;
			result[i] = url[i] + SynacastUrlCypherKey[keyIndex];
		}
		return Base64Encoding::Encode(result);
	}

	tstring Decode(const tstring& url)
	{
		const size_t PPL_KEY_LENGTH = _tcslen(SynacastUrlCypherKey);

		tstring url2 = Base64Encoding::Decode(url);
		Base64Encoding::GetDelimiter() = "";
		tstring url3 = Base64Encoding::Encode(url2);
		LIVE_ASSERT(url3 == url);
		tstring result;
		result.resize(url2.size());
		for (size_t i = 0; i < url2.size(); ++i)
		{
			size_t keyIndex = i % PPL_KEY_LENGTH;
			result[i] = url2[i] - SynacastUrlCypherKey[keyIndex];
		}
		return result;
	}


	void Parse(const tstring urlstr)
	{
		tstring scheme, body;
		tie(scheme, body) = strings::split_pair(urlstr, TEXT("://"));
		tstring decodedUrl = Decode(body);
		Uri uri(scheme, TEXT(""), decodedUrl);

		LIVE_ASSERT(strings::lower(scheme) == TEXT("synacast"));
		LIVE_ASSERT(!body.empty());

		tstring param = uri.GetParam(TEXT("channel"));
		LIVE_ASSERT(!param.empty());
		m_channelGUID.Parse(param);

		param = uri.GetParam(TEXT("ko"));
		LIVE_ASSERT(!param.empty());
		ParseTrackers(param);

		param = uri.GetParam(TEXT("name"));
		//LIVE_ASSERT(!param.empty());
		m_name = param;

		param = uri.GetParam(TEXT("M"));
	}
	void ParseTrackers(const tstring& param)
	{
		trackers_output_iterator outputer(m_trackers);
		strings::split(outputer, param, SynacastUrlParamInnerDelimiter);
	}

protected:
	Guid m_channelGUID;
	TrackerAddressInfoCollection m_trackers;
	tstring m_name;
	PeerAddressCollection m_mdsList;
	bool m_isTestChannel;
};
#endif


