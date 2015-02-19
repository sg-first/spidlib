#pragma once
#include "stdafx.h"

#include <windows.h>
#include <malloc.h>
#include <string>

class HookRegistration
{
public :
  HookRegistration(char *failure,char *success,int trelkey);//��ʼ���࣬HOOK MessageBoxA����
  void registration(int key,string fail,string title);//�û�ע��ʱ���õĺ���
  virtual void successRegistration();//ע��ɹ�ʱ���õĺ���
  virtual void failRegistration();//ע��ʧ��ʱ���õĺ���
  virtual void hookError();//hookʧ�ܵ��õĺ���

  
private:
  __declspec(naked)/*need't add assembl auxiliarily*/ void newMessagebox();//�µ�Messagebox����
  int __stdcall REG(int key);//������ע����֤����
  char *err;//��֤�������ʾ��Ϣ
  char *regok;//��֤����ȷ��ʾ��Ϣ
  char *FunAddr;//��Messagebox��ַ
  int relkey;//ע����
};
