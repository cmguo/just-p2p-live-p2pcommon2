
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_TRACKER_PACKET_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_TRACKER_PACKET_H_

#include <synacast/protocol/PacketBase.h>
#include <synacast/protocol/PacketHead.h>
#include <synacast/protocol/base.h>
#include <synacast/protocol/data/CandidatePeerInfo.h>
#include <synacast/protocol/data/PeerMinMaxIO.h>
#include <synacast/protocol/data/PeerStatusIO.h>
#include <synacast/protocol/data/PeerAddressIO.h>
#include <synacast/protocol/TrackerProtocol.h>
#include <ppl/io/variant_writer.h>
#include <ppl/io/variant_reader.h>
#include <synacast/protocol/PacketBuilder.h>
#include <boost/array.hpp>
#include <vector>

using ppl::io::variant_writer_uint8;
using ppl::io::variant_reader_uint8;
using ppl::io::variant_writer_uint16;
using ppl::io::variant_reader_uint16;
using ppl::io::variant_writer_uint32;
using ppl::io::variant_reader_uint32;
using ppl::io::variant_reader_util;




inline data_output_stream& operator<<( data_output_stream& os, const CANDIDATE_PEER_INFO& info )
{
	return os << info.Address << info.CoreInfo;
}
inline data_input_stream& operator>>( data_input_stream& is, CANDIDATE_PEER_INFO& info )
{
	return is >> info.Address >> info.CoreInfo;
}

inline data_output_stream& operator<<( data_output_stream& os, const INNER_CANDIDATE_PEER_INFO& info )
{
	assert( 0 == info.PublicHostCount );
	return os << info.DetectedAddress << info.StunServerAddress << info.CoreInfo << info.PublicHostCount;
}
inline data_input_stream& operator>>( data_input_stream& is, INNER_CANDIDATE_PEER_INFO& info )
{
	return is >> info.DetectedAddress >> info.StunServerAddress >> info.CoreInfo >> info.PublicHostCount;
}




/// list请求报文
class PTListPeersRequest : public PacketBase
{
public:
	/// Peer状态信息
	PEER_STATUS_INFO StatusInfo;

	/// 第几次向Tracker请求Peer列表
	UINT16 RequestTimes;

	/// 请求最多返回多少个Peer列表
	UINT16 RequestCount;

	/// Peer的内核版本号
	UINT32 PeerVersion;

	/// 是否是嵌入式系统用 0 – 为不是，1 – 为Windows CE 版本
	UINT8 PeerIsEmbedded;

	/// 操作系统的语言编号
	UINT16 Language;

	PTListPeersRequest() : PacketBase( PT_ACTION_LIST )
	{
		StatusInfo.Clear();
		RequestTimes = 0;
		RequestCount = 0;
		PeerVersion = 0;
		PeerIsEmbedded = PPL_IS_EMBEDDED_SYSTEM;
		Language = ::GetSystemDefaultLangID();
	}

	virtual bool read_object(data_input_stream& is)
	{
		if ( is >> StatusInfo >> RequestTimes >> RequestCount )
		{
			// 剩下3个字段是新加的，老版本的报文中可能没有
			is >> PeerVersion >> PeerIsEmbedded >> Language;
			return true;
		}
		return false;
	}
	virtual void write_object(data_output_stream& os) const
	{
		os << StatusInfo << RequestTimes << RequestCount << PeerVersion << PeerIsEmbedded << Language;
	}
	virtual size_t get_object_size() const
	{
		return PEER_STATUS_INFO::object_size + sizeof(UINT16) * 2 + sizeof(UINT32) + sizeof(UINT8) + sizeof(UINT16);
	}
};


class PTListPeersResponse : public PacketBase
{
public:
	std::vector<CANDIDATE_PEER_INFO> Peers;

	PTListPeersResponse() : PacketBase( PT_ACTION_LIST )
	{
	}

	virtual bool read_object(data_input_stream& is)
	{
		return is >> variant_reader_uint8::make( Peers, CANDIDATE_PEER_INFO::object_size );
	}
	virtual void write_object(data_output_stream& os) const
	{
		os << variant_writer_uint8::make( Peers );
	}
	virtual size_t get_object_size() const
	{
		return sizeof(UINT8) + CANDIDATE_PEER_INFO::object_size * Peers.size();
	}
};



