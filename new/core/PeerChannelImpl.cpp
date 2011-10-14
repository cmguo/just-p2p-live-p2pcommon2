
#include "StdAfx.h"

#include "PeerChannelImpl.h"
#include "PeerConnection.h"
#include "common/PacketSender.h"
#include "framework/socket.h"
#include "framework/log.h"

#include <synacast/protocol/PacketBase.h>
#include <synacast/protocol/PacketHead.h>
#include <synacast/protocol/MediaPacket.h>


const size_t MAX_MULTI_TCP_CONNECTIONS = 5;



PeerChannel* PeerChannelFactory::CreateTCP( PeerConnection& pc, boost::shared_ptr<TCPConnectionPacketSender> packetSender, tcp_socket_ptr sock )
{
	return new TCPPeerChannel( pc, packetSender, sock );
}

PeerChannel* PeerChannelFactory::CreateUDP( PeerConnection& pc, boost::shared_ptr<UDPConnectionPacketSender> packetSender, const SimpleSocketAddress& remoteSocketAddr, UINT32 localSessionKey, UINT32 remoteSessionKey )
{
	return new UDPPeerChannel( pc, packetSender, remoteSocketAddr, localSessionKey, remoteSessionKey );
}




PeerChannelImpl::PeerChannelImpl(PeerConnection& pc) : m_Connection(pc)
{
}



TCPPeerChannel::TCPPeerChannel( PeerConnection& pc, boost::shared_ptr<TCPConnectionPacketSender> packetSender, tcp_socket_ptr sock ) 
	: PeerChannelImpl(pc), m_TotalPacketCount(0),m_SentPacketCount( 0 ),m_PacketSender(packetSender), m_Socket(sock)
{
	m_Socket->set_listener(this);
	m_Socket->receive();
}

TCPPeerChannel::~TCPPeerChannel()
{
	m_Socket->close();
}

void TCPPeerChannel::on_socket_receive(tcp_socket* sender, BYTE* data, size_t size)
{
	LIVE_ASSERT(m_Socket.get() == sender);
	m_TotalPacketCount++;
	if (this->HandlePacket(data, size))
	{
		sender->receive();
	}
}

void TCPPeerChannel::on_socket_receive_failed(tcp_socket* sender, int errcode)
{
	LIVE_ASSERT(m_Socket.get() == sender);
	m_Connection.OnPeerChannelError(errcode, ERROR_TYPE_NETWORK);
}

size_t TCPPeerChannel::SendPacket(const PacketBase& packet)
{
	++m_SentPacketCount;
	return m_PacketSender->Send(packet, m_Socket);
}

int TCPPeerChannel::GetSendPending()
{
	return m_Socket->sending_count();
}

bool TCPPeerChannel::HandlePacket(const BYTE* data, size_t size)
{
	if ( size < 1 )
	{
		return m_Connection.OnPeerChannelError(PP_LEAVE_INVALID_PACKET, ERROR_TYPE_PROTOCOL);
	}
	PacketInputStream is( data, size );
	UINT8 action = 0;
	is >> action;
	if ( !is )
	{
		return m_Connection.OnPeerChannelError(PP_LEAVE_INVALID_PACKET, ERROR_TYPE_PROTOCOL);
	}
	LIVE_ASSERT(action != 0);
	return m_Connection.OnPeerChannelData(is, action);
}



UDPPeerChannel::UDPPeerChannel( 
		PeerConnection& pc, boost::shared_ptr<UDPConnectionPacketSender> packetSender, 
		const SimpleSocketAddress& remoteSocketAddr, UINT32 localSessionKey, UINT32 remoteSessionKey )
	: PeerChannelImpl(pc), 
	  m_LocalSessionKey(localSessionKey),m_RemoteSessionKey(remoteSessionKey), m_SequenceID(0),
	  m_RemoteSocketAddress(remoteSocketAddr), m_TotalPacketCount(0), m_ReceivedPacketCount(0), 
	  m_PacketSender(packetSender), m_LastRequestAction( 0 ), m_LastInRequestTransactionID( 0 )
{
	// Added by Tady, 081108: For multi-request-packet.
	m_LastOutRequestTransactionID = 0;
	m_bIsLastMRPResponseReceived = false;
}

