#include<stdio.h>
#include<stdlib.h>
#include<WinSock2.h>
#define BUF_SIZE 1024

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
	// Ŭ���̾�Ʈ���� ���� �޽����� ���� ����
	char message[BUF_SIZE];
	// ���� ���ڿ��� ����
	int strLen, i;
	
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
	// Ŭ���̾�Ʈ �ּ� ũ�� accept�Լ����� ����ε� ���� �ʱ�ȭ �ȴ�.
	szClntAddr = sizeof(clntAddr);

	for (i = 0; i < 5; i++)
	{
		//accept�� ȣ���� �Ϸ�Ǹ� Ŭ���̾�Ʈ �ּұ��� ������ szClntAddr�� ����Ʈ ������ ä������.
		// �����û�� ���� ������ ������ �Ǹ� Ŭ���̾�Ʈ �ּ������� ������ ����ü�� Ŭ���̾�Ʈ �ּ�,��Ʈ��ȣ�� ����, ũ�⵵ ����
		// �̶� accept�Լ� �ȿ��� �ڵ����� ���ο� ������ �����ǰ� ũ���̾�Ʈ ���ϰ� ����ȴ�.(���������� ������ �ϻ�!)
		hClntSock = accept(hServSock, (SOCKADDR*)&clntAddr, &szClntAddr);
		if (hClntSock == INVALID_SOCKET)
		{
			ErrorHandling("accept() error");
		}
		else
		{
			printf("Connected client %d \n", i + 1);
		}
		// Ŭ���̾�Ʈ�κ��� �޾Ƽ� message�� �� ���� �ѹ� ȣ�� �ɶ����� BUF_SIZE��ŭ�� ����
		// ���� ���ڿ� ũ�⸦ strLen�� ����
		/*
		�׷��� ������ ���� recv�� send�� ȣ��� ������ ���� ������� �̷�� ���µ� �̴� ������ �ִ�.
		TCP�� �������� ��谡 �������� �ʱ� ������ ���� �� �̻��� send�Լ��� ���� ���޵� ���ڿ� ������ ������ �ѹ���
		������ ���۵� �� �ִ�. �̷��� �Ǹ� ������ Ŭ���̾�Ʈ���� �� �̻��� ���ڿ��� �ѹ��� �޾Ƽ� ������ �߻��� �� �ִ�.
		*/
		// ����� Ŭ���̾�Ʈ�� q�� ���� ���Ḧ ��Ű�� ������(���� �����ϸ�) ������ Ŭ���̾�Ʈ�� ������ ����
		// while������ send�Լ��� ���� �ݺ� ȣ����.
		// �׸��� Ŭ���̾�Ʈ 2�� ����� ó�� �ϳ��� ������ ����� �Ŀ� ���߿� �ϳ��� ���ť�� �ְ� �Ǵµ�
		// ó������ ���� �����ϸ� while���� ���������� ���ϰ� �ǰ� �̷����� closesocket�Լ��� ȣ����� �ʾ�
		// ���ť�� �ִ� Ŭ���̾�Ʈ�� ������ ���ť�� �ӹ��� �ȴ�.
		
		// ������ Ŭ���̾�Ʈ�� ����� ���¿��� Ŭ���̾�Ʈ�� ������ ������ �޾Ƽ� ������ ���̻� while���� �ݺ�����
		// �ʴ´�. �������δ� recv���� �޴°� ��� ���� �ǳ���.
		printf("-CC0-");
		while ((strLen = recv(hClntSock, message, BUF_SIZE, 0)) != 0)
		{
			printf("CC");
			// Ŭ���̾�Ʈ���� ������ strLen��ŭ
			send(hClntSock, message, strLen, 0);
		}
		printf("CC2");
		// acceptȣ��� ���� ������� ���� �ݱ�(���� ����)
		closesocket(hClntSock);
	}
	
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