/// keep-alive请求报文
class PTKeepAliveRequest : public PacketBase
{
public:
	PEER_STATUS_INFO StatusInfo;

	PTKeepAliveRequest() : PacketBase( PT_ACTION_KEEPALIVE )
	{
		StatusInfo.Clear();
	}

	virtual bool read_object(data_input_stream& is)
	{
		return is >> StatusInfo;
	}
	virtual void write_object(data_output_stream& os) const
	{
		os << StatusInfo;
	}
	virtual size_t get_object_size() const
	{
		return PEER_STATUS_INFO::object_size;
	}
};

class PTKeepAliveResponse : public PacketBase
{
public:
	UINT32 DetectedIP;
	UINT16 DetectedUDPPort;

	PTKeepAliveResponse() : PacketBase( PT_ACTION_KEEPALIVE )
	{
		DetectedIP = 0;
		DetectedUDPPort = 0;
	}

	virtual bool read_object(data_input_stream& is)
	{
		return is >> DetectedIP >> DetectedUDPPort;
	}
	virtual void write_object(data_output_stream& os) const
	{
		os << DetectedIP << DetectedUDPPort;
	}
	virtual size_t get_object_size() const
	{
		return sizeof(UINT32) + sizeof(UINT16);
	}
};


/// 注册请求报文
class PTRegisterRequest : public PacketBase
{
public:
	UINT16 TCPPort;
	UINT16 UDPPort;
	PEER_CORE_INFO CoreInfo;
	UINT32 PieceSize;
	UINT32 CookieType;

	std::vector<UINT32> RealIPs;
	string Username;
	string Password;
	UINT32 PeerVersion;
	UINT8 IsEmbedded;

	PTRegisterRequest() : PacketBase( PT_ACTION_REGISTER )
	{
		TCPPort = 0;
		UDPPort = 0;
		CoreInfo.Clear();
		PieceSize = 0;
		CookieType = COOKIE_NONE;
		PeerVersion = 0;
		IsEmbedded = PPL_IS_EMBEDDED_SYSTEM;
	}

	virtual bool read_object(data_input_stream& is)
	{
		UINT8 ipCount = 0;
		UINT8 usernameLen = 0;
		UINT8 passwordLen = 0;
		if ( is >> TCPPort >> UDPPort >> CoreInfo >> PieceSize >> CookieType >> ipCount >> usernameLen >> passwordLen )
		{
			if ( is.read_vector( ipCount, RealIPs, sizeof(UINT32) ) && is.read_string( usernameLen, Username ) && is.read_string( passwordLen, Password ) )
			{
				return is >> PeerVersion >> IsEmbedded;
			}
		}
		return false;
	}
	virtual void write_object(data_output_stream& os) const
	{
		os << TCPPort << UDPPort << CoreInfo << PieceSize << CookieType;
		os.write_uint8( static_cast<UINT8>( RealIPs.size() ) );
		os.write_uint8( static_cast<UINT8>( Username.size() ) );
		os.write_uint8( static_cast<UINT8>( Password.size() ) );
		os << RealIPs << Username << Password << PeerVersion << IsEmbedded;
	}

	virtual size_t get_object_size() const
	{
		return sizeof(UINT16) * 2 + PEER_CORE_INFO::object_size + sizeof(UINT32) * 2 
			+ sizeof(UINT8) * 3 + sizeof(UINT32) * RealIPs.size() + Username.size() + Password.size() 
			+ sizeof(UINT32) + sizeof(UINT8);
	}

};


/// register回应报文
class PTRegisterResponse : public PacketBase
{
public:
	UINT32 DetectedIP;
	UINT16 KeepAliveInterval;

	PTRegisterResponse() : PacketBase( PT_ACTION_REGISTER )
	{
		DetectedIP = 0;
		KeepAliveInterval = 0;
	}

	virtual bool read_object(data_input_stream& is)
	{
		return is >> DetectedIP >> KeepAliveInterval;
	}
	virtual void write_object(data_output_stream& os) const
	{
		os << DetectedIP << KeepAliveInterval;
	}

