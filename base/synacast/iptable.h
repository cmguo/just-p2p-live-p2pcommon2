
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_IPTABLE_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_IPTABLE_H_


#include <ppl/mswin/ado.h>


class IPTable
{
public:
	IPTable()
	{
	}
	bool Open()
	{
		Path path;
		Module().BuildLocalFilePath(path, "city_ip.mdb");
		try
		{
			m_conn.OpenJet(path);
		}
		catch (_com_error& e)
		{
			TRACE("open city iptable database failed %s\n", e.ErrorMessage());
			return false;
		}
		return true;
	}
	bool IsOpen() const
	{
		return m_conn.IsOpen();
	}

	string Query(unsigned long ip)
	{
		ip = ntohl(ip);
		string sqlText = strings::format("select * from ip where start_ip <= %lu and %lu <= end_ip;", ip, ip);
		m_dt.Query(sqlText.c_str(), m_conn);
		if (!m_dt.IsValid() || !m_dt.HasData())
			return "";
		unsigned long startIP = m_dt.GetReal("start_ip");
		unsigned long endIP = m_dt.GetReal("end_ip");
		assert(startIP <= ip && ip <= endIP);
		string province = m_dt.GetString("province");
		string city = m_dt.GetString("city");
		string isp = m_dt.GetString("isp");
		return isp + " " + province + " " + city;
	}

private:
	AdoConnection m_conn;
	AdoDataTable m_dt;
};

#endif
