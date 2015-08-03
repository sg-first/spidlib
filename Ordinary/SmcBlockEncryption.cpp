#include "SmcBlockEncryption.h"

void SmcBlockEncryption::selfEncryption(char *key)
{SmcBlockEncryption::encryption(GetModuleHandle(0),key);}

void SmcBlockEncryption::encryption(char *pBuf,char *key)
{
    // SMC解密区段
    const char *szSecName = ".SMC";
    short nSec;
    PIMAGE_DOS_HEADER pDosHeader;
    PIMAGE_NT_HEADERS pNtHeader;
    PIMAGE_SECTION_HEADER pSec;
    pDosHeader=(PIMAGE_DOS_HEADER)pBuf;
    pNtHeader=(PIMAGE_NT_HEADERS)&pBuf[pDosHeader->e_lfanew];
    nSec=pNtHeader->FileHeader.NumberOfSections;
    pSec=(PIMAGE_SECTION_HEADER)&pBuf[ sizeof(IMAGE_NT_HEADERS)+pDosHeader->e_lfanew];
    for (int i=0;i<nSec;i++)
    {
        if (strcmp((char *)&pSec->Name,szSecName)==0)
        {
            int pack_size;
            char *packStart; 
            pack_size=pSec->SizeOfRawData;
            packStart = &pBuf[pSec->VirtualAddress];
            SmcBlockEncryption::xorPlus(packStart,pack_size,key,strlen(key));
            //解密成功
            return;
        }
        pSec++;
    }
}

bool SmcBlockEncryption::ecrypt(HANDLE hFile,char *key)
{
    // SMC加密区段
    HANDLE hMap;
    const char *szSecName = ".SMC";
    char *pBuf;
    int size;
    short nSec;
    PIMAGE_DOS_HEADER pDosHeader;
    PIMAGE_NT_HEADERS pNtHeader;
    PIMAGE_SECTION_HEADER pSec;

    size = GetFileSize(hFile,0);
    hMap=CreateFileMapping(hFile,NULL,PAGE_READWRITE,0,size,NULL);
    if (hMap==INVALID_HANDLE_VALUE)
    {
		_viewf:
        return false;
    }
    pBuf=(char *)MapViewOfFile(hMap,FILE_MAP_WRITE|FILE_MAP_READ,0,0,size);
    if(!pBuf) goto _viewf;
    pDosHeader=(PIMAGE_DOS_HEADER)pBuf;
    pNtHeader=(PIMAGE_NT_HEADERS)&pBuf[pDosHeader->e_lfanew];
    if (pNtHeader->Signature!=IMAGE_NT_SIGNATURE)
    {
        SmcBlockEncryption::clear(pBuf,hMap);
		return false;
    }
    nSec=pNtHeader->FileHeader.NumberOfSections;
    pSec=(PIMAGE_SECTION_HEADER)&pBuf[ sizeof(IMAGE_NT_HEADERS)+pDosHeader->e_lfanew];
    for (int i=0;i<nSec;i++)
    {
        if (strcmp((char *)&pSec->Name,szSecName)==0)
        {
            int pack_size;
            char *packStart; 
            pack_size=pSec->SizeOfRawData;
            packStart = &pBuf[pSec->PointerToRawData];
            SmcBlockEncryption::xorPlus(packStart,pack_size,key,strlen(key));
            SmcBlockEncryption::clear(pBuf,hMap);
			return true;
        }
        pSec++;
    }
	SmcBlockEncryption::clear(pBuf,hMap);
    return false;
}

void SmcBlockEncryption::clear(char *pbuf,HANDLE &hmap)
{
	UnmapViewOfFile(pbuf);
    CloseHandle(hmap);
}

void SmcBlockEncryption::xorPlus(char *soure,int dLen,char *Key,int Klen)//明文数据指针，明文数据长度，密钥数据指针，密钥数据长度
{
    for (int i=0;i<dLen;)
    {
        for (int j=0;(j<Klen) && (i<dLen);j++,i++)
        {
            soure[i]=soure[i] ^ Key[j];
            soure[i]=~soure[i];
        }
    }
}

