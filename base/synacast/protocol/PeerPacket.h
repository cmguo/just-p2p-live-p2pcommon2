
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_PEER_PACKET_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_PEER_PACKET_H_

/**
 * @file
 * @brief peer-peer协议报文
 */



#include <synacast/protocol/MediaPacket.h>
#include <synacast/protocol/PeerError.h>
#include <synacast/protocol/data/NetInfoIO.h>
#include <synacast/protocol/data/PeerMinMaxIO.h>
#include <synacast/protocol/data/PeerAddressIO.h>
#include <synacast/protocol/data/PeerStatusIO.h>
#include <synacast/protocol/data/DegreeInfoIO.h>
#include <synacast/protocol/data/CandidatePeerInfo.h>
#include <ppl/util/time_counter.h>
#include <ppl/data/guid_io.h>
#include <ppl/io/variant_reader.h>
#include <ppl/io/variant_writer.h>

using ppl::io::variant_reader_util;
using ppl::io::variant_reader_uint8;
using ppl::io::variant_reader_uint16;
using ppl::io::variant_writer_uint16;
using ppl::io::variant_reader_util;

#include <boost/shared_ptr.hpp>


inline data_output_stream& operator<<( data_output_stream& os, const PeerExchangeItem& info )
{
	LIVE_ASSERT( info.PeerType <= 15 );
	// 限制为LSB 4bits
	UINT8 val = info.PeerType & 0x0F;
	return os << info.Address << val;
}
inline data_input_stream& operator>>( data_input_stream& is, PeerExchangeItem& info )
{
	UINT8 val = 0;
	if ( is >> info.Address >> val )
	{
		// 取LSB 4bits
		info.PeerType = val & 0x0F;
	}
	return is;
}




/// 重定向时的peer项信息
struct REDIRECT_PEER_INFO
{
	GUID PeerGUID;
	PEER_ADDRESS Address;
	PEER_ADDRESS OuterAddress;
	UINT32 AppVersion;
	INT8 DegreeLeft;
	/// 连接类型，udp或者tcp，0为tcp，1为udp
	UINT8 ConnectionType;

	/* 接下来是
	PeerType	4bit	Peer类型 MSB
	PeerNetType	2bit	网络类型（0,1-Inner,2-Outer,3-UPNP）
	Embed		2bit	是否嵌入式设备，LSB 2bit有效：0 – Desktop，1 – Embed
	*/

	PeerNetTypeEnum PeerNetType;
	PeerTypeEnum PeerType;
	bool IsEmbedded;

	enum { object_size = sizeof(GUID) + PEER_ADDRESS::object_size * 2 + sizeof(UINT32) + sizeof(UINT8) * 4 };

	void Clear()
	{
		FILL_ZERO( PeerGUID );
		Address.Clear();
		OuterAddress.Clear();
		AppVersion = 0;
		DegreeLeft = 0;
		ConnectionType = 0;
		PeerNetType = PNT_INVALID;
		PeerType = NORMAL_PEER;
		IsEmbedded = false;
	}
};

inline data_output_stream& operator<<( data_output_stream& os, const REDIRECT_PEER_INFO& info )
{
	// LSB 2bits
	UINT8 embedType = ( info.IsEmbedded ? 1 : 0 );
	// 中间 2bits
	UINT8 peerNetTypeVal = static_cast<UINT8>( info.PeerNetType ) & 0x03;
	// MSB 4bits
	UINT8 peerTypeVal = static_cast<UINT8>( info.PeerType ) & 0x0F;
	UINT8 val = (peerTypeVal << 4) | (peerNetTypeVal << 2) | embedType;
	UINT8 reserved = 0;
	os  << info.PeerGUID 
		<< info.Address 
		<< info.OuterAddress 
		<< info.AppVersion 
		<< info.DegreeLeft 
		<< info.ConnectionType 
		<< val 
		<< reserved;
	return os;
}

