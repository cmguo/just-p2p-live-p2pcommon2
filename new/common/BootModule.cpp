
#include "StdAfx.h"

#include "BootModule.h"
#include <boost/bind.hpp>
#include "common/VersionNumber.h"
#include <ppl/io/data_input_stream.h>
#include <ppl/io/data_output_stream.h>

const string PPL_BOOT_SERVER_NAME = "boot.ppvod.tv";
const string PPL_BOOT_SERVER_IP = "59.151.34.54";
const UINT16 PPL_BOOT_SERVER_PORT = 7101;


#include <ppl/concurrent/thread.h>

class NameResolverThread : public ppl::concurrent::thread
{
public:
	typedef boost::function<void (u_long)> CallbackType;
	explicit NameResolverThread(const string& name, CallbackType callback) : m_Name(name), m_Callback(callback)
	{
	}

protected:
	virtual void do_run()
	{
		u_long ip = ResolveHostName(m_Name);
		m_Callback(ip);
	}
	virtual void do_interrupt()
	{
	}

private:
	string m_Name;
	CallbackType m_Callback;
};



BootModule::BootModule(UDPSenderPtr packetSender) : m_PacketSender( packetSender ), m_ServerIP(0), m_PacketBuilder(P2P_MODULE_VERSION_NUMBER_UINT16)
{
	m_TimeoutTimer.set_callback(boost::bind(&BootModule::OnTimeout, this));
}

BootModule::~BootModule()
{
	if (m_ResolverThread)
	{
		m_ResolverThread->stop(10);
	}
}

void BootModule::QueryNTSList( NTSListCallbackType callback )
{
	m_Callback= callback;
	if ( m_ServerIP != 0 )
	{
		// 已经解析过，直接使用
		// 向boot server发包
		this->DoQueryNTSList();
	}
	else
	{
		// 先解析boot server的地址，再来发包
		m_ResolverThread.reset(new NameResolverThread(
			PPL_BOOT_SERVER_NAME, 
			NameResolverThread::CallbackType(boost::bind(&BootModule::OnServerNameResolved, this, _1))
			));
		m_ResolverThread->start();
	}
}

void BootModule::OnServerNameResolved( u_long ip )
{
	if ( ip != 0 && ip != INADDR_BROADCAST )
	{
		m_ServerIP = ip;
	}
	else
	{
		m_ServerIP = ::inet_addr(PPL_BOOT_SERVER_IP.c_str());
	}

	// 发包
	this->DoQueryNTSList();
}

void BootModule::DoQueryNTSList()
{
	m_TimeoutTimer.start(6000);
	LIVE_ASSERT(m_ServerIP != 0 && m_ServerIP != INADDR_BROADCAST);
	m_PacketBuilder.Build();
	m_PacketSender->Send(m_PacketBuilder.GetData(), m_PacketBuilder.GetSize(), InetSocketAddress(m_ServerIP, PPL_BOOT_SERVER_PORT));
}

static inline boost::uint16_t little_endian_to_host_short(
    boost::uint16_t v)
{
    char * ch = (char *)&v; 
    return (boost::uint16_t)ch[1] << 8 
        | (boost::uint16_t)ch[0];
}

bool BootModule::HandlePacket( BYTE* data, size_t size, const InetSocketAddress& remoteAddr )
{

	const size_t total_header_size = sizeof(CNetHeader) + sizeof(CCommandProtocolHeader);
	if ( size < total_header_size )
	{
		return false;
	}

	CNetHeader* pstNetHeader = (CNetHeader*)data;
	if ( NHTYPE_CONFUSE_PFS != little_endian_to_host_short(pstNetHeader->m_usNetType ))
		return false;

	//去除混淆协议
	StunRequestPacketBuilder::unConfuse((INT8*)data, size,NHTYPE_CONFUSE_PFS);

	CCommandProtocolHeader* lpCmdPLHeader = (CCommandProtocolHeader*)(data+NETHEADER_LENGTH);
	{
		BYTE* realData = reinterpret_cast<BYTE*>( lpCmdPLHeader + 1 );
		size_t realSize = size - total_header_size;
		if ( PFS_CMD_HEADER == lpCmdPLHeader->pfsSymbol && RTN_SN == lpCmdPLHeader->protocolType )
		if ( realSize > sizeof(UINT32) * 2 )
		{
			UINT32 snCount = READ_MEMORY( realData, UINT32 );
			realData += sizeof(UINT32);
			UINT32 ntsCount = READ_MEMORY( realData, UINT32 );
			realData += sizeof(UINT32);
			const size_t sn_item_size = 6;
			const size_t nts_item_size = 6;
			if ( snCount > 0 && snCount < 20 && ntsCount > 0 && ntsCount < 20 )
			{
				if ( size >= snCount * sn_item_size + ntsCount * nts_item_size )
				{
					realData += (snCount * sn_item_size);
					std::vector<InetSocketAddress> servers;
					for ( UINT32 index = 0; index < ntsCount; ++index )
					{
						//u_long ip = *(u_long *)(realData);
                                                u_long ip;
                                                memcpy(&ip, realData, sizeof(ip));
						realData += sizeof(u_long);
						//u_short port = *(u_short *)realData;
                                                u_short port;
                                                memcpy(&port, realData, sizeof(port));
						realData += sizeof(u_short);
						if ( ip != 0 && port != 0 )
						{
							servers.push_back(InetSocketAddress(ip, ntohs(port)));
						}
						else
						{
							LIVE_ASSERT(false);
						}
					}
					m_TimeoutTimer.stop();
					m_Callback(servers);
					return true;
				}
			}
		}
	}
	LIVE_ASSERT(false);
	return false;
}

void BootModule::OnTimeout()
{
	std::vector<InetSocketAddress> servers;
	m_Callback(servers);
}



