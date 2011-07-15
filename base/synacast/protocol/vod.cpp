
#include "StdAfx.h"

#include <synacast/protocol/vod.h>
#include <synacast/util/IDGenerator.h>

#include <ppl/util/time_counter.h>

#include <synacast/protocol/PacketStream.h>

using namespace ppl::util::detail;

static ppl::io::data_output_stream & operator<<(ppl::io::data_output_stream & os, CPeerAddr const & addr)
{
        os.write_n(&addr, sizeof(addr));
        return os;
}

static inline boost::uint16_t host_to_little_endian_short(
    boost::uint16_t v)
{
    char ch[2];
    ch[0] = char(v & 0x00ff);
    ch[1] = char((v >> 8) & 0x00ff);
    return *(boost::uint16_t *)ch;
}


UINT16  VODPacketBuilder::CheckSum(UINT16 *pucBuffer, INT32 size) 
{
	assert(pucBuffer);

	unsigned long cksum=0;

	while (size > 1) 
	{
		cksum += *pucBuffer++;
		size -= sizeof(UINT16);
	}
	if (size) 
	{
		cksum += *(UINT8*)pucBuffer;
	}
	cksum = (cksum >> 16) + (cksum & 0xffff);
	cksum += (cksum >>16);
	return (UINT16)(~cksum);
}

void VODPacketBuilder::Confuse(INT8 *pcBuf, UINT32 uBufLen, UINT32 uMsgType)
{
	if (NULL == pcBuf || uBufLen < sizeof(CNetHeader) )
		return;

	//���buf���ȴ���Э�鳤�ȣ���ֻ���ǰ���Э�鳤��
	UINT32 uProtocolLen = sizeof(CNetHeader);
	switch(uMsgType)
	{
	case NHTYPE_CONFUSE_PFS_SHORT_P2PHEADER:
	case NHTYPE_CONFUSE_KEEPALIVE_SHORT_P2PHEADER:
		uProtocolLen = sizeof(CNetHeader) + PFS_SHORT_PROTOCOL_HEADER_LENGTH;
		break;

	case NHTYPE_CONFUSE_PFS:
	case NHTYPE_CONFUSE_KEEPALIVE:
		uProtocolLen = sizeof(CNetHeader) + PFS_PROTOCOL_HEADER_LENGTH;
		break;

	default:	//ʶ���˵�P2PЭ��ͷ��ֱ�ӷ���
		return;
	}

	if (uBufLen > uProtocolLen )
	{
		uBufLen = uProtocolLen;
	}

	CNetHeader *pnh = (CNetHeader*) pcBuf;    
	INT8 * ptr_a = (INT8 *)&pnh->m_uConfusionKey;
	INT8 * ptr_b = ptr_a + 1;

	INT8 * ptr_beg = (INT8 *)&pnh->m_usVersion;
	INT8 * ptr_end = ptr_beg + uBufLen - 4;

	if (ptr_b == ptr_end) 
	{
		ptr_b = ptr_beg; // �����������Σ�KyeΪ1B�����������
	}

	int len = uBufLen - 4;
	while (len --)
	{
		* ptr_b =  (*ptr_a) ^ (*ptr_b);
		ptr_a ++;
		ptr_b ++;
		if (ptr_a == ptr_end) 
		{
			ptr_a = ptr_beg;
		}
		if (ptr_b == ptr_end) 
		{
			ptr_b = ptr_beg;
		}
	}

}

/******************************************************************
������: unConfuse
�����б�:
INT8 *pcBuf��(�������)��Ҫȥ������������Buffer
UINT32 uBufLen��(����)Buffer�ĳ���
����ֵ:
��
����: 
�������pcBuf����������VoDЭ��(����ͷ��+P2Pͷ��)��ȥ������
�����pcBuf��ȥ������֮������ݣ�
ע�⣺
1.���������buffer����С������ͷ������
2.֧�ֳ���P2Pͷ�����������ȷP2Pheader������
******************************************************************/
void VODPacketBuilder::unConfuse(INT8 *pcBuf, UINT32 uBufLen, UINT32 uMsgType)
{
	if (NULL == pcBuf || uBufLen < sizeof(CNetHeader) )
		return;

	//���buf���ȴ���Э�鳤�ȣ���ֻ���ǰ���Э�鳤��
	UINT32 uProtocalLen = sizeof(CNetHeader);
	switch(uMsgType)
	{
	case NHTYPE_CONFUSE_PFS_SHORT_P2PHEADER:
	case NHTYPE_CONFUSE_KEEPALIVE_SHORT_P2PHEADER:
		uProtocalLen = sizeof(CNetHeader) + PFS_SHORT_PROTOCOL_HEADER_LENGTH;
		break;

	case NHTYPE_CONFUSE_PFS:
	case NHTYPE_CONFUSE_KEEPALIVE:
		uProtocalLen = sizeof(CNetHeader) + PFS_PROTOCOL_HEADER_LENGTH;
		break;

	default:	//ʶ���˵�P2PЭ��ͷ��ֱ�ӷ���
		return;
	}

	if (uBufLen > uProtocalLen )
	{
		uBufLen = uProtocalLen;
	}

	CNetHeader *pnh = (CNetHeader*) pcBuf;    
	INT8 * ptr_a = (INT8 *)&pnh->m_uConfusionKey;
	INT8 * ptr_b = ptr_a - 1;


	INT8 * ptr_beg = (INT8 *)&pnh->m_usVersion - 1;
	INT8 * ptr_end = ptr_beg + uBufLen - 4;

	if (ptr_b == ptr_beg) 
	{
		ptr_b = ptr_end; // �����������Σ�Key����ǰ
	}

	int len = uBufLen - 4;
	while (len --)
	{
		* ptr_a =  (*ptr_a) ^ (*ptr_b);
		ptr_a --;
		ptr_b --;
		if (ptr_a == ptr_beg) 
		{
			ptr_a = ptr_end;
		}
		if (ptr_b == ptr_beg) 
		{
			ptr_b = ptr_end;
		}
	}
}


