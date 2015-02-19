#program once
#include "stdafx.h"

typedef enum _SYSTEM_INFORMATION_CLASS   
{   
 SystemBasicInformation,                 //  0 Y N   
 SystemProcessorInformation,             //  1 Y N   
 SystemPerformanceInformation,           //  2 Y N   
 SystemTimeOfDayInformation,             //  3 Y N   
 SystemNotImplemented1,                  //  4 Y N   
 SystemProcessesAndThreadsInformation,   //  5 Y N   
 SystemCallCounts,                       //  6 Y N   
 SystemConfigurationInformation,         //  7 Y N   
 SystemProcessorTimes,                   //  8 Y N   
 SystemGlobalFlag,                       //  9 Y Y   
 SystemNotImplemented2,                  // 10 Y N   
 SystemModuleInformation,                // 11 Y N   
 SystemLockInformation,                  // 12 Y N   
 SystemNotImplemented3,                  // 13 Y N   
 SystemNotImplemented4,                  // 14 Y N   
 SystemNotImplemented5,                  // 15 Y N   
 SystemHandleInformation,                // 16 Y N   
 SystemObjectInformation,                // 17 Y N   
 SystemPagefileInformation,              // 18 Y N  
 SystemInstructionEmulationCounts,       // 19 Y N   
 SystemInvalidInfoClass1,                // 20   
 SystemCacheInformation,                 // 21 Y Y   
 SystemPoolTagInformation,               // 22 Y N   
 SystemProcessorStatistics,              // 23 Y N   
 SystemDpcInformation,                   // 24 Y Y   
 SystemNotImplemented6,                  // 25 Y N   
 SystemLoadImage,                        // 26 N Y   
 SystemUnloadImage,                      // 27 N Y   
 SystemTimeAdjustment,                   // 28 Y Y   
 SystemNotImplemented7,                  // 29 Y N   
 SystemNotImplemented8,                  // 30 Y N   
 SystemNotImplemented9,                  // 31 Y N   
 SystemCrashDumpInformation,             // 32 Y N   
 SystemExceptionInformation,             // 33 Y N   
 SystemCrashDumpStateInformation,        // 34 Y Y/N   
 SystemKernelDebuggerInformation,        // 35 Y N   
 SystemContextSwitchInformation,         // 36 Y N   
 SystemRegistryQuotaInformation,         // 37 Y Y   
 SystemLoadAndCallImage,                 // 38 N Y   
 SystemPrioritySeparation,               // 39 N Y   
 SystemNotImplemented10,                 // 40 Y N   
 SystemNotImplemented11,                 // 41 Y N   
 SystemInvalidInfoClass2,                // 42   
 SystemInvalidInfoClass3,                // 43   
 SystemTimeZoneInformation,              // 44 Y N   
 SystemLookasideInformation,             // 45 Y N   
 SystemSetTimeSlipEvent,                 // 46 N Y   
 SystemCreateSession,                    // 47 N Y   
 SystemDeleteSession,                    // 48 N Y   
 SystemInvalidInfoClass4,                // 49   
 SystemRangeStartInformation,            // 50 Y N   
 SystemVerifierInformation,              // 51 Y Y   
 SystemAddVerifier,                      // 52 N Y   
 SystemSessionProcessesInformation       // 53 Y N   
} SYSTEM_INFORMATION_CLASS;  

typedef struct _SYSTEM_MODULE_INFORMATION_ENTRY 
{
 HANDLE Section;
 PVOID MappedBase;
 PVOID Base;
 ULONG Size;
 ULONG Flags;
 USHORT LoadOrderIndex;
 USHORT InitOrderIndex;
 USHORT LoadCount;
 USHORT PathLength;
 CHAR ImageName[256];
} SYSTEM_MODULE_INFORMATION_ENTRY, *PSYSTEM_MODULE_INFORMATION_ENTRY;

typedef struct _SYSTEM_MODULE_INFORMATION 
{
 ULONG Count;
 SYSTEM_MODULE_INFORMATION_ENTRY Module[1];
} SYSTEM_MODULE_INFORMATION, *PSYSTEM_MODULE_INFORMATION;

typedef struct _HOOK_INFO
{
 ULONG old_address; //  被HOOK函数地址 
 ULONG new_address ; //处理函数地址
 ULONG Type;
 ULONG index;  //SSDT 表索引号
 ULONG SSDT_TABLE;
 UCHAR data [5];
}HOOK_INFO,*PHOOK_INFO;

typedef struct _Protect_list
{
 struct _Protect_list *Before;
 PHOOK_INFO info;
 struct _Protect_list *Next;
}Protect_list,*PProtect_list;