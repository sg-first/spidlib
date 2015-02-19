#include "stdafx.h"
#include "AntiDebugging.h"

LONG WINAPI AntiDebugging::debug_kill(_In_  struct _EXCEPTION_POINTERS *ExceptionInfo)
{
        abnormal();
        SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)
        ExceptionInfo->ContextRecord->Eax);
        //modify the EIP register value
        ExceptionInfo->ContextRecord->Eip += 2;
        isDebug=false;//Confirmation is debugging
        return EXCEPTION_CONTINUE_EXECUTION;//pop up window does not collapse, continue program execution
}

AntiDebugging::AntiDebugging()
{
        isDebug=true;
        SetUnhandledExceptionFilter(debug_kill);//Hanging hook,debug_kill will be called when exception arises
        _asm//to be honest,what meaning
        {
                xor eax,eax
                div  eax
        }
}

void AntiDebugging::check()
{
        if(isDebug)
		{debuging();}
}

