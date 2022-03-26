#ifdef _WIN32  
	#define _WINSOCK_DEPRECATED_NO_WARNINGS
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include <WinSock2.h>
	#pragma comment(lib, "ws2_32.lib")
#else 
	#include <unistd.h>
	#include <arpa/inet.h>
	#include <string.h>
	
	#define SOCKET int
	#define INVALID_SOCKET  (SOCKET)(~0)
	#define SOCKET_ERROR            (-1)
#endif

#include <stdio.h>
#include <vector>

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

struct Login: public DataHeader {
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

std::vector<SOCKET> g_clients;

int processor(SOCKET _cSock) {
	char szRecv[1024] = {};
	int nLen = (int)recv(_cSock, szRecv, sizeof(DataHeader), 0);
	DataHeader* header = (DataHeader*)szRecv;

	if (nLen <= 0) {
		printf("Client exit already, request end.\n");
		return -1;
	}

	switch (header->cmd) {
	case CMD_LOGIN: {
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		Login* login = (Login*)szRecv;
		printf("Accept client <socket=%d> request: CMD_LOGIN data length: %d name: %s password: %s\n",
			_cSock, login->dataLength, login->userName, login->passWord);
		// 忽略合法性判断
		LoginResult ret;
		send(_cSock, (const char*)&ret, sizeof(LoginResult), 0);
		break;
	}
	case CMD_LOGOUT: {
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		LogOut* logOut = (LogOut*)szRecv;
		printf("Accept client <socket=%d> request: CMD_LOGOUT data length: %d name: %s\n",
			_cSock, logOut->dataLength, logOut->userName);
		// 忽略合法性判断
		LogOutResult ret;
		send(_cSock, (const char*)&ret, sizeof(LogOutResult), 0);
		break;
	}
	default: {
		DataHeader header = { 0, CMD_ERROR };
		send(_cSock, (const char*)&header, sizeof(DataHeader), 0);
		break;
	}
	}

	return 0;
}

int main() {
#ifdef _WIN32 
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);
#endif

	// 1.建立socket 套接字
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	// 2.bind端口号
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(9998);
	
#ifdef _WIN32
	_sin.sin_addr.S_un.S_addr = INADDR_ANY; // inet_addr("127.0.0.1");
#else
	_sin.sin_addr.s_addr = INADDR_ANY; // inet_addr("127.0.0.1");
#endif
	
	if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(_sin))) {
		printf("ERROR: bind port failure\n");
		return 0;
	}
	else {
		printf("bind port success\n");
	}

	// 3.监听网络端口
	if (SOCKET_ERROR == listen(_sock, 5)) {
		printf("ERROR: listen network port failure\n");
	}
	else {
		printf("Listen network port success\n");
	}

	while (true) {
		fd_set fdRead;
		fd_set fdWrite;
		fd_set fdExp;

		FD_ZERO(&fdRead);
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdExp);

		FD_SET(_sock, &fdRead);
		FD_SET(_sock, &fdWrite);
		FD_SET(_sock, &fdExp);
		
		SOCKET maxSock = _sock;
		for (int n = (int)g_clients.size() - 1; n >= 0; --n) {
			FD_SET(g_clients[n], &fdRead);
			if (maxSock < g_clients[n]) {
				maxSock = g_clients[n];
			}
		}
		
		timeval t = { 1, 0 };
		// nfds 是一个整数值，是指fd_set集合中所有描述符(socket)的范围，而不是数量
		// 即是所有文件描述符的最大值+1，在windows中该参数无作用
		int ret = select(maxSock + 1, &fdRead, &fdWrite, &fdExp, &t);
		if (ret < 0) {
			printf("Server exit.\n");
			break;
		}

		if (FD_ISSET(_sock, &fdRead)) {
			FD_CLR(_sock, &fdRead);

			// accept
			sockaddr_in clientAddr = {};
			int nAddrLen = sizeof(sockaddr_in);
			SOCKET _cSock = INVALID_SOCKET;
			
#ifdef _WIN32
			_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
#else 
			_cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t*)&nAddrLen);
#endif

			if (INVALID_SOCKET == _cSock) {
				printf("ERROR: accept invalid client SOCKET...\n");
			}
			else {
				for (int n = (int)g_clients.size() - 1; n >= 0; --n) {
					NewUserJoin userJoin;
					userJoin.sock = _cSock;
					send(g_clients[n], (const char*)&userJoin, sizeof(NewUserJoin), 0);
				}
				g_clients.push_back(_cSock);
				printf("New client join:socket = %d, IP = %s \n", (int)_cSock, inet_ntoa(clientAddr.sin_addr));
			} 
		}
		
		for (int n = (int)g_clients.size() - 1; n >= 0; --n) {
			if (FD_ISSET(g_clients[n], &fdRead)) {
				if (-1 == processor(g_clients[n])) {
					auto iter = g_clients.begin() + n;
					if (iter != g_clients.end()) {
						g_clients.erase(iter);
					}
				}
			}
		}

		printf("Free time handle others work...\n");
	}

#ifdef _WIN32 
	for (int n = (int)g_clients.size() - 1; n >= 0; --n) {
		closesocket(g_clients[n]);
	}
	closesocket(_sock);
	WSACleanup();
#else 
	for (int n = (int)g_clients.size() - 1; n >= 0; --n) {
		close(g_clients[n]);
	}
	close(_sock);
#endif

	printf("Server exit already");
	getchar();
	return 0;
}
