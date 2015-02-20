#include "SSDTfunction.h"

ULONG SSDTfunction::GetReallySSDTFunctionAddress(ULONG index,ULONG _SSDT)
{
        /*��ȡ����
        ��ȡSSDT���ַ
        ��ȡntkrnlpa.exe�����ַ
        SSDT���ַ-ntkrnlpa.exe�����ַ = SSDT��RVA
        ����RVA��ntkrnlpa.exe�ļ��е� FileOffet
        ���뱣����Ӳ���ϵ�SSDT��*/
        ULONG needlen;
        ULONG i;
        PVOID buf;
        ULONG ntkrnlpaBase=0;
        ULONG MemorySSDT; // �ڴ���SSDT���ַ
        ULONG SSDT_RVA=0;
        PSYSTEM_MODULE_INFORMATION pModules;
        HANDLE handle=NULL;
        UNICODE_STRING FileName;
        OBJECT_ATTRIBUTES fileInfo={0};
        NTSTATUS status;
        IO_STATUS_BLOCK ioStatus;
        FILE_STANDARD_INFORMATION fsi={0};
        unsigned char *pFileBuff=NULL;
        ULONG NumberOfSection=0; //������Ŀ
        PIMAGE_DOS_HEADER pDosHead=NULL;
        PIMAGE_NT_HEADERS pNtHead=NULL;
        PIMAGE_SECTION_HEADER pSection=NULL; //��������
        ULONG addr2=0;
        ULONG addr1=0;
        ULONG ret_address;
        ULONG ssa;
        PWCHAR kernelName;
        ZwQuerySystemInformation(SystemModuleInformation,NULL,0,&needlen);
        buf=(PVOID)ExAllocatePoolWithTag(NonPagedPool,needlen,1448);
        if (buf==NULL){ DbgPrint("�����ڴ�ʧ�ܣ���\n");return STATUS_UNSUCCESSFUL;}
        ZwQuerySystemInformation(SystemModuleInformation,(PVOID)buf,needlen,&needlen);
        pModules=(PSYSTEM_MODULE_INFORMATION)buf;
        ntkrnlpaBase=(ULONG)pModules->Module[0].Base;
        if (ntkrnlpaBase==0)
        {
                DbgPrint("�ں˻�ַ��ȡʧ�ܣ�\n");
                return STATUS_UNSUCCESSFUL;
        }
        MemorySSDT=*(ULONG*)_SSDT;
        if (MemorySSDT==0)
        {
                DbgPrint("��ȡSSDT���ַʧ�ܡ�\n");
                return STATUS_UNSUCCESSFUL;
        }else 
		{
		  if (MemorySSDT<ntkrnlpaBase || MemorySSDT>(ntkrnlpaBase+pModules->Module[0].Size)) //SSDT����ntkrnlpaģ����
          {
                DbgPrint("SSDT���ַ��Ч��\n");
                return STATUS_UNSUCCESSFUL;
          }
		}
        SSDT_RVA = MemorySSDT-ntkrnlpaBase;
        if (SSDT_RVA==0)
        {
                DbgPrint("SSDT RVAδȡ����\n");
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
        kernelName = ssa ? L"\\SystemRoot\\system32\\ntkrnlpa.exe" : L"\\SystemRoot\\system32\\ntoskrnl.exe";//ssaΪ1 kernelName=\\SystemRoot\\system32\\ntkrnlpa.exe
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
                if ((SSDT_RVA>pSection.VirtualAddress) && (SSDT_RVA<(pSection.VirtualAddress+pSection.SizeOfRawData)))  //�ж��Ƿ�λ��ĳ������֮��
                {
                        /*���ݵ��ļ�ƫ��=(����RVA - ��RVA) + �ڵ��ļ�ƫ��
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
        // �ض�λ
        ret_address-=pNtHead->OptionalHeader.ImageBase;
        ret_address+=ntkrnlpaBase;
        //�ͷ���Դ
        ExFreePool(pFileBuff);
        ExFreePool(buf);
        ZwClose(handle);
        return ret_address;
}

ULONG SSDTfunction::Get_SSDT_Proc_index(UNICODE_STRING FunctionName)
{
        /*��ȡ���裺
        �򿪲����� ntdll.dll
        �ӵ��������ҵ�ZwXXX����
        ���ݵ�������ҵ���ZwXXX������RVA�����FileOfset
        Ȼ�� ���� pfile+Fileoffset�����з����
          public NtOpenFile
          NtOpenFile proc near
          mov eax, 0B3h ; NtOpenFile
          mov edx, 7FFE0300h
          call dword ptr [edx]
          retn 18h
          NtOpenFile endp
          mov eax, 0B3h  0b3h��������
        ����ָ��Ļ����볤����5
        B8 B3 00 00 00
        B8 ��ָ����
        B3 00 00 00 �ǲ�������Ҳ��������
        �������Ǵ� pfile+Fileoffset+1 �ĵط�����4�ֽ����ݵ�һ�������ͱ�����ok��*/
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
        PIMAGE_SECTION_HEADER pSection=NULL; //��������
        ULONG Fil_offset;
        PULONG names_list;
        ULONG FunctionNum;
        PULONG AddressOfNameOrdinals;
        unsigned short Index=0;
        PULONG addr=NULL;
        ULONG addr2;
        ULONG addr3;
        unsigned short *xh;  //ָ����ű�
        NtdllName = L"\\SystemRoot\\system32\\Ntdll.dll";
        RtlInitUnicodeString(&FileName,NtdllName);
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
        pFileBuf=(unsigned char *)ExAllocatePoolWithTag(NonPagedPool,(size_t)fsi.EndOfFile.QuadPart,1450);
        if (pFileBuf==NULL)
        {
         DbgPrint("Ϊ�ļ����仺����ʧ�ܣ���\n");
         return STATUS_UNSUCCESSFUL;
        }
        ZwReadFile(handle,NULL,NULL,NULL,&ioStatus,pFileBuf,(size_t)fsi.EndOfFile.QuadPart,0,NULL);
        pDosHead = (PIMAGE_DOS_HEADER)pFileBuf;
        pNtHead = (PIMAGE_NT_HEADERS)(pFileBuf + pDosHead->e_lfanew);
        pSection = (PIMAGE_SECTION_HEADER)((ULONG)pNtHead+sizeof(IMAGE_NT_HEADERS));  //��λ������
        IED=(PIMAGE_EXPORT_DIRECTORY)pNtHead->OptionalHeader.DataDirectory[0].VirtualAddress;
        Fil_offset = RvaToRaw(pDosHead,pNtHead->OptionalHeader.DataDirectory[0].VirtualAddress);
        IED = (PIMAGE_EXPORT_DIRECTORY)(pFileBuf+Fil_offset);
        RvaOfNames=(PULONG)IED->AddressOfNames;
        RvaOfNames=(PULONG)((ULONG)(RvaOfNames)+((ULONG)pFileBuf));
        names_list = (PULONG)(pFileBuf+RvaToRaw(pDosHead,IED->AddressOfNames));
        for (i=0;i<IED->NumberOfNames;i++) //��������
        {
            UNICODE_STRING name_unicode;
            ANSI_STRING name_ansi;
            FunName  = (CHAR *)(pFileBuf+RvaToRaw(pDosHead,names_list));
            RtlInitAnsiString(&name_ansi,FunName);
            RtlAnsiStringToUnicodeString(&name_unicode,&name_ansi,TRUE);
            if(RtlCompareUnicodeString(&name_unicode,&FunctionName,TRUE)==0)
            {
                 DbgPrint("�ҵ�����:%d    �����ǣ�%S",i,name_unicode);
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
        PIMAGE_SECTION_HEADER pSection=NULL; //��������
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
				  //���ݵ��ļ�ƫ��=(����RVA - ��RVA) + �ڵ��ļ�ƫ��
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
                mov ebx,SSDT  //ebxָ��SSDT
                mov ebx,[ebx] //��һ�ű�ĵ�ַ
                mov eax,_index
                add ebx,eax  //ƫ��
                mov ebx,[ebx]
                mov address_curr,ebx
                pop eax       
                pop ebx
        }
        return address_curr;
}