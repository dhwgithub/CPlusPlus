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
		printf("��������Ͽ����ӣ��������.\n");
		return -1;
	}

	switch (header->cmd) {
	case CMD_LOGIN_RESULT: {
		recv(_Sock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		LoginResult* login = (LoginResult*)szRecv;
		printf("�յ��������Ϣ: CMD_LOGIN_RESULT, ���ݳ��ȣ�%d\n", login->dataLength);
		break;
	}
	case CMD_LOGOUT_RESULT: {
		recv(_Sock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		LogOutResult* logout = (LogOutResult*)szRecv;
		printf("�յ��������Ϣ: CMD_LOGOUT_RESULT, ���ݳ��ȣ�%d\n", logout->dataLength);
		break;
	}
	case CMD_NEW_USER_JOIN: {
		recv(_Sock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		NewUserJoin* userJoin = (NewUserJoin*)szRecv;
		printf("�յ��������Ϣ: CMD_LOGOUT_RESULT, ���ݳ��ȣ�%d\n", userJoin->dataLength);
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
			printf("�˳�cmdThread�߳�\n");
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
			printf("��֧�ֵ�����\n");
		}
	}
}

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

	// �����߳�
	std::thread t1(cmdThread, _sock);
	t1.detach();  // �����̷߳���

	while (g_bRun) {
		fd_set fdReads;
		FD_ZERO(&fdReads);
		FD_SET(_sock, &fdReads);  // ���뼯��
		timeval t = { 1, 0 };
		int ret = select(_sock, &fdReads, NULL, NULL, &t);
		if (ret < 0) {
			printf("select �������\n");
			break;
		}

		// �ж��Ƿ��ڼ�����
		if (FD_ISSET(_sock, &fdReads)) {
			FD_CLR(_sock, &fdReads);

			if (-1 == processor(_sock)) {
				printf("select �������\n");
				break;
			}
		}

		// printf("����ʱ�䴦������ҵ��\n");

		// Sleep(1000);
	}
	
	// close
	closesocket(_sock);

	printf("�ͻ������˳�");
	getchar();
	// ���win socket ����
	WSACleanup();
	return 0;
}