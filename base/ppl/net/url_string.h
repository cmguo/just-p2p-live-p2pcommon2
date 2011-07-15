/*
  $Id: url_string.cc,v 1.4 2002/04/04 17:30:29 t16 Exp $

  URL string implementation.
  Copyright (C) 2002  Kasper Peeters <k.peeters@damtp.cam.ac.uk>

  This program and its components are free software; you can
  redistribute it and/or modify it under the terms of the GNU General
  Public License as published by the Free Software Foundation; either
  version 2, or (at your option) any later version.
 
  This program and its components are distributed in the hope that it
  will be useful, but WITHOUT ANY WARRANTY; without even the implied
  warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
  the GNU General Public License for more details.
 
  You should have received a copy of the GNU General Public License in
  the file COPYING accompanying this program; if not, write to the
  Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef _LIVE_P2PCOMMON2_BASE_PPL_NET_URL_STRING_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_NET_URL_STRING_H_


#include <string>
#include <iostream>
#include <sstream>
#include <utility>
#include <algorithm>
#include <stdexcept>



class url_string {
	public:
		url_string(const std::string& url, const std::string& referrer);
		url_string(const std::string& url, const url_string& referrer);
		url_string(const url_string& url);
		url_string(const std::string& url);
		url_string();
		virtual ~url_string();
		
		std::string       get_scheme(void) const;         // 'scheme:'
		std::string       get_original(void) const;       // string used in constructor, unparsed
		std::string       get_referrer(void) const;
		std::string       get_url(void) const;            // full url
		std::string       get_net_path(void) const;       // network host
		std::string       get_absolute_path(void) const;  // always return full path
		std::string       get_relative_path(void) const;  // in case the url was relative
		std::string       get_object(void) const;         // path removed, just the file
		std::string       get_parameters(void) const;     // ';parameters'
		int             	get_port(void) const;
		std::string       get_query(void) const;          // '?query'
		std::string       get_fragment(void) const;       // '#fragment'
		std::string       get_username() const;
		std::string       get_passwd() const;
		
		url_string& operator=(const url_string &);
		
		void              set_username(const std::string&);
		void              set_passwd(const std::string&);
	protected:
		virtual int default_port(void) const;

		/* Decode the URL as if it were a relative URL starting at iterator */
		/* All the following functions assuming they are working on original_url */
		virtual void DoRelDecode(std::string::const_iterator) const;
		virtual std::string::const_iterator DecodeScheme(std::string::const_iterator) const;
		virtual std::string::const_iterator DecodeNetPath(std::string::const_iterator) const;
		virtual std::string::const_iterator DecodeAbsPath(std::string::const_iterator) const;
		virtual std::string::const_iterator DecodeRelPath(std::string::const_iterator) const;
		virtual std::string::const_iterator DecodeParams(std::string::const_iterator) const;
		virtual std::string::const_iterator DecodeQuery(std::string::const_iterator) const;
		virtual std::string::const_iterator DecodeFragment(std::string::const_iterator) const;

		/* Called if the URL starts with scheme:, if you don't do anything
			(default), then DoRelDecode is called after scheme: */
		virtual std::string::const_iterator HandleAbs(std::string::const_iterator) const;
		void         finish_decoding_(void) const;
		virtual void fixup_(void) const;

		std::string original_url;     /* What was given in constructor */
		std::string referrer;         /* What URL this was embedded in (copy of it) */

		mutable std::string url;           /* A non-relative URL created from above */
		mutable std::string scheme;       /* First section of URL */
		mutable std::string net_path;     /* Hostname, usually */
		mutable std::string abs_path;     /* Absolute path */
		mutable std::string rel_path;     /* Relative path */
		mutable std::string params;
		mutable std::string query;
		mutable std::string fragment;
		mutable std::string passwd;
		mutable std::string username;
		mutable int         port;
		
		mutable bool is_relative_;              /* If starts with scheme:, is not relative*/
		mutable bool decoding_finished_;        /* Whether we have finished parsing the URL */
};


