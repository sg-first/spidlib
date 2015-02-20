#include "SSDTfunction.h"

ULONG SSDTfunction::GetReallySSDTFunctionAddress(ULONG index,ULONG _SSDT)
{
        /*获取步骤
        获取SSDT表基址
        获取ntkrnlpa.exe载入基址
        SSDT表基址-ntkrnlpa.exe载入基址 = SSDT表RVA
        计算RVA在ntkrnlpa.exe文件中的 FileOffet
        读入保存在硬盘上的SSDT表*/
        ULONG needlen;
        ULONG i;
        PVOID buf;
        ULONG ntkrnlpaBase=0;
        ULONG MemorySSDT; // 内存中SSDT表基址
        ULONG SSDT_RVA=0;
        PSYSTEM_MODULE_INFORMATION pModules;
        HANDLE handle=NULL;
        UNICODE_STRING FileName;
        OBJECT_ATTRIBUTES fileInfo={0};
        NTSTATUS status;
        IO_STATUS_BLOCK ioStatus;
        FILE_STANDARD_INFORMATION fsi={0};
        unsigned char *pFileBuff=NULL;
        ULONG NumberOfSection=0; //区块数目
        PIMAGE_DOS_HEADER pDosHead=NULL;
        PIMAGE_NT_HEADERS pNtHead=NULL;
        PIMAGE_SECTION_HEADER pSection=NULL; //所有区段
        ULONG addr2=0;
        ULONG addr1=0;
        ULONG ret_address;
        ULONG ssa;
        PWCHAR kernelName;
        ZwQuerySystemInformation(SystemModuleInformation,NULL,0,&needlen);
        buf=(PVOID)ExAllocatePoolWithTag(NonPagedPool,needlen,1448);
        if (buf==NULL){ DbgPrint("分配内存失败！！\n");return STATUS_UNSUCCESSFUL;}
        ZwQuerySystemInformation(SystemModuleInformation,(PVOID)buf,needlen,&needlen);
        pModules=(PSYSTEM_MODULE_INFORMATION)buf;
        ntkrnlpaBase=(ULONG)pModules->Module[0].Base;
        if (ntkrnlpaBase==0)
        {
                DbgPrint("内核基址获取失败！\n");
                return STATUS_UNSUCCESSFUL;
        }
        MemorySSDT=*(ULONG*)_SSDT;
        if (MemorySSDT==0)
        {
                DbgPrint("获取SSDT表地址失败。\n");
                return STATUS_UNSUCCESSFUL;
        }else 
		{
		  if (MemorySSDT<ntkrnlpaBase || MemorySSDT>(ntkrnlpaBase+pModules->Module[0].Size)) //SSDT表不再ntkrnlpa模块中
          {
                DbgPrint("SSDT表地址无效！\n");
                return STATUS_UNSUCCESSFUL;
          }
		}
        SSDT_RVA = MemorySSDT-ntkrnlpaBase;
        if (SSDT_RVA==0)
        {
                DbgPrint("SSDT RVA未取到！\n");
                return STATUS_UNSUCCESSFUL;
        }
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
        kernelName = ssa ? L"\\SystemRoot\\system32\\ntkrnlpa.exe" : L"\\SystemRoot\\system32\\ntoskrnl.exe";//ssa为1 kernelName=\\SystemRoot\\system32\\ntkrnlpa.exe
        RtlInitUnicodeString(&FileName,kernelName);
        InitializeObjectAttributes(&fileInfo,&FileName,OBJ_CASE_INSENSITIVE,NULL,NULL);
        status=ZwCreateFile(&handle,GENERIC_READ,&fileInfo,&ioStatus,NULL,FILE_ATTRIBUTE_NORMAL,FILE_SHARE_READ|FILE_SHARE_WRITE,FILE_OPEN,FILE_SYNCHRONOUS_IO_NONALERT,NULL,0);
        if (ioStatus.Information!=1)
        {
                DbgPrint("文件打开失败！\n");
                return STATUS_UNSUCCESSFUL;
        }
        ZwQueryInformationFile(handle,&ioStatus,&fsi,sizeof(FILE_STANDARD_INFORMATION),FileStandardInformation);
        if ((LONG)fsi.EndOfFile.QuadPart==0)
        {
                DbgPrint("获取文件大小失败！\n");
                return STATUS_UNSUCCESSFUL;
        }
        pFileBuff=(unsigned char *)ExAllocatePoolWithTag(NonPagedPool,(size_t)fsi.EndOfFile.QuadPart,1449);
        if (pFileBuff==NULL)
        {
                DbgPrint("为文件分配缓冲区失败！！\n");
                return STATUS_UNSUCCESSFUL;
        }
        ZwReadFile(handle,NULL,NULL,NULL,&ioStatus,pFileBuff,(size_t)fsi.EndOfFile.QuadPart,0,NULL);
        pDosHead=(PIMAGE_DOS_HEADER)pFileBuff;
        pNtHead=(PIMAGE_NT_HEADERS)((ULONG)pDosHead+(ULONG)(pDosHead->e_lfanew));
        NumberOfSection=pNtHead->FileHeader.NumberOfSections;
        pSection=(PIMAGE_SECTION_HEADER)((ULONG)pNtHead+sizeof(IMAGE_NT_HEADERS));
        for(i=0;i<NumberOfSection;i++)
        {
                if ((SSDT_RVA>pSection.VirtualAddress) && (SSDT_RVA<(pSection.VirtualAddress+pSection.SizeOfRawData)))  //判断是否位于某个区块之间
                {
                        /*数据的文件偏移=(数据RVA - 节RVA) + 节的文件偏移
                        DbgPrint("RVA :%x   %d ",SSDT_RVA,i);
                        DbgPrint("RVA %d",pSection.VirtualAddress);*/
                        addr2=SSDT_RVA-pSection.VirtualAddress;
                        addr1=addr2+pSection.PointerToRawData;
                        break;
                }
        }
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
        // 重定位
        ret_address-=pNtHead->OptionalHeader.ImageBase;
        ret_address+=ntkrnlpaBase;
        //释放资源
        ExFreePool(pFileBuff);
        ExFreePool(buf);
        ZwClose(handle);
        return ret_address;
}

