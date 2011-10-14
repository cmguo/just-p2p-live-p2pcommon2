
#include "StdAfx.h"


#include <synacast/protocol/data/SubPieceUnit.h>
#include <synacast/protocol/data/SimpleSocketAddress.h>
#include <synacast/protocol/data/PeerAddress.h>
#include <synacast/protocol/data/PeerAddressUtil.h>
#include <synacast/protocol/data/PeerMinMax.h>
#include <synacast/protocol/data/PeerStatus.h>
#include <synacast/protocol/data/NetInfo.h>
#include <synacast/protocol/data/TrackerAddress.h>
#include <synacast/protocol/data/CandidatePeerInfo.h>

#include <synacast/protocol/base.h>
#include <synacast/protocol/DataIO.h>
#include <synacast/protocol/data/NetType.h>
#include <synacast/protocol/MediaPacket.h>
#include <synacast/protocol/MonoMediaPiece.h>
#include <synacast/protocol/DataSigner.h>

#include <ppl/io/data_input_stream.h>
#include <ppl/net/inet.h>
#include <ppl/text/conv.h>
#include <ppl/data/numeric.h>

#include <ppl/io/data_output_stream.h>
#include <ppl/io/data_input_stream.h>
#include <ppl/data/guid_io.h>
#include <ppl/data/tuple.h>
#include <ppl/stl/stream.h>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/classification.hpp>


#include <boost/static_assert.hpp>

BOOST_STATIC_ASSERT(PPL_MAX_PIECE_DATA_LENGTH == 65535);


bool operator==(const PEER_CORE_INFO& val1, const PEER_CORE_INFO& val2);
bool operator==(const NET_TYPE& val1, const NET_TYPE& val2);

bool operator==(const ::CANDIDATE_PEER_INFO& val1, const ::CANDIDATE_PEER_INFO& val2);
bool operator==(const PEER_STATUS& val1, const PEER_STATUS& val2);
bool operator==(const PEER_STATUS_INFO& x, const PEER_STATUS_INFO& y);

//bool operator==(const EXTRA_PEER_NET_INFO& val1, const EXTRA_PEER_NET_INFO& val2);

/// 生成随机的PEER_STATUS
void RandomData(PEER_STATUS& status);

/// 生成随机的PEER_ADDRESS
void RandomData(PEER_ADDRESS& addr);

/// 生成随机的CANDIDATE_PEER_INFO
void RandomPeer(::CANDIDATE_PEER_INFO& info);

/// 生成随机的CANDIDATE_PEER_INFO
void RandomPeers(size_t count, ::CANDIDATE_PEER_INFO peers[]);

#include <ppl/net/inet.h>













std::ostream& operator<<( std::ostream& os, const data_input_stream& is )
{
	return os << "(data_input_stream:" << is.total_size() << ":" << is.position() << ":" << is.available() << ")";
}




void PeerAddressUtil::ParseAddressList( std::set<PEER_ADDRESS>& addrs, const tstring& s )
{
	addrs.clear();
	std::vector<tstring> peers;
	boost::split( peers, s, boost::is_any_of( "|" ) );
	for (size_t index = 0; index < peers.size(); ++index )
	{
		tstring peer = peers[index];
		boost::trim( peer );
		std::vector<tstring> res;
		boost::split( res, peer, boost::is_any_of( _T(":") ), boost::token_compress_on );
		if ( res.size() == 3 )
		{
			PEER_ADDRESS addr;
			addr.IP = ::inet_addr(CT2A(res[0].c_str()));
			if ( addr.IP != INADDR_NONE && addr.IP != INADDR_ANY && numeric<UINT16>::try_parse( res[1], addr.TcpPort ) && numeric<UINT16>::try_parse( res[2], addr.UdpPort ) )
			{
				addrs.insert(addr);
			}
		}
	}
}

