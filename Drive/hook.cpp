#include "SSDTfunction.h"
#include "stru.h"

#include<ntimage.h>
#define SSDT_HOOK 1  //ֱ��SSDT HOOK
#define SSDT_CURR_HOOK 2//SSDT���е�ǰ�����ı���HOOK 

Protect_list head_Protect;
BOOLEAN Start=FALSE;
HANDLE handle_Protect;  //�����߳̾��
KEVENT s_event;
ULONG Anti_HOOK = 0;
void Thread_proc(PVOID a);
void Thread_p_proc(PVOID a);
void Blue();

//������ôһ��NativeAPI������ʹ��ǰ��������
NTSTATUS NTAPI ZwQuerySystemInformation(SYSTEM_INFORMATION_CLASS SystemInformationClass,PVOID SystemInformation,ULONG SystemInformationLength,PULONG ReturnLength);

NTSTATUS Hook(PHOOK_INFO info)
{
 NTSTATUS status = STATUS_UNSUCCESSFUL;
 _asm
 {
    //�ر�д����
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
 // ѡģʽ
 if(SSDTfunction::GetReallySSDTFunctionAddress(info->index,info->SSDT_TABLE)==SSDTfunction::Get_Current_Proc_address(info->index,info->SSDT_TABLE))
 {
 // ��ǰ������ԭʼ������ͬ  ֱ��SSDT-HOOK
 ULONG indexs = info->index * 4;
 _asm
 {
   pushad
    mov ecx,info
    mov ebx,[ecx+HOOK_INFO.SSDT_TABLE]
    mov ebx,[ebx]  // ��һ�ű�
    mov eax,indexs
    add ebx,eax
    mov eax,[ebx]  //����ԭ����
    mov [ecx+HOOK_INFO.old_address],eax
    // ��HOOK_INFO.new_address��ַд��
    mov eax,[ecx+HOOK_INFO.new_address]
    mov [ebx],eax
   popad
 }
 info->Type = SSDT_HOOK;
 status = STATUS_SUCCESS;
 }
 else
 {
   //��ת��ַ-��ǰJMP���ڵ�ַ -5��E9 + 32λ��ַ�� [���]
   ULONG indexs = info->index * 4;
   ULONG jmpaddr;
   jmpaddr = info->new_address - info ->old_address -5;
   _asm
   {  
     pushad
      mov ecx,info
      mov ebx,[ecx+HOOK_INFO.old_address]  //ebx
      mov al,0xE9   //E9 jmp������
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
 _asm //�ָ�д����
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

ULONG IsHook(PHOOK_INFO info)
{
 ULONG STATUS=0;
 if(SSDT_HOOK==info->Type) //SSDT-HOOK
 {
    ULONG *addr = (PULONG)(*((PULONG)info->SSDT_TABLE));
    ULONG  CurrentAddr;
    PULONG pNewAddr;
    //ULONG pTrueAddr = *(PULONG)(&info->data[1]);
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
  // InineHook
  PUCHAR now_data;
  ULONG *ssdt =  (PULONG)(*((PULONG)info->SSDT_TABLE));
  now_data =(PUCHAR) info->old_address;
  if (ssdt[info->index]!=(ULONG)now_data)  //���жϵ�ַ�Ƿ���ͬ
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

ULONG StartProtect(PHOOK_INFO info)
{
 /*�������ܣ�����HOOK����
  ������ m=1����HOOK���� info ��һ������
  ����ֵ������״̬�������info�����Ϣ*/
   NTSTATUS status;
   head_Protect.Before=NULL; //��ʼ��ͷ
   head_Protect.Next=NULL;
   head_Protect.info=info;
   DbgPrint("���г�ʼ����ϣ�\n");
   Start=TRUE;
   status=PsCreateSystemThread(&handle_Protect,0,NULL,NULL,NULL,Thread_proc,NULL);
   if (!NT_SUCCESS(status))
   {

    DbgPrint("�����̴߳���ʧ�ܣ�����\n");

    return -1;

   }

   ZwClose(handle_Protect);

   return 1;

 

}

void Thread_proc(PVOID a) //�̺߳���

{

 ULONG MAX=4000;

 PProtect_list n=NULL,next=NULL;

 HANDLE hHandle;

 DbgPrint("��������׼����������\n");

 while(head_Protect.info==NULL)

 {

  continue;

 }

 KeInitializeEvent(&s_event,SynchronizationEvent,FALSE);

 PsCreateSystemThread(&hHandle,0,NULL,NULL,NULL,Thread_p_proc,NULL);

 ZwClose(hHandle);

 while (TRUE)

 {

  n=&head_Protect;

  next=n;

  while(next!=NULL)

  {

   if(IsHook(next->info)==0)  //��HOOK

   {

    //����HOOK

    KeSetEvent(&s_event,0,TRUE);   // ֪ͨ

    My_Sleep(200);

    DbgPrint("Hook:����HOOK����\n");

    next->info->old_address = SSDTfunction::Get_Current_Proc_address(next->info->index,next->info->SSDT_TABLE);

    Hook(next->info);

    Anti_HOOK++;

   }

   next=next->Next;

  }

  if (Anti_HOOK>30)  //����HOOK 30�� ���޿��̣�ʵ�д��

  {

   Blue();

  }

  My_Sleep(MAX);

  KeSetEvent(&s_event,0,TRUE);   // ֪ͨ

 }

 DbgPrint("�����߳��Ѿ��رգ���\n");

 PsTerminateSystemThread(STATUS_SUCCESS);

}

BOOLEAN Add_Protect(PHOOK_INFO pinfo)

{

 PProtect_list next;

 DbgPrint("��ӱ������У�\n");

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

   //�ҵ���

   PProtect_list n;

   n=(PProtect_list)ExAllocatePoolWithTag(NonPagedPool,sizeof(Protect_list),1447);

   if (n==NULL)

   {

    DbgPrint("�������ʧ��\n");

    return FALSE;

   }

   n->info=pinfo;

   n->Before=next;

   n->Next=NULL;

   next->Next=n;

   DbgPrint("������гɹ���\n");

   return TRUE;

  }

  next=next->Next;  //��һ����

 }

 DbgPrint("�����д���\n");

 return FALSE;

}

  

 SSDT_RVA = MemorySSDT-ntkrnlpaBase;

 if (SSDT_RVA==0)

 {

  DbgPrint("SSDT RVAδȡ����\n");

  return STATUS_UNSUCCESSFUL;

 }

 //DbgPrint("SSDT RVA:%x",SSDT_RVA);

 __asm

 {

  pushad

   _emit 0x0f

   _emit 0x20

   _emit 0xe0    //mov eax,cr4

   shr eax,4

   and eax,1

   mov ssa,eax

   popad

 }

 kernelName = ssa?L"\\SystemRoot\\system32\\ntkrnlpa.exe" : L"\\SystemRoot\\system32\\ntoskrnl.exe";

 RtlInitUnicodeString(&FileName,kernelName);

 InitializeObjectAttributes(&fileInfo,&FileName,OBJ_CASE_INSENSITIVE,NULL,NULL);

 status=ZwCreateFile(&handle,GENERIC_READ,&fileInfo,&ioStatus,NULL,FILE_ATTRIBUTE_NORMAL,FILE_SHARE_READ|FILE_SHARE_WRITE,FILE_OPEN,FILE_SYNCHRONOUS_IO_NONALERT,NULL,0);

 if (ioStatus.Information!=1)

 {

  DbgPrint("�ļ���ʧ�ܣ�\n");

  return STATUS_UNSUCCESSFUL;

 }

 ZwQueryInformationFile(handle,&ioStatus,&fsi,sizeof(FILE_STANDARD_INFORMATION),FileStandardInformation);

 if ((LONG)fsi.EndOfFile.QuadPart==0)

 {

  DbgPrint("��ȡ�ļ���Сʧ�ܣ�\n");

  return STATUS_UNSUCCESSFUL;

 }

 pFileBuff=(unsigned char *)ExAllocatePoolWithTag(NonPagedPool,(size_t)fsi.EndOfFile.QuadPart,1449);

 if (pFileBuff==NULL)

 {

  DbgPrint("Ϊ�ļ����仺����ʧ�ܣ���\n");

  return STATUS_UNSUCCESSFUL;

 }

 ZwReadFile(handle,NULL,NULL,NULL,&ioStatus,pFileBuff,(size_t)fsi.EndOfFile.QuadPart,0,NULL);

 pDosHead=(PIMAGE_DOS_HEADER)pFileBuff;

 pNtHead=(PIMAGE_NT_HEADERS)((ULONG)pDosHead+(ULONG)(pDosHead->e_lfanew));

 NumberOfSection=pNtHead->FileHeader.NumberOfSections;

 pSection=(PIMAGE_SECTION_HEADER)((ULONG)pNtHead+sizeof(IMAGE_NT_HEADERS));

 for(i=0;i<NumberOfSection;i++)

 {

  if ((SSDT_RVA>pSection[i].VirtualAddress) && (SSDT_RVA<(pSection[i].VirtualAddress+pSection[i].SizeOfRawData)))  //�ж��Ƿ�λ��ĳ������֮��

  {

   //���ݵ��ļ�ƫ��=(����RVA - ��RVA) + �ڵ��ļ�ƫ��

   //DbgPrint("RVA :%x   %d ",SSDT_RVA,i);

   //DbgPrint("RVA %d",pSection[i].VirtualAddress);

   addr2=SSDT_RVA-pSection[i].VirtualAddress;

   addr1=addr2+pSection[i].PointerToRawData;

   break;

  }

 }

 //DbgPrint("File Offset:%x",addr1);

 ret_address=addr1+index*4;

 _asm

 {

  pushad

   mov ecx,pFileBuff

   mov ebx,ret_address

   mov eax,[ebx+ecx]

   mov ret_address,eax 

  popad

 }

 // �ض�λ

 ret_address-=pNtHead->OptionalHeader.ImageBase;

 ret_address+=ntkrnlpaBase;

 //�ͷ���Դ

 ExFreePool(pFileBuff);

 ExFreePool(buf);

 ZwClose(handle);

return ret_address;

}

void Blue()

{

 _asm

 {

  mov eax,0;

  mov [eax],100;

 }

  

}

void Thread_p_proc(PVOID a)  //˫�̱߳���

{

 LARGE_INTEGER my_interval;

 NTSTATUS status;

 my_interval.QuadPart=DELAY_ONE_MILLISECOND; //DELAY_ONE_MILLISECOND=(-10)*1000

 my_interval.QuadPart*=5000;  //4500 //����

 while(TRUE)

 {

  DbgPrint("Whlie \n");

 status=KeWaitForSingleObject(&s_event,Executive,KernelMode,0,&my_interval);

if (status == STATUS_TIMEOUT)  //��ʱ

 {

  DbgPrint("�̳߳�ʱ��   ��������\n ");

  Blue();

 }

 DbgPrint("KeResetEvent\n");

 KeResetEvent(&s_event);

 }

 return;

}