	virtual size_t get_object_size() const
	{
		return sizeof(UINT32) + sizeof(UINT16);
	}
};


/// join请求报文
class PTJoinRequest : public PacketBase
{
public:
	UINT16 TCPPort;
	UINT16 UDPPort;
	PEER_CORE_INFO CoreInfo;
	UINT32 CookieValue;

	std::vector<UINT32> RealIPs;
	UINT32 PeerVersion;
	UINT8 IsEmbedded;
	UINT16 Language;

	PTJoinRequest() : PacketBase( PT_ACTION_JOIN )
	{
		TCPPort = 0;
		UDPPort = 0;
		CoreInfo.Clear();
		CookieValue = 0;
		PeerVersion = 0;
		IsEmbedded = PPL_IS_EMBEDDED_SYSTEM;
		Language = ::GetSystemDefaultLangID();
	}

	virtual bool read_object(data_input_stream& is)
	{
		return is 
			>> TCPPort 
			>> UDPPort 
			>> CoreInfo 
			>> CookieValue 
			>> variant_reader_uint8::make(RealIPs, sizeof(UINT32)) 
			>> PeerVersion 
			>> IsEmbedded 
			>> Language;
	}
	virtual void write_object(data_output_stream& os) const
	{
		os  << TCPPort 
			<< UDPPort 
			<< CoreInfo 
			<< CookieValue 
			<< variant_writer_uint8::make(RealIPs) 
			<< PeerVersion 
			<< IsEmbedded 
			<< Language;
	}

	virtual size_t get_object_size() const
	{
		return sizeof(UINT16) * 2 + PEER_CORE_INFO::object_size + sizeof(UINT32) 
			+ sizeof(UINT8) + sizeof(UINT32) * RealIPs.size() 
			+ sizeof(UINT32) + sizeof(UINT8) + sizeof(UINT16);
	}
};

/// join回应报文
class PTJoinResponse : public PacketBase
{
public:
	/// Tracker探测到的ip，可以帮助确认内外网
	UINT32 DetectedIP;
	/// 向Tracker保活的间隔时间(秒)
	UINT16 KeepAliveInterval;
	/// Tracker探测到的udp端口
	UINT16 DetectedUDPPort;
	boost::array<UINT8, 10> Reserved;

	PTJoinResponse() : PacketBase( PT_ACTION_JOIN )
	{
		DetectedIP = 0;
		KeepAliveInterval = 0;
		DetectedUDPPort = 0;
		Reserved.assign( 0 );
	}

	virtual bool read_object(data_input_stream& is)
	{
		is  >> DetectedIP
			>> KeepAliveInterval 
			>> DetectedUDPPort;
		if ( is )
		{
			return is.read_n( Reserved.c_array(), Reserved.size() );
		}
		return false;
	}
	virtual void write_object(data_output_stream& os) const
	{
		os  << DetectedIP 
			<< KeepAliveInterval 
			<< DetectedUDPPort;
		os.write_n( Reserved.data(), Reserved.size() );
	}

	virtual size_t get_object_size() const
	{
		return sizeof(UINT32) + sizeof(UINT16) + sizeof(UINT16) + Reserved.size();
	}
};


/// leave请求报文
class PTLeaveRequest : public PacketBase
{
public:
	string Username;
	string Password;

	PTLeaveRequest() : PacketBase( PT_ACTION_LEAVE )
	{
	}

	virtual bool read_object(data_input_stream& is)
	{
		UINT8 usernameLen = 0;
		UINT8 passwordLen = 0;
		if ( is >> usernameLen >> passwordLen )
		{
			return is.read_string( usernameLen, Username ) && is.read_string( passwordLen, Password );
		}
		return false;
	}

	virtual void write_object(data_output_stream& os) const
	{
		os.write_uint8( static_cast<UINT8>( Username.size() ) );
		os.write_uint8( static_cast<UINT8>( Password.size() ) );
		os << Username << Password;
	}

	virtual size_t get_object_size() const
	{
		return sizeof(UINT8) * 2 + Username.size() + Password.size();
	}

};



