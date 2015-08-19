#include "stdafx.h"
#include "Easy.h"

const char* lianjie(const char* str1, const char* str2)
{
    std::string src1(str1);
    std::string src2(str1);
    std::string dest = src1 + src2;
    return dest.c_str();
}

void easy::RenameFile(const char* name,const char* newname)
{
	rename(name,newname);
}

bool easy::FileExists(const char* name)
{
	if(_access(name,0)==-1)
	{return false;}
	else
	{return true;}
}

void easy::WriteFile(const char* name,const char* text)
{
	fstream file(name);
    file<<text;	
    file.close();
}

string easy::ReadFile(const char* name)
{
	string op;
    fstream file(name);
    file>>op;	
    file.close();
	return op;
}

void easy::OpenProgram(LPCSTR name,LPSTR caner)
{
	STARTUPINFO si; 
    memset(&si, 0, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_SHOW;
    PROCESS_INFORMATION pi;
    CreateProcess(name,caner,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi);
}

void easy::CreateFile(const char* name)
{
	 fstream newbat(name);
     newbat.open(name,ios::out|ios::app);
}

void easy::CreateDirectory(const char* name)
{
	::CreateDirectory(name,NULL);
}

int easy::FolderEmpty(const char* path)
{
	const char* canyi=lianjie(path,"\\*.*");
	HANDLE hFind;
    WIN32_FIND_DATA FindFileData;
    if((hFind=FindFirstFileA(path,&FindFileData)) != INVALID_HANDLE_VALUE)
    {
        BOOL bFind = TRUE;    
        BOOL EmptyDirectory = TRUE;
        while(bFind)
        {
            if(strcmp(FindFileData.cFileName,".") == 0
                || strcmp(FindFileData.cFileName,"..") == 0)
            {
                bFind = FindNextFile(hFind,&FindFileData);
            }
            else
            {
                //有子文件夹也算非空
                EmptyDirectory = FALSE;
                break;
            }
        }
        if(EmptyDirectory)
        {
            return 0;
        }
        else
        {
            return 1;
        }
    }
    else
    {
        return -1;
    }
    return 0;
}

string easy::GetDocumentsPath()
{
	char m_lpszDefaultDir[MAX_PATH];
    char   szDocument[MAX_PATH]={0};   
    memset(m_lpszDefaultDir,0,_MAX_PATH); 
    LPITEMIDLIST pidl=NULL;   
    SHGetSpecialFolderLocation(NULL, CSIDL_PERSONAL,&pidl);//第二个参数为代表我的文档的宏，换做其它宏也可以查找其它系统文件夹
    if   (pidl   &&   SHGetPathFromIDList(pidl,   szDocument))   
    {   
        GetShortPathName(szDocument,m_lpszDefaultDir,_MAX_PATH);   
    }
	string op=m_lpszDefaultDir;
	return op;
}

string easy::GetProgramPath()
{
TCHAR _szPath[MAX_PATH + 1]={0};
GetModuleFileName(NULL, _szPath, MAX_PATH);
(_tcsrchr(_szPath, _T('\\')))[1] = 0;
string strPath;
for (int n=0;_szPath[n];n++)
{
if (_szPath[n]!=_T('\\'))
{
   strPath +=_szPath[n] ;
}
else
{
   strPath += _T("\\");
}
}
return strPath;
}

void easy::WriteINI(const char* ziduan,const char* vim,const char* val,const char* name)
{
	WritePrivateProfileString(ziduan,vim,val,name);
}

LPSTR easy::ReadINI(const char* ziduan,const char* vim,const char* name)
{
char op[1024];
GetPrivateProfileString(ziduan,vim,"0",op,1024,name); 
return op;
}

 void easy::MsgBox(const char* text,const char*title,int can)
 {
	 MessageBox(NULL,text,title,can);
 }

 int easy::RandNumber(int fanwei)
 {
	 return rand()%fanwei;
 }

 HWND easy::FindWindow(const char* windowclass,const char*title)
 {
	 return ::FindWindow(windowclass,title);
 }

 void easy::ChangeWindow(HWND windows,int zhuangtai)
 {
	 ShowWindow(windows,zhuangtai);
 }

 void easy::OpenFile(const char* name,const char* can)
 {
	 ShellExecute(NULL,"open",name,can,NULL,SW_SHOWNORMAL);
 }

 void easy::CreateFolder(const char* name)
 {
	 _mkdir(name);
 }

char** easy::EnumFiles(const char *directory, int *count)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	char result[MAX_RESULT][MAX_PATH];
	char **returnresult;
	char pattern[MAX_PATH];
	int i = 0, j;
	// 开始查找
	strcpy(pattern, directory);
	strcat(pattern, "\\*.*");
	hFind = FindFirstFile(pattern, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		*count = 0;
		return NULL;
	}
	else
	{
		do
		{strcpy(result[i++], FindFileData.cFileName);}
		while(FindNextFile(hFind, &FindFileData) != 0);
	}
	// 查找结束
	FindClose(hFind);
	// 复制到结果中
	returnresult = (char **)calloc(i, sizeof(char *));
	for(j = 0; j < i; j++)
	{
		returnresult[j] = (char *)calloc(MAX_PATH, sizeof(char));
		strcpy(returnresult[j], result[j]);
	}
	*count = i;
	return returnresult;
}
/*调用的代码：
	int count;
	char **result = EnumFiles(要查找的目录, &count);
	for (int i=0;i<count;i++)
	{
		//result[i]为每次循环到的文件名
	}
*/
