
#ifndef _LIVE_P2PCOMMON2_NEW_COMMON_IPTABLE_H_
#define _LIVE_P2PCOMMON2_NEW_COMMON_IPTABLE_H_

#include <synacast/protocol/data/NetType.h>
#include <ppl/os/module.h>
#include <ppl/os/paths.h>


class IPTable
{
public:
	typedef NET_TYPE (*FUNC_LocateIP)(UINT);

	IPTable() : m_funcLocateIP(NULL) { }

	bool Load(const tstring& baseDir)
	{
		tstring dllPath = ppl::os::paths::combine( baseDir, _T("tpi.dll") );
		if (m_dll.load(dllPath.c_str()))
		{
			m_funcLocateIP = reinterpret_cast<FUNC_LocateIP>(m_dll.get_export_item("Locate"));
			LIVE_ASSERT(m_funcLocateIP != NULL);
			return m_funcLocateIP != NULL;
		}
		return false;
	}

	NET_TYPE LocateIP(UINT ip)
	{
		NET_TYPE netType = { 0, 0, 0, 0 };
		if (m_funcLocateIP != NULL)
		{
			netType = m_funcLocateIP(ip);
		}
		return netType;
	}

	bool IsValid() const
	{
		return m_dll.is_open() && m_funcLocateIP != NULL;
	}

private:
	ppl::os::loadable_module m_dll;
	FUNC_LocateIP m_funcLocateIP;
};


inline UINT GetNetTypeDistanceValue(int x, int y)
{
	if (x == 0 || y == 0)
		return 0;
	return x == y;
}


inline UINT CalcNetTypeDistance(const NET_TYPE& x, const NET_TYPE& y)
{
	UINT isp = GetNetTypeDistanceValue(x.ISP, y.ISP);
	UINT country = GetNetTypeDistanceValue(x.Country, y.Country);
	// 是否是同一国家的同一省
	UINT province = GetNetTypeDistanceValue(x.Province, y.Province) && (x.Country == y.Country);
	// 是否是同一国家同一省的同一市
	UINT city = GetNetTypeDistanceValue(x.City, y.City) && (x.Country == y.Country) && (x.Province == y.Province);
	// isp跟city同级，province比city低一级
	return isp * 4 + city * 4 + province * 2 + country;
}

#endif