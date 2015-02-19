#program once
#include "stdafx.h"

//提示：本库使用WDK项目，head.h自行补全，包含DDK目录中的头文件
#include "head.h"

class SSDTfunction
{
public :
  static ULONG GetReallySSDTFunctionAddress(ULONG index,ULONG _SSDT);//作用：通过函数名获取索引和原始函数地址，参数一：函数索引，参数二：KeServiceDescriptorTable的地址
  static ULONG Get_SSDT_Proc_index(UNICODE_STRING FunctionName);//作用：通过函数名得到函数做引，参数一：函数名
  static ULONG RvaToRaw(PIMAGE_DOS_HEADER pDosHead,ULONG RVA);//作用：转换RVA到FileOffset，参数一：RVA
  static ULONG Get_Current_Proc_address(ULONG index,LONG SSDT);//作用：获取SSDT表中的函数的当前地址,参数一：index是函数索引（用Get_SSDT_Proc_index获取），参数二：SSDT是系统服务表基址，返回值：失败返回-1，成功返回地址
};