UDPPeerChannel::~UDPPeerChannel()
{
}

size_t UDPPeerChannel::SendPacket( const PacketBase& packet )
{
	VIEW_DEBUG("Session: DoSendPacket " << packet.GetAction() << " " << m_Connection << " " << make_tuple(m_LocalSessionKey, m_RemoteSessionKey));
	++m_SequenceID;
	UINT32 transactionID = 0;
	UINT8 action = packet.GetAction();
	if ( PPT_SUB_PIECE_REQUEST == m_LastRequestAction && PPT_SUB_PIECE_DATA == action )
	{
		transactionID = m_LastInRequestTransactionID;
	}
	size_t reval = m_PacketSender->Send(packet, transactionID, m_SequenceID, m_LocalSessionKey, m_RemoteSocketAddress);

	if ( PPT_SUB_PIECE_REQUEST == action ) // Added by Tady, 081108: For multi-request-packet.
	{
		m_LastOutRequestTransactionID = m_PacketSender->GetCurTransactionID();
		m_iOldOutRequestTransactionIDs[m_PacketSender->GetCurTransactionID() % OldRequestIDArray_Len] = m_PacketSender->GetCurTransactionID();
//		m_bIsLastMRPResponseReceived = false;
	}
	return reval;
}

size_t UDPPeerChannel::SendPacket(const PacketBase& packet, bool bNeedRepeatMRP)
{
	VIEW_DEBUG("Session: DoSendPacket " << packet.GetAction() << " " << m_Connection);
	++m_SequenceID;
	UINT32 transactionID = 0;
	UINT8 action = packet.GetAction();

	if ( PPT_SUB_PIECE_REQUEST == m_LastRequestAction && PPT_SUB_PIECE_DATA == action )
	{
		transactionID = m_LastInRequestTransactionID;
	}
	if (PPT_SUB_PIECE_REQUEST == action && bNeedRepeatMRP == true)
	{
		transactionID = m_LastOutRequestTransactionID;
	}

	size_t reval = m_PacketSender->Send(packet, transactionID, m_SequenceID, m_LocalSessionKey, m_RemoteSocketAddress);

	if ( PPT_SUB_PIECE_REQUEST == action ) // Added by Tady, 081108: For multi-request-packet.
	{
		if (bNeedRepeatMRP == true)
		{
			LIVE_ASSERT(m_LastOutRequestTransactionID == m_PacketSender->GetCurTransactionID());
		}
		m_LastOutRequestTransactionID = m_PacketSender->GetCurTransactionID();
		m_bIsLastMRPResponseReceived = false;
	}
	return reval;
}

// Just for Sending MRP.
size_t UDPPeerChannel::SendPacket(boost::shared_ptr<SubPieceUnitRequestPacket> packet)
{
	VIEW_DEBUG("Session: DoSendPacket " << packet->GetAction() << " " << m_Connection);
	++m_SequenceID;
	UINT32 transactionID = 0;
	UINT8 action = packet->GetAction();

	LIVE_ASSERT(action == PPT_SUB_PIECE_REQUEST);
	if ( PPT_SUB_PIECE_REQUEST == m_LastRequestAction && PPT_SUB_PIECE_DATA == action )
	{// Actually no usefull.
		transactionID = m_LastInRequestTransactionID;
	}
	size_t reval = m_PacketSender->Send(*packet, transactionID, m_SequenceID, m_LocalSessionKey, m_RemoteSocketAddress);

	if (action == PPT_SUB_PIECE_REQUEST)
	{
//////////////////////////////////////////////////////////////////////////
// 		static int stCountSingle = 0;
// 		static int stCount5b = 0;
// 		static int stCount10b = 0;
// 		static int stCount15b = 0;
// 		static int stCount15 = 0;
// 		static int stOther = 0;
// 		static int stTotal = 0;
// 		
// 
// 		if (packet->GetRequestCount() == 1)
// 		{
// 			stCountSingle++;
// 		}
// 		else if (packet->GetRequestCount() <= 5)
// 		{
// 			stCount5b++;
// 		}
// 		else if (packet->GetRequestCount() <= 10)
// 		{
// 			stCount10b++;
// 		}
// 		else if (packet->GetRequestCount() < 15)
// 		{
// 			stCount15b++;
// 		}
// 		else if (packet->GetRequestCount() == 15)
// 		{
// 			stCount15++;
// 		}
// 		else
// 		{
// 			stOther++;
// 		}
// 		stTotal++;
//		stTotalsub += packet->GetRequestCount();
//////////////////////////////////////////////////////////////////////////
		m_iOldOutRequestTransactionIDs[m_PacketSender->GetCurTransactionID() % OldRequestIDArray_Len] = m_PacketSender->GetCurTransactionID();
		if (packet->GetRequestCount() > 1)
		{
			MRPElem newPacket;
			newPacket.packet = packet;
			newPacket.sendTimes = 1;
			newPacket.transactionID = m_PacketSender->GetCurTransactionID();
			m_MRPList.push_back(newPacket);
		}
	}

	return reval;
}

