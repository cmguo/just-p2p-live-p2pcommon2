
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_MSWIN_INTERLOCKED_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_MAWIN_INTERLOCKED_H_

/**
 * @file
 * @brief interlocked���ܵķ�װ��
 */


/// interlocked��ʽ��ͬ������
class Interlocked
{
public:
	/// ����
	static long Increment(long& val)
	{
		return ::InterlockedIncrement(&val);
	}

	/// ����
	static long Decrement(long& val)
	{
		return ::InterlockedDecrement(&val);
	}
};


/// ͬ������������
class InterlockedInteger
{
public:
	InterlockedInteger() : m_val(0) { }

	long Increment() { return Interlocked::Increment(m_val); }
	long Decrement() { return Interlocked::Decrement(m_val); }

private:
	long m_val;
};


#endif