inline url_string::url_string(const url_string &u) 
	{
	referrer = u.referrer;
	original_url = u.original_url;
	decoding_finished_ = u.decoding_finished_;
	if(decoding_finished_) {
		scheme = u.scheme;
		url = u.url;
		net_path = u.net_path;
		abs_path = u.abs_path;
		rel_path = u.rel_path;
		params = u.params;
		port = u.port;
		if(port==0) port=default_port(); //is this correct?
		query = u.query;
		fragment = u.fragment;
		username = u.username;
		passwd = u.passwd;
		}
	}

inline url_string::url_string(const std::string& u) 
	: original_url(u), decoding_finished_(0) 
	{
	//port=default_port();
	}

inline url_string::url_string(const std::string& u, const std::string& ref) 
	: original_url(u), decoding_finished_(0), referrer(ref)
	{
	//port=default_port(); why set ports on not decoded urls? decoding should set those
	}

inline url_string::url_string(const std::string& u, const url_string& ref) 
	: original_url(u), decoding_finished_(0), referrer(ref.get_url())
	{
	//port=default_port(); why set ports on not decoded urls? decoding should set those
	}

inline url_string::url_string() 
	{
	port=default_port();
	decoding_finished_=1; //no use to try anything on nothing
	//is_relative ?
	}

inline url_string::~url_string() 
	{
	}

inline url_string & url_string::operator=(const url_string &u) 
	{
	//port=default_port();
	referrer = u.referrer;
	original_url = u.original_url;
	decoding_finished_ = u.decoding_finished_;
	if(decoding_finished_) {
		scheme = u.scheme;
		url = u.url;
		net_path = u.net_path;
		abs_path = u.abs_path;
		rel_path = u.rel_path;
		params = u.params;
		port = u.port;
		if(port==0) port=default_port();
		query = u.query;
		fragment = u.fragment;
		username = u.username;
		passwd = u.passwd;
		}
	return *this;
	}

inline void url_string::finish_decoding_(void) const
	{
	if(decoding_finished_) return;
	std::string::const_iterator loc;
	loc = DecodeScheme(original_url.begin());
	if(is_relative_) { // No scheme found in original_url.
		DoRelDecode(loc);
		} 
	else {// It may still be relative. 
		loc = HandleAbs(loc);
		if(loc != original_url.end()) { // They left us some work to do.
			DoRelDecode(loc);
			}
		}
	fixup_();
	decoding_finished_ = 1;
	}

inline int url_string::default_port(void) const
	{
	return 0;
	}

inline void url_string::DoRelDecode(std::string::const_iterator loc) const
	{
	std::string::const_iterator loc2;
	loc=DecodeNetPath(loc);
	if((loc2 = DecodeAbsPath(loc)) == loc) {
		loc2 = DecodeRelPath(loc);
		}
	loc = DecodeParams(loc2);
	loc = DecodeQuery(loc);
	DecodeFragment(loc);	
	}

inline std::string::const_iterator url_string::DecodeScheme(std::string::const_iterator loc) const
	{
	//starts at loc searches for a "scheme" and returns
	//the position for the next decoding
	std::string::const_iterator orig = loc, end=original_url.end();
	//skip spaces
	scheme="";
	while(loc < end) {
		if(isalnum(*loc)||*loc=='.') {
			loc++;
			} else break;
		}
	//step through the letters which are supposed to be the scheme    
	while(loc < end) {
		if(isalpha(*loc))
			loc++;
		else
			break;
		}
	
	if(loc==end || *loc != ':') {
		is_relative_ = true;
		if(referrer.size()!=0)
			scheme = url_string(referrer).get_scheme();
		if(scheme.size()==0)
			throw std::logic_error("no scheme, and no scheme in referrer either");
		return orig;
		}
	is_relative_ = false;
	scheme = original_url.substr(std::distance(original_url.begin(), orig),
										  std::distance(orig, loc));
	std::transform(scheme.begin(),scheme.end(),scheme.begin(),tolower);
	loc++; // eat ':'
	return loc;
	}

