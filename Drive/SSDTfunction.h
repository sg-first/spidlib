#program once
#include "stdafx.h"

//��ʾ������ʹ��WDK��Ŀ��head.h���в�ȫ������DDKĿ¼�е�ͷ�ļ�
#include "head.h"

class SSDTfunction
{
public :
  static ULONG GetReallySSDTFunctionAddress(ULONG index,ULONG _SSDT);//���ã�ͨ����������ȡ������ԭʼ������ַ������һ��������������������KeServiceDescriptorTable�ĵ�ַ
  static ULONG Get_SSDT_Proc_index(UNICODE_STRING FunctionName);//���ã�ͨ���������õ���������������һ��������
  static ULONG RvaToRaw(PIMAGE_DOS_HEADER pDosHead,ULONG RVA);//���ã�ת��RVA��FileOffset������һ��RVA
  static ULONG Get_Current_Proc_address(ULONG index,LONG SSDT);//���ã���ȡSSDT���еĺ����ĵ�ǰ��ַ,����һ��index�Ǻ�����������Get_SSDT_Proc_index��ȡ������������SSDT��ϵͳ������ַ������ֵ��ʧ�ܷ���-1���ɹ����ص�ַ
};
