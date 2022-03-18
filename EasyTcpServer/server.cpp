#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <windows.h>
#include <WinSock2.h>
#include <stdio.h>
#include <vector>

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

std::vector<SOCKET> g_clients;


int processor(SOCKET _cSock) {
	char szRecv[1024] = {};
	int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0);
	DataHeader* header = (DataHeader*)szRecv;

	if (nLen <= 0) {
		printf("�ͻ������˳����������.\n");
		return -1;
	}

	switch (header->cmd) {
	case CMD_LOGIN: {
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		Login* login = (Login*)szRecv;
		printf("�յ�����: CMD_LOGIN ���ݳ��ȣ�%d ������%s ���룺%s\n",
			login->dataLength, login->userName, login->passWord);
		// ���ԺϷ����ж�
		LoginResult ret;
		send(_cSock, (const char*)&ret, sizeof(LoginResult), 0);
		break;
	}
	case CMD_LOGOUT: {
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		LogOut* logOut = (LogOut*)szRecv;
		printf("�յ�����: CMD_LOGOUT ���ݳ��ȣ�%d ������%s\n",
			logOut->dataLength, logOut->userName);
		// ���ԺϷ����ж�
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

	return 1;
}

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

		for (int n = (int)g_clients.size() - 1; n >= 0; --n) {
			FD_SET(g_clients[n], &fdRead);
		}
		
		timeval t = { 0, 0 };
		// nfds ��һ������ֵ����ָfd_set����������������(socket)�ķ�Χ������������
		// ���������ļ������������ֵ+1����windows�иò���������
		int ret = select(_sock + 1, &fdRead, &fdWrite, &fdExp, &t);
		if (ret < 0) {
			printf("��������˳�.\n");
			break;
		}

		if (FD_ISSET(_sock, &fdRead)) {
			FD_CLR(_sock, &fdRead);

			// accept
			sockaddr_in clientAddr = {};
			int nAddrLen = sizeof(sockaddr_in);
			SOCKET _cSock = INVALID_SOCKET;
			_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
			if (INVALID_SOCKET == _cSock) {
				printf("ERROR: ���յ���Ч�ͻ���SOCKET...\n");
			}

			g_clients.push_back(_cSock);
			printf("�¿ͻ��˼���:socket = %d, IP = %s \n", (int)_sock, inet_ntoa(clientAddr.sin_addr));
		}

		for (int n = 0; n < fdRead.fd_count; ++n) {
			if (-1 == processor(fdRead.fd_array[n])) {
				auto iter = find(g_clients.begin(), g_clients.end(), fdRead.fd_array[n]);
				if (iter != g_clients.end()) {
					g_clients.erase(iter);
				}
			}
		}
	}

	for (size_t n = g_clients.size() - 1; n >= 0; --n) {
		closesocket(g_clients[n]);
	}

	// close
	closesocket(_sock);
	
	printf("��������˳�");
	getchar();
	WSACleanup();
	return 0;
}