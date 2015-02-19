#pragma once
#include "stdafx.h"

#include <stdio.h>
#include <windows.h>

class AntiDebugging
{
public:
   AntiDebugging();//初始化反调试组件
   bool isDebug;//是否正在被调试
   void check();//检查是否在被调试
   virtual void abnormal();//异常时的回调
   virtual void debuging();//确认被调试时的回调

private:
   LONG WINAPI debug_kill(_In_  struct _EXCEPTION_POINTERS *ExceptionInfo);
};
