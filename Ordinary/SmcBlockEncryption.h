#include "head.h"

class SmcBlockEncryption
{
public:
	static void selfEncryption(char *key);
	static void encryption(char *pBuf,char *key);
	static bool ecrypt(HANDLE hFile,char *key);

private:
	static void clear(char *pbuf,HANDLE &hmap);
	static void xorPlus(char *soure,int dLen,char *Key,int Klen);
};

//1.要加密的区段写成这样
/*
#pragma code_seg(".SMC")//以下代码置入SMC代码段
void Fun1()
{
    //函数体
}
#pragma code_seg()
#pragma comment(linker, "/SECTION:.SMC,ERW")//置可读可执行
*/
//2.提前在程序中安放解密该区段的代码（可以使用selfEncryption）
//3.编译后调用SmcBlockEncryption::ecrypt对编译后程序进行加密
//4.调用时防止解密失败注意异常
/*
    __try
    {
        Fun1();
    }
    __except(1)
    {
        UnPack(KeyBuffer); //修正数据
        AfxMessageBox(_T("Key不正确，请重新输入。"));
    }
*/