inline data_input_stream& operator>>( data_input_stream& is, REDIRECT_PEER_INFO& info )
{
	UINT8 val = 0;
	UINT8 reserved = 0;
	is  >> info.PeerGUID 
		>> info.Address 
		>> info.OuterAddress 
		>> info.AppVersion 
		>> info.DegreeLeft 
		>> info.ConnectionType 
		>> val 
		>> reserved;
	if ( is )
	{
		UINT8 peerTypeVal = (val & 0xF0) >> 4; // MSB 4bit
		UINT8 peerNetTypeVal = (val & 0x0C) >> 2; // 中间2bit
		UINT8 embedType = val & 0x03; // LSB 2bit
		info.IsEmbedded = (1 == embedType);
		info.PeerType = static_cast<PeerTypeEnum>( peerTypeVal );
		info.PeerNetType = static_cast<PeerNetTypeEnum>( peerNetTypeVal );
	}
	return is;
}








class PPDetect : public PacketBase
{
public:
	/// 发起探测时对方的标记地址
	PEER_ADDRESS KeyAddress;

	/// 发送报文的时间
	UINT32 SendOffTime;

	PPDetect() : PacketBase( PPT_DETECT )
	{
		this->KeyAddress.Clear();
		this->SendOffTime = time_counter::get_system_count32();
	}

	virtual bool read_object(data_input_stream& is)
	{
		return is 
			>> KeyAddress 
			>> SendOffTime;
	}
	virtual void write_object(data_output_stream& os) const
	{
		os  << KeyAddress 
			<< SendOffTime;
	}
	virtual size_t get_object_size() const
	{
		return PEER_ADDRESS::object_size + sizeof(UINT32);
	}
};

class PPRedetect : public PPDetect
{
public:
	PPRedetect()
	{
		this->SetAction(PPT_REDETECT);
	}
};




/// 使用huffman压缩资源位图的announce报文
class PPHuffmanAnnounce : public PacketBase
{
public:
	PeerStatusEx Status;
	UINT32 MinIndex;
	UINT8 HuffmanTimes;
	boost::shared_ptr<pool_byte_string> ResourceData;

	PPHuffmanAnnounce() : PacketBase( PPT_HUFFMAN_ANNOUNCE ), ResourceData( new pool_byte_string )
	{
		MinIndex = 0;
		HuffmanTimes = 0;
	}

	virtual bool read_object(data_input_stream& is)
	{
		return is 
			>> Status 
			>> MinIndex 
			>> HuffmanTimes 
			>> variant_reader_uint16::make( *ResourceData );
	}

	virtual void write_object(data_output_stream& os) const
	{
		LIVE_ASSERT(ResourceData->size() <= 4500);
		os  << Status 
			<< MinIndex 
			<< HuffmanTimes 
			<< variant_writer_uint16::make( *ResourceData );
	}

	virtual size_t get_object_size() const
	{
		return PeerStatusEx::object_size 
			+ sizeof(UINT32) 
			+ sizeof(UINT8) 
			+ sizeof(UINT16) + ResourceData->size();
	}
};



class HandshakePacket : public PacketBase
{
public:
	/// LSB 1bit
	bool MustBeVIP;

	/// 仅用于udp握手
	UINT32 SessionKeyA;

	/// 仅用于udp握手
	UINT32 SessionKeyB;

	HandshakePacket() : PacketBase( PPT_HANDSHAKE )
	{
		this->MustBeVIP = false;
		this->SessionKeyA = 0;
		this->SessionKeyB = 0;
	}

	virtual bool read_object(data_input_stream& is)
	{
		UINT8 val = 0;
		is  >> val 
			>> SessionKeyA 
			>> SessionKeyB;
		if ( is )
		{
			MustBeVIP = ( (val & 0x01) != 0 );
			return true;
		}
		return false;
	}
	virtual void write_object(data_output_stream& os) const
	{
		UINT8 val = MustBeVIP ? 1 : 0;
		os  << val 
			<< SessionKeyA 
			<< SessionKeyB;
	}
	virtual size_t get_object_size() const
	{
		return sizeof(UINT8) + sizeof(UINT32) * 2;
	}
};



