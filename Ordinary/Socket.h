#pragma once
#include "stdafx.h"

#pragma comment(lib,"ws2_32.lib")
#include <windows.h>
#include <assert.h>
#define IpSize  15
#define  ERR_RUNING 0x1000 /*�׽����Ѿ���������*/
#define MAXBUFF 1024     /*Ĭ�ϻ�������С*/

class SocketServe;

typedef struct _ChildLink
{
    int Id; /*�ͻ������*/
    SOCKADDR_IN address; /*�ͻ��˵�ַ*/
    SOCKET SocketId;    /*�ͻ����׽���*/
    SocketServe *pSocketclass;
    HANDLE hThread;
}ChildLink,*lpChildLink;

//�ͻ��˳���
typedef void (* CallBack_Close)();//����ǵ����ӶϿ�ʱ�Ļص�
typedef void (* CallBack_Recv)(char *data,int len);//����ǵ��յ���Ϣʱ�Ļص���data��һ��ָ�룬ָ�����ݻ�������len�����ݵĳ���
//����˳���
typedef void(* CallBack_ChildIn)(int ChildId);//���пͻ�����ʱ���¼�
typedef void(* CallBack_ChildExit)(int ChildId);//��ʧȥ�ͻ����ӵ�ʱ�����
typedef void(* Callback_ServeRecv)(int ChildId,char *data,int len);//���յ��ͻ���Ϣʱ���¼�

class SocketChild
{
public:
    SocketChild (char *ip,short int port);
    ~SocketChild();
    void SetCallBack_Close(CallBack_Close callback); //���õ����ӶϿ�ʱ�Ļص�������1Ϊ�ص��ĺ���ָ�룬�޷���ֵ
    void SetCallBack_Recv(CallBack_Recv callback); //���ý��յ���Ϣʱ�Ļص�������1Ϊ�ص��ĺ���ָ�룬�޷���ֵ
    bool Connect(); //��������
    void Close();  //�ر�����
    bool Send(char *buffer,int len); //��������
    SOCKET sockId;
    CallBack_Close callback_close;
    CallBack_Recv callback_recv;

private:
    char ip[IpSize];
    short int port;
    bool IsRun; // �Ƿ����������������
    SOCKADDR_IN address;
    int LastError;
    DWORD thread_Id;
    HANDLE hThread;
};


class SocketServe
{
public:
    SocketServe(short int Port);
    ~SocketServe();
    void SetCallBack_ChildIn(CallBack_ChildIn callback); //�����пͻ��˽���Ļص�
    void SetCallBack_ChildExit(CallBack_ChildExit callback);//���ÿͻ��˳��Ļص�
    void SetCallBack_Recv(Callback_ServeRecv callback); //���ý��յ���Ϣ�Ļص�
    bool Start(int MaxChild);
    bool Send(int ChildId,char *buffer,int len);//������Ϣ
    bool GetChildInfoById(ChildLink *buffer,int ChildId);
    char *GetChildIpById(int ChildId);
    /*���溯����ֹ�ڻص������е���!*/
    void CloseChild(int ChildId); /*�ر�ĳ�ͻ���*/
    void CloseServe(); /*�رշ�����*/
	
    int _FindNode();
    SOCKET sockId;
    CallBack_ChildIn callback_childIn;
    CallBack_ChildExit callback_childExit;
    Callback_ServeRecv callback_serverecv;
    ChildLink *Childlist;

private:
    short int listen_port;
    int MaxChildren;
    SOCKADDR_IN address;
    HANDLE thread_listen;
};