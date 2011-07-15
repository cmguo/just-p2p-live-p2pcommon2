
#ifndef _LIVE_P2PCOMMON2_NEW_MEDIA_MEDIA_SERVER_LISTENER_H_
#define _LIVE_P2PCOMMON2_NEW_MEDIA_MEDIA_SERVER_LISTENER_H_


/**
 * @brief Graph使用方式
 * 0 -  选项1 (见下面附录)
 * 1 -  选项2 (见下面附录)
 * 2 -  选项3 (见下面附录)
 * 内核一启动，就设置这个值，就可以判断用什么方式的流来输出了
 */
enum GraphModeEnum
{
	GRAPH_1 = 0, 
	GRAPH_2 = 1, 
	GRAPH_3 = 2, 
};

/// mediaserver的事件类
class MediaServerListener
{
public:
	virtual ~MediaServerListener() { }

	virtual void NotifyGUI(UINT msg, WPARAM wp, LPARAM lp) = 0;

	virtual int GetGraphMode() = 0;
	virtual int GetNetWriterMode() = 0;
};


#endif