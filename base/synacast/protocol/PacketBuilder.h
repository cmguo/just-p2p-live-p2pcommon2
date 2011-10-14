
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_PACKET_BUILDER_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_PACKET_BUILDER_H_

#include <ppl/config.h>

#include <synacast/protocol/base.h>
#include <synacast/protocol/PacketHead.h>
#include <synacast/protocol/PacketBase.h>
#include <synacast/protocol/TrackerProtocol.h>
#include <synacast/protocol/PacketHeadIO.h>
#include <synacast/protocol/PacketObfuscator.h>
#include <synacast/util/IDGenerator.h>

#include <ppl/util/random.h>
#include <ppl/util/macro.h>
#include <utility>

#include <ppl/io/data_input_stream.h>
#include <ppl/io/data_output_stream.h>



#if defined(_PPL_PLATFORM_MSWIN)
#elif defined(_PPL_PLATFORM_LINUX)

inline UINT16 GetSystemDefaultLangID()
{
	return 2052;
}

#else
#error invalid GetSystemDefaultLangID
#endif


inline void CheckPredefinedAction(UINT8 action)
{
	const UINT8 PREDEFINED_ACTIONS[] = 
	{
		PT_ACTION_JOIN					, 
		PT_ACTION_LIST					, 
		PT_ACTION_KEEPALIVE				,  
		PT_ACTION_LEAVE					, 
		PT_ACTION_REGISTER				, 

		PTS_ACTION_JOIN_REQUEST			, 
		PTS_ACTION_LIST_PEERS_REQUEST	,
		PTS_ACTION_KEEP_ALIVE_REQUEST	, 
		PTS_ACTION_REGISTER_REQUEST		, 
		PTS_ACTION_LEAVE_REQUEST		, 
		PTS_ACTION_SOURCE_EXIT_REQUEST	, 

		PTS_ACTION_PROXY_MESSAGE		, 


		PPT_ERROR			,
//		PPT_HELLO			,
//		PPT_ECHO			,
//		PPT_SIMPLE_HELLO	, 
//		PPT_ANNOUNCE		,
//		PPT_REQUEST			,
//		PPT_MEDIA			, 

		PPT_DETECT , 
		PPT_REDETECT , 

		PPT_HANDSHAKE			, 
//		PPT_FINISH				, 
		PPT_SUB_PIECE_REQUEST	, 
		PPT_SUB_PIECE_DATA		, 
		PPT_PEER_EXCHANGE		, 

//		PPT_SIMPLE_ECHO			, 
		PPT_HUFFMAN_ANNOUNCE	, 

		PPT_DATA_COLLECTING_REQUEST, 
		PPT_DATA_COLLECTING_RESPONSE, 

		PPT_DHT_REQUEST, 
		PPT_DHT_RESPONSE, 

//		PPT_SESSION_ERROR				,
//		PPT_SESSION_SUB_PIECE_REQUEST	, 
//		PPT_SESSION_SUB_PIECE_DATA		, 
//		PPT_SESSION_HUFFMAN_ANNOUNCE	, 
	};

	const size_t predefined_action_count = SIZEOF_ARRAY(PREDEFINED_ACTIONS);
	LIVE_ASSERT(std::find(PREDEFINED_ACTIONS, PREDEFINED_ACTIONS + predefined_action_count, action) != PREDEFINED_ACTIONS + predefined_action_count);
}



/// 报文打包的基类
class PacketBuilderBase : private boost::noncopyable
{
public:
	/// 预留11个字节的头部，也就是checksum的3个字节和随机数填充的8个字节（随机数实际最多7个字节，但为了便于生成，填充2的UINT32的随机数）
	enum { MAX_SHUFFLE_SIZE = 11 };
	explicit PacketBuilderBase( size_t bufferSize ) : m_Size( 0 ), m_ShuffleSize( 0 )
	{
		LIVE_ASSERT( bufferSize <= USHRT_MAX );
		this->m_Buffer.resize( bufferSize + MAX_SHUFFLE_SIZE );
	}

