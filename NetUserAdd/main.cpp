#ifndef UNICODE
#define UNICODE
#endif
#include<Windows.h>
#include<iostream>
#include<lm.h>

#pragma comment(lib,"netapi32.lib")
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


