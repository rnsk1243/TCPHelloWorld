#include<stdio.h>
#include<stdlib.h>
#include<WinSock2.h>
void ErrorHandling(char* message);

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	// ���� ��ũ���͸� ������ ����(��Ʈ���� �� ��)
	SOCKET hServSock, hClntSock;
	// ������ Ŭ���̾�Ʈ�� �ּ�,��Ʈ������ ���� ����ü ����
	SOCKADDR_IN servAddr, clntAddr;

	// Ŭ���̾�Ʈ �ּ� ���̸� ���� ����
	int szClntAddr;
	// Ŭ���̾�Ʈ���� ���� �޽���
	char message[] = "Hello World!";
	if (argc != 2)
	{
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
	// ���� 2.2������ ����ϰڴ�.
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		ErrorHandling("WSAStartup() error!");
	}
	// ��������(������) ���� IPv4����ϰڴ�, TCP����ϰ���!
	hServSock = socket(PF_INET, SOCK_STREAM, 0);
	if (hServSock == INVALID_SOCKET)
	{
		ErrorHandling("socket() error");
	}
	// ���� �ּҿ� ��Ʈ��ȣ�� ���� ����ü�� �켱 0���� �ʱ�ȭ
	memset(&servAddr, 0, sizeof(servAddr));
	// TCP����� ��
	servAddr.sin_family = AF_INET;
	// ���� �ּ� 32��Ʈ ��Ʈ��ũ ����(�򿣵��)�� ��ȯ�� �Ҵ�
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	// ��Ʈ��ȣ ��Ʈ��ũ ������ �Ҵ�
	servAddr.sin_port = htons(atoi(argv[1]));
	
	// �����ּҿ� ��Ʈ��ȣ�� ä���� ����ü�� �������Ͽ� �����Ŵ
	// �������ϵ�ũ����, ����ü�� �ּ�(�̶� SOCKADDR�� �ּ����·� ��ȯ�� �־��ش�), ����ü�� ũ��
	if (bind(hServSock, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR )
	{
		ErrorHandling("bind() error");
	}
	// ���������� ������ ��Ȱ�� �� �� �ֵ��� �����û �����¿� �����ϵ��� ��.
	// Ŭ���̾�Ʈ �����ûť�� ������ �����û��� ũ�⸦ ��Ÿ��
	// �����⿪���� �� ����(������ ���� ��������), �����ûť�� ũ��
	if (listen(hServSock, 5) == SOCKET_ERROR)
	{
		ErrorHandling("listen() error");
	}

	szClntAddr = sizeof(clntAddr);
	//accept�� ȣ���� �Ϸ�Ǹ� Ŭ���̾�Ʈ �ּұ��� ������ szClntAddr�� ����Ʈ ������ ä������.
	// �����û�� ���� ������ ������ �Ǹ� Ŭ���̾�Ʈ �ּ������� ������ ����ü�� Ŭ���̾�Ʈ �ּ�,��Ʈ��ȣ�� ����, ũ�⵵ ����
	// �̶� accept�Լ� �ȿ��� �ڵ����� ���ο� ������ �����ǰ� ũ���̾�Ʈ ���ϰ� ����ȴ�.(���������� ������ �ϻ�!)
	hClntSock = accept(hServSock, (SOCKADDR*)&clntAddr, &szClntAddr);
	if (hClntSock == INVALID_SOCKET)
	{
		ErrorHandling("accept() error");
	}
	// Ŭ���̾�Ʈ ���Ͽ��� �޽����� ����
	send(hClntSock, message, sizeof(message), 0);
	closesocket(hClntSock);
	closesocket(hServSock);
	WSACleanup();
	return 0;
}

void ErrorHandling(char * message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
