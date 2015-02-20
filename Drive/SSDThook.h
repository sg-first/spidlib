#program once
#include "stdafx.h"

#include "windows.h"
//提示：本库使用WDK项目，head.h自行补全，包含DDK目录中的头文件
#include "head.h"

class SSDThook
{
public :
  static NTSTATUS Hook(PHOOK_INFO info);//hook一个指定函数
  static ULONG IsHook(PHOOK_INFO info);//是否正在被hook
  static ULONG StartProtect(PHOOK_INFO info);//保护HOOK数据.参数：m=1开启HOOK防护,info第一个数据，返回值，返回状态，并填充info相关信息
  static BOOLEAN Add_Protect(PHOOK_INFO pinfo);//添加HOOK到保护队列
  static void Blue();//蓝屏
  
private:
  static void Thread_proc(PVOID a);
  static void Thread_p_proc(PVOID a);
}