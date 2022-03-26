#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <WinSock2.h>
#include <stdio.h>
#include <thread>

#pragma comment(lib, "ws2_32.lib")


enum CMD {
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_ERROR,
	CMD_NEW_USER_JOIN
};

struct DataHeader {
	short dataLength;
	short cmd;
};

struct LogOut : public DataHeader {
	LogOut() {
		dataLength = sizeof(LogOut);
		cmd = CMD_LOGOUT;
	}
	char userName[32];
};

struct LogOutResult : public DataHeader {
	LogOutResult() {
		dataLength = sizeof(LogOutResult);
		cmd = CMD_LOGOUT_RESULT;
		result = 0;
	}
	int result;
};

struct Login : public DataHeader {
	Login() {
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char userName[32];
	char passWord[32];
};

struct LoginResult : public DataHeader {
	LoginResult() {
		dataLength = sizeof(LoginResult);
		cmd = CMD_LOGIN_RESULT;
		result = 0;
	}
	int result;
};

struct NewUserJoin : public DataHeader {
	NewUserJoin() {
		dataLength = sizeof(NewUserJoin);
		cmd = CMD_NEW_USER_JOIN;
		sock = 0;
	}
	int sock;
};


int processor(SOCKET _Sock) {
	char szRecv[1024] = {};
	int nLen = recv(_Sock, szRecv, sizeof(DataHeader), 0);
	DataHeader* header = (DataHeader*)szRecv;

	if (nLen <= 0) {
		printf("与服务器断开连接，任务结束.\n");
		return -1;
	}

	switch (header->cmd) {
	case CMD_LOGIN_RESULT: {
		recv(_Sock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		LoginResult* login = (LoginResult*)szRecv;
		printf("收到服务端消息: CMD_LOGIN_RESULT, 数据长度：%d\n", login->dataLength);
		break;
	}
	case CMD_LOGOUT_RESULT: {
		recv(_Sock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		LogOutResult* logout = (LogOutResult*)szRecv;
		printf("收到服务端消息: CMD_LOGOUT_RESULT, 数据长度：%d\n", logout->dataLength);
		break;
	}
	case CMD_NEW_USER_JOIN: {
		recv(_Sock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		NewUserJoin* userJoin = (NewUserJoin*)szRecv;
		printf("收到服务端消息: CMD_LOGOUT_RESULT, 数据长度：%d\n", userJoin->dataLength);
		break;
	}
	}

	return 1;
}

bool g_bRun = true;

void cmdThread(SOCKET sock) {
	while (true) {
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit")) {
			g_bRun = false;
			printf("退出cmdThread线程\n");
			break;
		}
		else if (0 == strcmp(cmdBuf, "login")) {
			Login login;
			strcpy(login.userName, "zzr");
			strcpy(login.passWord, "zzr");
			send(sock, (const char*)&login, sizeof(login), 0);
		}
		else if (0 == strcmp(cmdBuf, "logout")) {
			LogOut logout;
			strcpy(logout.userName, "zzr");
			send(sock, (const char*)&logout, sizeof(logout), 0);
		}
		else {
			printf("不支持的命令\n");
		}
	}
}

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

	// 启动线程
	std::thread t1(cmdThread, _sock);
	t1.detach();  // 与主线程分离

	while (g_bRun) {
		fd_set fdReads;
		FD_ZERO(&fdReads);
		FD_SET(_sock, &fdReads);  // 加入集合
		timeval t = { 1, 0 };
		int ret = select(_sock, &fdReads, NULL, NULL, &t);
		if (ret < 0) {
			printf("select 任务结束\n");
			break;
		}

		// 判断是否在集合中
		if (FD_ISSET(_sock, &fdReads)) {
			FD_CLR(_sock, &fdReads);

			if (-1 == processor(_sock)) {
				printf("select 任务结束\n");
				break;
			}
		}

		// printf("空闲时间处理其他业务\n");

		// Sleep(1000);
	}
	
	// close
	closesocket(_sock);

	printf("客户端已退出");
	getchar();
	// 清除win socket 环境
	WSACleanup();
	return 0;
}