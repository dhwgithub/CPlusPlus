#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

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

	// 1.����socket �׽���
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	// 2.bind�˿ں�
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(9999);
	_sin.sin_addr.S_un.S_addr = INADDR_ANY; // inet_addr("127.0.0.1");
	if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(_sin))) {
		printf("ERROR: �󶨶˿ں�ʧ��\n");
	}
	else {
		printf("�󶨶˿ںųɹ�\n");
	}

	// 3.��������˿�
	if (SOCKET_ERROR == listen(_sock, 5)) {
		printf("ERROR: ��������˿�ʧ��\n");
	}
	else {
		printf("��������˿ڳɹ�\n");
	}

	// accept
	sockaddr_in clientAddr = {};
	int nAddrLen = sizeof(sockaddr_in);
	SOCKET _cSock = INVALID_SOCKET;
	_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
	if (INVALID_SOCKET == _cSock) {
		printf("ERROR: ���յ���Ч�ͻ���SOCKET...\n");
	}
	printf("�¿ͻ��˼���:socket = %d, IP = %s \n", (int)_sock, inet_ntoa(clientAddr.sin_addr));

	while (true) {
		DataHeader header = {};

		// ���ܿͻ�����������
		int nLen = recv(_cSock, (char*)&header, sizeof(DataHeader), 0);
		if (nLen <= 0) {
			printf("�ͻ������˳����������.\n");
			break;
		}
		printf("�յ�����: %d ���ݳ��ȣ�%d\n", header.cmd, header.dataLength);
		
		switch (header.cmd) {
		case CMD_LOGIN: {
			Login login = {};
			recv(_cSock, (char*)&login, sizeof(Login), 0);
			// ���ԺϷ����ж�
			LoginResult ret = {1};
			send(_cSock, (const char*)&header, sizeof(DataHeader), 0);
			send(_cSock, (const char*)&ret, sizeof(LoginResult), 0);
			break;
		}
		case CMD_LOGINOUT: {
			LoginOut loginOut = {};
			recv(_cSock, (char*)&loginOut, sizeof(LoginOut), 0);
			// ���ԺϷ����ж�
			LoginOutResult ret = { 1 };
			send(_cSock, (const char*)&header, sizeof(DataHeader), 0);
			send(_cSock, (const char*)&ret, sizeof(LoginOutResult), 0);
			break;
		}
		default: {
			header.cmd = CMD_ERROR;
			header.dataLength = 0;
			send(_cSock, (const char*)&header, sizeof(DataHeader), 0);
			break;
		}	
		}
	}
	// close
	closesocket(_sock);
	
	printf("��������˳�");
	getchar();
	WSACleanup();
	return 0;
}