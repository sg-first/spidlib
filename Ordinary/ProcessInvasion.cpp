#include "ProcessInvasion.h"

bool ProcessInvasion::IAT_HOOK(PVOID imageBase,char *HookApiName,PVOID newAddr)
{
    PIMAGE_DOS_HEADER pDosHead;
    PIMAGE_NT_HEADERS pNtHead;
    pDosHead = (PIMAGE_DOS_HEADER)imageBase;
    pNtHead = (PIMAGE_NT_HEADERS)p_add(imageBase,pDosHead->e_lfanew);
    //扫描导入表
    PIMAGE_IMPORT_DESCRIPTOR pImport;
    IMAGE_IMPORT_DESCRIPTOR null_immport={0};
    pImport=(PIMAGE_IMPORT_DESCRIPTOR)RvaToVa(imageBase,pNtHead->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
    while (memcmp(pImport,&null_immport,sizeof(IMAGE_IMPORT_DESCRIPTOR)))
    {
        PIMAGE_THUNK_DATA thunk;
        DWORD *IAT;
        thunk =(PIMAGE_THUNK_DATA) RvaToVa(imageBase,pImport->OriginalFirstThunk);
        IAT=(DWORD *)RvaToVa(imageBase,pImport->FirstThunk);
        while (thunk->u1.AddressOfData)
        {
            DWORD test;
            char *strApiName;
            test = *(DWORD *)thunk;
            PIMAGE_IMPORT_BY_NAME ApiName;
            if(test&IMAGE_ORDINAL_FLAG32)
                continue;
            ApiName=(PIMAGE_IMPORT_BY_NAME)RvaToVa(imageBase,thunk->u1.AddressOfData);
            strApiName=(char *)&ApiName->Name;
            if (!strcmp(HookApiName,strApiName))
            {
                DWORD old;
                //设置IAT
                //printf("\t%s Address:%x IAT:%x\n",strApiName,AddressOfFunction,IAT);
                VirtualProtect(IAT,4,PAGE_EXECUTE_READWRITE,&old);
                *(DWORD *)IAT = (DWORD)newAddr;
                VirtualProtect(IAT,4,old,NULL);
                return true;
            }
            thunk++;
            IAT++;
        }
        pImport++;
    }
    return false;
}