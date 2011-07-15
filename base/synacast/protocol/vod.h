
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

//Э��ͷ����36�ֽ�
typedef struct CCommandProtocolHeader
{
	//pfs��־,1�ֽ�,Ĭ��Ϊcommand����
	UINT8 pfsSymbol;

	//Э������,1�ֽ�
	UINT8 protocolType;

	//���Ӳ���,1�ֽ�
	UINT8 addParam;

	//��������������,1�ֽ�    
	UINT8 codeType;

	//����������,2�ֽ�    
	UINT16 length;

	//���ò���4�ֽ�
	UINT32 reserve;

	//ʱ���,4�ֽ�    
	UINT32 requestsendtime;

	//4�ֽڵ���ԴID����֤ȫ��Ψһ�ģ���Ϊ��Դ���ݵ�MD5ֵ��
	char strResID[34];

	//��ý�����ݷ�Ƭ��index
	UINT32 piece_index;

}COMMANDPROTOCOLHEADER, *LPCOMMANDPROTOCOLHEADER;

//����Э��ͷ����
typedef struct ShortP2PProtocolHeader
{
	//pfs��־,1�ֽ�,Ĭ��Ϊcommand����
	UINT8 pfsSymbol;

	//Э������,1�ֽ�
	UINT8 protocolType;

	//����������,2�ֽ�    
	UINT16 length;

	//���ò���4�ֽ�
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
		//ָ��������ԣ���ip�������ip��ȵĻ�����port����
		if(m_uIP < _A.m_uIP)  
			return true;

		if(m_uIP == _A.m_uIP) 
			return m_usPort < _A.m_usPort;

		return false;
	}  

	// IP��ַ�������ֽ���
	UINT32  m_uIP;
	// Port�������ֽ���
	UINT16  m_usPort;
	// �Ƿ������ڵ��־
	bool    m_bIsNAT;
	// NAT����
	INT8    m_cNatType;
};

struct CNetHeader
{
	UINT16 m_usNetType;     //0���籨������
	UINT16 m_usCheckSum;    //2У���
	UINT16 m_usVersion;     //4���ְ汾��
	UINT16 m_usReserve;     //6�����ֶ�
	UINT32 m_uConfusionKey; //8Э�������Key
	UINT32 m_uFileHandle;   //12������(P2Pʹ��)
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

//P2PͷЭ�����Ͷ���
//ת�����󣬽���NAT��
#define REQ_TRANSNAT						0x17
//���󷵻ع���port
#define REQ_PUB_PORT						0x5C
//���ع���port
#define RTN_PUB_PORT						0x5D
//������ת������
#define REQ_TRANSMIT_PACKEG					0x62
#define RTN_TRANSMIT_PACKEG					0x63

#define REQ_SN						0x20
#define RTN_SN						0x21


//pfsЭ���ײ�����
const UINT32 PFS_PROTOCOL_HEADER_LENGTH = sizeof(COMMANDPROTOCOLHEADER);
//pfs����Э��ͷ����
const UINT32 PFS_SHORT_PROTOCOL_HEADER_LENGTH = sizeof(SHORTP2PPROTOCOLHEADER);

////���õ�����Э������
//enum Enum_Protocol_Type
//{
//	TCP = 10,
//	UDP
//};


// ����ͷ����
const UINT32  NETHEADER_LENGTH = sizeof(CNetHeader);

// UDP���ݻ���������
// 1024Ϊ���ݶεĳ���
// ���һλ��0
const UINT32 UDP_BUF_LEN = (1024 + NETHEADER_LENGTH + PFS_SHORT_PROTOCOL_HEADER_LENGTH + 1);

// ����ṹ���������ȣ�ȡUDP���ֵ
const UINT32 TRANSPORT_BUF_LEN = UDP_BUF_LEN;


//����ͷ����Ϣ����
#define  NHTYPE_REQUEST             0x01
//�����������㴦��
#define  NHTYPE_KEEPALIVE           0x05

//֧�ֻ�������Э�������ͷ������
#define  NHTYPE_CONFUSE_DIFF        0x20
//VOD UDPЭ�����������ݴ���ȵ���������
#define  NHTYPE_CONFUSE_PFS         (NHTYPE_REQUEST + NHTYPE_CONFUSE_DIFF)
//VOD UDPЭ�鱥�����Ͱ�
#define  NHTYPE_CONFUSE_KEEPALIVE   (NHTYPE_KEEPALIVE + NHTYPE_CONFUSE_DIFF)
//VOD UDPЭ���о���P2Pͷ������
#define  NHTYPE_CONFUSE_PFS_SHORT_P2PHEADER		0x22
//VOD UDPЭ�龫��P2Pͷ�����͵ı������Ͱ�
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