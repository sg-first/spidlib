#pragma once
#include "stdafx.h"

class ProcessInvasion
{
public:
	static bool IAT_HOOK(PVOID imageBase,char *HookApiName,PVOID newAddr);//imageBase：要IAT HOOK的模块基址，HookApiName：被HOOK函数的名称，newAddr：新的函数地址
};