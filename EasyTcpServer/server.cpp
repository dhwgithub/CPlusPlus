#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <windows.h>
#include <WinSock2.h>
#include <stdio.h>

#pragma comment(lib, "ws2_32.lib")

enum CMD {
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_ERROR
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
		
		switch (header.cmd) {
		case CMD_LOGIN: {
			Login login = {};
			recv(_cSock, (char*)&login + sizeof(DataHeader), sizeof(Login) - sizeof(DataHeader), 0);
			printf("�յ�����: CMD_LOGIN ���ݳ��ȣ�%d ������%s ���룺%s\n", 
				login.dataLength, login.userName, login.passWord);
			// ���ԺϷ����ж�
			LoginResult ret;
			send(_cSock, (const char*)&ret, sizeof(LoginResult), 0);
			break;
		}
		case CMD_LOGOUT: {
			LogOut logOut = {};
			recv(_cSock, (char*)&logOut + sizeof(DataHeader), sizeof(LogOut) - sizeof(DataHeader), 0);
			printf("�յ�����: CMD_LOGOUT ���ݳ��ȣ�%d ������%s\n",
				logOut.dataLength, logOut.userName);
			// ���ԺϷ����ж�
			LogOutResult ret;
			send(_cSock, (const char*)&ret, sizeof(LogOutResult), 0);
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