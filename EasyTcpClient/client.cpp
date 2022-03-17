#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <WinSock2.h>
#include <stdio.h>

#pragma comment(lib, "ws2_32.lib")


enum CMD {
	CMD_LOGIN,
	CMD_LOGINOUT,
	CMD_ERROR
};

struct DataHeader {
	short dataLength;
	short cmd;
};

struct LoginResult {
	int result;
};

struct LoginOutResult {
	int result;
};

struct LoginOut {
	char userName[32];
};

struct Login {
	char userName[32];
	char passWord[32];
};

int main() {
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);

	// 1.建立socket 套接字
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == _sock) {
		printf("ERROR: 建立客户端SOCKET失败...\n");
	}
	else {
		printf("建立客户端SOCKET成功...\n");
	}
	// connect
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(9999);
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
	if (SOCKET_ERROR == ret) {
		printf("ERROR: 连接失败...\n");
	}
	else {
		printf("连接成功...\n");
	}

	while (true) {
		char cmdBuf[128] = {};
		scanf("%s", &cmdBuf);
		if (0 == strcmp(cmdBuf, "exit")) {
			break;
		}
		else if (0 == strcmp(cmdBuf, "login")) {
			Login login = { "zzr", "999" };
			DataHeader dh = { sizeof(login), CMD_LOGIN};
			send(_sock, (const char*)&dh, sizeof(dh), 0);
			send(_sock, (const char*)&login, sizeof(login), 0);

			DataHeader retHeader = {};
			LoginResult loginRet = {};
			recv(_sock, (char*)&retHeader, sizeof(retHeader), 0);
			recv(_sock, (char*)&loginRet, sizeof(loginRet), 0);
			printf("LoginResult: %d\n", loginRet.result);
		}
		else if (0 == strcmp(cmdBuf, "logout")) {
			LoginOut logout = { "zzr" };
			DataHeader dh = { sizeof(logout), CMD_LOGIN};
			send(_sock, (const char*)&dh, sizeof(dh), 0);
			send(_sock, (const char*)&logout, sizeof(logout), 0);

			DataHeader retHeader = {};
			LoginOutResult logoutRet = {};
			recv(_sock, (char*)&retHeader, sizeof(retHeader), 0);
			recv(_sock, (char*)&logoutRet, sizeof(logoutRet), 0);
			printf("LoginOutResult: %d\n", logoutRet.result);
		}
		else {
			printf("不支持的命令，请重新输入.\n");
		}
	}
	
	// close
	closesocket(_sock);

	printf("客户端已退出");
	getchar();
	// 清除win socket 环境
	WSACleanup();
	return 0;
}