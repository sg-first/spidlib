#include "stdafx.h"
#include "AsynchronousIO.h"
 
void AsynchronousIO::nitSocketLib()
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2),&wsaData);
}

AsynchronousIO::AsynchronousIO(int port,int resttime)
{
	this->resttime=resttimel;
	SOCKET sock;
    SOCKADDR_IN sockAddr={0};
    InitSocketLib();
    sock=socket(AF_INET,SOCK_STREAM,0);
    if(sock==INVALID_SOCKET)
    {failed();}
    sockAddr.sin_family=AF_INET;
    sockAddr.sin_addr.S_un.S_addr=0;   
    sockAddr.sin_port=htons(BindPort);
    //把套接字bind到相应的地址描述符上
    bind(sock,(SOCKADDR *)&sockAddr,sizeof(SOCKADDR_IN));
    //重点设置套接字为非阻塞模式
    int iMod = 1;
    if(SOCKET_ERROR==ioctlsocket(sock,FIONBIO,(u_long *)&iMod))
    {failed();}
    listen(sock, port);
	EventLoop();
}

void AsynchronousIO::EventLoop()
{
    while(true)
    {
        SOCKET newSocket;
        SOCKADDR_IN newAddr={0};
        int len;
        len=sizeof(SOCKADDR_IN);
        newSocket = accept(sock,(SOCKADDR *)&newAddr,&len);
        if (newSocket==INVALID_SOCKET)
        {
            //并不是真正的出错了
            int err;
            err=WSAGetLastError();
            if (err==WSAEWOULDBLOCK)
            {
                //无法立即完成任务
                Sleep(200);
                continue;
            }
			else
            {
                //真正的出错
                AcceptError();
            }
        }
        //进入循环接收消息的状态
        while (true)
        {
            int iResult;
            char recvBuffer[1024];
            memset(recvBuffer,0,1024);
            iResult=recv(newSocket,recvBuffer,1024,0);
            if (iResult>0)
            {
				//收到消息
				Recvice(inet_ntoa(newAddr.sin_addr),ntohs(newAddr.sin_port),iResult,recvBuffer);
                closesocket(newSocket);
                break;
            }
			else 
			{
				if (iResult==0)
				{
					//连接被关闭了
					shutdown_:
					ConnectDisconnect(inet_ntoa(newAddr.sin_addr));
					break;
				}
				else
				{
					int err;
					err=WSAGetLastError();
					if (err==WSAEWOULDBLOCK)
					{
						Sleep(resttime); //休息一下继续轮询
						continue;
					}
					else 
					{goto shutdown_;}
				}
			}
        }
    }
}