inline std::string::const_iterator url_string::DecodeNetPath(std::string::const_iterator loc)  const
	{
	if(referrer.size()!=0) {
		//initilize those, if we bail out we use those of the referrer
		url_string tmp(referrer);
		username=tmp.get_username();
		passwd=tmp.get_passwd();
		port=tmp.get_port();
		if(port==0) port=default_port();
		net_path=tmp.get_net_path();
		} 
	else
		{
		port=default_port();
		}
	std::string::const_iterator orig = loc;
	if(loc == original_url.end()) return loc;
	/* Eat spaces */
	while(loc != original_url.end() && isspace(*loc)) {
	    loc++;
	    }
	if(loc == original_url.end()) return orig;
	
	if(*loc == '/') loc++; /* this could be more terse */
	else return orig;
	if(loc == original_url.end()) return orig;
	
	if(*loc == '/') loc++;
	else return orig;
	
	if(loc == original_url.end()) return orig;
	//ok there should be some host indicator
	//reset username etc
	username.erase();
	port=default_port();
	passwd.erase();
	
	orig = loc;
	/* The next line is not exactly correct -- but unless someone
	   wants to figure out international stuff,  this will work */
	while(loc != original_url.end() && *loc!='/' && *loc!='#') {
		loc++;
		}
	
	net_path = original_url.substr(std::distance(original_url.begin(), orig), 
											 std::distance(orig,loc));
	std::string::iterator i;
	std::string::reverse_iterator ri=std::find(net_path.rbegin(),net_path.rend(),'@');
	if(ri!=net_path.rend()) {
		username=net_path.substr(0,std::distance(ri, net_path.rend()));
		net_path=net_path.substr(std::distance(ri, net_path.rend()), 
										 std::distance(net_path.rbegin(), ri));
		//check username for passwd
		i=find(username.begin(),username.end(),':');
		if(i!=username.end()) {
			i++;
			passwd=username.substr(std::distance(username.begin(),i), std::distance(i,username.end())-1);
			username=username.substr(0,std::distance(username.begin(),i)-1);
			}
		}
	
	i=std::find(net_path.begin(),net_path.end(),':');
	if(i!=net_path.end()) {
		std::string::iterator nameend=i;
		i++;
		port=0;
		while(i!=net_path.end() && isdigit(*i)) {
			port = port * 10;
			port += *i - '0';
			i++;
			}
		net_path=net_path.substr(0,std::distance(net_path.begin(),nameend));
		} 
	return loc;
	}

inline std::string::const_iterator url_string::DecodeAbsPath(std::string::const_iterator loc)  const
	{
	std::string::const_iterator orig=loc,end=original_url.end();
	abs_path="";
	if(loc == end) {
		abs_path="/";
		return loc;
		}
	if(*loc != '/') return loc;
	orig = loc;
	while(loc < end && *loc!=';' && *loc!='#' && *loc!='?') {
		loc++;
		}
	abs_path = original_url.substr(std::distance(original_url.begin(), orig), 
											 std::distance(orig,loc));

	return loc;
	}

inline std::string::const_iterator url_string::DecodeRelPath(std::string::const_iterator loc) const
	{
	rel_path="";
	std::string::const_iterator orig=loc, end=original_url.end();
	if(loc == end) return loc;
	while(loc < end && *loc!=';' && *loc!='#' && *loc!='?') loc++;
	rel_path = original_url.substr(std::distance(original_url.begin(), orig),
											 std::distance(orig,loc));

	if (referrer.size()!=0) 
		abs_path = url_string(referrer).get_absolute_path();

	if (abs_path.empty()) {
		abs_path=rel_path;
		return loc;
		}
	if (rel_path.empty()) return loc;
	
	int i=abs_path.size()-1;
	while(i>0 && abs_path[i]!='/') i--;
	if (abs_path[i]!='/') {
		abs_path=rel_path;
		return loc;
		}
	abs_path = abs_path.substr(0,i+1)+rel_path;
	
	return loc;
	}

inline std::string::const_iterator url_string::DecodeParams(std::string::const_iterator loc)  const
	{
	params="";
	std::string::const_iterator orig=loc, end=original_url.end();
	if(loc == end) return loc;
	while(loc < end && *loc!='?' && *loc !='#') loc++;
	if (loc-orig>0) 
		params = original_url.substr(std::distance(original_url.begin(), orig)+1,
											  std::distance(orig, loc)-1);
	return loc;
	}