void VODPacketBuilder::InitNetHeader( CNetHeader& header, UINT16 netType )
{
	header.m_usNetType = host_to_little_endian_short(netType);
	header.m_usCheckSum = 0;
	header.m_usVersion = host_to_little_endian_short(m_PeerVersion);
	header.m_usReserve = 0;
	header.m_uConfusionKey = ::GetTickCount();
	header.m_uFileHandle = NULL;

}

VODPacketBuilder::VODPacketBuilder( UINT16 peerVersion ) : m_PeerVersion(peerVersion), m_DataLength( 0 )
{

}


StunRequestPacketBuilder::StunRequestPacketBuilder( boost::shared_ptr<IDGenerator> transactionID, UINT16 peerVersion )
	: VODPacketBuilder( peerVersion )
	, m_TransactionID( transactionID )
{

}

void StunRequestPacketBuilder::BuildLoginPacket( const CPeerAddr& peerAddr )
{
	this->InitPacket();

	m_RequestPacket.ProtocolHeader.protocolType = REQ_PUB_PORT;
	m_RequestPacket.ProtocolHeader.length = host_to_little_endian_short(sizeof(CPeerAddr));

	WRITE_MEMORY(m_RequestPacket.Data, peerAddr, CPeerAddr);

	this->BuildPacket();
}

void StunRequestPacketBuilder::BuildTransmitPacket( const BYTE* data, size_t size, const CPeerAddr& localAddr, const CPeerAddr& targetAddr )
{
	this->InitPacket();

	m_RequestPacket.ProtocolHeader.protocolType = REQ_TRANSMIT_PACKEG;
	m_RequestPacket.ProtocolHeader.length = host_to_little_endian_short(size + sizeof(CPeerAddr) * 2);

	PacketOutputStream os(m_RequestPacket.Data, PPL_STUN_REQUEST_DATA_LENGTH_MAX);
	//os.write_struct(localAddr);
	//os.write_struct(targetAddr);
        os << localAddr << targetAddr;
	os.write_n(data, size);

	this->BuildPacket();
}

void StunRequestPacketBuilder::InitPacket()
{

	m_RequestPacket.ProtocolHeader.pfsSymbol = PFS_CMD_HEADER;
	m_RequestPacket.ProtocolHeader.protocolType = 0;
	m_RequestPacket.ProtocolHeader.length = 0;
	m_RequestPacket.ProtocolHeader.reserve = 0;

	this->InitStunNetHeader(m_RequestPacket.NetHeader);
}

void StunRequestPacketBuilder::BuildPacket()
{
	size_t usBufLen = sizeof(CNetHeader) + sizeof(ShortP2PProtocolHeader) + host_to_little_endian_short(m_RequestPacket.ProtocolHeader.length);

	INT8* pcBuf = reinterpret_cast<INT8*>( &m_RequestPacket );
	Confuse(pcBuf,usBufLen, host_to_little_endian_short(m_RequestPacket.NetHeader.m_usNetType));
	m_RequestPacket.NetHeader.m_usCheckSum = CheckSum((UINT16*)pcBuf, usBufLen);

	m_DataLength = usBufLen;
}

void StunRequestPacketBuilder::InitStunNetHeader( CNetHeader& header )
{
	this->InitNetHeader(header, NHTYPE_CONFUSE_PFS_SHORT_P2PHEADER);
}

VODBootServerRequestPacketBuilder::VODBootServerRequestPacketBuilder( UINT16 peerVersion )
	: VODPacketBuilder(peerVersion)
{
	
}

void VODBootServerRequestPacketBuilder::Build()
{
	FILL_ZERO(m_RequestPacket.ProtocolHeader);
	m_RequestPacket.ProtocolHeader.pfsSymbol = PFS_CMD_HEADER;
	m_RequestPacket.ProtocolHeader.protocolType = REQ_SN;
	m_RequestPacket.ProtocolHeader.length = 0;

	this->InitNetHeader(m_RequestPacket.NetHeader, NHTYPE_CONFUSE_PFS);

	size_t usBufLen = sizeof(CNetHeader) + sizeof(CCommandProtocolHeader) + m_RequestPacket.ProtocolHeader.length;

	INT8* pcBuf = reinterpret_cast<INT8*>( &m_RequestPacket );
	Confuse(pcBuf,usBufLen,  NHTYPE_CONFUSE_PFS);
	m_RequestPacket.NetHeader.m_usCheckSum = CheckSum((UINT16*)pcBuf, usBufLen);

	m_DataLength = usBufLen;
}
