#include "SSDTfunction.h"
#include "stru.h"
#include "SSDThook.h"

#include<ntimage.h>
#define SSDT_HOOK 1//直接SSDT HOOK
#define SSDT_CURR_HOOK 2//SSDT表中当前函数的暴力HOOK 

Protect_list head_Protect;
BOOLEAN Start=FALSE;
HANDLE handle_Protect;//保护线程句柄
KEVENT s_event;
ULONG Anti_HOOK=0;

//对于这么一个NativeAPI，我们使用前必须声明
NTSTATUS NTAPI ZwQuerySystemInformation(SYSTEM_INFORMATION_CLASS SystemInformationClass,PVOID SystemInformation,ULONG SystemInformationLength,PULONG ReturnLength);


NTSTATUS SSDThook::Hook(PHOOK_INFO info)
{
 NTSTATUS status = STATUS_UNSUCCESSFUL;
 _asm
 {
    //关闭写保护
    push eax
    cli
    mov eax, cr0
    and eax, ~0x10000
    mov cr0, eax
    pop eax
 }
 if(info==NULL)
 {
  goto __exit;
 }
 else 
 {
   if (info->old_address == 0 || info->new_address == 0)
   {
     DbgPrint("old_address == 0 or new_address  == 0");
     goto __exit;
   }
 }
 // 选模式
 if(SSDTfunction::GetReallySSDTFunctionAddress(info->index,info->SSDT_TABLE)==SSDTfunction::Get_Current_Proc_address(info->index,info->SSDT_TABLE))
 {
 // 当前函数和原始函数相同  直接SSDT-HOOK
 ULONG indexs = info->index * 4;
 _asm
 {
   pushad
    mov ecx,info
    mov ebx,[ecx+HOOK_INFO.SSDT_TABLE]
    mov ebx,[ebx]  // 第一张表
    mov eax,indexs
    add ebx,eax
    mov eax,[ebx]  //保存原来的
    mov [ecx+HOOK_INFO.old_address],eax
    // 将HOOK_INFO.new_address地址写入
    mov eax,[ecx+HOOK_INFO.new_address]
    mov [ebx],eax
   popad
 }
 info->Type = SSDT_HOOK;
 status = STATUS_SUCCESS;
 }
 else
 {
   //跳转地址-当前JMP所在地址 -5（E9 + 32位地址） [相对]
   ULONG indexs = info->index * 4;
   ULONG jmpaddr;
   jmpaddr = info->new_address - info ->old_address -5;
   _asm
   {  
     pushad
      mov ecx,info
      mov ebx,[ecx+HOOK_INFO.old_address]  //ebx
      mov al,0xE9   //E9 jmp机器码
      mov byte PTR [ebx],al
      mov eax,jmpaddr
      mov [ebx+1],eax
     popad
  }
  info->data[0]=0xE9;
  *((PULONG)(&info->data[1])) = jmpaddr;
  info->Type=SSDT_CURR_HOOK;
  status = STATUS_SUCCESS;
 }
 __exit:
 _asm //恢复写保护
 {
  push eax
  mov eax, cr0
  or eax, 0x10000
  mov cr0, eax
  sti
  pop eax
 }
 return status;
}


ULONG SSDThook::IsHook(PHOOK_INFO info)
{
 ULONG STATUS=0;
 if(SSDT_HOOK==info->Type) //SSDT-HOOK
 {
    ULONG *addr = (PULONG)(*((PULONG)info->SSDT_TABLE));
    ULONG  CurrentAddr;
    PULONG pNewAddr;
    CurrentAddr= addr[info->index];
    if(CurrentAddr!=info->new_address)
    {
     STATUS = 0;
     goto __exit;
    }
  STATUS = 1;
 }
 else
 {
  //InineHook
  PUCHAR now_data;
  ULONG *ssdt =  (PULONG)(*((PULONG)info->SSDT_TABLE));
  now_data =(PUCHAR) info->old_address;
  if (ssdt[info->index]!=(ULONG)now_data)//先判断地址是否相同
  {
   STATUS = 0;
   goto __exit;
  }
  if(now_data[0]!= 0xE9)
  {
    STATUS = 0;
    goto __exit;
  }
  else 
  {
    if(*((PULONG)(&now_data[1])) != (*(PULONG)(&info->data[1])))
    {
     STATUS = 0;
     goto __exit;
    }
  }
  STATUS = 1;
 }
 __exit:
 return STATUS;
}