// Do some repeats.
void UDPPeerChannel::OnTimerForMRP()
{
	for (MRPList::iterator iter = m_MRPList.begin(); iter != m_MRPList.end(); )
	{
		++m_SequenceID;
		//size_t reval = m_PacketSender->Send(*(iter->packet), iter->transactionID, m_SequenceID, m_LocalSessionKey, m_RemoteSocketAddress);
		iter->sendTimes++;

		if (iter->sendTimes > iter->packet->GetRequestCount() || iter->sendTimes >= 4)
		{
			m_MRPList.erase(iter++);
		}
		else
			iter++;
	}
}

bool UDPPeerChannel::HandleUDPSessionPacket( data_input_stream& is, const NEW_UDP_PACKET_HEAD& head, const UDP_SESSION_INFO& sessionInfo, const SimpleSocketAddress& sockAddr )
{
	VIEW_DEBUG("Session: ReceivePacket " << (int)head.Action << " " << m_Connection << " " << make_tuple(m_LocalSessionKey, m_RemoteSessionKey) << " " << sockAddr);
	++m_ReceivedPacketCount;
	LIMIT_MIN( m_TotalPacketCount, sessionInfo.SequenceID );
	m_LastRequestAction = head.Action & 127;

	// Added by Tady, 081108: For multi-request-packet.
	if (m_LastRequestAction == PPT_SUB_PIECE_REQUEST)
	{
		if (head.TransactionID == m_iOldInRequestTransactionIDs[head.TransactionID % OldRequestIDArray_Len])
 			return true; // It's the same MRP with last one.
		m_iOldInRequestTransactionIDs[head.TransactionID % OldRequestIDArray_Len] = head.TransactionID;
		m_LastInRequestTransactionID = head.TransactionID;
	}
	else if (m_LastRequestAction == PPT_SUB_PIECE_DATA 
		/*&& sessionHead.TransactionID == m_LastOutRequestTransactionID*/)
	{
//		stTotalRecieveSub++;
		if (head.TransactionID == m_iOldOutRequestTransactionIDs[head.TransactionID % OldRequestIDArray_Len])
		{
			m_bIsLastMRPResponseReceived = true;
		}
		for (MRPList::iterator iter = m_MRPList.begin(); iter != m_MRPList.end(); iter++)
		{
			if (iter->transactionID == head.TransactionID)
			{ // It's the response of one MRP.
				//UINT repeatTimes = iter->sendTimes;
				//UINT requestCount = iter->packet->GetRequestCount();
				m_MRPList.erase(iter);
				break;
			}
		}
	}

// 	if (m_LastRequestAction == PPT_SUB_PIECE_DATA)
// 	{
// 		if (m_LastOutRequestTransactionID == sessionHead.TransactionID)
// 		{
// 			m_LastOutRequestTransactionID = sessionHead.TransactionID;
// 		}
// 	}
	return m_Connection.OnPeerChannelData( is, head.Action );
}
