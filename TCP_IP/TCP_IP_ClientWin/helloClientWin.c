#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<stdio.h>
#include<stdlib.h>
#include<winsock2.h>

void ErrorHandling(char* message);

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	// ���������� �ϱ� ���� ���� ��ũ���͸� ������ ����
	SOCKET hSocket;
	// ������ ������ ������ ����ü ����
	SOCKADDR_IN servAddr;
	int idx = 0;
	// �������� ���� ������ ���� ����
	char message[30];
	int strLen = 0;
	int readLen = 0;
	if (argc != 3) {
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	// ���� 2.2���� ���
	if (WSAStartup((2, 2), &wsaData) != 0)
	{
		ErrorHandling("WSAStartup() error!");
	}
	// ���� ����
	hSocket = socket(PF_INET, SOCK_STREAM, 0);
	if (hSocket == INVALID_SOCKET)
	{
		ErrorHandling("socket() error");
	}
	// ������ ������ ���� ����ü�� �ϴ� ��� 0���� �ʱ�ȭ��Ŵ
	memset(&servAddr, 0, sizeof(servAddr));
	// TCP����Ұ��� ����
	servAddr.sin_family = AF_INET;
	// inet_addr(������ ip�ּҸ����� ������ �ּҰ�) �Լ��� ȣ���Ͽ� 32��Ʈ ��Ʈ��ũ����(�򿣵��)���ְ� �װ��� ������.
	servAddr.sin_addr.s_addr = inet_addr(argv[1]);
	// ��Ʈ(�򿣵��) �ʱ�ȭ
	servAddr.sin_port = htons(atoi(argv[2]));
	// ������ ��û������
	// connect�Լ��� ���� ����Ǹ� ������ ��û��� ���°���(���� ������ �ȵ� ����)
	// �̶� Ŀ�ο� ���� Ŭ���̾�Ʈ ip�ּ�(ȣ��Ʈ)�� ���Ƿ� �Ҵ�� ��Ʈ������ Ŭ���̾�Ʈ ���Ͽ� �Ҵ�Ǿ� ���������. 
	if (connect(hSocket, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
	{
		ErrorHandling("connect() error!");
	}

	//recv �Լ��� ���� �������� ���� �޽����� �Է�(����)�Ѵ�.
	while (readLen = recv(hSocket, &message[idx++], 1, 0))
	{
		if (strLen == -1)
		{
			ErrorHandling("read() error!");
		}
		strLen += readLen;
	}

	
	printf("Message from server: %s \n", message);
	printf("Function read call count: %d \n", strLen);
	closesocket(hSocket);
	WSACleanup();
	return 0;
}

void ErrorHandling(char * message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}