	const BYTE* GetData() const
	{
		LIVE_ASSERT( m_ShuffleSize <= MAX_SHUFFLE_SIZE );
		return this->m_Buffer.data() + MAX_SHUFFLE_SIZE - m_ShuffleSize;
	}
	size_t GetSize() const
	{
		LIVE_ASSERT( m_ShuffleSize <= MAX_SHUFFLE_SIZE );
		return this->m_Size + m_ShuffleSize;
	}

	boost::shared_ptr<IDGenerator> GetTransactionID() const { return m_TransactionID; }
	//const IDGenerator& GetTransactionID() const { return *m_TransactionID; }

protected:
	void BuildBody( data_output_stream& os, const PacketBase& body, bool needShuffle )
	{
		body.write_object( os );
		m_Size = os.position();
		this->DoShuffle(needShuffle);
		CheckBuffer();
	}
	/// 协议混淆
	void DoShuffle( bool needShuffle )
	{
		if ( false == needShuffle )
		{
			m_ShuffleSize = 0;
			return;
		}
		std::pair<BYTE, WORD> checksum = this->CalcChecksum();
		checksum.second ^= 0xE903;
		size_t len = PacketObfuscator::CalcPaddingLength( checksum.first );
		LIVE_ASSERT( len == 1 || len == 3 || len == 5 || len == 7 );
		m_ShuffleSize = len + 3;
		RandomGenerator rnd;
		//UINT32* buf = reinterpret_cast<UINT32*>( m_Buffer.data() + 3 );
                UINT32 buf[2];
		buf[0] = rnd.NextDWord();
		buf[1] = rnd.NextDWord();
                memcpy(m_Buffer.data() + 3, buf, sizeof(buf));
		m_Buffer[ MAX_SHUFFLE_SIZE - m_ShuffleSize ] = checksum.first;
		//WRITE_MEMORY( m_Buffer.data() + MAX_SHUFFLE_SIZE - m_ShuffleSize + 1, checksum.second, WORD );
                *( m_Buffer.data() + MAX_SHUFFLE_SIZE - m_ShuffleSize + 1) = (BYTE)checksum.second;
                *( m_Buffer.data() + MAX_SHUFFLE_SIZE - m_ShuffleSize + 2) = (BYTE)(checksum.second >> 8);
		//UINT32 key = READ_MEMORY( m_Buffer.data() + MAX_SHUFFLE_SIZE - m_ShuffleSize, UINT32 );
                //UINT32 key = *(UINT32 *)(m_Buffer.data() + MAX_SHUFFLE_SIZE - m_ShuffleSize);
                UINT32 key;
                memcpy(&key, m_Buffer.data() + MAX_SHUFFLE_SIZE - m_ShuffleSize, sizeof(key));
		LIVE_ASSERT( m_Size > sizeof( UINT32 ) * 2 );
		//UINT32* dst = reinterpret_cast<UINT32*>( m_Buffer.data() + MAX_SHUFFLE_SIZE );
                UINT32 dst[2];
                memcpy(dst, m_Buffer.data() + MAX_SHUFFLE_SIZE, sizeof(dst));
		dst[0] ^= key;
		dst[1] ^= key;
                memcpy(m_Buffer.data() + MAX_SHUFFLE_SIZE, dst, sizeof(dst));
	}
	std::pair<BYTE, WORD> CalcChecksum() const
	{
		return ChecksumCalculator::Calc( this->m_Buffer.data() + MAX_SHUFFLE_SIZE, m_Size );
	}

	void CheckBuffer() const
	{
		LIVE_ASSERT( m_Buffer.size() >= m_Size + MAX_SHUFFLE_SIZE );
	}

protected:
	/// 获取有连接的命令字
	UINT8 GetConnectionAction( UINT8 action )
	{
		CheckPredefinedAction( action );
		return action | PPL_P2P_CONNECTION_ACTION_FLAG;
	}

	void BuildPacket( const PacketBase& body )
	{
		PacketOutputStream os( m_Buffer.data() + MAX_SHUFFLE_SIZE, m_Buffer.size() - MAX_SHUFFLE_SIZE );
		DoBuildPacket( os, body );
	}
	virtual void DoBuildPacket( data_output_stream& os, const PacketBase& body )
	{
		LIVE_ASSERT(false);
	}

protected:
	boost::shared_ptr<IDGenerator> m_TransactionID;
	pool_byte_buffer m_Buffer;
	size_t m_Size;
	size_t m_ShuffleSize;
};


