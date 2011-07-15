
#ifndef _LIVE_P2PCOMMON2_NEW_UTIL_RESOURCE_CHANGE_RECORDER_H_
#define _LIVE_P2PCOMMON2_NEW_UTIL_RESOURCE_CHANGE_RECORDER_H_

class BitMap;



/// 资源变化记录器
class ResourceChangeRecorder
{
public:
	void Add(UINT32 piece)
	{
		m_changedPieces.insert(piece);
	}

	void Remove(UINT32 piece)
	{
		m_changedPieces.erase(piece);
	}

	void RemoveLower(UINT32 piece);

	void Clear()
	{
		m_changedPieces.clear();
	}

	BitMap Build(UINT32 minIndex);

private:
	set<UINT32>		m_changedPieces;
};

#endif

