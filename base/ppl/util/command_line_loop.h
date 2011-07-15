
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_UTIL_COMMAND_LINE_LOOP_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_UTIL_COMMAND_LINE_LOOP_H_


#include <ppl/util/command_dispatcher.h>
#include <ppl/io/line_reader.h>
#include <ppl/data/strings.h>
#include <ppl/util/macro.h>

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <iostream>
#include <map>
#include <vector>
#include <string>


typedef boost::function1<void, std::vector<std::string>> command_callback_type;

class command_line_loop
{
public:
	typedef std::map<std::string, command_callback_type> callback_collection;

	command_line_loop()
	{
		this->register_command( "help", boost::bind( &command_line_loop::on_help, this ) );
	}

	void run(command_dispatcher& cmddisp)
	{
		on_help();
		for ( ;; )
		{
			std::string line = line_reader::read_line();
			std::vector<std::string> args0;
			std::vector<std::string> args;

			strings::split(std::back_inserter(args0), line, ' ');
			if ( args0.empty() )
			{
				continue;
			}
			args.reserve(args0.size());
			for ( size_t index = 0; index < args0.size(); ++index )
			{
				std::string s = strings::trim(args0[index]);
				if ( false == s.empty() )
				{
					args.push_back( s );
				}
			}
			if ( args.empty() )
			{
				printf("error : no command\n");
				continue;
			}
			std::string command = args[0];
			//printf("%s\n", command.c_str());
			if( command == "exit" )
			{
				//MainThread::IOS().post(boost::bind(&UdpTrackerModule::Stop, UdpTrackerModule::Inst()));
				//DataThread::IOS().post(boost::bind(&DataWriter::Stop, DataWriter::Inst()));
				break;
			}
			callback_collection::const_iterator iter = m_callbacks.find( command );
			if ( iter != m_callbacks.end() )
			{
				cmddisp.post_command( boost::bind( iter->second, args ) );
			}
			else
			{
				printf("unrecognized command : %s\n", command.c_str());
			}
		}
	}


	bool register_command( const std::string& cmd, command_callback_type callback )
	{
		if ( cmd.empty() )
		{
			assert( false );
			return false;
		}
		if ( "exit" == cmd )
		{
			assert( false );
			return false;
		}
		m_callbacks[cmd] = callback;
		return true;
	}
	bool unregister_command( const std::string& cmd )
	{
		if ( cmd.empty() )
		{
			assert( false );
			return false;
		}
		if ( "exit" == cmd )
		{
			assert( false );
			return false;
		}
		m_callbacks.erase(cmd);
		return true;
	}
	void on_help()
	{
		puts("Command: (case sentive)");
		puts("  exit");
		STL_FOR_EACH_CONST( callback_collection, m_callbacks, iter )
		{
			printf("  %s\n", iter->first.c_str());
		}
	}

private:
	callback_collection m_callbacks;
};

#endif


