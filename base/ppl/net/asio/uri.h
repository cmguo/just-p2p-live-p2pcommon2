#ifndef _LIVE_P2PCOMMON2_BASE_PPL_NET_ASIO_URI_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_NET_ASIO_URI_H_

#include <string>
using std::string;

const size_t NOPOS = static_cast<size_t>(-1);

class Uri
{
public:
	Uri(const string &url);

	Uri(const char *url);

	bool replacepath(string file_name);

	bool replacefile(string path_name);

	string getfileurl();

	string getrequest();

	string geturl()
	{
		return url_;
	}

	string getprotocol();

	string getdomain();


	string getport();

	string getpath();

	string getfile();

	string getparameter(const string& key);

	string getparameter();

	string gethost();

	string getreferer();

private:
	void parseurl();

private:
	string url_;
	size_t protcalpos_;
	size_t host_start_pos_;
	size_t host_end_pos_;
	size_t domain_end_pos_;
	size_t port_start_pos_;
	size_t port_end_pos_;
	size_t path_start_pos_;
	size_t path_end_pos_;
	size_t file_start_pos_;
	size_t file_end_pos_;
	size_t param_start_pos_;
	size_t param_end_pos_;
	size_t request_start_pos_;
	size_t request_end_pos_;
};


inline Uri::Uri( const string &url ) : url_(url),
protcalpos_(NOPOS),
host_start_pos_(NOPOS),
host_end_pos_(NOPOS),
domain_end_pos_(NOPOS),
port_start_pos_(NOPOS),
port_end_pos_(NOPOS),
path_start_pos_(NOPOS),
path_end_pos_(NOPOS),
file_start_pos_(NOPOS),
file_end_pos_(NOPOS),
param_start_pos_(NOPOS),
param_end_pos_(NOPOS),
request_start_pos_(NOPOS),
request_end_pos_(NOPOS)
{
	parseurl();
}

inline Uri::Uri( const char *url ) : url_(url),
protcalpos_(NOPOS),
host_start_pos_(NOPOS),
host_end_pos_(NOPOS),
domain_end_pos_(NOPOS),
port_start_pos_(NOPOS),
port_end_pos_(NOPOS),
path_start_pos_(NOPOS),
path_end_pos_(NOPOS),
file_start_pos_(NOPOS),
file_end_pos_(NOPOS),
param_start_pos_(NOPOS),
param_end_pos_(NOPOS),
request_start_pos_(NOPOS),
request_end_pos_(NOPOS)
{
	parseurl();
}

inline bool Uri::replacepath( string file_name )
{
	url_ = getreferer() + file_name;
	return true;
}

inline bool Uri::replacefile( string path_name )
{
	if (file_start_pos_ != NOPOS)
	{
		url_ = url_.substr(0, file_start_pos_) + path_name;

		return true;
	}
	else
		return false;
}

inline string Uri::getfileurl()
{
	if (file_end_pos_ != NOPOS)
		return url_.substr(0, file_end_pos_);
	else
		return "";
}

inline string Uri::getrequest()
{
	if (request_start_pos_ != NOPOS)
		return url_.substr(request_start_pos_, request_end_pos_ - request_start_pos_);
	else
		return "";
}

inline string Uri::getprotocol()
{
	if(protcalpos_ != NOPOS)
		return url_.substr(0, protcalpos_);
	else
		return "http";
}

inline string Uri::getport()
{
	if(port_start_pos_ != NOPOS)
		return url_.substr(port_start_pos_, port_end_pos_ - port_start_pos_);
	else
		return "80";
}

inline string Uri::getpath()
{
	if(path_start_pos_ != NOPOS)
		return url_.substr(path_start_pos_, path_end_pos_ - path_start_pos_);
	else
		return "/";
}

inline string Uri::getfile()
{
	if(file_start_pos_ != NOPOS)
		return url_.substr(file_start_pos_, file_end_pos_ - file_start_pos_);
	else
		return "";
}

inline string Uri::getparameter( const string& key )
{
	string para = getparameter();
	size_t startp_, endp_;
	if((startp_ = para.find(key)) == string::npos)
		return "";
	else
	{
		startp_ = para.find('=', startp_) + 1;
		endp_ = para.find('&', startp_);
		if (endp_ == string::npos)
			endp_ = para.length();
		return para.substr(startp_, endp_ - startp_);
	}
}

inline string Uri::getparameter()
{
	if(param_start_pos_ != NOPOS)
		return url_.substr(param_start_pos_, param_end_pos_ - param_start_pos_);
	else
		return "";
}

inline string Uri::gethost()
{
	if (host_start_pos_ != NOPOS)
		return url_.substr(host_start_pos_, host_end_pos_ - host_start_pos_);
	else
		return "";
}

inline string Uri::getreferer()
{
	if (host_end_pos_ != NOPOS)
		return url_.substr(0, host_end_pos_);
	else
		return "";
}

inline void Uri::parseurl()
{
	string::size_type beginpos = 0, endpos = 0;

	beginpos = url_.find(':', 0);
	endpos = url_.find('/', 0);
	if(beginpos != string::npos && beginpos < endpos && beginpos < url_.find('.', 0))
	{
		protcalpos_ = beginpos;
		beginpos = url_.find('/', endpos + 1) + 1;
		host_start_pos_ = beginpos;
	}
	else
	{
		host_start_pos_ = 0;
		beginpos = host_start_pos_;
	}

	endpos = url_.find(':', beginpos);
	if(endpos != string::npos)
	{
		port_start_pos_ = endpos + 1;
		beginpos = endpos;
		domain_end_pos_ = endpos;
	}

	endpos = url_.find('/', beginpos);
	if(endpos != string::npos)
	{
		if (domain_end_pos_ == NOPOS)
			domain_end_pos_ = endpos;
		if (port_start_pos_ != NOPOS)
			port_end_pos_ = endpos;
		path_start_pos_ = endpos;
		request_start_pos_ = endpos;
		host_end_pos_ = endpos;
	}
	else
	{
		url_ = url_ + '/';
		endpos = url_.find('/', beginpos);
		path_start_pos_ = endpos;
		if (domain_end_pos_ == NOPOS)
			domain_end_pos_ = url_.length() - 1;
		if (port_start_pos_ != NOPOS)
			port_end_pos_ = endpos;
		request_start_pos_ = endpos;
		host_end_pos_ = endpos;
	}

	beginpos = endpos;

	//endpos = url_.find('.', beginpos);
	//if(endpos != string::npos)
	//{
	//	beginpos = url_.rfind('/', endpos) + 1;
	//	file_start_pos_ = beginpos;
	//}

	beginpos = url_.find('?', beginpos);
	if(beginpos != string::npos)
	{
		param_start_pos_ = beginpos + 1;
		file_end_pos_ = beginpos;
		path_end_pos_ = beginpos;

		file_start_pos_= url_.rfind('/', beginpos) + 1;

	} else
	{
		file_end_pos_ = url_.length();
		path_end_pos_ = url_.length();

		file_start_pos_= url_.rfind('/', url_.length()) + 1;
	}

	beginpos = url_.rfind('#', url_.length() - 1);
	if(beginpos != string::npos)
	{
		request_end_pos_ = beginpos;
		param_end_pos_ = beginpos;
	} else
	{
		request_end_pos_ = url_.length();
		param_end_pos_ = url_.length();
	}
}

inline string Uri::getdomain()
{
	if(host_start_pos_!= NOPOS)
		return url_.substr(host_start_pos_, domain_end_pos_- host_start_pos_);
	else
		return "";
}

#endif
