
#ifndef _LIVE_P2PCOMMON2_NEW_MEDIA_MEDIA_SERVER_LISTENER_H_
#define _LIVE_P2PCOMMON2_NEW_MEDIA_MEDIA_SERVER_LISTENER_H_


/**
 * @brief Graphʹ�÷�ʽ
 * 0 -  ѡ��1 (�����渽¼)
 * 1 -  ѡ��2 (�����渽¼)
 * 2 -  ѡ��3 (�����渽¼)
 * �ں�һ���������������ֵ���Ϳ����ж���ʲô��ʽ�����������
 */
enum GraphModeEnum
{
	GRAPH_1 = 0, 
	GRAPH_2 = 1, 
	GRAPH_3 = 2, 
};

/// mediaserver���¼���
class MediaServerListener
{
public:
	virtual ~MediaServerListener() { }

	virtual void NotifyGUI(UINT msg, WPARAM wp, LPARAM lp) = 0;

	virtual int GetGraphMode() = 0;
	virtual int GetNetWriterMode() = 0;
};


#endif