//////////////////////////////////////////////////////////////////////////
// 以下是安全协议的报文
//////////////////////////////////////////////////////////////////////////



/// peer加入到tracker的请求报文
class PTSJoinRequest : public PacketBase
{
public:
	/// 本地地址列表
	std::vector<PEER_ADDRESS> LocalAddresses;

	/// cookie值，用于以前的防盗链机制，现在一般没有使用
	UINT32 CookieValue;

	/// 从启动到现在的时间（单位：秒）
	UINT32 TimeFromStart;

	PTSJoinRequest() : PacketBase( PTS_ACTION_JOIN_REQUEST )
	{
		CookieValue = 0;
		TimeFromStart = 0;
	}

	virtual bool read_object(data_input_stream& is)
	{
		return is 
			>> variant_reader_uint8::make(LocalAddresses, PEER_ADDRESS::object_size) 
			>> CookieValue 
			>> TimeFromStart;
	}
	virtual void write_object(data_output_stream& os) const
	{
		assert(LocalAddresses.size() <= 15);
		assert(LocalAddresses.size() > 0);
		os  << variant_writer_uint8::make(LocalAddresses) 
			<< CookieValue 
			<< TimeFromStart;
	}
	virtual size_t get_object_size() const
	{
		return sizeof(UINT8) + PEER_ADDRESS::object_size * LocalAddresses.size() 
			+ sizeof(UINT32) * 2;
	}
};

class PTSJoinResponse : public PacketBase
{
public:
	UINT16 KeepAliveInterval;

	PTSJoinResponse() : PacketBase( PTS_ACTION_JOIN_RESPONSE )
	{
		KeepAliveInterval = 0;
	}

	virtual bool read_object(data_input_stream& is)
	{
		return is >> KeepAliveInterval;
	}
	virtual void write_object(data_output_stream& os) const
	{
		os << KeepAliveInterval;
	}
	virtual size_t get_object_size() const
	{
		return sizeof(UINT16);
	}
};

/// peer的退出请求报文（不需要response）
class PTSLeaveRequest : public EmptyPacket
{
public:
	PTSLeaveRequest() : EmptyPacket(PTS_ACTION_LEAVE_REQUEST)
	{ }
};


class PTSListPeersRequest : public PacketBase
{
public:
	/// 请求最多返回的Peer数量
	UINT16 RequestPeerCount;

	/// 总共向Tracker请求的List次数
	UINT16 RequestPeerTimes;

	/// 本地地址列表
	std::vector<PEER_ADDRESS> LocalAddresses;

	PTSListPeersRequest() : PacketBase( PTS_ACTION_LIST_PEERS_REQUEST ) 
	{
		RequestPeerCount = 0;
		RequestPeerTimes = 0;
	}

	virtual bool read_object(data_input_stream& is)
	{
		return is 
			>> RequestPeerCount 
			>> RequestPeerTimes 
			>> variant_reader_uint8::make(LocalAddresses, PEER_ADDRESS::object_size);
	}
	virtual void write_object(data_output_stream& os) const
	{
		assert(LocalAddresses.size() < 15);
		assert(LocalAddresses.size() > 0);
		os  << RequestPeerCount 
			<< RequestPeerTimes 
			<< variant_writer_uint8::make(LocalAddresses);
	}
	virtual size_t get_object_size() const
	{
		return sizeof(UINT16) + sizeof(UINT16) 
			+ sizeof(UINT8) + PEER_ADDRESS::object_size * LocalAddresses.size();
	}
};


class PTSListPeersResponse : public PacketBase
{
public:
	/// 公网节点和同一内网的节点
	std::vector<CANDIDATE_PEER_INFO> WANPeers;

	/// 需要通过nat穿越的内网节点
	std::vector<INNER_CANDIDATE_PEER_INFO> LANPeers;

	/// SourceTimeStamp是新加的，老版本没有（供peer启动时定位下载位置）
	UINT64 SourceTimeStamp;

	PTSListPeersResponse() : PacketBase( PTS_ACTION_LIST_PEERS_RESPONSE )
	{
		SourceTimeStamp = 0;
	}