inline std::string::const_iterator url_string::DecodeQuery(std::string::const_iterator loc)  const
	{
	query="";
	std::string::const_iterator orig=loc, end=original_url.end();
	if(loc == end) return loc;
	while(loc < end && *loc !='#') loc++;
	if (loc-orig>0) 
		query = original_url.substr(std::distance(original_url.begin(), orig)+1,
											 std::distance(orig,loc)-1);
	return loc;
	}

inline std::string::const_iterator url_string::DecodeFragment(std::string::const_iterator loc)  const
	{
	fragment.erase();
	std::string::const_iterator orig=loc, end=original_url.end();
	if(loc == end) return loc;
	while(loc<end) 
		loc++;
	if(std::distance(orig,loc)>0) 
		fragment = original_url.substr(std::distance(original_url.begin(), orig)+1, 
												 std::distance(orig,loc)-1);
	return loc;
	}

/* Default, do nothing try it as if it were a relative path */
inline std::string::const_iterator url_string::HandleAbs(std::string::const_iterator loc) const
	{
	return loc;
	}

/* We provide a basic sort of relative -> absolute path conversion.
   This is correct for http and probably for most other schemes.
   If this is not the correct one, use a derived class.
   After this, abs_path and url should be correct. */

inline void url_string::fixup_() const
	{
	std::ostringstream ss1;
	ss1 << scheme << "://";
	if (!username.empty()) {
		ss1 << username;
		if(!passwd.empty())
			ss1 << ":" << passwd;
		ss1 << "@";	
		}
	ss1 << net_path;
	if(port!=default_port())
		ss1 << ":" << port;

	// handle ../ pseudo-directories, replace /foo/bar/../foobar with /foo/foobar
	unsigned int uploc, prevslash;
	while((uploc=abs_path.find("/../"))!=abs_path.npos) {
		if(uploc==0) break;
		prevslash=abs_path.rfind("/",uploc-1);
		if(prevslash+1>uploc)
			break; // no more path elements to remove
		abs_path=abs_path.substr(0,prevslash+1)+abs_path.substr(uploc+4);
		}
   // replace /./ elements with /
	while((uploc=abs_path.find("/./"))!=abs_path.npos) {
		abs_path=abs_path.substr(0,uploc)+abs_path.substr(uploc+2);
		}
	ss1 << abs_path;

	if(!params.empty())
		ss1 << ";" << params;
	if(!query.empty())
		ss1 << "?" << query;
	if(!fragment.empty())
		ss1 << "#" << fragment;

	url=ss1.str();
	}


inline std::string url_string::get_url(void) const 
	{
	finish_decoding_();
	return url;
	}

inline std::string url_string::get_scheme() const 
	{
	/* Scheme is always decoded */
	finish_decoding_();
	return scheme;
	}

inline std::string url_string::get_net_path() const
	{
	finish_decoding_();
	return net_path;
	}

inline std::string url_string::get_absolute_path() const
	{
	finish_decoding_();
	return abs_path;
	}

inline std::string url_string::get_relative_path() const
	{
	finish_decoding_();
	return rel_path;
	}

inline std::string url_string::get_object() const
	{
	unsigned int pos;

	if((pos=abs_path.rfind("/"))<=abs_path.npos) {
		return abs_path.substr(pos+1);
		}
	else return abs_path;
	}

inline std::string url_string::get_parameters() const
	{
	finish_decoding_();
	return params;
	}

inline int url_string::get_port(void) const
	{
	finish_decoding_();
	return port;
	}

inline std::string url_string::get_query() const
	{
	finish_decoding_();
	return query;
	}

inline std::string url_string::get_fragment() const
	{
	finish_decoding_();
	return fragment;
	}

inline std::string url_string::get_username() const
	{
	finish_decoding_();
	return username;
	}

inline std::string url_string::get_passwd() const
	{
	finish_decoding_();
	return passwd;
	}

inline std::string url_string::get_original() const 
	{
	return original_url;
	}

inline std::string url_string::get_referrer() const 
	{
	return referrer;
	}

inline void url_string::set_username(const std::string& name)
	{
	finish_decoding_();
	username=name;
	}

inline void url_string::set_passwd(const std::string& pw)
	{
	finish_decoding_();
	passwd=pw;
	}


#endif