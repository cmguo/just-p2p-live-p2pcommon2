
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_QQWRY_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_QQWRY_H_

#include <ppl/os/file.h>
#include <ppl/os/file_mapping.h>
#include <sstream>
using std::ostringstream;
using std::make_pair;
using std::pair;


typedef unsigned int IP_TYPE;

const int  IP_SIZE = 4;
const int  OFFSET_SIZE = 3;
const int  INDEX_RECORD_SIZE = IP_SIZE + OFFSET_SIZE;


struct IPLocation
{
public:
	IPLocation() : StartIP(0), EndIP(0)
	{
	}

	IPLocation(u_long startIP, u_long endIP, const string& country, const string& area) : StartIP(startIP), EndIP(endIP), Country(country), Area(area)
	{
	}

	u_long StartIP;
	u_long EndIP;
	string Country;
	string Area;
};


/*
class IpLocater{
private:
	File dbfile;
	size_t first_index;
	size_t last_index;
	enum {REDIRECT_MODE_1 = 0x01,REDIRECT_MODE_2 = 0x02};
protected:

public:
	typedef IPLocation result_type;

	IpLocater() : first_index(0), last_index(0)
	{
	}
	~IpLocater()
	{
	}

	bool IsOpen() const
	{
		return this->dbfile.IsOpen();
	}
	bool Open( const char* dbfilename )
	{
		//if (!dbfile.Open(dbfilename, "rb"))
		//	return false;
		if (!dbfile.Open(dbfilename, FILE_GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, OPEN_EXISTING))
			return false;

		dbfile.Read(&first_index,sizeof(size_t));
		dbfile.Read(&last_index,sizeof(size_t));
		return true;
	}

	int getRecordCount() const
	{
		return (last_index - first_index ) / INDEX_RECORD_SIZE + 1;
	}
	
	string readString(const int offset = 0 )
	{
		if ( offset ) {
			dbfile.Seek(offset,SEEK_SET);
		}
	
		char ch = dbfile.ReadByte();
		ostringstream sstr ;
		while ( ch != 0 && ch != EOF ) {
			sstr << ch;
			ch = dbfile.ReadByte();
		}
		return sstr.str();
	}

	inline int readInt3(const int offset = 0 )
	{
		if ( offset ) {
			dbfile.Seek(offset,SEEK_SET);
		}
	
		int result = 0;
		dbfile.Read(&result,3);
		return result;
	}

	string readAreaAddr(const int offset = 0){
		if ( offset ) {
			dbfile.Seek(offset, SEEK_SET);;
		}
		char b = dbfile.ReadByte();
		if ( b == REDIRECT_MODE_1 || b == REDIRECT_MODE_2) {
			int areaOffset=0;
			dbfile.Read(&areaOffset,3);
			if ( areaOffset ) {
				return readString( areaOffset );
			}else{
				return "Unkown";
			}
		}else{
			dbfile.Seek(-1,SEEK_CUR);
			return readString();
		}
	}

	unsigned int readLastIp(const int offset){
		dbfile.Seek(offset, SEEK_SET);
		unsigned int ip = 0;
		dbfile.Read(&ip,sizeof(unsigned int));
		return ip;
	}

	IPLocation readFullAddr(const int offset,int ip = 0, u_long startIP = 0 ){
		u_long endIP = 0;
		string country, area;

		dbfile.Seek(offset, SEEK_SET);
		dbfile.Read(&endIP, 4);

		char ch = dbfile.ReadByte();
		if ( ch == REDIRECT_MODE_1 ) {
			int countryOffset = 0;
			dbfile.Read(&countryOffset,3);
			
			dbfile.Seek(countryOffset,SEEK_SET);
			char byte = dbfile.ReadByte();
			if ( byte == REDIRECT_MODE_2 ) {
				int p = 0;
				dbfile.Read(&p,3);
				country = readString(p);
				dbfile.Seek(countryOffset+4,SEEK_SET);
			}else{
				country = readString(countryOffset);
			}
			area = readAreaAddr(); // current position
		}else if ( ch == REDIRECT_MODE_2 ) {
			int p = 0;
			dbfile.Read(&p,3);
			country = readString(p);
			area = readAreaAddr( offset + 8);
		}else{
			dbfile.Seek(-1,SEEK_CUR);
			country = readString();
			area = readAreaAddr();
		}

		return IPLocation(startIP, endIP, country, area);
	}

	int find(unsigned int ip,int left ,int right){
		if ( right-left == 1) {
			return left;
		}else{
			int middle = (left + right) / 2;
			
			int offset = first_index + middle * INDEX_RECORD_SIZE;
			dbfile.Seek(offset, SEEK_SET);;
			unsigned int new_ip = 0;
			dbfile.Read(&new_ip,sizeof(unsigned int));
			
			if ( ip >= new_ip ) {
				return find(ip,middle,right);
			}else{
				return find(ip,left,middle);
			}
		}
	}

	string getIpAddr( unsigned int ip)
	{
		IPLocation location = Locate( ip );
		return location.Country + " -- " + location.Area;
	}

	IPLocation Locate(unsigned int ip)
	{
		int index = find(ip,0, getRecordCount() - 1 );
		int index_offset = first_index + index * INDEX_RECORD_SIZE;
		int addr_offset = 0;
		u_long startIP = 0;
		dbfile.Seek(index_offset,SEEK_SET);
		dbfile.Read(&startIP, 4);
		dbfile.Read(&addr_offset,3);
		return readFullAddr( addr_offset,ip, startIP );
	}
};
*/