void PeerAddressUtil::ParseIPList( std::set<UINT32>& ips, const tstring& s )
{
	ips.clear();
	std::vector<tstring> peers;
	boost::split( peers, s, boost::is_any_of( "|" ) );
	for (size_t index = 0; index < peers.size(); ++index )
	{
		tstring peer = peers[index];
		UINT32 ip = ::inet_addr(CT2A(peer.c_str()));
		if ( ip != INADDR_NONE && ip != INADDR_ANY )
		{
			ips.insert(ip);
		}
	}
}

bool PeerAddressUtil::ParseAddress( PEER_ADDRESS& addr, const string& s )
{
	std::vector<string> res;
	boost::split( res, s, boost::is_any_of( ":" ), boost::token_compress_on );
	if ( res.size() != 3 )
		return false;
	IPAddress ipaddr( res[0].c_str() );
	UINT16 tcpPort = numeric<UINT16>::parse( res[1], 0 );
	UINT16 udpPort = numeric<UINT16>::parse( res[2], 0 );
	if ( ipaddr.IsInvalidClient() || 0 == tcpPort || 0 == udpPort )
	{
		return false;
	}
	addr.IP = ipaddr.GetAddress();
	addr.TcpPort = tcpPort;
	addr.UdpPort = udpPort;
	return true;
}

std::ostream& operator<<(std::ostream& os, const TRACKERADDR& trackerAddr)
{
	const char* type = (trackerAddr.Type == PPL_TRACKER_TYPE_TCP) ? "TCP" : ((trackerAddr.Type == PPL_TRACKER_TYPE_HTTP) ? "HTTP" : "UDP");
	os << "(" << type << ":" << IPAddress(trackerAddr.IP) << ":" << trackerAddr.Port << ")";
	return os;
}


//#ifdef NEED_LOG

std::ostream& operator<<(std::ostream& os, const PEER_MINMAX& minmax)
{
	os << "(" << minmax.MinIndex << "-" << minmax.MaxIndex << ":" << minmax.MaxIndex - minmax.MinIndex << ")";
	return os;
}

std::ostream& operator<<(std::ostream& os, const PEER_ADDRESS& addr)
{
	os << "(" << IPAddress(addr.IP) << ":" << addr.TcpPort << ":" << addr.UdpPort << ")";
	return os;
}

std::ostream& operator<<(std::ostream& os, const PORT_PAIR& ports)
{
	os << "(" << ports.TCPPort << ":" << ports.UDPPort << ")";
	return os;
}

std::ostream& operator<<(std::ostream& os, const SimpleSocketAddress& addr)
{
	os << "(" << IPAddress(addr.IP) << ":" << addr.Port << ")";
	return os;
}

std::ostream& operator<<(std::ostream& os, const TRACKER_ADDRESS& trackerAddr)
{
	const char* type = (trackerAddr.Type == PPL_TRACKER_TYPE_TCP) ? "TCP" : ((trackerAddr.Type == PPL_TRACKER_TYPE_HTTP) ? "HTTP" : "UDP");
	os << "(" << type << ":" << IPAddress(trackerAddr.IP) << ":" << trackerAddr.Port << ")";
	return os;
}

std::ostream& operator<<(std::ostream& os, const TRACKER_LOGIN_ADDRESS& trackerAddr)
{
	return os << trackerAddr.ServerAddress << trackerAddr.LoginAddress;
}

std::ostream& operator<<(std::ostream& os, const NET_TYPE& netType)
{
	return os << netType.ISP << ":" << netType.Country << ":" << netType.Province << ":" << netType.City;
}

std::ostream& operator<<(std::ostream& os, const SubPieceUnit& unit)
{
	return os << ppl::make_tuple(unit.PieceIndex, static_cast<int>(unit.SubPieceIndex));
}

//#endif




#ifdef _RUN_TEST

#include <ppl/util/random.h>


bool operator==(const NET_TYPE& val1, const NET_TYPE& val2)
{
	return val1.Country == val2.Country && val1.ISP == val2.ISP && 
		val1.Province == val2.Province && val1.City == val2.City;
}

