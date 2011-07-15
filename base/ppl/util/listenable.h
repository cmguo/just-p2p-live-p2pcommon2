
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_UTIL_LISTENABLE_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_UTIL_LISTENABLE_H_


/**
 * @file
 * @brief 事件侦听器相关的定义
 */


namespace ppl { namespace util {


/// 可被侦听的对象，提供set_listener接口和默认的listener
template <typename ListenerT, typename TrivialListenerT = ListenerT>
class listenable
{
public:
	typedef ListenerT listener_type;
	typedef TrivialListenerT trivial_listener_type;

	listenable()
	{
		set_listener(0);
	}

	void set_listener(listener_type* listener)
	{
		// null-object模式，避免m_listener为null的情况
		m_listener = ((listener != 0) ? listener : null_listener());
	}

protected:
	listener_type* get_listener() const	{ return m_listener; }

	static listener_type* null_listener()
	{
		static trivial_listener_type listener;
		return &listener;
	}

private:
	listener_type* m_listener;
};


} }


#endif
