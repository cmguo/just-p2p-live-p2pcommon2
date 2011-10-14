
#ifndef _LIVE_P2PCOMMON2_BASE_SYNACAST_LIVE_LOGWRAPPER_H_
#define _LIVE_P2PCOMMON2_BASE_SYNACAST_LIVE_LOGWRAPPER_H_

#include <ppl/os/module.h>

#include <synacast/protocol/LogServer.h>

#include <boost/noncopyable.hpp>

#include <stdarg.h>



class LogMarkerWrapper : private boost::noncopyable
{
public:
	LogMarkerWrapper()
	{
	}

	bool Load(const tstring& dllpath)
	{
		m_dllpath = dllpath;
		if ( false == m_dll.load( dllpath.c_str() ) )
			return false;
		m_funcSendMSG = reinterpret_cast<FUNC_SendMSG>( m_dll.get_export_item("SendMSG"));
		m_funcPostMSG = reinterpret_cast<FUNC_PostMSG>( m_dll.get_export_item("PostMSG"));
		m_funcUploadMSG = reinterpret_cast<FUNC_UploadMSG>( m_dll.get_export_item("UploadMSG"));
		return true;
	}

	bool IsLoaded() const
	{
		return m_dll.is_open();
	}

	void EnsureLoaded()
	{
		LIVE_ASSERT( false == m_dllpath.empty() );
		if ( m_dllpath.empty() )
			return;
		if ( m_dll.is_open() )
			return;
		this->Load(m_dllpath);
	}

	int SendMSG(UINT32 appID, const char* data, size_t size, UINT8 dataType)
	{
		if ( NULL == m_funcSendMSG )
			return EPI_ERROR_DLL_NO_FUNC;
		return m_funcSendMSG(appID, data, size, dataType);
	}

	int PostMSG(UINT32 appID, const char* data, size_t size, UINT8 dataType)
	{
		if ( NULL == m_funcPostMSG )
			return EPI_ERROR_DLL_NO_FUNC;
		return m_funcPostMSG(appID, data, size, dataType);
	}

	int TraceMSG(EPILogSendingTypeEnum sendingType, UINT32 appID, UINT8 dataType, const char* formatString, ...)
	{
		const size_t max_buffer_size = 4095;
		char buf[max_buffer_size + 1];

		va_list args;
		va_start(args, formatString);
#if defined(_PPL_PLATFORM_MSWIN)
        int len = _vsnprintf(buf, max_buffer_size, formatString, args);
#else
        int len = vsnprintf(buf, max_buffer_size, formatString, args);
#endif
		va_end(args);

		if ( len <= 0 )
		{
			LIVE_ASSERT(false);
			return EPI_ERROR_TRACE_FAILED;
		}

		if ( (size_t)len > max_buffer_size )
		{
			LIVE_ASSERT(false);
			return EPI_ERROR_LARGE_PACKET_TO_SEND;
		}

		if ( EPI_DATA_POST == sendingType )
		{
			return PostMSG(appID, buf, len, dataType);
		}
		else if ( EPI_DATA_SEND == sendingType )
		{
			return SendMSG(appID, buf, len, dataType);
		}
		LIVE_ASSERT(false);
		return EPI_ERROR_TRACE_FAILED;
	}

	int UploadMSG(UINT32 appID, const char* filename, UINT8 dataType)
	{
		if ( NULL == m_funcUploadMSG )
			return EPI_ERROR_DLL_NO_FUNC;
		return m_funcUploadMSG(appID, filename, dataType);
	}

private:
	ppl::os::loadable_module m_dll;
	FUNC_SendMSG m_funcSendMSG;
	FUNC_PostMSG m_funcPostMSG;
	FUNC_UploadMSG m_funcUploadMSG;

	tstring m_dllpath;
};

#endif