/*
bool operator==(const EXTRA_PEER_NET_INFO& x, const EXTRA_PEER_NET_INFO& y)
{
	return x.DetectedIP == y.DetectedIP && x.DetectedUDPPort == y.DetectedUDPPort && x.DegreeLeft == y.DegreeLeft && x.InDegree == y.InDegree;
}*/

bool operator==(const PEER_MINMAX& val1, const PEER_MINMAX& val2)
{
	return val1.MinIndex == val2.MinIndex && val1.MaxIndex == val2.MaxIndex;
}

bool operator==(const PEER_CORE_INFO& x, const PEER_CORE_INFO& y)
{
	return false;
}


bool operator==(const CANDIDATE_PEER_INFO& val1, const CANDIDATE_PEER_INFO& val2)
{
	return val1.Address == val2.Address && val1.CoreInfo == val2.CoreInfo;
}

bool operator==(const PEER_STATUS& val1, const PEER_STATUS& val2)
{
	return val1.UploadBWLeft == val2.UploadBWLeft 
		&& val1.DegreeLeft == val2.DegreeLeft 
		&& val1.OutDegree == val2.OutDegree 
		&& val1.InDegree == val2.InDegree 
		&& val1.SkipPercent == val2.SkipPercent;
}

bool operator==(const PEER_STATUS_INFO& x, const PEER_STATUS_INFO& y)
{
	return x.MinMax == y.MinMax && x.Status == y.Status;
}


void RandomPeer(CANDIDATE_PEER_INFO& info)
{
	RandomGenerator rnd;
	info.Address.IP = rnd.NextDWord();
	info.Address.TcpPort = rnd.NextWord();
	info.Address.UdpPort = rnd.NextWord();
	info.CoreInfo.PeerType = rnd.NextByte();
	info.CoreInfo.Reserved = rnd.NextByte();
	for (int i = 0; i < 7; ++i)
	{
		info.CoreInfo.Reserved1[i] = rnd.NextByte();
	}
}

void RandomPeers(size_t count, CANDIDATE_PEER_INFO peers[])
{
	Random rnd;
	for (size_t i = 0; i < count; ++i)
	{
		CANDIDATE_PEER_INFO& info = peers[i];
		RandomPeer(info);
	}
}

void RandomData(PEER_ADDRESS& addr)
{
	RandomGenerator rnd;
	addr.IP = rnd.NextDWord();
	addr.TcpPort = rnd.NextWord();
	addr.UdpPort = rnd.NextWord();
}

void RandomData(PEER_STATUS& status)
{
	Random rnd;
	status.DegreeLeft = static_cast<INT8>( rnd.NextByte() );
	status.InDegree = rnd.NextByte();
	status.OutDegree = rnd.NextByte();
	status.Qos = rnd.NextWord();
	status.SkipPercent = rnd.NextByte();
	status.UploadBWLeft = rnd.NextWord();
}


#endif





#ifdef _DEBUG
void CheckDataPiece(const MonoMediaDataPiece* piece)
{
	LIVE_ASSERT(piece != NULL);
	LIVE_ASSERT(piece->GetPieceType() == PPDT_MEDIA_DATA);
	LIVE_ASSERT(piece->GetPieceIndex() > piece->GetHeaderPiece());
	//LIVE_ASSERT(piece->CheckValid());
}
void CheckHeaderPiece(const MonoMediaHeaderPiece* piece)
{
	LIVE_ASSERT(piece != NULL);
	LIVE_ASSERT(piece->GetPieceType() == PPDT_MEDIA_HEADER);
	//LIVE_ASSERT(piece->CheckValid());
}

