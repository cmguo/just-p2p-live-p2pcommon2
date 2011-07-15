#ifndef _LIVE_P2PCOMMON2_BASE_PPL_UTIL_APP_PARAMS_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_UTIL_APP_PARAMS_H_

#include <ppl/data/strings.h>
#include <ppl/data/numeric.h>
#include <vector>
#include <algorithm>


namespace ppl { namespace util {

class app_params
{
public:
	const std::vector<tstring>& get_params() const { return m_params; }

	void load(const tstring& arg)
	{
		m_params.clear();
		strings::split(std::back_inserter(m_params), arg, _T(' '));
		//boost::algorithm::split(m_params, arg, boost::is_any_of(" \t"));
	}

	bool load(int argc, TCHAR* argv[])
	{
		m_params.clear();
		if (argc < 1)
			return false;
		m_params.clear();
		m_params.reserve(argc - 1);
		for (int i = 1; i < argc; ++i)
		{
			m_params.push_back(tstring(argv[i]));
		}
		return true;
	}
	bool hash_param( const tstring& name ) const
	{
		return std::find(m_params.begin(), m_params.end(), name) != m_params.end();
	}
	tstring get_param( const tstring& name ) const
	{
		tstring result;
		std::vector<tstring>::const_iterator pos = std::find(m_params.begin(), m_params.end(), name);
		if (pos != m_params.end())
		{
			++pos;
			if (pos != m_params.end())
			{
				result = *pos;
			}
		}
		return result;
	}


	template <typename ValueT>
	ValueT get_numeric_param( const tstring& name, ValueT defaultValue ) const
	{
		ValueT val;
		if ( try_get_numeric_param( name, val ) )
			return val;
		return defaultValue;
	}
	template <typename ValueT>
	bool try_get_numeric_param( const tstring& name, ValueT& val ) const
	{
		tstring s = get_param( name );
		if ( s.empty() )
			return false;
		return numeric<ValueT>::try_parse( s, val );
	}

private:
	std::vector<tstring> m_params;
};

} }

#endif