class PeerExchangePacket : public PacketBase
{
public:
	/// request报文发送时间
	UINT32 SendOffTime;

	/// 是否为response（IsResponse和peerCount合用一个字节，IsResponse占用MSB 1bit，peerCount占用LSB的6bits为PeerCount(最多63个)，剩下1bit保留
	bool IsResponse;

	/// PeerExchangeItem结构的大小
	UINT8 PeerNetInfoSize;

	/// 候选的Peers列表
	std::vector<PeerExchangeItem> Peers;

	PeerExchangePacket() : PacketBase( PPT_PEER_EXCHANGE )
	{
		SendOffTime = time_counter::get_system_count32();
		IsResponse = false;
		PeerNetInfoSize = PeerExchangeItem::object_size;
	}

	virtual bool read_object(data_input_stream& is)
	{
		UINT8 val = 0;
		is  >> SendOffTime 
			>> val 
			>> PeerNetInfoSize;
		if ( is && PeerNetInfoSize >= PeerExchangeItem::object_size )
		{
			LIVE_ASSERT(PeerNetInfoSize == PeerExchangeItem::object_size);
			// MSB 1bit
			IsResponse = ((val & 0x80) != 0);
			// LSB 6bits
			UINT8 itemCount = (val & 0x3F);
			return variant_reader_util::read_padded_vector( is, itemCount, Peers, PeerExchangeItem::object_size, PeerNetInfoSize );
		}
		return false;
	}

	virtual void write_object(data_output_stream& os) const
	{
		LIVE_ASSERT(PeerNetInfoSize == PeerExchangeItem::object_size);
		LIVE_ASSERT( Peers.size() <= 63 );
		UINT8 itemSize = PeerExchangeItem::object_size;
		UINT8 val = static_cast<UINT8>( Peers.size() );
		if ( IsResponse )
		{
			val |= 0x80;
		}
		os  << SendOffTime 
			<< val 
			<< itemSize 
			<< Peers;
	}

	virtual size_t get_object_size() const
	{
		LIVE_ASSERT(PeerNetInfoSize == PeerExchangeItem::object_size);
		return sizeof(UINT32) 
			+ sizeof(UINT8) 
			+ sizeof(UINT8) 
			+ PeerExchangeItem::object_size * Peers.size();
	}
};


class PPErrorPacket : public PacketBase
{
public:
	UINT16 ErrorIndex;
	const serializable* ErrorInfo;

	PPErrorPacket( UINT16 errcode, const serializable* errorInfo) 
		: PacketBase( PPT_ERROR )
		, ErrorIndex( errcode ) 
		, ErrorInfo( errorInfo )
	{
	}

	virtual void write_object( data_output_stream& os ) const
	{
		os << ErrorIndex;
		if ( ErrorInfo )
		{
			os.write_uint16( static_cast<UINT16>( ErrorInfo->get_object_size() ) );
			ErrorInfo->write_object( os );
		}
		else
		{
			os.write_uint16( 0 );
		}
	}

	virtual size_t get_object_size() const
	{
		size_t objsize = sizeof(UINT16) * 2;
		if ( ErrorInfo )
		{
			objsize += ErrorInfo->get_object_size();
		}
		return objsize;
	}
};

class PPErrorRefuse : public serializable
{
public:
	UINT32 RefuseReason;

	explicit PPErrorRefuse( UINT32 reason = 0 ) : RefuseReason( reason )
	{
	}

	virtual bool read_object( data_input_stream& is )
	{
		return is >> RefuseReason;
	}
	virtual void write_object(data_output_stream& os) const
	{
		os << RefuseReason;
	}
	virtual size_t get_object_size() const
	{
		return sizeof( UINT32 );
	}
};