ULONG SSDTfunction::Get_SSDT_Proc_index(UNICODE_STRING FunctionName)
{
        /*获取步骤：
        打开并读入 ntdll.dll
        从导出表中找到ZwXXX函数
        根据导入表里找到的ZwXXX函数的RVA计算出FileOfset
        然后 根据 pfile+Fileoffset处进行反汇编
          public NtOpenFile
          NtOpenFile proc near
          mov eax, 0B3h ; NtOpenFile
          mov edx, 7FFE0300h
          call dword ptr [edx]
          retn 18h
          NtOpenFile endp
          mov eax, 0B3h  0b3h就是索引
        这条指令的机器码长度是5
        B8 B3 00 00 00
        B8 是指令码
        B3 00 00 00 是操作数，也就是索引
        所以我们从 pfile+Fileoffset+1 的地方读入4字节数据到一个整数型变量就ok了*/
        PWCHAR NtdllName;
        HANDLE handle=NULL;
        UNICODE_STRING FileName;
        OBJECT_ATTRIBUTES fileInfo={0};
        NTSTATUS status;
        IO_STATUS_BLOCK ioStatus;
        PUCHAR pFileBuf;   // Ntdll.dll
        FILE_STANDARD_INFORMATION fsi={0};
        PIMAGE_DOS_HEADER pDosHead=NULL;
        PIMAGE_NT_HEADERS pNtHead=NULL;
        PIMAGE_EXPORT_DIRECTORY IED=NULL;
        ULONG i;
        CHAR *FunName;
        PULONG RvaOfNames;
        PIMAGE_SECTION_HEADER pSection=NULL; //所有区段
        ULONG Fil_offset;
        PULONG names_list;
        ULONG FunctionNum;
        PULONG AddressOfNameOrdinals;
        unsigned short Index=0;
        PULONG addr=NULL;
        ULONG addr2;
        ULONG addr3;
        unsigned short *xh;  //指向序号表
        NtdllName = L"\\SystemRoot\\system32\\Ntdll.dll";
        RtlInitUnicodeString(&FileName,NtdllName);
        InitializeObjectAttributes(&fileInfo,&FileName,OBJ_CASE_INSENSITIVE,NULL,NULL);
        status=ZwCreateFile(&handle,GENERIC_READ,&fileInfo,&ioStatus,NULL,FILE_ATTRIBUTE_NORMAL,FILE_SHARE_READ|FILE_SHARE_WRITE,FILE_OPEN,FILE_SYNCHRONOUS_IO_NONALERT,NULL,0);
        if (ioStatus.Information!=1)
        {
           DbgPrint("文件打开失败！\n");
           return STATUS_UNSUCCESSFUL;
        }
        ZwQueryInformationFile(handle,&ioStatus,&fsi,sizeof(FILE_STANDARD_INFORMATION),FileStandardInformation);
        if ((LONG)fsi.EndOfFile.QuadPart==0)
        {
           DbgPrint("获取文件大小失败！\n");
           return STATUS_UNSUCCESSFUL;
        }        
        pFileBuf=(unsigned char *)ExAllocatePoolWithTag(NonPagedPool,(size_t)fsi.EndOfFile.QuadPart,1450);
        if (pFileBuf==NULL)
        {
         DbgPrint("为文件分配缓冲区失败！！\n");
         return STATUS_UNSUCCESSFUL;
        }
        ZwReadFile(handle,NULL,NULL,NULL,&ioStatus,pFileBuf,(size_t)fsi.EndOfFile.QuadPart,0,NULL);
        pDosHead = (PIMAGE_DOS_HEADER)pFileBuf;
        pNtHead = (PIMAGE_NT_HEADERS)(pFileBuf + pDosHead->e_lfanew);
        pSection = (PIMAGE_SECTION_HEADER)((ULONG)pNtHead+sizeof(IMAGE_NT_HEADERS));  //定位到区块
        IED=(PIMAGE_EXPORT_DIRECTORY)pNtHead->OptionalHeader.DataDirectory[0].VirtualAddress;
        Fil_offset = RvaToRaw(pDosHead,pNtHead->OptionalHeader.DataDirectory[0].VirtualAddress);
        IED = (PIMAGE_EXPORT_DIRECTORY)(pFileBuf+Fil_offset);
        RvaOfNames=(PULONG)IED->AddressOfNames;
        RvaOfNames=(PULONG)((ULONG)(RvaOfNames)+((ULONG)pFileBuf));
        names_list = (PULONG)(pFileBuf+RvaToRaw(pDosHead,IED->AddressOfNames));
        for (i=0;i<IED->NumberOfNames;i++) //查找名称
        {
            UNICODE_STRING name_unicode;
            ANSI_STRING name_ansi;
            FunName  = (CHAR *)(pFileBuf+RvaToRaw(pDosHead,names_list));
            RtlInitAnsiString(&name_ansi,FunName);
            RtlAnsiStringToUnicodeString(&name_unicode,&name_ansi,TRUE);
            if(RtlCompareUnicodeString(&name_unicode,&FunctionName,TRUE)==0)
            {
                 DbgPrint("找到函数:%d    名称是：%S",i,name_unicode);
                 FunctionNum = i;
                 break;
            }
            }
        xh = (unsigned short * )(RvaToRaw(pDosHead,IED->AddressOfNameOrdinals)+pFileBuf);
        addr = (PULONG)(RvaToRaw(pDosHead,IED->AddressOfFunctions)+pFileBuf);
        Index = xh[FunctionNum];
        Index = Index + (unsigned short)IED->Base;
        addr2 = addr[Index];
        addr3 = (ULONG)(RvaToRaw(pDosHead,addr2)+pFileBuf);
        _asm
        {
           pushad
              mov ebx,addr3
              mov eax,dword ptr[ebx+1]
              mov addr3,eax
           popad
        }
        ZwClose(handle);
        ExFreePool(pFileBuf);
        addr3-=1;
        return addr3;
}

