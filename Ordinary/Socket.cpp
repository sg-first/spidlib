#include "stdafx.h"
#include "Socket.h"

static bool IsInit_=false;

int WINAPI SocketChild_tread(PVOID lpClass)
{
    SocketChild *Socket_Class;
    CallBack_Recv c_recv;
    CallBack_Close c_close;
    Socket_Class =(SocketChild*)lpClass;
    c_recv = Socket_Class->callback_recv;
    c_close =Socket_Class->callback_close;
    char buffer[MAXBUFF]={0};
	
    while (1)
    {
        int ret=0;
        ret = recv(Socket_Class->sockId,buffer,MAXBUFF,0);
        if (ret==SOCKET_ERROR)
        {
            if(c_close)
            {
                c_close();
                closesocket(Socket_Class->sockId);
                break;
            }
        }
		else
        {
            if (c_recv)
            {c_recv(buffer,ret);}
        }
    }

    return 1;
}


//SocketChild类的实现
SocketChild::SocketChild(char *ServerIp,short int ServerPort)
{
    //初始化SocketChild
    WSADATA data;
    if(!IsInit_)
    {IsInit_=!WSAStartup(MAKEWORD(1,1),&data);}
    strcpy(ip,ServerIp);
    port = ServerPort;
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.S_un.S_addr = inet_addr(ServerIp);
    IsRun =false;
    callback_close = NULL;
    callback_recv = NULL;
}

void SocketChild::SetCallBack_Close(CallBack_Close callback)
{
    assert(callback);
    callback_close  = callback;
}

void SocketChild::SetCallBack_Recv(CallBack_Recv callback)
{
    assert(callback);
    callback_recv = callback;
}

bool SocketChild::Connect()
{
    if (IsRun)
	{
        LastError = ERR_RUNING;
        return false;
    }
    sockId = socket(AF_INET,SOCK_STREAM,0);
    if (sockId==INVALID_SOCKET)
    {
        LastError = WSAGetLastError();
        return false;
    }
    if(connect(sockId,(SOCKADDR *)&address,sizeof(SOCKADDR)))
    {
        closesocket(sockId);
        LastError = WSAGetLastError();
        return false;
    }
    //启动相关线程
    hThread=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)SocketChild_tread,this,0,&thread_Id);
    if (hThread==INVALID_HANDLE_VALUE)
    {
        closesocket(sockId);
        LastError = GetLastError();
        return false;
    }
    CloseHandle(hThread);
    IsRun = true;
    return true;
}

bool SocketChild::Send(char *buffer,int len)
{
    if(!buffer||!len)
    {return false;}
    if(send(sockId,buffer,len,0)==SOCKET_ERROR)
    {return false;}
    else
    {return true;}
}

void SocketChild::Close()
{
    TerminateThread(hThread,0);
    CloseHandle(hThread);
    closesocket(sockId);
}

SocketChild::~SocketChild()
{Close();}


//SocketServe类的实现
int SocketServe::_FindNode()
{
    for (int i=0;i<MaxChildren;i++)
    {
        if (Childlist[i].hThread==0)
        {return i;}
    }
    return -1;
}

int WINAPI Thread_child(ChildLink *pLink)
{
    ChildLink link;
    char buffer[MAXBUFF]={0};
    int ret;
    SocketServe *SocketClass;
    CallBack_ChildExit childexit;
    Callback_ServeRecv Srecv;
    Sleep(100);
    memcpy(&link,pLink,sizeof(ChildLink));
    SocketClass = link.pSocketclass;
    childexit=SocketClass->callback_childExit;
    Srecv=SocketClass->callback_serverecv;
	
    while (true)
    {
        ret = recv(link.SocketId,buffer,MAXBUFF,0);
        if (ret==0||ret==SOCKET_ERROR)
        {
            if (childexit)
            {
                childexit(link.Id);
                CloseHandle(SocketClass->Childlist[link.Id].hThread);
                SocketClass->Childlist[link.Id].hThread = 0;
                return 0;
            }

        }
		else 
		{
		  if (Srecv)
          {Srecv(link.Id,buffer,ret);}
		}
    }
    return 0;
}

