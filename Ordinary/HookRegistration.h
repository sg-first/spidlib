#pragma once
#include "stdafx.h"

#include <windows.h>
#include <malloc.h>
#include <string>

class HookRegistration
{
public :
  HookRegistration(char *failure,char *success,int trelkey);//初始化类，HOOK MessageBoxA函数
  void registration(int key,string fail,string title);//用户注册时调用的函数
  virtual void successRegistration();//注册成功时调用的函数
  virtual void failRegistration();//注册失败时调用的函数
  virtual void hookError();//hook失败调用的函数

  
private:
  __declspec(naked)/*need't add assembl auxiliarily*/ void newMessagebox();//新的Messagebox函数
  int __stdcall REG(int key);//真正的注册验证函数
  char *err;//验证码错误提示信息
  char *regok;//验证码正确提示信息
  char *FunAddr;//真Messagebox地址
  int relkey;//注册码
};
