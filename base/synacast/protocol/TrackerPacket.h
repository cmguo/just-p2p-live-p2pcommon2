
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




/// list������
class PTListPeersRequest : public PacketBase
{
public:
	/// Peer״̬��Ϣ
	PEER_STATUS_INFO StatusInfo;

	/// �ڼ�����Tracker����Peer�б�
	UINT16 RequestTimes;

	/// ������෵�ض��ٸ�Peer�б�
	UINT16 RequestCount;

	/// Peer���ں˰汾��
	UINT32 PeerVersion;

	/// �Ƿ���Ƕ��ʽϵͳ�� 0 �C Ϊ���ǣ�1 �C ΪWindows CE �汾
	UINT8 PeerIsEmbedded;

	/// ����ϵͳ�����Ա��
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
			// ʣ��3���ֶ����¼ӵģ��ϰ汾�ı����п���û��
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



/// keep-alive������
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


/// ע��������
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


/// register��Ӧ����
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


/// join������
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

/// join��Ӧ����
class PTJoinResponse : public PacketBase
{
public:
	/// Tracker̽�⵽��ip�����԰���ȷ��������
	UINT32 DetectedIP;
	/// ��Tracker����ļ��ʱ��(��)
	UINT16 KeepAliveInterval;
	/// Tracker̽�⵽��udp�˿�
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


/// leave������
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
// �����ǰ�ȫЭ��ı���
//////////////////////////////////////////////////////////////////////////



/// peer���뵽tracker��������
class PTSJoinRequest : public PacketBase
{
public:
	/// ���ص�ַ�б�
	std::vector<PEER_ADDRESS> LocalAddresses;

	/// cookieֵ��������ǰ�ķ��������ƣ�����һ��û��ʹ��
	UINT32 CookieValue;

	/// �����������ڵ�ʱ�䣨��λ���룩
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

/// peer���˳������ģ�����Ҫresponse��
class PTSLeaveRequest : public EmptyPacket
{
public:
	PTSLeaveRequest() : EmptyPacket(PTS_ACTION_LEAVE_REQUEST)
	{ }
};


class PTSListPeersRequest : public PacketBase
{
public:
	/// ������෵�ص�Peer����
	UINT16 RequestPeerCount;

	/// �ܹ���Tracker�����List����
	UINT16 RequestPeerTimes;

	/// ���ص�ַ�б�
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
	/// �����ڵ��ͬһ�����Ľڵ�
	std::vector<CANDIDATE_PEER_INFO> WANPeers;

	/// ��Ҫͨ��nat��Խ�������ڵ�
	std::vector<INNER_CANDIDATE_PEER_INFO> LANPeers;

	/// SourceTimeStamp���¼ӵģ��ϰ汾û�У���peer����ʱ��λ����λ�ã�
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
		// �����Э����չ����wanPeerInfoSize���ܻ����CANDIDATE_PEER_INFO::object_size����������¶�ȡ��֪�����ݣ���������Ĳ���
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
			// ��ǰ��INNER_CANDIDATE_PEER_INFO��СΪ49��Ϊ�����ϴ�Э���޸�ʱ���µĲ����������Ŀǰ�Ⱥ������ִ�С��lanPeers
			// ֱ�ӷ���true�����Ժ����ֶ�
			is >> SourceTimeStamp;
			return true;
		}
		// �����Э����չ����lanPeerInfoSize���ܻ����INNER_CANDIDATE_PEER_INFO::object_size����������¶�ȡ��֪�����ݣ���������Ĳ���
		if ( false == variant_reader_util::read_padded_vector( is, lanPeerCount, LANPeers, INNER_CANDIDATE_PEER_INFO::object_size, lanPeerInfoSize ) )
			return false;
		// SourceTimeStamp���¼ӵ��ֶΣ�����û��
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
	/// peer״̬��Ϣ
	PeerStatusEx Status;

	/// peer��Դ��Χ
	PEER_MINMAX MinMax;

	/// ��Ƭ��С������Peer��ʱ���ã���0 ��Sourceʹ�õģ�
	UINT32 PieceSize;

	/// ���ʣ���λ��byte/s������Peer��ʱ���ã���0��Sourceʹ�õģ�
	UINT32 ByteRate;

	/// stun��ַ��Ϣ�������Ϊ�գ���һ��Ϊstun server address���ڶ���Ϊstun detected address
	std::vector<PEER_ADDRESS> StunAddresses;

	/// source��ǰ��ʱ���λ�ã�����peer��ʼ��λ��source�ϱ�ʹ�ã�
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
	/// ���ص�ַ�б�
	std::vector<PEER_ADDRESS> LocalAddresses;

	/// Ƶ�����û���
	string Username;

	/// Ƶ����¼tracker������
	string Password;

	/// source��Դ��Χ
	PEER_MINMAX MinMax;

	/// piece��С��Ŀǰû��ʹ�ã�
	UINT32 PieceSize;

	/// �����ʣ�byte/s��Ŀǰû��ʹ�ã�
	UINT32 ByteRate;
	/// ������ǰ�ķ��������ƣ�����һ��û��ʹ��
	UINT32 CookieType;

	/// �����������ڵ�ʱ�䣨��λ���룩
	UINT32 TimeFromStart;

	/// source��ǰ��ʱ���λ�ã�����peer��ʼ��λ
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
	/// �����������
	UINT8 ErrorAction;
	/// �������
	INT8 ErrorIndex;

	/// ������Ϣ���ȣ�Ŀǰû���õ�������Ϣ��ErrorLengthӦ��Ϊ0��
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
	/// �����ڣ�ÿ��ת����1��TTL=0��ת����ʼ������СӦ����2
	UINT8 TTL;

	/// ת���ߵ�ַ����1,3��B; 2��C; 4��A��
	PEER_ADDRESS ViaAddress;

	/// ǰһ�����ͷ��ĵ�ַ��2��, B��дA����
	PEER_ADDRESS DetectedAddress;

	/// �����ߵ�ַ����1,2��A; 3,4��C��
	PEER_ADDRESS FromAddress;

	/// �����ߵ�ַ����1,2��C; 3,4��A��
	PEER_ADDRESS ToAddress;

	/// �Ƿ���response��ռ��һ���ֽڣ�Ŀǰ��LSB��һ��bit��Ч��0-Request, 1-Response��
	bool IsResponse;

	/// ��Ϣ��ţ���A��������һ��Transaction�в���
	UINT32 MessageID;

	/// ����ת���ı���
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
			// ȡ���λ��bit
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