ULONG SSDThook::StartProtect(PHOOK_INFO info)
{
   NTSTATUS status;
   head_Protect.Before=NULL; //初始化头
   head_Protect.Next=NULL;
   head_Protect.info=info;
   DbgPrint("队列初始化完毕！\n");
   Start=TRUE;
   status=PsCreateSystemThread(&handle_Protect,0,NULL,NULL,NULL,Thread_proc,NULL);
   if (!NT_SUCCESS(status))
   {
    DbgPrint("保护线程创建失败！！！\n");
    return -1;
   }
   ZwClose(handle_Protect);
   return 1;
}


void SSDThook::Thread_proc(PVOID a)
{
 ULONG MAX=4000;
 PProtect_list n=NULL,next=NULL;
 HANDLE hHandle;
 DbgPrint("保护队列准备就绪！\n");
 while(head_Protect.info==NULL)
 {continue;}
 KeInitializeEvent(&s_event,SynchronizationEvent,FALSE);
 PsCreateSystemThread(&hHandle,0,NULL,NULL,NULL,Thread_p_proc,NULL);
 ZwClose(hHandle);
 while (1)
 {
  n=&head_Protect;
  next=n;
  while(next!=NULL)
  {
   if(IsHook(next->info)==0)//被HOOK
   {
    //重填HOOK
    KeSetEvent(&s_event,0,TRUE);// 通知
    Sleep(200);//原来是My_Sleep，但没有找到定义
    DbgPrint("Hook:被反HOOK！！\n");
    next->info->old_address = SSDTfunction::Get_Current_Proc_address(next->info->index,next->info->SSDT_TABLE);
    Hook(next->info);
    Anti_HOOK++;
   }
   next=next->Next;
  }
  if (Anti_HOOK>30)//被反HOOK 30次 忍无可忍，实行打击
  {Blue();}
  Sleep(MAX);//原来是My_Sleep，但没有找到定义
  KeSetEvent(&s_event,0,TRUE);// 通知
 }
 DbgPrint("保护线程已经关闭！！\n");
 PsTerminateSystemThread(STATUS_SUCCESS);
}


BOOLEAN SSDThook::Add_Protect(PHOOK_INFO pinfo)
{
 PProtect_list next;
 DbgPrint("添加保护队列！\n");
 if (pinfo==NULL)
 {
  DbgPrint("PHOOK_INFO == NULL \n");
  return FALSE;
 }
 next=&head_Protect;
 while (next!=NULL)
 {
  if (next->Next==NULL)
  {
   //找到空
   PProtect_list n;
   n=(PProtect_list)ExAllocatePoolWithTag(NonPagedPool,sizeof(Protect_list),1447);
   if (n==NULL)
   {
    DbgPrint("分配队列失败\n");
    return FALSE;
   }
   n->info=pinfo;
   n->Before=next;
   n->Next=NULL;
   next->Next=n;
   DbgPrint("加入队列成功！\n");
   return TRUE;
  }
  next=next->Next;//下一个表
 }
 DbgPrint("队列有错误！\n");
 return FALSE;
}


void SSDThook::Blue()//打击
{
 _asm
 {
  mov eax,0;
  mov [eax],100;
 }
}


void SSDThook::Thread_p_proc(PVOID a)
{
 LARGE_INTEGER my_interval;
 NTSTATUS status;
 my_interval.QuadPart=DELAY_ONE_MILLISECOND;//DELAY_ONE_MILLISECOND=(-10)*1000
 my_interval.QuadPart*=5000;//4500毫秒
 while(1)
 {
  DbgPrint("Whlie \n");
  status=KeWaitForSingleObject(&s_event,Executive,KernelMode,0,&my_interval);
  if(status == STATUS_TIMEOUT)//超时
  {
    DbgPrint("线程超时！   蓝屏！！\n ");
    Blue();
  }
 DbgPrint("KeResetEvent\n");
 KeResetEvent(&s_event);
 }
 return;
}