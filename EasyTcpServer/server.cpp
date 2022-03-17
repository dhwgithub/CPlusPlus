#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <windows.h>
#include <WinSock2.h>
#include <stdio.h>

#pragma comment(lib, "ws2_32.lib")

struct DataPackage {
	int age;
	char name[32];
};

int main() {
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);

	// 1.建立socket 套接字
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	// 2.bind端口号
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(9999);
	_sin.sin_addr.S_un.S_addr = INADDR_ANY; // inet_addr("127.0.0.1");
	if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(_sin))) {
		printf("ERROR: 绑定端口号失败\n");
	}
	else {
		printf("绑定端口号成功\n");
	}

	// 3.监听网络端口
	if (SOCKET_ERROR == listen(_sock, 5)) {
		printf("ERROR: 监听网络端口失败\n");
	}
	else {
		printf("监听网络端口成功\n");
	}

	// accept
	sockaddr_in clientAddr = {};
	int nAddrLen = sizeof(sockaddr_in);
	SOCKET _cSock = INVALID_SOCKET;
	_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
	if (INVALID_SOCKET == _cSock) {
		printf("ERROR: 接收到无效客户端SOCKET...\n");
	}
	printf("新客户端加入:socket = %d, IP = %s \n", (int)_sock, inet_ntoa(clientAddr.sin_addr));

	char _recvBuf[128] = {};
	while (true) {
		// 接受客户端请求数据
		int nLen = recv(_cSock, _recvBuf, 128, 0);
		if (nLen <= 0) {
			printf("客户端已退出，请求结束.\n");
			break;
		}
		printf("收到命令: %s\n", _recvBuf);
		
		// 处理请求
		if (0 == strcmp(_recvBuf, "getInfo")) {
			DataPackage dp = { 18, "木辛" };
			send(_cSock, (const char*)&dp, sizeof(DataPackage), 0);
		}
		else {
			char msgBuf[] = "???";
			send(_cSock, msgBuf, strlen(msgBuf) + 1, 0);
		}
	}
	// close
	closesocket(_sock);
	
	printf("服务端已退出");
	getchar();
	WSACleanup();
	return 0;
}