int WINAPI Thread_WaitForChild(LPVOID StartInfo)
{
    SocketServe *Socket_Class;
    Socket_Class = (SocketServe *)StartInfo;
    int len;
    SOCKADDR_IN ChildAddr={0};
    SOCKET ChildSock;
    CallBack_ChildIn childin;
    CallBack_ChildExit childexit;
    DWORD Id;
    ChildLink link={0};
    childin = Socket_Class->callback_childIn;
    childexit=Socket_Class->callback_childExit;

    while (1)
    {

        len = sizeof(SOCKADDR);
        ChildSock=accept(Socket_Class->sockId,(SOCKADDR *)&ChildAddr,&len);
        if (ChildSock==SOCKET_ERROR)
        {continue;}

        link.Id =Socket_Class->_FindNode();
        if (link.Id==-1)  //队列不够用的情况
        {
            assert(NULL);
            closesocket(ChildSock); //拒绝连接
            continue;
        }
        link.address=ChildAddr;
        link.SocketId = ChildSock;
        link.pSocketclass = Socket_Class;
        //创建线程
        memcpy(&Socket_Class->Childlist[link.Id],&link,sizeof(ChildLink));
        Socket_Class->Childlist[link.Id].hThread=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Thread_child,&link,0,&Id);
        if (childin)
        {hildin(link.Id);}

        Sleep(200); // 休息等待线程拷贝相关数据
    }
	
    return 0;
}

SocketServe::SocketServe(short int port)
{
    WSADATA data;
    if(!IsInit_)
    {IsInit_=!WSAStartup(MAKEWORD(1,1),&data);}
    listen_port = port;
    address.sin_family=AF_INET;
    address.sin_port=htons(port);
    address.sin_addr.S_un.S_addr=htonl(INADDR_ANY);
    callback_childExit = NULL;
    callback_childIn=NULL;
    callback_serverecv=NULL;
}

void SocketServe::SetCallBack_ChildIn(CallBack_ChildIn callback)
{
    assert(callback);
    callback_childIn = callback;
}

void SocketServe::SetCallBack_Recv(Callback_ServeRecv callback)
{
    assert(callback);
    callback_serverecv = callback;
}

void SocketServe::SetCallBack_ChildExit(CallBack_ChildExit callback)
{
    assert(callback);
    callback_childExit = callback;
}

bool SocketServe::Start(int MaxChild)
{
    DWORD id;
    sockId = socket(AF_INET,SOCK_STREAM,0);
    if(INVALID_SOCKET==sockId)
    {
        assert(NULL);
        return false;
    }
    if(bind(sockId,(SOCKADDR*)&address,sizeof(SOCKADDR)))
    {
        closesocket(sockId);
        assert(NULL);
        return false;
    }
    if(listen(sockId,MaxChild))
    {
        closesocket(sockId);
        assert(NULL);
        return false;
    }
    Childlist = new ChildLink[MaxChild];
    memset(Childlist,0,sizeof(ChildLink)*MaxChild);
    MaxChildren=MaxChild;
    thread_listen=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Thread_WaitForChild,this,0,&id);
    if (thread_listen==INVALID_HANDLE_VALUE)
    { return false;}

    return true;
}

void SocketServe::CloseChild(int ChildId)
{
    assert(ChildId>=0 && ChildId<=MaxChildren);
    TerminateThread(Childlist[ChildId].hThread,0);
    CloseHandle(Childlist[ChildId].hThread);
    Childlist[ChildId].hThread = 0;
    closesocket(Childlist[ChildId].SocketId);
    return;
}

bool SocketServe::Send(int ChildId,char *buffer,int len)
{
    if(send(Childlist[ChildId].SocketId,buffer,len,0)==SOCKET_ERROR)
    {return false;}
	else
    {return true;}
}

void SocketServe::CloseServe()
{
    for (int i=0;i<MaxChildren;i++)
    {
        if (Childlist[i].hThread!=0)
        {
            TerminateThread(Childlist[i].hThread,0);
            CloseHandle(Childlist[i].hThread);
            closesocket(Childlist[i].SocketId);
        }
    }
    TerminateThread(thread_listen,0);
    Sleep(200);
    delete Childlist;
    closesocket(sockId);
}

bool SocketServe::GetChildInfoById(ChildLink *buffer,int ChildId)
{
    assert(ChildId>=0 && ChildId<=MaxChildren);
    if (buffer && ChildId<=MaxChildren)
    {
        if (Childlist[ChildId].hThread)
        {
            memcpy(buffer,&Childlist[ChildId],sizeof(ChildLink));
            return true;
        }
    }
    return false;
}

char *SocketServe::GetChildIpById(int ChildId)
{
    ChildLink link;
    assert(ChildId>=0 && ChildId<=MaxChildren);
    if(GetChildInfoById(&link,ChildId))
    {return inet_ntoa(link.address.sin_addr);}
    return NULL;
}

SocketServe::~SocketServe()
{CloseServe();}