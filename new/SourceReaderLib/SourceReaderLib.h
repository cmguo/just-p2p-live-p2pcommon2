// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� SOURCEREADERLIB_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// SOURCEREADERLIB_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�

#ifdef SOURCEREADERLIB_DLL

#ifdef SOURCEREADERLIB_EXPORTS
#define SOURCEREADERLIB_API __declspec(dllexport)
#else
#define SOURCEREADERLIB_API __declspec(dllimport)
#endif

#else

#ifdef SOURCEREADERLIB_EXPORTS
#define SOURCEREADERLIB_API 
#else
#define SOURCEREADERLIB_API 
#endif

#endif


//// �����Ǵ� SourceReaderLib.dll ������
//class SOURCEREADERLIB_API CSourceReaderLib {
//public:
//	CSourceReaderLib(void);
//	// TODO: �ڴ�������ķ�����
//};
//
//extern SOURCEREADERLIB_API int nSourceReaderLib;
//
//SOURCEREADERLIB_API int fnSourceReaderLib(void);
