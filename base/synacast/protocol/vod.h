
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_VOD_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_PROTOCOL_VOD_H_

#include <synacast/protocol/data/SimpleSocketAddress.h>
#include <synacast/protocol/data/PeerAddress.h>
#include <ppl/net/inet.h>
#include <ppl/data/int.h>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

class IDGenerator;

#pragma pack(push, 1)

//协议头长度36字节
typedef struct CCommandProtocolHeader
{
	//pfs标志,1字节,默认为command类型
	UINT8 pfsSymbol;

	//协议类型,1字节
	UINT8 protocolType;

	//附加参数,1字节
	UINT8 addParam;

	//数据区编码类型,1字节    
	UINT8 codeType;

	//数据区长度,2字节    
	UINT16 length;

	//备用参数4字节
	UINT32 reserve;

	//时间戳,4字节    
	UINT32 requestsendtime;

	//4字节的资源ID，保证全局唯一的（定为资源内容的MD5值）
	char strResID[34];

	//流媒体数据分片的index
	UINT32 piece_index;

}COMMANDPROTOCOLHEADER, *LPCOMMANDPROTOCOLHEADER;

//精简协议头定义
typedef struct ShortP2PProtocolHeader
{
	//pfs标志,1字节,默认为command类型
	UINT8 pfsSymbol;

	//协议类型,1字节
	UINT8 protocolType;

	//数据区长度,2字节    
	UINT16 length;

	//备用参数4字节
	UINT32 reserve;

}SHORTP2PPROTOCOLHEADER, *LPSHORTP2PPROTOCOLHEADER;

#pragma pack(pop)

// Peer-Address
struct CPeerAddr
{
	CPeerAddr():m_uIP(0),m_usPort(0),m_bIsNAT(false),m_cNatType(0)
	{}

	CPeerAddr(UINT32 uIP,UINT16 usPort,bool bIsNAT):m_uIP(uIP),m_usPort(usPort),m_bIsNAT(bIsNAT)
	{}    

	bool operator !=(const CPeerAddr &_A) const
	{
		return (m_uIP != _A.m_uIP || m_usPort != _A.m_usPort);
	}

	bool operator ==(const CPeerAddr &_A) const
	{
		return (m_uIP == _A.m_uIP && m_usPort == _A.m_usPort);
	}    

	bool operator <(const CPeerAddr &_A) const
	{
		//指定排序策略，按ip排序，如果ip相等的话，按port排序
		if(m_uIP < _A.m_uIP)  
			return true;

		if(m_uIP == _A.m_uIP) 
			return m_usPort < _A.m_usPort;

		return false;
	}  

	// IP地址，网络字节序
	UINT32  m_uIP;
	// Port，网络字节序
	UINT16  m_usPort;
	// 是否内网节点标志
	bool    m_bIsNAT;
	// NAT类型
	INT8    m_cNatType;
};

struct CNetHeader
{
	UINT16 m_usNetType;     //0网络报文类型
	UINT16 m_usCheckSum;    //2校验和
	UINT16 m_usVersion;     //4数字版本号
	UINT16 m_usReserve;     //6保留字段
	UINT32 m_uConfusionKey; //8协议混淆的Key
	UINT32 m_uFileHandle;   //12网络句柄(P2P使用)
};


const size_t PPL_STUN_REQUEST_DATA_LENGTH_MAX = 1024 * 5;

#pragma pack(push, 1)

struct StunRequestPacket
{
	CNetHeader NetHeader;
	ShortP2PProtocolHeader ProtocolHeader;
	BYTE Data[PPL_STUN_REQUEST_DATA_LENGTH_MAX];
};

struct CommandProtocolPacket
{
	CNetHeader NetHeader;
	CCommandProtocolHeader ProtocolHeader;
	BYTE Data[PPL_STUN_REQUEST_DATA_LENGTH_MAX];
};

#pragma pack(pop)

//P2P头协议类型定义
//转发请求，进行NAT打洞
#define REQ_TRANSNAT						0x17
//请求返回公网port
#define REQ_PUB_PORT						0x5C
//返回公网port
#define RTN_PUB_PORT						0x5D
//无条件转发报文
#define REQ_TRANSMIT_PACKEG					0x62
#define RTN_TRANSMIT_PACKEG					0x63

#define REQ_SN						0x20
#define RTN_SN						0x21


//pfs协议首部长度
const UINT32 PFS_PROTOCOL_HEADER_LENGTH = sizeof(COMMANDPROTOCOLHEADER);
//pfs精简协议头长度
const UINT32 PFS_SHORT_PROTOCOL_HEADER_LENGTH = sizeof(SHORTP2PPROTOCOLHEADER);

////采用的网络协议类型
//enum Enum_Protocol_Type
//{
//	TCP = 10,
//	UDP
//};


