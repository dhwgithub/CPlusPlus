#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <WinSock2.h>
#include <stdio.h>

#pragma comment(lib, "ws2_32.lib")

int main() {
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);

	// 1.����socket �׽���
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == _sock) {
		printf("ERROR: �����ͻ���SOCKETʧ��...\n");
	}
	else {
		printf("�����ͻ���SOCKET�ɹ�...\n");
	}
	// connect
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(9999);
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
	if (SOCKET_ERROR == ret) {
		printf("ERROR: ����ʧ��...\n");
	}
	else {
		printf("���ӳɹ�...\n");
	}

	while (true) {
		char cmdBuf[128] = {};
		scanf("%s", &cmdBuf);
		if (0 == strcmp(cmdBuf, "exit")) {
			break;
		}
		else {
			send(_sock, cmdBuf, strlen(cmdBuf) + 1, 0);
		}

		// ���ܷ�������Ϣ
		char recvBuf[256] = {};
		int nlen = recv(_sock, recvBuf, 256, 0);
		if (nlen > 0) {
			printf("���յ�����: %s\n", recvBuf);
		}
	}
	
	// close
	closesocket(_sock);

	printf("�ͻ������˳�");
	getchar();
	// ���win socket ����
	WSACleanup();
	return 0;
}