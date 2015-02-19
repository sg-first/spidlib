#pragma once
#include "stdafx.h"

#include <iostream>
using namespace std;
#include <windows.h>
#include <string>
#include <stdio.h>
#include <io.h> //�ļ�����ͷ�ļ�
#include <fstream>//�ļ���д
#include <shlobj.h>//����ϵͳ·��
#pragma comment( lib, "shell32.lib")//����ϵͳ·��ying
#include <cstdlib>//�����
#include <direct.h>//�����ļ���

class easy
{
public:
	static void RenameFile(const char* name,const char* newname);//�������ļ�������1Ϊԭ�ļ�·��������2Ϊ�ļ�·��+�����֣��޷���ֵ
	static bool FileExists(const char* name);//����ļ��Ƿ���ڣ�����1Ϊ�ļ�·�������ڷ����棬��֮���
	static void WriteFile(const char* name,const char* text);//д�ı��ļ�������1Ϊ�ļ�·��������2Ϊд�����ݣ��޷���ֵ
	static string ReadFile(const char* name);//���ı��ļ�������1Ϊ�ļ�·���������ļ�����
	static void OpenProgram(LPCSTR name,LPSTR caner=NULL);//�򿪳��򣬲���1Ϊ����·�����޷���ֵ
	static void CreateFile(const char* name);//�����ļ�������1ΪҪ�������ļ�·�����޷���ֵ
	static void CreateDirectory(const char* name);//����·��������1ΪҪ������·��·�����޷���ֵ
	static int FolderEmpty(const char* path);//���Ŀ¼�Ƿ�Ϊ�գ�����1Ϊ�ļ���·��������ʵ����"E:\\music"��������ֵ��Ϊ��Ϊ1����Ϊ0��-1Ϊ·��������
	static string GetDocumentsPath();//��ȡ�ҵ��ĵ�·�����޲����������ҵ��ĵ�·��
	static string GetProgramPath();//��ȡ����Ŀ¼���޲��������س���Ŀ¼
	static void WriteINI(const char* ziduan,const char* vim,const char* val,const char* name);//д�����ļ�������1Ϊ�ֶ���������2ΪҪд��ı�����������3ΪҪд���ֵ������4Ϊini·�������������·�������޷���ֵ
	static LPSTR ReadINI(const char* ziduan,const char* vim,const char* name);//�������ļ�������1Ϊ�ֶ���������2ΪҪ���ı�����������3ΪINI·�������ر�������
	static void MsgBox(const char* text,const char*title,int can=MB_OK);//��Ϣ�򣬲���1Ϊ���ݣ�����2Ϊ���⣬����3Ϊ��Ϣ������ʹ��|�ָ��������Ϣ������������MsgBox��Ϣ�������޷���ֵ
	static int RandNumber(int fanwei);//���������������1Ϊ��Χ����100��Ϊ0-99��Χ�ڣ������ز����������
	static HWND FindWindow(const char* windowclass,const char*title=NULL);//���Ҵ��ھ��������1Ϊ��������������2Ϊ���⣬���ش��ھ��
	static void ChangeWindow(HWND windows,int zhuangtai);//�ı䴰��״̬������1Ϊ���ھ��������2ΪҪ�ı��״̬������״̬�ĺ���������ChangeWindow״̬�����б��޷���ֵ
	static  void OpenFile(const char* name,const char* can=NULL);//���ļ�������1Ϊ�ļ���������2Ϊ���ݲ������޷���ֵ
	static void CreateFolder(const char* name);//�����ļ��У�����1Ϊ�ļ��������޷���ֵ
};

/*MsgBox��Ϣ�����б�
ָ�����б�־�е�һ������ʾ��Ϣ���еİ�ť����־�ĺ������¡�
MB_ABORTRETRYIGNORE����Ϣ����������ť��Abort��Retry��Ignore��
MB_OK����Ϣ����һ����ť��OK������ȱʡֵ��
MB_OKCANCEL����Ϣ����������ť��OK��Cancel��
MB_RETRYCANCEL����Ϣ����������ť��Retry��Cancel��
MB_YESNO����Ϣ����������ť��Yes��No��
MB_YESNOCANCEL����Ϣ����������ť��Yes��No��Cancel��
ָ�����б�־�е�һ������ʾ��Ϣ���е�ͼ�꣺��־�ĺ������¡�
MB_ICONWARNING��һ����̾�ų�������Ϣ��
MB_ICONASTERISK��һ��ԲȦ��Сд��ĸi��ɵ�ͼ���������Ϣ��
MB_ICONQUESTION:һ��������ͼ���������Ϣ��
MB_ICONHAND��һ��ֹͣ��Ϣͼ���������Ϣ��*/

/*ChangeWindow״̬�����б�
SW_HIDE�����ش��ڲ������������ڡ�
SW_MAXIMIZE�����ָ���Ĵ��ڡ�
SW_MINIMIZE����С��ָ���Ĵ��ڲ��Ҽ�����Z���е���һ�����㴰�ڡ�
SW_RESTORE�������ʾ���ڡ����������С������󻯣���ϵͳ�����ڻָ���ԭ���ĳߴ��λ�á��ڻָ���С������ʱ��Ӧ�ó���Ӧ��ָ�������־��
SW_SHOW���ڴ���ԭ����λ����ԭ���ĳߴ缤�����ʾ���ڡ�
SW_SHOWMAXIMIZED������ڲ�������󻯡�
SW_SHOWMINIMIZED������ڲ�������С����
SW_SHOWMINNOACTIVE��������С�����������Ȼά�ּ���״̬��
SW_SHOWNA���Դ���ԭ����״̬��ʾ���ڡ��������Ȼά�ּ���״̬��
SW_SHOWNOACTIVATE���Դ������һ�εĴ�С��״̬��ʾ���ڡ��������Ȼά�ּ���״̬��
SW_SHOWNORMAL�������ʾһ�����ڡ�������ڱ���С������󻯣�ϵͳ����ָ���ԭ���ĳߴ�ʹ�С��Ӧ�ó����ڵ�һ����ʾ���ڵ�ʱ��Ӧ��ָ���˱�־��*/