	virtual bool read_object(data_input_stream& is)
	{
		WANPeers.clear();
		LANPeers.clear();
		UINT16 wanPeerCount = 0;
		UINT8 wanPeerInfoSize = 0;
		is >> wanPeerCount >> wanPeerInfoSize;
		if ( !is )
			return false;
		if ( wanPeerInfoSize < CANDIDATE_PEER_INFO::object_size )
			return false;
		// 如果有协议扩展，则wanPeerInfoSize可能会大于CANDIDATE_PEER_INFO::object_size，这种情况下读取已知的内容，跳过扩充的部分
		if ( false == variant_reader_util::read_padded_vector( is, wanPeerCount, WANPeers, CANDIDATE_PEER_INFO::object_size, wanPeerInfoSize ) )
			return false;

		UINT16 lanPeerCount = 0;
		UINT8 lanPeerInfoSize = 0;
		is >> lanPeerCount >> lanPeerInfoSize;
		if ( !is )
			return false;
		if ( lanPeerInfoSize < INNER_CANDIDATE_PEER_INFO::object_size )
			return false;
		if ( lanPeerInfoSize == 49 )
		{
			// 以前的INNER_CANDIDATE_PEER_INFO大小为49，为避免上次协议修改时导致的不兼容情况，目前先忽略这种大小的lanPeers
			// 直接返回true，忽略后续字段
			is >> SourceTimeStamp;
			return true;
		}
		// 如果有协议扩展，则lanPeerInfoSize可能会大于INNER_CANDIDATE_PEER_INFO::object_size，这种情况下读取已知的内容，跳过扩充的部分
		if ( false == variant_reader_util::read_padded_vector( is, lanPeerCount, LANPeers, INNER_CANDIDATE_PEER_INFO::object_size, lanPeerInfoSize ) )
			return false;
		// SourceTimeStamp是新加的字段，可以没有
		is >> SourceTimeStamp;
		return true;
	}
	virtual void write_object(data_output_stream& os) const
	{
		{
			os.write_uint16( static_cast<UINT16>( WANPeers.size() ) );
			os.write_uint8( CANDIDATE_PEER_INFO::object_size );
			os << WANPeers;
		}
		{
			os.write_uint16( static_cast<UINT16>( LANPeers.size() ) );
			os.write_uint8( INNER_CANDIDATE_PEER_INFO::object_size );
			os << LANPeers;
		}
		os << SourceTimeStamp;
	}
	virtual size_t get_object_size() const
	{
		return sizeof(UINT16) + sizeof(UINT8) + CANDIDATE_PEER_INFO::object_size * WANPeers.size() 
			+ sizeof(UINT16) + sizeof(UINT8) + INNER_CANDIDATE_PEER_INFO::object_size * LANPeers.size() 
			+ sizeof(UINT64);
	}
};


class PTSKeepAliveRequest : public PacketBase
{
public:
	/// peer状态信息
	PeerStatusEx Status;

	/// peer资源范围
	PEER_MINMAX MinMax;

	/// 分片大小，对于Peer暂时无用，填0 （Source使用的）
	UINT32 PieceSize;

	/// 码率，单位：byte/s，对于Peer暂时无用，填0（Source使用的）
	UINT32 ByteRate;

	/// stun地址信息，如果不为空，第一个为stun server address，第二个为stun detected address
	std::vector<PEER_ADDRESS> StunAddresses;

	/// source当前的时间戳位置，用于peer起始定位（source上报使用）
	UINT64 SourceTimeStamp;

	PTSKeepAliveRequest() : PacketBase( PTS_ACTION_KEEP_ALIVE_REQUEST )
	{
		FILL_ZERO(MinMax);
		PieceSize = 0;
		ByteRate = 0;
		SourceTimeStamp = 0;
	}

	void SetStunServerInfo( bool needNAT, const PEER_ADDRESS& stunServerAddress, const PEER_ADDRESS& stunDetectedAddress )
	{
		StunAddresses.clear();
		if ( needNAT && stunServerAddress.IsUDPValid() && stunDetectedAddress.IsUDPValid() )
		{
			StunAddresses.push_back(stunServerAddress);
			StunAddresses.push_back(stunDetectedAddress);
		}
	}