/// 老的udp报文，仅用于非安全的peer-tracker协议
class UDPPacketBuilder : public PacketBuilderBase
{
public:
	/// 报文头
	OLD_UDP_PACKET_HEAD PacketHead;
	/// 请求头包含频道guid和peer guid，回应头则包含频道guid和source minmax
	GUID ChannelGUID;
	GUID PeerGUID;

	explicit UDPPacketBuilder(boost::shared_ptr<IDGenerator> transactionID) : PacketBuilderBase( 4 * 1024 )
	{
		FILL_ZERO( PacketHead );
		FILL_ZERO( ChannelGUID );
		FILL_ZERO( PeerGUID );
		m_TransactionID = transactionID;
		this->PacketHead.Magic = SYNACAST_MAGIC;
		this->PacketHead.ProtocolVersion = SYNACAST_VERSION;
		this->PacketHead.AppType = PPL_P2P_LIVE2;
	}

	void Build(const PacketBase& body, UINT32 transactionID)
	{
		LIVE_ASSERT( SYNACAST_MAGIC == this->PacketHead.Magic );
		LIVE_ASSERT( SYNACAST_VERSION == this->PacketHead.ProtocolVersion );
		LIVE_ASSERT( PPL_P2P_LIVE2 == this->PacketHead.AppType );

		this->InitHead( body.GetAction(), PT_ACTION_TYPE_REQUEST, transactionID );
		BuildPacket( body );
	}
	void InitHeadInfo(const GUID& channelGUID, const GUID& peerGUID)
	{
		this->ChannelGUID = channelGUID;
		this->PeerGUID = peerGUID;
	}

protected:
	void InitHead(UINT8 action, INT8 actionType, UINT32 transactionID)
	{
		LIVE_ASSERT( action > 0 );
		this->PacketHead.Action = action;
		this->PacketHead.ActionType = actionType;
		this->PacketHead.TransactionID = m_TransactionID->Get(transactionID);
		LIVE_ASSERT( this->PacketHead.TransactionID != 0 );
		CheckPredefinedAction( action );
		LIVE_ASSERT( this->PacketHead.AppType == PPL_P2P_LIVE2 );
	}
	virtual void DoBuildPacket( data_output_stream& os, const PacketBase& body )
	{
		os << PacketHead << ChannelGUID << PeerGUID;
		LIVE_ASSERT( os.position() == OLD_UDP_PACKET_HEAD::object_size + sizeof(GUID) * 2 );
		BuildBody( os, body, false );
		LIVE_ASSERT( os.position() == OLD_UDP_PACKET_HEAD::object_size + sizeof(GUID) * 2 + body.get_object_size() );
	}
};


class SecureTrackerRequestPacketBuilder : public PacketBuilderBase
{
public:
	NEW_UDP_PACKET_HEAD PacketHead;
	GUID ChannelGUID;
	GUID PeerGUID;
	SECURE_REQUEST_HEAD RequestHead;

	explicit SecureTrackerRequestPacketBuilder(boost::shared_ptr<IDGenerator> transactionID) : PacketBuilderBase( 4 * 1024 )
	{
		FILL_ZERO( PacketHead );
		FILL_ZERO( ChannelGUID );
		FILL_ZERO( PeerGUID );
		FILL_ZERO( RequestHead );
		m_TransactionID = transactionID;
		this->PacketHead.ReservedActionType = 0;
		this->PacketHead.ProtocolVersion = SYNACAST_VERSION_REQUEST;
		this->RequestHead.Platform = PPL_PROTOCOL_PLATFORM;
		this->RequestHead.Language = ::GetSystemDefaultLangID();
	}