#ifdef BOOST_SHARED_PTR_HPP_INCLUDED
void CheckDataPiece(MonoMediaDataPiecePtr piece)
{
	LIVE_ASSERT(piece);
	CheckDataPiece(piece.get());
	//LIVE_ASSERT(piece->CheckValid());
}
void CheckHeaderPiece(MonoMediaHeaderPiecePtr piece)
{
	LIVE_ASSERT(piece);
	CheckHeaderPiece(piece.get());
	//LIVE_ASSERT(piece->CheckValid());
}
void CheckSubPiece(SubMediaPiecePtr subPiece)
{
	LIVE_ASSERT(subPiece);
	//	LIVE_ASSERT(subPiece->GetPieceType() == PPDT_MEDIA_DATA);
	//	LIVE_ASSERT(subPiece->GetPieceLength() == subPiece->GetSubPieceDataSize());
	LIVE_ASSERT(subPiece->GetSubPieceIndex() < subPiece->GetSubPieceCount());
	//	LIVE_ASSERT(subPiece->GetPieceIndex() > subPiece->GetHeaderPiece());
	//	LIVE_ASSERT(subPiece->GetSubPieceCount() < 64 && subPiece->GetSubPieceCount() > 0);
	//	LIVE_ASSERT(subPiece->GetSubPieceDataSize() > 0 && subPiece->GetSubPieceDataSize() <= 1400);
}
#endif

#endif




#ifdef _PPL_RUN_TEST

#include <ppl/util/random.h>
#include <ppl/util/test_case.h>
#include <boost/array.hpp>
#include <synacast/protocol/PeerPacket.h>

class PeerPacketTestCase : public ppl::util::test_case
{
public:
	virtual void do_run()
	{
		BYTE buf[2 * 1024];
		{
			SubPieceUnitRequestPacket req;
			req.AddOneSubPieceUnit( SubPieceUnit( 2, 3 ) );
			LIVE_ASSERT( req.get_object_size() == 8 );
			req.SubPieces.assign( 15, SubPieceUnit( 12, 43 ) );
			LIVE_ASSERT( req.get_object_size() == 78 );
			req.Write( buf, 1920 );
			{
				SubPieceUnitRequestPacket req2;
				LIVE_ASSERT( req2.Read( buf, req.get_object_size() ) );
				LIVE_ASSERT( req2.SubPieceLength == req.SubPieceLength );
				LIVE_ASSERT( req2.SubPieces == req.SubPieces );
			}
			{
				SubPieceUnitRequestPacket emptyPacket;
				emptyPacket.Write( buf, 1920 );
				SubPieceUnitRequestPacket emptyPacketParser;
				LIVE_ASSERT( emptyPacketParser.Read( buf, emptyPacket.get_object_size() ) );
			}
		}
		{
			PPHuffmanAnnounce announce;
			LIVE_ASSERT( announce.get_object_size() == 17 );
		}
	}
};


inline void CheckPacketValid(const MonoMediaPiece& packet)
{
	//	LIVE_ASSERT(packet.CheckValid());
	//	LIVE_ASSERT(packet.GetPieceLength() + sizeof(PP_MEDIA) + sizeof(UINT8) + MediaPiecePacket::SIGNATURE_LENGTH == packet.GetSize());
}

class MediaPieceTestCase : public ppl::util::test_case
{
	virtual void DoRun()
	{
		MonoMediaDataPiece dataPiece(3, 2, 1, 5, "12345");
		//	LIVE_ASSERT(dataPiece.CheckValid());
		LIVE_ASSERT(dataPiece.GetPieceIndex() == 3);
		LIVE_ASSERT(dataPiece.GetTimeStamp() == 2);
		LIVE_ASSERT(dataPiece.GetHeaderPiece() == 1);
		LIVE_ASSERT(dataPiece.GetPieceLevel() == 1);
		LIVE_ASSERT(dataPiece.GetPieceType() == PPDT_MEDIA_DATA);
	}
};
CPPUNIT_TEST_SUITE_REGISTRATION(MediaPieceTestCase);