class PPRedirectInfo : public serializable
{
public:
	/// PeerCount	1		候选Peer数量，LSB 6bit有效，但目前最多30个
	/// 一个候选Peer信息大小
	UINT8 PeerInfoSize;
	/// Peers		vary	一个RedirectPeerInfo数组
	std::vector<REDIRECT_PEER_INFO> Peers;

	PPRedirectInfo()
	{
		PeerInfoSize = REDIRECT_PEER_INFO::object_size;
	}

	virtual bool read_object( data_input_stream& is )
	{
		UINT8 val = 0;
		if ( is >> val >> PeerInfoSize )
		{
			UINT8 peerCount = val & 0x3F;
			return variant_reader_util::read_padded_vector( is, peerCount, Peers, REDIRECT_PEER_INFO::object_size, PeerInfoSize );
		}
		LIVE_ASSERT(false);
		return false;
	}
	virtual void write_object(data_output_stream& os) const
	{
		LIVE_ASSERT( Peers.size() < 30 );
		LIVE_ASSERT(PeerInfoSize == REDIRECT_PEER_INFO::object_size);
		LIVE_ASSERT(Peers.size() < 30);
		UINT8 itemSize = REDIRECT_PEER_INFO::object_size;
		UINT8 val = static_cast<UINT8>( Peers.size() );
		val &= 0x3F;
		os << val << itemSize << Peers;
	}
	virtual size_t get_object_size() const
	{
		LIVE_ASSERT( Peers.size() < 30 );
		LIVE_ASSERT(PeerInfoSize == REDIRECT_PEER_INFO::object_size);
		return sizeof(UINT8) * 2 + REDIRECT_PEER_INFO::object_size * Peers.size();
	}
};


class PPErrorRefuseNoDegree : public PPErrorRefuse
{
public:
	PPRedirectInfo RedirectInfo;

	PPErrorRefuseNoDegree() : PPErrorRefuse( PP_REFUSE_NO_DEGREE )
	{
	}

	virtual void write_object( data_output_stream& os ) const
	{
		PPErrorRefuse::write_object( os );
		RedirectInfo.write_object( os );
	}
	virtual size_t get_object_size() const
	{
		return PPErrorRefuse::get_object_size() + RedirectInfo.get_object_size();
	}
};

class PPErrorNoPiece : public serializable
{
public:
	explicit PPErrorNoPiece( SubPieceUnit subPieceUnit ) : m_SubPieceUnit( subPieceUnit )
	{
	}

	virtual bool read_object( data_input_stream& is )
	{
		LIVE_ASSERT(false);
		return false;
	}
	virtual void write_object(data_output_stream& os) const
	{
		os << m_SubPieceUnit;
	}
	virtual size_t get_object_size() const
	{
		return SubPieceUnit::object_size;
	}

protected:
	SubPieceUnit m_SubPieceUnit;
};

class PPErrorKick : public serializable
{
public:
	explicit PPErrorKick( UINT32 banTime, UINT32 kickReason ) : m_BanTime( banTime ), m_KickReason( kickReason )
	{
	}

	virtual bool read_object( data_input_stream& is )
	{
		LIVE_ASSERT(false);
		return false;
	}
	virtual void write_object(data_output_stream& os) const
	{
		os << m_BanTime << m_KickReason;
	}
	virtual size_t get_object_size() const
	{
		return sizeof( UINT32 ) * 2;
	}

protected:
	UINT32 m_BanTime;
	UINT32 m_KickReason;
};


class PPErrorInfo : public serializable
{
public:
	UINT16 ErrorCode;
	UINT16 ErrorLength;

	PPErrorInfo()
	{
		ErrorCode = 0;
		ErrorLength = 0;
	}

	virtual bool read_object( data_input_stream& is )
	{
		return is >> ErrorCode >> ErrorLength;
	}
	virtual void write_object(data_output_stream& os) const
	{
		os << ErrorCode << ErrorLength;
	}
	virtual size_t get_object_size() const
	{
		return sizeof( UINT16 ) * 2;
	}

};


#endif