	void Build(const PacketBase& body, UINT32 transactionID)
	{
		this->PacketHead.Action = body.GetAction();
		this->PacketHead.TransactionID = m_TransactionID->Get(transactionID);

		LIVE_ASSERT( 0 == this->PacketHead.ReservedActionType );
		LIVE_ASSERT( SYNACAST_VERSION_REQUEST == this->PacketHead.ProtocolVersion );
		LIVE_ASSERT( PPL_PROTOCOL_PLATFORM == this->RequestHead.Platform );
		LIVE_ASSERT( ::GetSystemDefaultLangID() == this->RequestHead.Language );
		LIVE_ASSERT( body.GetAction() > 0 );
		LIVE_ASSERT( this->PacketHead.TransactionID != 0 );
		CheckPredefinedAction( body.GetAction() );

		BuildPacket( body );
	}
	void Init(const GUID& channelGUID, const GUID& peerGUID, const PEER_CORE_INFO& coreInfo, UINT32 appVersion)
	{
		this->ChannelGUID = channelGUID;
		this->PeerGUID = peerGUID;
		this->RequestHead.CoreInfo = coreInfo;
		this->RequestHead.AppVersion = appVersion;
	}

	void UpdateCoreInfo( const PEER_CORE_INFO& coreInfo )
	{
		this->RequestHead.CoreInfo = coreInfo;
	}

protected:
	virtual void DoBuildPacket( data_output_stream& os, const PacketBase& body )
	{
		os << PacketHead << ChannelGUID << PeerGUID << RequestHead;
		LIVE_ASSERT( os.position() == NEW_UDP_PACKET_HEAD::object_size + sizeof(GUID) * 2 + SECURE_REQUEST_HEAD::object_size );
		BuildBody( os, body, true );
		LIVE_ASSERT( os.position() == NEW_UDP_PACKET_HEAD::object_size + sizeof(GUID) * 2 + SECURE_REQUEST_HEAD::object_size + body.get_object_size() );
	}
};

class SecureTrackerProxyPacketBuilder : public PacketBuilderBase
{
public:
	NEW_UDP_PACKET_HEAD PacketHead;
	GUID ChannelGUID;
	GUID PeerGUID;

	explicit SecureTrackerProxyPacketBuilder(boost::shared_ptr<IDGenerator> transactionID) : PacketBuilderBase( 33 * 1024 )
	{
		FILL_ZERO( PacketHead );
		FILL_ZERO( ChannelGUID );
		FILL_ZERO( PeerGUID );
		m_TransactionID = transactionID;
		this->PacketHead.ReservedActionType = 0;
		this->PacketHead.ProtocolVersion = SYNACAST_VERSION_REQUEST;
	}

	void Build(const PacketBase& body, UINT32 transactionID)
	{
		this->PacketHead.Action = body.GetAction();
		this->PacketHead.TransactionID = m_TransactionID->Get(transactionID);

		PacketHead.CheckValid(SYNACAST_VERSION_REQUEST);
		CheckPredefinedAction( body.GetAction() );

		this->BuildPacket( body );
	}
	void Init(const GUID& channelGUID, const GUID& peerGUID)
	{
		ChannelGUID = channelGUID;
		PeerGUID = peerGUID;
	}

protected:
	virtual void DoBuildPacket( data_output_stream& os, const PacketBase& body )
	{
		os << PacketHead << ChannelGUID << PeerGUID;
		LIVE_ASSERT( os.position() == NEW_UDP_PACKET_HEAD::object_size + sizeof(GUID) * 2 );
		BuildBody( os, body, true );
		LIVE_ASSERT( os.position() == NEW_UDP_PACKET_HEAD::object_size + sizeof(GUID) * 2 + body.get_object_size() );
	}
};


/// udp无连接报文
class UDPConnectionlessPacketBuilder : public PacketBuilderBase
{
public:
	NEW_UDP_PACKET_HEAD PacketHead;
	PACKET_PEER_INFO PeerInfo;

	explicit UDPConnectionlessPacketBuilder( boost::shared_ptr<IDGenerator> transactionID ) : PacketBuilderBase( 4 * 1024 )
	{
		FILL_ZERO( PacketHead );
		FILL_ZERO( PeerInfo );
		m_TransactionID = transactionID;
		//this->m_Head.AppType = PPL_P2P_LIVE2;
		//this->m_Head.Magic = SYNACAST_MAGIC;
		this->PacketHead.ProtocolVersion = SYNACAST_VERSION_REQUEST;
	}

