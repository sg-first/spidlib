#include "stdafx.h"
#include "HookRegistration.h"

int __stdcall HookRegistration::REG(int key)
{
 if (key==relkey)
 //success
 {
 successRegistration();
 return 1;
 }
 //failure
 else
 {
 failRegistration();
 return 0;
 }
}

__declspec(naked) void HookRegistration::newMessagebox()
{
  _asm
 {
     push ebp
     mov ebp,esp
     cmp [ebp+20],0xAA
     jne next
     push [ebp+8]//transmission parameters, passed as the first parameter to MessageBoxA, is passed to the REG key
     call REG
     push ebx
     mov ebx,0
     mov [ebp+8],ebx
     mov [ebp+20],ebx
     pop ebx
     cmp eax,1
     jne iferr//jump to register failed
     mov eax,regok
     mov [ebp+12],eax
     jmp next

  iferr:// the contents implementation when failed to register
    mov eax,err
    mov [ebp+12],eax
  
  next:
    mov eax,FunAddr
    add eax,5
    jmp eax
 }
}

HookRegistration::HookRegistration(char *failure,char *success,int trelkey)
{
 err=failure;
 regok=success;
 FunAddr=NULL;
 relkey=trelkey;
 
 unsigned char JmpCode[5];
 int addr2;
 DWORD oldProtect;
 FunAddr=(char *)GetProcAddress(GetModuleHandleA("User32.dll"),"MessageBoxA");//take address of MessageBoxA function.
 if (!FunAddr)//if we don't get, get the error
 {hookError();}
 JmpCode[0]=0xE9;
 addr2 = (int)MyMessagebox-(int)FunAddr-5;//what address
 *((int*)&JmpCode[1])=addr2;
 VirtualProtect(FunAddr,5,PAGE_EXECUTE_READWRITE,&oldProtect);//locking a block of memory
 memcpy(FunAddr,JmpCode,5);//copy FunAddr to JmpCode
 VirtualProtect(FunAddr,5,oldProtect,NULL);//once again the protection
}

void HookRegistration::registration(int key,string fail,string title)
{MessageBox(key,fail,title,MB_OK);}
