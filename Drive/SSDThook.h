#program once
#include "stdafx.h"

#include "windows.h"
//��ʾ������ʹ��WDK��Ŀ��head.h���в�ȫ������DDKĿ¼�е�ͷ�ļ�
#include "head.h"

class SSDThook
{
public :
  static NTSTATUS Hook(PHOOK_INFO info);//hookһ��ָ������
  static ULONG IsHook(PHOOK_INFO info);//�Ƿ����ڱ�hook
  static ULONG StartProtect(PHOOK_INFO info);//����HOOK����.������m=1����HOOK����,info��һ�����ݣ�����ֵ������״̬�������info�����Ϣ
  static BOOLEAN Add_Protect(PHOOK_INFO pinfo);//���HOOK����������
  static void Blue();//����
  
private:
  static void Thread_proc(PVOID a);
  static void Thread_p_proc(PVOID a);
}