	void Build( const PacketBase& body, UINT32 transactionID )
	{
		this->PacketHead.Action = body.GetAction();
		this->PacketHead.TransactionID = m_TransactionID->Get( transactionID );

		PacketHead.CheckValid(SYNACAST_VERSION_REQUEST);
		CheckPredefinedAction( body.GetAction() );

		BuildPacket( body );
	}

protected:
	virtual void DoBuildPacket( data_output_stream& os, const PacketBase& body )
	{
		os << PacketHead << PeerInfo;
		LIVE_ASSERT( os.position() == NEW_UDP_PACKET_HEAD::object_size + PACKET_PEER_INFO::object_size );
		BuildBody( os, body, true );
		LIVE_ASSERT( os.position() == NEW_UDP_PACKET_HEAD::object_size + PACKET_PEER_INFO::object_size + body.get_object_size() );
	}
};

/// tcp无连接报文
class TCPConnectionlessPacketBuilder : public PacketBuilderBase
{
public:
	TCP_PACKET_HEAD PacketHead;
	PACKET_PEER_INFO PeerInfo;

	TCPConnectionlessPacketBuilder() : PacketBuilderBase( 33 * 1024 )
	{
		FILL_ZERO( PacketHead );
		FILL_ZERO( PeerInfo );
		//this->m_Head.AppType = PPL_P2P_LIVE2;
		this->PacketHead.ProtocolVersion = SYNACAST_VERSION_REQUEST;
	}

	void Build(const PacketBase& body)
	{
		this->PacketHead.Action = body.GetAction();

		PacketHead.CheckValid(SYNACAST_VERSION_REQUEST);
		CheckPredefinedAction( body.GetAction() );

		BuildPacket( body );
	}

protected:
	virtual void DoBuildPacket( data_output_stream& os, const PacketBase& body )
	{
		os << PacketHead << PeerInfo;
		LIVE_ASSERT( os.position() == TCP_PACKET_HEAD::object_size + PACKET_PEER_INFO::object_size );
		BuildBody( os, body, true );
		LIVE_ASSERT( os.position() == TCP_PACKET_HEAD::object_size + PACKET_PEER_INFO::object_size + body.get_object_size() );
	}
};


/// udp有连接报文
class UDPConnectionPacketBuilder : public PacketBuilderBase
{
public:
	NEW_UDP_PACKET_HEAD PacketHead;
	UDP_SESSION_INFO SessionInfo;

	explicit UDPConnectionPacketBuilder(boost::shared_ptr<IDGenerator> transactionID) : PacketBuilderBase( 4 * 1024 )
	{
		FILL_ZERO( PacketHead );
		FILL_ZERO( SessionInfo );
		m_TransactionID = transactionID;
		//this->m_Head.AppType = PPL_P2P_LIVE2;
		//this->m_Head.Magic = SYNACAST_MAGIC;
		this->PacketHead.ProtocolVersion = SYNACAST_VERSION_REQUEST;
	}

	void Build(const PacketBase& body, UINT32 transactionID, UINT32 sequenceID, UINT32 sessionKey)
	{
		this->PacketHead.Action = GetConnectionAction( body.GetAction() );
		this->PacketHead.TransactionID = m_TransactionID->Get( transactionID );
		this->SessionInfo.SequenceID = sequenceID;
		this->SessionInfo.SessionKey = sessionKey;
		BuildPacket( body );
	}

protected:
	virtual void DoBuildPacket( data_output_stream& os, const PacketBase& body )
	{
		os << PacketHead << SessionInfo;
		LIVE_ASSERT( os.position() == NEW_UDP_PACKET_HEAD::object_size + UDP_SESSION_INFO::object_size );
		BuildBody( os, body, true );
		LIVE_ASSERT( os.position() == NEW_UDP_PACKET_HEAD::object_size + UDP_SESSION_INFO::object_size + body.get_object_size() );
	}
};

/// tcp有连接报文
class TCPConnectionPacketBuilder : public PacketBuilderBase
{
public:
	TCPConnectionPacketBuilder() : PacketBuilderBase(33 * 1024)
	{
	}

	void Build(const PacketBase& body)
	{
		BuildPacket( body );
	}

protected:
	virtual void DoBuildPacket( data_output_stream& os, const PacketBase& body )
	{
		UINT8 action = GetConnectionAction( body.GetAction() );
		os.write_uint8( action );
		LIVE_ASSERT( os.position() == 1 );
		BuildBody( os, body, false );
		LIVE_ASSERT( os.position() == 1 + body.get_object_size() );
	}
};

#endif
