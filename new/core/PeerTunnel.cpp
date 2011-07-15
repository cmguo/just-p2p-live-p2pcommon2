
#include "StdAfx.h"

#include "PeerTunnel.h"
#include "PeerConnection.h"
#include "Downloader.h"
#include "common/MediaStorage.h"
#include <ostream>

const SubPieceUnit SubPieceUnit::ZERO;


PeerTunnel::PeerTunnel(PeerConnection& connection, Downloader& downloader, int tunnelIndex) 
: m_Downloader(downloader)
, m_TunnelIndex(tunnelIndex)
, m_taskQueueMaxSize(200)
, m_lastTaskQueueSize(0)
, m_Connection(connection),m_WindowSize( 0 ), m_lastRequestSuccedRate( 0.0 ), m_RequestTimes(0)
, m_taskCollectionPtr(NULL) // Added by Tady, 072408.
, m_bIsFreezing(false) // Added by Tady, 011611: Spark!
{
}

std::ostream& operator<<(std::ostream& os, const PeerTunnel& val)
{
	os << val.GetConnection() << "|" << val.GetIndex();
	return os;
}

