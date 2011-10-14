#ifndef _LIVE_P2PCOMMON2_BASE_PPL_MSWIN_EVENT_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_MAWIN_EVENT_H_

#include <ppl/mswin/windows.h>
#include <boost/noncopyable.hpp>
#include <assert.h>



namespace ppl { namespace mswin {


/// win32事件对象
class event : private boost::noncopyable
{
public:
	event() : m_handle(NULL)
	{
	}
	~event()
	{
		this->close();
	}
	bool is_open() const { return m_handle != NULL; }
	
	void close()
	{
		if (is_open())
		{
			::CloseHandle(m_handle);
			m_handle = NULL;
		}
	}

	BOOL signal()
	{
		LIVE_ASSERT(is_open());
		return ::SetEvent(m_handle);
	}
	BOOL reset()
	{
		LIVE_ASSERT(is_open());
		return ::ResetEvent(m_handle);
	}
	BOOL pulse()
	{
		LIVE_ASSERT(is_open());
		return ::PulseEvent(m_handle);
	}

	DWORD wait()
	{
		return wait(INFINITE);
	}
	DWORD wait(DWORD milliseconds)
	{
		return ::WaitForSingleObject(m_handle, milliseconds);
	}
	DWORD get_state()
	{
		return this->wait(0);
	}
	bool is_signaled()
	{
		return get_state() == WAIT_OBJECT_0;
	}
	HANDLE native_handle() const { return m_handle; }

	bool create(bool isManualReset, bool initialState = false, LPCTSTR name = NULL)
	{
		this->close();
		attach(::CreateEvent(NULL, isManualReset ? TRUE : FALSE, initialState ? TRUE : FALSE, name));
		return is_open();
	}

	bool open(LPCTSTR name, DWORD desiredAccess, bool inheritHandle = false)
	{
		LIVE_ASSERT(name != NULL);
		this->close();
		attach(::OpenEvent(desiredAccess, inheritHandle ? TRUE : FALSE, name));
		return is_open();
	}

protected:
	void attach(HANDLE hEvent)
	{
		LIVE_ASSERT(!is_open());
		m_handle = hEvent;
	}

private:
	HANDLE	m_handle;
};


/// 手动重置的win32事件对象
class manual_reset_event : public event
{
public:
	explicit manual_reset_event(BOOL initState = FALSE, LPCTSTR name = NULL)
	{
		attach(::CreateEvent(NULL, TRUE, initState, name));
		LIVE_ASSERT(is_open());
	}
};

/// 自动重置的win32事件对象
class auto_reset_event : public event
{
public:
	explicit auto_reset_event(BOOL initState = FALSE, LPCTSTR name = NULL)
	{
		attach(::CreateEvent(NULL, FALSE, initState, name));
		LIVE_ASSERT(is_open());
	}
};



class wait_handle
{
public:
	static DWORD wait_any(const event* handles, DWORD count, DWORD milliseconds = INFINITE)
	{
		return ::WaitForMultipleObjects(count, reinterpret_cast<const HANDLE*>(handles), FALSE, milliseconds);
	}
};

} }


#endif