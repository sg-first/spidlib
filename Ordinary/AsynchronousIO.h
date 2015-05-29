#pragma once
#include "stdafx.h"

#include <WinSock2.h>
#pragma comment(lib,"ws2_32.lib")

#define  BindPort 1447

class AsynchronousIO
{
public:
	AsynchronousIO(int port,int resttime);

private:
	void InitSocketLib();
	void EventLoop();
	virtual void failed();//创建套接字失败或设置异步失败时的回调
	virtual void AcceptError();//事件循环出错时的回调
	virtual void ConnectDisconnect(char* IP);//连接断开时的回调
	virtual void Recvice(char* IP,uint16_t host,int Result,char recvBuffer[1024]);//接收消息时的回调
	int resttime;
};