class MediaPacketTestCase : public ppl::util::test_case
{
	Random m_random;
	boost::array<BYTE, 32*1024> m_buffer;
public:
	void TestMediaHeader()
	{
		/*		MediaHeaderPiecePacket mediaHeader(113, PPMT_MEDIA_ASF_FILE, 8, "ASF.FILE", 10, "asf.file11", 4, "4444");
		string info(mediaHeader.GetInfo(), mediaHeader.GetInfoLength());
		string header(mediaHeader.GetHeader(), mediaHeader.GetHeaderLength());
		string profile(mediaHeader.GetProfile(), mediaHeader.GetProfileLength());
		LIVE_ASSERT(info == "ASF.FILE");
		LIVE_ASSERT(header == "asf.file11");
		LIVE_ASSERT(profile == "4444");
		LIVE_ASSERT(mediaHeader.GetMediaType() == PPMT_MEDIA_ASF_FILE);
		LIVE_ASSERT(mediaHeader.GetPieceIndex() == 113);
		std::vector<BYTE> key(20, 20);
		mediaHeader.Sign(key);
		mediaHeader.Write(m_buffer.c_array(), mediaHeader.GetSize());

		MediaHeaderPiecePacketParser mediaHeader2(mediaHeader.GetData(), mediaHeader.GetSize());
		{
		string info(mediaHeader.GetInfo(), mediaHeader.GetInfoLength());
		string header(mediaHeader.GetHeader(), mediaHeader.GetHeaderLength());
		string profile(mediaHeader.GetProfile(), mediaHeader.GetProfileLength());
		LIVE_ASSERT(info == "ASF.FILE");
		LIVE_ASSERT(header == "asf.file11");
		LIVE_ASSERT(profile == "4444");
		LIVE_ASSERT(mediaHeader.GetMediaType() == PPMT_MEDIA_ASF_FILE);
		LIVE_ASSERT(mediaHeader.GetPieceIndex() == 113);
		}
		LIVE_ASSERT(mediaHeader2.CheckValid());
		LIVE_ASSERT(mediaHeader2->PieceLength + sizeof(PP_MEDIA) == mediaHeader2->GetSize());
		{
		MediaHeaderPiecePacketParser mediaHeader3(mediaHeader.GetData(), mediaHeader.GetSize() - 1);
		LIVE_ASSERT(!mediaHeader3.CheckValid());
		}*/
	}
	void TestMediaPayload()
	{
		/*		PPMediaPayloadPacket mediaPayload(2, 7, "Payload");
		CheckPacketValid(mediaPayload);
		string payload(mediaPayload.GetPayload(), mediaPayload.GetPayloadLength());
		LIVE_ASSERT(payload == "Payload");

		PPMediaPayloadPacket mediaPayload2(mediaPayload.GetData(), mediaPayload.GetSize());
		payload.assign(mediaPayload2.GetPayload(), mediaPayload2.GetPayloadLength());
		LIVE_ASSERT(payload == "Payload");
		CheckPacketValid(mediaPayload2);
		{
		PPMediaPayloadPacket mediaPayload3(mediaPayload.GetData(), mediaPayload.GetSize() - 1);
		LIVE_ASSERT(!mediaPayload3.CheckValid());
		}*/
	}
	void TestMediaData()
	{
		/*		string str = m_random.GenerateRandomString(30);
		MediaDataPiecePacket oldMediaData(5, 66665, 55, str.size(), str.data(), 20, reinterpret_cast<const BYTE*>(string(20, '1').data()));
		string str2((const BYTE*)oldMediaData.GetMediaData(), oldMediaData.GetMediaDataLength());
		LIVE_ASSERT(str == str2);
		LIVE_ASSERT(oldMediaData.GetTimeStamp() == 66665);

		MediaDataPiecePacket datapack(110, 111, 011);
		bool res = datapack.AppendData(1, "1");
		LIVE_ASSERT(res);
		res = datapack.AppendData(2, "22");
		LIVE_ASSERT(res);
		res = datapack.AppendData(3, "333");
		LIVE_ASSERT(res);
		res = datapack.AppendData(4, "4444");
		LIVE_ASSERT(res);
		std::vector<BYTE> key(20, 30);
		datapack.Sign(key);
		datapack.Write(m_buffer.c_array(), datapack.GetSize());

		CheckPacketValid(datapack);

		MediaDataPiecePacket datapack2(datapack.GetData(), datapack.GetSize());
		int len = datapack2.GetPieceLength();
		LIVE_ASSERT(len == 10 + sizeof(PP_MEDIA_DATA));
		str.assign((const BYTE*)datapack2.GetMediaData(), datapack2.GetMediaDataLength());
		LIVE_ASSERT(str == "1223334444");

		{
		MediaDataPiecePacket datapack3(datapack.GetData(), datapack.GetSize() - 1);
		LIVE_ASSERT(!datapack3.CheckValid());
		}*/
	}
	void TestData2()
	{
	}
protected:
	virtual void DoRun()
	{
		TestMediaHeader();
		TestMediaPayload();
		TestMediaData();
		TestData2();
	}
};

