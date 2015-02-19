#pragma once
#include "stdafx.h"

#include <iostream>
using namespace std;
#include <windows.h>
#include <string>
#include <stdio.h>
#include <io.h> //文件操作头文件
#include <fstream>//文件读写
#include <shlobj.h>//查找系统路径
#pragma comment( lib, "shell32.lib")//查找系统路径ying
#include <cstdlib>//随机数
#include <direct.h>//创建文件夹

class easy
{
public:
	static void RenameFile(const char* name,const char* newname);//重命名文件，参数1为原文件路径，参数2为文件路径+新名字，无返回值
	static bool FileExists(const char* name);//检测文件是否存在，参数1为文件路径，存在返回真，反之则假
	static void WriteFile(const char* name,const char* text);//写文本文件，参数1为文件路径，参数2为写入内容，无返回值
	static string ReadFile(const char* name);//读文本文件，参数1为文件路径，返回文件内容
	static void OpenProgram(LPCSTR name,LPSTR caner=NULL);//打开程序，参数1为程序路径，无返回值
	static void CreateFile(const char* name);//创建文件，参数1为要创建的文件路径，无返回值
	static void CreateDirectory(const char* name);//创建路径，参数1为要创建的路径路径，无返回值
	static int FolderEmpty(const char* path);//检查目录是否为空，参数1为文件夹路径（参数实例："E:\\music"），返回值不为空为1，空为0，-1为路径不存在
	static string GetDocumentsPath();//获取我的文档路径，无参数，返回我的文档路径
	static string GetProgramPath();//获取程序目录，无参数，返回程序目录
	static void WriteINI(const char* ziduan,const char* vim,const char* val,const char* name);//写配置文件，参数1为字段名，参数2为要写入的变量名，参数3为要写入的值，参数4为ini路径（不允许相对路径），无返回值
	static LPSTR ReadINI(const char* ziduan,const char* vim,const char* name);//读配置文件，参数1为字段名，参数2为要读的变量名，参数3为INI路径，返回变量内容
	static void MsgBox(const char* text,const char*title,int can=MB_OK);//信息框，参数1为内容，参数2为标题，参数3为信息参数（使用|分割），更多信息参数请参照类后MsgBox信息参数表，无返回值
	static int RandNumber(int fanwei);//产生随机数，参数1为范围（如100则为0-99范围内），返回产生的随机数
	static HWND FindWindow(const char* windowclass,const char*title=NULL);//查找窗口句柄，参数1为窗口类名，参数2为标题，返回窗口句柄
	static void ChangeWindow(HWND windows,int zhuangtai);//改变窗口状态，参数1为窗口句柄，参数2为要改变的状态，更多状态的宏请参照类后ChangeWindow状态参数列表，无返回值
	static  void OpenFile(const char* name,const char* can=NULL);//打开文件，参数1为文件名，参数2为传递参数，无返回值
	static void CreateFolder(const char* name);//创建文件夹，参数1为文件夹名，无返回值
};

/*MsgBox信息参数列表：
指定下列标志中的一个来显示消息框中的按钮，标志的含义如下。
MB_ABORTRETRYIGNORE：消息框含有三个按钮：Abort，Retry和Ignore。
MB_OK：消息框含有一个按钮：OK。这是缺省值。
MB_OKCANCEL：消息框含有两个按钮：OK和Cancel。
MB_RETRYCANCEL：消息框含有两个按钮：Retry和Cancel。
MB_YESNO：消息框含有两个按钮：Yes和No。
MB_YESNOCANCEL：消息框含有三个按钮：Yes，No和Cancel。
指定下列标志中的一个来显示消息框中的图标：标志的含义如下。
MB_ICONWARNING：一个惊叹号出现在消息框。
MB_ICONASTERISK：一个圆圈中小写字母i组成的图标出现在消息框。
MB_ICONQUESTION:一个问题标记图标出现在消息框。
MB_ICONHAND：一个停止消息图标出现在消息框。*/

/*ChangeWindow状态参数列表：
SW_HIDE：隐藏窗口并激活其他窗口。
SW_MAXIMIZE：最大化指定的窗口。
SW_MINIMIZE：最小化指定的窗口并且激活在Z序中的下一个顶层窗口。
SW_RESTORE：激活并显示窗口。如果窗口最小化或最大化，则系统将窗口恢复到原来的尺寸和位置。在恢复最小化窗口时，应用程序应该指定这个标志。
SW_SHOW：在窗口原来的位置以原来的尺寸激活和显示窗口。
SW_SHOWMAXIMIZED：激活窗口并将其最大化。
SW_SHOWMINIMIZED：激活窗口并将其最小化。
SW_SHOWMINNOACTIVE：窗口最小化，激活窗口仍然维持激活状态。
SW_SHOWNA：以窗口原来的状态显示窗口。激活窗口仍然维持激活状态。
SW_SHOWNOACTIVATE：以窗口最近一次的大小和状态显示窗口。激活窗口仍然维持激活状态。
SW_SHOWNORMAL：激活并显示一个窗口。如果窗口被最小化或最大化，系统将其恢复到原来的尺寸和大小。应用程序在第一次显示窗口的时候应该指定此标志。*/