// 网络头长度
const UINT32  NETHEADER_LENGTH = sizeof(CNetHeader);

// UDP数据缓冲区长度
// 1024为数据段的长度
// 最后一位补0
const UINT32 UDP_BUF_LEN = (1024 + NETHEADER_LENGTH + PFS_SHORT_PROTOCOL_HEADER_LENGTH + 1);

// 传输结构缓冲区长度，取UDP最大值
const UINT32 TRANSPORT_BUF_LEN = UDP_BUF_LEN;


//网络头部信息类型
#define  NHTYPE_REQUEST             0x01
//保活包，网络层处理
#define  NHTYPE_KEEPALIVE           0x05

//支持混淆加密协议的网络头部类型
#define  NHTYPE_CONFUSE_DIFF        0x20
//VOD UDP协议中请求、数据传输等的网络类型
#define  NHTYPE_CONFUSE_PFS         (NHTYPE_REQUEST + NHTYPE_CONFUSE_DIFF)
//VOD UDP协议饱和类型包
#define  NHTYPE_CONFUSE_KEEPALIVE   (NHTYPE_KEEPALIVE + NHTYPE_CONFUSE_DIFF)
//VOD UDP协议中精简P2P头部类型
#define  NHTYPE_CONFUSE_PFS_SHORT_P2PHEADER		0x22
//VOD UDP协议精简P2P头部类型的饱和类型包
#define  NHTYPE_CONFUSE_KEEPALIVE_SHORT_P2PHEADER	0x23

#define PFS_CMD_HEADER				0xAC









class CPeerAddrUtil
{
public:
	static PEER_ADDRESS ToPeerAddress(const CPeerAddr& src)
	{
		PEER_ADDRESS result;
		result.IP = src.m_uIP;
		result.UdpPort = ntohs( src.m_usPort );
		result.TcpPort = 0;
		return result;
	}
	static CPeerAddr FromPeerAddress(const PEER_ADDRESS& src, bool isNAT, INT8 natType)
	{
		CPeerAddr result;
		result.m_bIsNAT = isNAT;
		result.m_cNatType = natType;
		AssignFromPeerAddress(result, src);
		return result;
	}
	static void AssignFromPeerAddress(CPeerAddr& target, const PEER_ADDRESS& src)
	{
		target.m_uIP = src.IP;
		target.m_usPort = htons(src.UdpPort);
	}
	static CPeerAddr FromSimpleSocketAddress(const SimpleSocketAddress& src, bool isNAT, INT8 natType)
	{
		CPeerAddr result;
		result.m_bIsNAT = isNAT;
		result.m_cNatType = natType;
		result.m_uIP = src.IP;
		result.m_usPort = htons(src.Port);
		return result;
	}
	static InetSocketAddress ToSocketAddress(const CPeerAddr& addr)
	{
		return InetSocketAddress(addr.m_uIP, ntohs(addr.m_usPort));
	}
};



class VODPacketBuilder : private boost::noncopyable
{
public:
	explicit VODPacketBuilder(UINT16 peerVersion);
	void InitNetHeader(CNetHeader& header, UINT16 netType);

	static void unConfuse(INT8 *pcBuf, UINT32 uBufLen, UINT32 uMsgType);
	static UINT16 CheckSum(UINT16 *pucBuffer, INT32 size);
	static void Confuse(INT8 *pcBuf, UINT32 uBufLen, UINT32 uMsgType);

	size_t GetSize() const { return m_DataLength; }

protected:
	UINT16 m_PeerVersion;
	size_t m_DataLength;
};




class StunRequestPacketBuilder : public VODPacketBuilder
{
public:
	explicit StunRequestPacketBuilder( boost::shared_ptr<IDGenerator> transactionID, UINT16 peerVersion );

	void BuildLoginPacket(const CPeerAddr& peerAddr);
	void BuildTransmitPacket(const BYTE* data, size_t size, const CPeerAddr& localAddr, const CPeerAddr& targetAddr);

	const BYTE* GetData() const { return reinterpret_cast<const BYTE*>( &m_RequestPacket ); }

	void InitStunNetHeader(CNetHeader& header);

protected:
	void InitPacket();
	void BuildPacket();

private:
	boost::shared_ptr<IDGenerator> m_TransactionID;
	StunRequestPacket m_RequestPacket;
};


class VODBootServerRequestPacketBuilder : public VODPacketBuilder
{
public:
	explicit VODBootServerRequestPacketBuilder(UINT16 peerVersion);

	void Build();

	const BYTE* GetData() const { return reinterpret_cast<const BYTE*>( &m_RequestPacket ); }

private:
	CommandProtocolPacket m_RequestPacket;
};



#endif