class IPLocator
{
private:
	ppl::os::file m_dbfile;
	ppl::os::file_mapping m_mmf;
	size_t first_index;
	size_t last_index;
	const unsigned char* m_buf;
	enum {REDIRECT_MODE_1 = 0x01,REDIRECT_MODE_2 = 0x02};
protected:

public:
	typedef IPLocation result_type;

	IPLocator() : first_index(0), last_index(0), m_buf(NULL)
	{
	}
	~IPLocator()
	{
	}

	bool IsOpen() const
	{
		return this->m_dbfile.is_open() && m_mmf.is_mapped() && m_buf != NULL;
	}
	bool Open( const char* dbfilename )
	{
		//if (!dbfile.Open(dbfilename, "rb"))
		//	return false;
		if (!m_dbfile.open_reading(dbfilename))
			return false;

		DWORD size = m_dbfile.get_size32();
		if (!m_mmf.create(m_dbfile.native_handle(), NULL, size, PAGE_READONLY))
			return false;
		m_mmf.map_all(FILE_MAP_READ);
		m_buf = reinterpret_cast<const unsigned char*>(m_mmf.get_view());
		if (NULL == m_buf)
			return false;

		first_index = READ_MEMORY(m_buf, size_t);
		last_index = READ_MEMORY(m_buf + 4, size_t);
		return true;
	}

	int getRecordCount() const
	{
		return (last_index - first_index ) / INDEX_RECORD_SIZE + 1;
	}
	
	string readString(const int offset)
	{
		char ch = m_buf[offset];
		int index = 1;
		ostringstream sstr ;
		while ( ch != 0 && ch != EOF )
		{
			sstr << ch;
			ch = m_buf[offset + index];
			++index;
		}
		return sstr.str();
	}

	inline int readInt3(const int offset)
	{
		int result = m_buf[offset] | (m_buf[offset + 1] << 8) | (m_buf[offset + 2] << 16);
		return result;
	}

	string readAreaAddr(const int offset)
	{
		char b = m_buf[offset];
		if ( b == REDIRECT_MODE_1 || b == REDIRECT_MODE_2)
		{
			int areaOffset=readInt3(offset + 1);
			if ( areaOffset )
			{
				return readString( areaOffset );
			}
			else
			{
				return "";
			}
		}
		else
		{
			return readString(offset);
		}
	}


	IPLocation readFullAddr(const int offset, u_long ip, u_long startIP)
	{
		string country, area;
		u_long endIP = READ_MEMORY(m_buf + offset, u_long);
		char ch = m_buf[offset + 4];
		if ( ch == REDIRECT_MODE_1 )
		{
			int countryOffset = readInt3(offset + 5);
			int areaOffset = 0;
			char byte = m_buf[countryOffset];
			if ( byte == REDIRECT_MODE_2 )
			{
				int p = readInt3(countryOffset+1);
				country = readString(p);
				areaOffset = countryOffset+4;
			}
			else
			{
				country = readString(countryOffset);
				areaOffset = countryOffset + country.size() + 1;
			}
			area = readAreaAddr(areaOffset);
		}
		else if ( ch == REDIRECT_MODE_2 )
		{
			int countryOffset = readInt3(offset + 5);
			country = readString(countryOffset);
			area = readAreaAddr( offset + 8);
		}
		else
		{
			country = readString(offset + 4);
			area = readAreaAddr(offset + 4 + country.size() + 1);
		}

		return IPLocation(startIP, endIP, country, area);
	}

	int find(unsigned int ip,int left ,int right)
	{
		if ( right-left == 1)
		{
			return left;
		}
		else
		{
			int middle = (left + right) / 2;
			
			int offset = first_index + middle * INDEX_RECORD_SIZE;
			unsigned int new_ip = READ_MEMORY(m_buf + offset, u_long);
			
			if ( ip >= new_ip )
			{
				return find(ip,middle,right);
			}
			else
			{
				return find(ip,left,middle);
			}
		}
	}

	string getIpAddr( unsigned int ip)
	{
		IPLocation location = Locate( ip );
		return location.Country + " -- " + location.Area;
	}

	IPLocation Locate(unsigned int ip)
	{
		int index = find(ip,0, getRecordCount() - 1 );
		int index_offset = first_index + index * INDEX_RECORD_SIZE;
		u_long startIP = READ_MEMORY(m_buf + index_offset, u_long);
		int addr_offset = readInt3(index_offset + 4);
		return readFullAddr( addr_offset,ip, startIP );
	}
};

#endif