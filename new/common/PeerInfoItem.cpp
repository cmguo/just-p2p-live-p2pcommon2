
#include "StdAfx.h"

#include "PeerInfoItem.h"
#include "GloalTypes.h"
#include <boost/scoped_ptr.hpp>


class StaticPeerInfoItemProvider : public PeerInfoItemProvider
{
public:
	explicit StaticPeerInfoItemProvider( CPeerInfoItem* item ) : m_Item(item)
	{
		m_Item->Acquire();
	}
	virtual ~StaticPeerInfoItemProvider()
	{
		m_Item->Release();
	}
	virtual CPeerInfoItem* GetItem()
	{
		return m_Item;
	}

private:
	CPeerInfoItem* m_Item;
};


class DynamicPeerInfoItemProvider : public PeerInfoItemProvider
{
public:
	DynamicPeerInfoItemProvider() : m_Item(new CPeerInfoItem())
	{
		m_Item->State = pis_unused;
		m_Item->Acquire();
	}
	virtual ~DynamicPeerInfoItemProvider()
	{
	}
	virtual CPeerInfoItem* GetItem()
	{
		return m_Item.get();
	}

private:
	boost::scoped_ptr<CPeerInfoItem> m_Item;
};


PeerInfoItemProvider* PeerInfoItemProviderFactory::CreateStatic( CPeerInfoItem* item )
{
	return new StaticPeerInfoItemProvider( item );
}

PeerInfoItemProvider* PeerInfoItemProviderFactory::CreateDynamic()
{
	return new DynamicPeerInfoItemProvider();
}