ULONG SSDTfunction::RvaToRaw(PIMAGE_DOS_HEADER pDosHead,ULONG RVA)
{
        PIMAGE_SECTION_HEADER pSection=NULL; //所有区段
        PIMAGE_NT_HEADERS pNtHead;
        ULONG Fil_offset;
        ULONG i=0;
        ULONG Raw = 0;
        pNtHead=(PIMAGE_NT_HEADERS)(((ULONG)pDosHead)+pDosHead->e_lfanew);
        pSection=(PIMAGE_SECTION_HEADER)((ULONG)pNtHead+sizeof(IMAGE_NT_HEADERS));
        if (RVA < pSection[0].VirtualAddress)
        {
             Raw =RVA;
             return Raw;
        }
        for (i=0;i<pNtHead->FileHeader.NumberOfSections;i++)
        {
                if (RVA > pSection.VirtualAddress && RVA < (pSection.VirtualAddress+pSection.SizeOfRawData))
                {
				  //数据的文件偏移=(数据RVA - 节RVA) + 节的文件偏移
                  Raw = (RVA - pSection.VirtualAddress)+pSection.PointerToRawData;
                  break;
                }
        }
        return Raw;
}

ULONG SSDTfunction::Get_Current_Proc_address(ULONG index,LONG SSDT)
{
        ULONG address_curr;
        ULONG _index=index*4;
        _asm
		{  
                push ebx
                push eax
                mov ebx,SSDT  //ebx指向SSDT
                mov ebx,[ebx] //第一张表的地址
                mov eax,_index
                add ebx,eax  //偏移
                mov ebx,[ebx]
                mov address_curr,ebx
                pop eax       
                pop ebx
        }
        return address_curr;
}