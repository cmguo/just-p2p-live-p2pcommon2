
#include "StdAfx.h"

#include "LogClient.h"
#include "common/IpPool.h"
#include "common/BaseInfo.h"
#include "common/PeerManagerStatistics.h"
#include "common/StreamBufferStatistics.h"
#include "common/PeerManager.h"
//#include "common/Flow"
#include <ppl/text/json/writer.h>
#include <ppl/data/numeric.h>
#include <ppl/data/guid.h>
#include <ppl/os/paths.h>
#include <fstream>
#include <vector>
#include <ppl/diag/trace.h>

const TCHAR ppl_log_buffer_quality_filename[] = _T("livebuf.dat");
const TCHAR ppl_log_flow_filename[] = _T("liveflow.dat");


LogClient::LogClient(const tstring& baseDir, const tstring& configDir, boost::shared_ptr<PeerInformation> peerInfo) 
	: m_ConfigDirectory(configDir)
	, m_PeerInformation(peerInfo)
	, m_BufferLogSaved(false)
	, m_FlowLogSaved(false)
{
	if ( IsEnabled() )
	{
		tstring dllpath = ppl::os::paths::combine(baseDir, _T("marker.dll"));
		m_Marker.Load(dllpath);
	}
#ifdef _DEBUG
	// debug版下尽快刷新数据
	m_LogDataSaveTime = time_counter( 0 );
#endif
}

LogClient::~LogClient()
{
}

bool LogClient::IsEnabled() const
{
	return m_PeerInformation->IsNormalPeer();
}

static void WriteJsonQualityData(const string& name, const std::vector<QualityData>& info, std::ofstream& fout, JsonWriter& writer)
{
	writer.WriteString(name);
	writer.WriteColon();
	JsonWriter::ArrayWriterEntry arrayEntry(fout);
	for ( size_t index = 0; index < info.size(); ++index )
	{
		if ( index > 0 )
		{
			writer.WriteComma();
		}
		const QualityData& data = info[index];
		{
			JsonWriter::ObjectWriterEntry objectEntry(fout);
			writer.WriteJsonNumber("PrepareTime", numeric<int>::format(data.PrepareTime));
			writer.WriteComma();
			writer.WriteJsonNumber("Peers", numeric<UINT>::format(data.ConnectedPeerCount));
			writer.WriteComma();
			writer.WriteJsonNumber("Down", numeric<UINT>::format(data.DownloadSpeed));
			writer.WriteComma();
			writer.WriteJsonNumber("Up", numeric<UINT>::format(data.UploadSpeed));
			writer.WriteComma();
			writer.WriteJsonNumber("Duration", numeric<UINT>::format(data.Duration));
			writer.WriteComma();
			writer.WriteJsonNumber("Time", numeric<UINT>::format(data.TimeOfChange));
		}
	}
}

void LogClient::SaveLog(const StreamBufferStatistics& streamStats, PeerManager& peerManager, const CIPPool& ipPool)
{
	// 每5分钟写一次
	if ( m_LogDataSaveTime.elapsed32() < 3 * 60 * 1000 )
		return;
	m_LogDataSaveTime.sync();
	DoSaveLog(streamStats, peerManager, ipPool);
}

void LogClient::SendLog()
{
	if ( false == IsEnabled() || false == m_Marker.IsLoaded() )
		return;

	if ( m_BufferLogSaved )
	{
		//m_Marker.EnsureLoaded();
		TRACE(_T("mir: send buffer tcp log\n"));
		tstring buflogfile = ppl::os::paths::combine(m_ConfigDirectory, ppl_log_buffer_quality_filename);
		m_Marker.UploadMSG(123, buflogfile.c_str(), EPI_DATA_JSON);
	}

	if ( m_FlowLogSaved )
	{
		//m_Marker.EnsureLoaded();
		TRACE(_T("mir: send flow tcp log\n"));
		tstring buflogfile = ppl::os::paths::combine(m_ConfigDirectory, ppl_log_flow_filename);
		m_Marker.UploadMSG(2100, buflogfile.c_str(), EPI_DATA_JSON);
	}

}