/*
class TestDynamicPacket : public DynamicPacket
{
friend class DynamicPacketTestCase;
};

class DynamicPacketTestCase : public ppl::util::test_case
{
public:
virtual void DoRun()
{
TestDynamicPacket packet;
CheckBuffer(packet, 0, true);

LIVE_ASSERT(!packet.IncreaseSize(1));
CheckBuffer(packet, 0, true);

packet.InitBuffer(111, 100);
CheckBuffer(packet, 100);

LIVE_ASSERT(packet.IncreaseSize(1));
CheckBuffer(packet, 101);

LIVE_ASSERT(packet.IncreaseSize(10));
CheckBuffer(packet, 111);

LIVE_ASSERT(!packet.IncreaseSize(1));
CheckBuffer(packet, 111);

string str("nihao, world.");
packet.Assign(str.data(), str.size());
CheckBuffer(packet, str.size());
LIVE_ASSERT(0 == memcmp(packet.GetData(), str.data(), str.size()));
}

void CheckBuffer(TestDynamicPacket& packet, size_t size, bool isNull = false)
{
LIVE_ASSERT(packet.GetSize() == size);
LIVE_ASSERT(packet.GetBuffer() == packet.GetData());
if (isNull)
{
LIVE_ASSERT(packet.GetData() == NULL);
}
else
{
LIVE_ASSERT(packet.GetData() != NULL);
}
}
};*/

class HMACTestCase : public ppl::util::test_case
{
	virtual void DoRun()
	{
		SignatureData result;
		{
			std::vector<BYTE> src(16);
			for (BYTE i = 0; i< 16; ++i)
			{
				src[i] = i * 11;
			}
			DataSigner signer(src);
			bool res = signer.Sign(&src[0], src.size(), result);
			LIVE_ASSERT(res);
		}
		{
			std::vector<BYTE> src(16);
			for (BYTE i = 0; i< 16; ++i)
			{
				src[i] = i * 11;
			}
			DataSigner signer(src);
			bool res = signer.Verify(&src[0], src.size(), result.data());
			LIVE_ASSERT(res);
		}
	}
};


//CPPUNIT_TEST_SUITE_REGISTRATION(DynamicPacketTestCase);

CPPUNIT_TEST_SUITE_REGISTRATION(MediaPacketTestCase);
CPPUNIT_TEST_SUITE_REGISTRATION(PeerPacketTestCase);
CPPUNIT_TEST_SUITE_REGISTRATION(HMACTestCase);

/*

CHMACSHA1 hmac;
BYTE key[8] = { 1, 2, 3, 4, 5};
BYTE src[8] = { 1, 2, 3, 4, 5};
BYTE digest[20] = { 0 };

bool res = hmac.HashData(key, 8, src, 8, digest);
LIVE_ASSERT(res);
*/

#endif








