
#ifndef _LIVE_P2PCOMMON2_NEW_COMMON_PEER_INFOITEM_H_
#define _LIVE_P2PCOMMON2_NEW_COMMON_PEER_INFOITEM_H_

/**
 * @file
 * @brief 共享内存LIVE_INFO中有256项CPeerInfoItem，如果实际连接数超过256，则使用动态方式分配CPeerInfoItem
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