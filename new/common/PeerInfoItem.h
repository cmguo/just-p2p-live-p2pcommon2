
#ifndef _LIVE_P2PCOMMON2_NEW_COMMON_PEER_INFOITEM_H_
#define _LIVE_P2PCOMMON2_NEW_COMMON_PEER_INFOITEM_H_

/**
 * @file
 * @brief �����ڴ�LIVE_INFO����256��CPeerInfoItem�����ʵ������������256����ʹ�ö�̬��ʽ����CPeerInfoItem
 */

#include "framework/memory.h"
#include <boost/noncopyable.hpp>

class CPeerInfoItem;

class PeerInfoItemProvider : public pool_object, private boost::noncopyable
{
public:
	virtual ~PeerInfoItemProvider() {}

	virtual CPeerInfoItem* GetItem() = 0;
};


class PeerInfoItemProviderFactory
{
public:
	static PeerInfoItemProvider* CreateStatic( CPeerInfoItem* item );
	static PeerInfoItemProvider* CreateDynamic();
};


#endif