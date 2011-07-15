// MKVReaderFace.h: interface for the CMKVReaderFace class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MKVREADERFACE_H__89250A12_DCB3_454A_B775_988FDBEFFB8C__INCLUDED_)
#define AFX_MKVREADERFACE_H__89250A12_DCB3_454A_B775_988FDBEFFB8C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

///打开一个源
typedef void * (WINAPI *LPFN_OPENREADER)(char *Source);

///关闭一个源
typedef void (WINAPI *LPFN_CLOSEREADER)(void *);

///读取头数据
typedef int (WINAPI *LPFN_READHEADDATA)(void *, BYTE *, int);

///读取数据
typedef int (WINAPI *LPFN_READUNITDATA)(void *, BYTE *, int, UINT32 *);

class CMKVReaderFace  
{
public:
	CMKVReaderFace();
	virtual ~CMKVReaderFace();

	BOOL Load(TCHAR *LibPath);
	void UnLoad();

	HINSTANCE			m_hLib;

private:
	LPFN_OPENREADER		m_OpenReader;
	LPFN_CLOSEREADER	m_CloseReader;
	LPFN_READHEADDATA	m_ReadHeadData;
	LPFN_READUNITDATA	m_ReadUnitData;

public:
	void * OpenReader(char * source);
	void CloseReader( void * readerHandle );
	int ReadHeadData( void * readerHandle, BYTE * data, int length );
	int ReadUnitData( void * readerHandle, BYTE * data, int bufferSize, UINT32 * timeStamp );
};

#endif // !defined(AFX_MKVREADERFACE_H__89250A12_DCB3_454A_B775_988FDBEFFB8C__INCLUDED_)