	virtual bool read_object(data_input_stream& is)
	{
		return is 
			>> Status 
			>> MinMax 
			>> PieceSize 
			>> ByteRate 
			>> variant_reader_uint8::make(StunAddresses, PEER_ADDRESS::object_size) 
			>> SourceTimeStamp;
	}
	virtual void write_object(data_output_stream& os) const
	{
		assert(StunAddresses.size() == 0 || StunAddresses.size() == 2);
		os  << Status 
			<< MinMax 
			<< PieceSize 
			<< ByteRate 
			<< variant_writer_uint8::make(StunAddresses) 
			<< SourceTimeStamp;
	}
	virtual size_t get_object_size() const
	{
		return PeerStatusEx::object_size 
			+ PEER_MINMAX::object_size 
			+ sizeof(UINT32) * 2 
			+ sizeof(UINT8) + PEER_ADDRESS::object_size * StunAddresses.size() 
			+ sizeof(UINT64);
	}
};

class PTSKeepAliveResponse : public PTSJoinResponse
{
public:
	PTSKeepAliveResponse()
	{
		this->SetAction( PTS_ACTION_KEEP_ALIVE_RESPONSE );
	}
};


class PTSRegisterRequest : public PacketBase
{
public:
	/// 本地地址列表
	std::vector<PEER_ADDRESS> LocalAddresses;

	/// 频道的用户名
	string Username;

	/// 频道登录tracker的密码
	string Password;

	/// source资源范围
	PEER_MINMAX MinMax;

	/// piece大小（目前没有使用）
	UINT32 PieceSize;

	/// 码流率（byte/s，目前没有使用）
	UINT32 ByteRate;
	/// 用于以前的防盗链机制，现在一般没有使用
	UINT32 CookieType;

	/// 从启动到现在的时间（单位：秒）
	UINT32 TimeFromStart;

	/// source当前的时间戳位置，用于peer起始定位
	UINT64 CurrentTimeStamp;

	PTSRegisterRequest() : PacketBase( PTS_ACTION_REGISTER_REQUEST )
	{
		MinMax.Clear();
		PieceSize = 0;
		ByteRate = 0;
		CookieType = COOKIE_NONE;
		TimeFromStart = 0;
		CurrentTimeStamp = 0;
	}

	virtual bool read_object(data_input_stream& is)
	{
		return is 
			>> variant_reader_uint8::make(LocalAddresses, PEER_ADDRESS::object_size) 
			>> variant_reader_uint8::make(Username) 
			>> variant_reader_uint8::make(Password) 
			>> MinMax 
			>> PieceSize 
			>> ByteRate 
			>> CookieType 
			>> TimeFromStart 
			>> CurrentTimeStamp;
	}
	virtual void write_object(data_output_stream& os) const
	{
		assert( Username.size() < 250 && Password.size() < 250 );
		assert( Username.size() > 0 && Password.size() > 0 );
		assert(LocalAddresses.size() < 15);
		assert(LocalAddresses.size() > 0);
		os  << variant_writer_uint8::make(LocalAddresses) 
			<< variant_writer_uint8::make(Username) 
			<< variant_writer_uint8::make(Password) 
			<< MinMax 
			<< PieceSize 
			<< ByteRate 
			<< CookieType 
			<< TimeFromStart 
			<< CurrentTimeStamp;
	}
	virtual size_t get_object_size() const
	{
		return sizeof(UINT8) + PEER_ADDRESS::object_size * LocalAddresses.size() 
			+ sizeof(UINT8) + Username.size() 
			+ sizeof(UINT8) + Password.size() 
			+ PEER_MINMAX::object_size 
			+ sizeof(UINT32) * 4 
			+ sizeof(UINT64);
	}
};

class PTSRegisterResponse : public PTSJoinResponse
{
public:
	PTSRegisterResponse()
	{
		this->SetAction( PTS_ACTION_REGISTER_RESPONSE );
	}
};


class PTSSourceExitRequest : public PacketBase
{
public:
	string Username;
	string Password;

	PTSSourceExitRequest() : PacketBase( PTS_ACTION_SOURCE_EXIT_REQUEST )
	{
	}

