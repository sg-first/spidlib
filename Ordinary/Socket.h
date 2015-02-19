#pragma once
#include "stdafx.h"

#pragma comment(lib,"ws2_32.lib")
#include <windows.h>
#include <assert.h>
#define IpSize  15
#define  ERR_RUNING 0x1000 /*套接字已经建立连接*/
#define MAXBUFF 1024     /*默认缓冲区大小*/

class SocketServe;

typedef struct _ChildLink
{
    int Id; /*客户端序号*/
    SOCKADDR_IN address; /*客户端地址*/
    SOCKET SocketId;    /*客户端套接字*/
    SocketServe *pSocketclass;
    HANDLE hThread;
}ChildLink,*lpChildLink;

//客户端程序
typedef void (* CallBack_Close)();//这个是当连接断开时的回调
typedef void (* CallBack_Recv)(char *data,int len);//这个是当收到消息时的回调，data是一个指针，指向数据缓冲区，len是数据的长度
//服务端程序
typedef void(* CallBack_ChildIn)(int ChildId);//当有客户进入时的事件
typedef void(* CallBack_ChildExit)(int ChildId);//当失去客户连接的时候调用
typedef void(* Callback_ServeRecv)(int ChildId,char *data,int len);//当收到客户消息时的事件

class SocketChild
{
public:
    SocketChild (char *ip,short int port);
    ~SocketChild();
    void SetCallBack_Close(CallBack_Close callback); //设置当连接断开时的回调，参数1为回调的函数指针，无返回值
    void SetCallBack_Recv(CallBack_Recv callback); //设置接收到消息时的回调，参数1为回调的函数指针，无返回值
    bool Connect(); //建立连接
    void Close();  //关闭连接
    bool Send(char *buffer,int len); //发送数据
    SOCKET sockId;
    CallBack_Close callback_close;
    CallBack_Recv callback_recv;

private:
    char ip[IpSize];
    short int port;
    bool IsRun; // 是否与服务器保持连接
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
    void SetCallBack_ChildIn(CallBack_ChildIn callback); //设置有客户端进入的回调
    void SetCallBack_ChildExit(CallBack_ChildExit callback);//设置客户退出的回调
    void SetCallBack_Recv(Callback_ServeRecv callback); //设置接收到消息的回调
    bool Start(int MaxChild);
    bool Send(int ChildId,char *buffer,int len);//发送消息
    bool GetChildInfoById(ChildLink *buffer,int ChildId);
    char *GetChildIpById(int ChildId);
    /*下面函数禁止在回调函数中调用!*/
    void CloseChild(int ChildId); /*关闭某客户端*/
    void CloseServe(); /*关闭服务器*/
	
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