void LogClient::DoSaveLog( const StreamBufferStatistics& streamStats, PeerManager& peerManager, const CIPPool& ipPool )
{
	if ( false == IsEnabled() || m_Marker.IsLoaded())
		return;

	const PeerManagerStatistics& mgrStats = peerManager.GetStatistics();
	const FlowMeasure& flow = peerManager.GetFlow();
	{
		tstring logFilename = ppl::os::paths::combine(m_ConfigDirectory, ppl_log_buffer_quality_filename);
		std::ofstream fout;
		fout.open( logFilename.c_str(), std::ios_base::trunc );
		if ( fout )
		{
			JsonWriter writer(fout);
			{
				JsonWriter::ObjectWriterEntry entry(fout);
				WriteJsonQualityData("vals", mgrStats.InternalInfo.BadInfo, fout, writer);
				writer.WriteComma();
				WriteJsonQualityData("gvals", mgrStats.InternalInfo.GoodInfo, fout, writer);
				writer.WriteComma();
				WriteJsonQualityData("exvals", mgrStats.ExternalInfo.BadInfo, fout, writer);
				writer.WriteComma();
				WriteJsonQualityData("exgvals", mgrStats.ExternalInfo.GoodInfo, fout, writer);
				writer.WriteComma();
				writer.WriteJsonNumber("MaxPeers", numeric<UINT>::format(mgrStats.MaxConnectedPeerCount));
				writer.WriteComma();
				writer.WriteJsonNumber("MaxDown", numeric<UINT>::format(flow.Download.GetMaxRate()));
				writer.WriteComma();
				writer.WriteJsonNumber("MaxUp", numeric<UINT>::format(flow.Upload.GetMaxRate()));
				writer.WriteComma();
				writer.WriteJsonNumber("Time", numeric<UINT>::format(m_PeerInformation->GetStartedSeconds()));
				writer.WriteComma();
				writer.WriteJsonNumber("Buf15s", numeric<UINT>::format(streamStats.BufferringUsedTime[15]));
				writer.WriteComma();
				writer.WriteJsonNumber("Buf30s", numeric<UINT>::format(streamStats.BufferringUsedTime[30]));
				writer.WriteComma();
				writer.WriteJsonString("IP", AnsiFormatIPAddress(m_PeerInformation->NetInfo->GetDetectedIP()));

				const PeerNetInfo& netInfo = *m_PeerInformation->NetInfo;
				writer.WriteComma();
				writer.WriteJsonNumber("NAT", numeric<int>::format(netInfo.GetNATType()));
				writer.WriteComma();
				writer.WriteJsonNumber("Net", numeric<int>::format(netInfo.CoreInfo.PeerNetType));
				writer.WriteComma();
				writer.WriteJsonNumber("InitNAT", numeric<UINT64>::format(mgrStats.InitiatedNATConnections));
				writer.WriteComma();
				writer.WriteJsonNumber("SInitNAT", numeric<UINT64>::format(mgrStats.SucceededInitiatedNATConnections));
				writer.WriteComma();
				writer.WriteJsonNumber("RecvNAT", numeric<UINT64>::format(mgrStats.ReceivedNATConnections));
				writer.WriteComma();
				writer.WriteJsonNumber("SRecvNAT", numeric<UINT64>::format(mgrStats.SucceededReceivedNATConnections));
				writer.WriteComma();
				writer.WriteJsonNumber("InitConn", numeric<UINT64>::format(mgrStats.ConnectorData.UDP.TotalInitiatedConnections));
				writer.WriteComma();
				writer.WriteJsonNumber("SInitConn", numeric<UINT64>::format(mgrStats.ConnectorData.UDP.TotalSucceededInitiatedConnections));
				writer.WriteComma();
				writer.WriteJsonNumber("RecvConn", numeric<UINT64>::format(mgrStats.ConnectorData.UDP.TotalRequestedConnections));
				writer.WriteComma();
				writer.WriteJsonNumber("SRecvConn", numeric<UINT64>::format(mgrStats.ConnectorData.UDP.TotalSucceededRequestedConnections));
				writer.WriteComma();
				writer.WriteJsonNumber("InitConnT", numeric<UINT64>::format(mgrStats.ConnectorData.TCP.TotalInitiatedConnections));
				writer.WriteComma();
				writer.WriteJsonNumber("SInitConnT", numeric<UINT64>::format(mgrStats.ConnectorData.TCP.TotalSucceededInitiatedConnections));
				writer.WriteComma();
				writer.WriteJsonNumber("RecvConnT", numeric<UINT64>::format(mgrStats.ConnectorData.TCP.TotalRequestedConnections));
				writer.WriteComma();
				writer.WriteJsonNumber("SRecvConnT", numeric<UINT64>::format(mgrStats.ConnectorData.TCP.TotalSucceededRequestedConnections));

				writer.WriteComma();
				writer.WriteJsonString("Channel", FormatAnsiGUID( m_PeerInformation->ChannelGUID ));
				writer.WriteComma();
				writer.WriteJsonNumber("ntsok", numeric<int>::format( netInfo.GetStunDetectedAddress().IsUDPValid() && netInfo.GetStunServerAddress().IsUDPValid() ));
				writer.WriteComma();
				writer.WriteJsonString("ntsd", AnsiFormatIPAddress( netInfo.GetStunDetectedAddress().IP ) );
				writer.WriteComma();
				writer.WriteJsonNumber("ntsdp", numeric<UINT16>::format( netInfo.GetStunDetectedAddress().UdpPort ) );
				writer.WriteComma();
				writer.WriteJsonNumber("ver", numeric<UINT16>::format( m_PeerInformation->AppVersionNumber16 ) );
			}
			fout.flush();
			fout.close();
			m_BufferLogSaved = true;
		}
	}

	{
		tstring logFilename = ppl::os::paths::combine(m_ConfigDirectory, ppl_log_flow_filename);
		std::ofstream fout;
		fout.open( logFilename.c_str(), std::ios_base::trunc );
		UINT totalDown(0), totalUp(0);
		if ( fout )
		{
			JsonWriter writer(fout);
			{
				JsonWriter::ObjectWriterEntry entry(fout);
				writer.WriteJsonString("IP", AnsiFormatIPAddress(m_PeerInformation->NetInfo->GetDetectedIP()));
				writer.WriteComma();
				writer.WriteJsonNumber("Time", numeric<UINT>::format(m_PeerInformation->GetStartedSeconds()));
				writer.WriteComma();
				writer.WriteJsonNumber("TDown", numeric<UINT>::format((UINT)(flow.Download.GetTotalBytes() / 1024)));
				writer.WriteComma();
				writer.WriteJsonNumber("TUp", numeric<UINT>::format((UINT)(flow.Upload.GetTotalBytes() / 1024)));
				writer.WriteComma();
				totalDown = flow.Download.GetTotalBytes() / 1024;
				totalUp =flow.Upload.GetTotalBytes() / 1024;
				writer.WriteString("peers");
				writer.WriteColon();
				UINT32 totalPeercount(0), savedPeercount(0);
				{
					JsonWriter::ArrayWriterEntry arrayEntry(writer.GetStream());
//					UINT32 ipPoolPeercount(0), ipPoolSavedPeerCount(0);
					peerManager.SaveLog(writer, m_PeerInformation->NetInfo->GetDetectedIP(), totalPeercount, savedPeercount);
					
					ipPool.SaveLog(writer, m_PeerInformation->NetInfo->GetDetectedIP(), totalPeercount, savedPeercount);
// 					totalPeercount += ipPoolPeercount;
// 					savedPeercount += ipPoolSavedPeerCount;
				}
				writer.WriteComma();
				writer.WriteJsonNumber("tpeers", numeric<UINT>::format(totalPeercount));
				writer.WriteComma();
				UINT peerPercent = (UINT)(savedPeercount * 100.0 / totalPeercount);
				writer.WriteJsonNumber("peerpercent", numeric<UINT>::format(peerPercent));
				writer.WriteComma();
				writer.WriteJsonString("Channel", FormatAnsiGUID( m_PeerInformation->ChannelGUID ));
			}
			fout.flush();
			fout.close();
			m_FlowLogSaved = true;
		}
	}
}