	virtual bool read_object(data_input_stream& is)
	{
		return is 
			>> variant_reader_uint8::make(Username) 
			>> variant_reader_uint8::make(Password);
	}
	virtual void write_object(data_output_stream& os) const
	{
		assert( Username.size() < 250 && Password.size() < 250 );
		assert( Username.size() > 0 && Password.size() > 0 );
		assert( Password.size() == 32 );
		os  << variant_writer_uint8::make(Username) 
			<< variant_writer_uint8::make(Password);
	}
	virtual size_t get_object_size() const
	{
		assert( Password.size() == 32 );
		return sizeof(UINT8) + Username.size() 
			+ sizeof(UINT8) + Password.size();
	}
};


class PTSErrorResponse : public PacketBase
{
public:
	/// 出错的命令字
	UINT8 ErrorAction;
	/// 错误代码
	INT8 ErrorIndex;

	/// 错误信息长度（目前没有用到错误信息，ErrorLength应该为0）
	UINT16 ErrorLength;

	PTSErrorResponse() : PacketBase( PTS_ACTION_ERROR_RESPONSE )
	{
		ErrorAction = 0;
		ErrorIndex = 0;
		ErrorLength = 0;
	}

	virtual bool read_object(data_input_stream& is)
	{
		return is 
			>> ErrorAction 
			>> ErrorIndex 
			>> ErrorLength;
	}
	virtual void write_object(data_output_stream& os) const
	{
		assert( ErrorLength == 0 );
		os  << ErrorAction 
			<< ErrorIndex 
			<< ErrorLength;
	}
	virtual size_t get_object_size() const
	{
		assert( ErrorLength == 0 );
		return sizeof(UINT8) 
			+ sizeof(INT8) 
			+ sizeof(UINT16);
	}
};

class PTSProxyMessagePacket : public PacketBase
{
public:
	/// 生存期，每次转发减1，TTL=0不转发，始发者最小应该是2
	UINT8 TTL;

	/// 转发者地址（第1,3步B; 2步C; 4步A）
	PEER_ADDRESS ViaAddress;

	/// 前一个发送方的地址（2步, B填写A’）
	PEER_ADDRESS DetectedAddress;

	/// 发送者地址（第1,2步A; 3,4步C）
	PEER_ADDRESS FromAddress;

	/// 接收者地址（第1,2步C; 3,4步A）
	PEER_ADDRESS ToAddress;

	/// 是否是response（占用一个字节，目前仅LSB的一个bit有效：0-Request, 1-Response）
	bool IsResponse;

	/// 消息编号，由A给出，在一个Transaction中不变
	UINT32 MessageID;

	/// 代理转发的报文
	std::vector<BYTE> MessageData;

	PTSProxyMessagePacket() : PacketBase( PTS_ACTION_PROXY_MESSAGE )
	{
		TTL = 0;
		ViaAddress.Clear();
		DetectedAddress.Clear();
		FromAddress.Clear();
		ToAddress.Clear();
		IsResponse = false;
		MessageID = 0;
	}

	virtual bool read_object(data_input_stream& is)
	{
		UINT8 val = 0;
		is  >> TTL 
			>> ViaAddress 
			>> DetectedAddress 
			>> FromAddress 
			>> ToAddress 
			>> val 
			>> MessageID 
			>> variant_reader_uint32::make(MessageData, 1);
		if ( is )
		{
			// 取最高位的bit
			IsResponse = (val & 0x80) != 0;
			return true;
		}
		return false;
	}

	virtual void write_object( data_output_stream& os ) const
	{
		UINT8 val = 0;
		if ( IsResponse )
		{
			val |= 0x80;
		}
		os  << TTL 
			<< ViaAddress 
			<< DetectedAddress 
			<< FromAddress 
			<< ToAddress 
			<< val 
			<< MessageID 
			<< variant_writer_uint32::make( MessageData );
	}
	virtual size_t get_object_size() const
	{
		return sizeof(UINT8) 
			+ sizeof(PEER_ADDRESS) * 4 
			+ sizeof(UINT8) 
			+ sizeof(UINT32) 
			+ sizeof(UINT32) + MessageData.size();
	}
};


#endif






