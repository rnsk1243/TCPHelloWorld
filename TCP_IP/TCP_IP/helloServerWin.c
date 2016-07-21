#include<stdio.h>
#include<stdlib.h>
#include<WinSock2.h>
#define OPSZ 4
#define BUF_SIZE 1024

void ErrorHandling(char* message);
int calculate(int opnum, int opnds[], char oprator);

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	// ���� ��ũ���͸� ������ ����(��Ʈ���� �� ��)
	SOCKET hServSock, hClntSock;
	// ������ Ŭ���̾�Ʈ�� �ּ�,��Ʈ������ ���� ����ü ����
	SOCKADDR_IN servAddr, clntAddr;
	// Ŭ���̾�Ʈ���� ���� ��
	char opinfo[BUF_SIZE];
	int result, opnd_cnt, i;
	int recvCnt, recvLen;
	int szClntAddr;
	
	int strLen;
	
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
		// �ǿ����� ����
		opnd_cnt = 0;
		hClntSock = accept(hServSock, (SOCKADDR*)&clntAddr, &szClntAddr);
		// Ŭ�󿡼� ó��°�� �־��� ������ ���� �ޱ�
		recv(hClntSock, &opnd_cnt, 1, 0);
		// ���� Ŭ�󿡼� �� ����Ʈ ���� ���� ����
		recvLen = 0;
		// ��ü ����Ʈ ���� > ���� ���� ����Ʈ �̸� ���� �� ������ �������Ƿ� true�� �� recv�ϱ����� while�� ����.
		// + 1 �� ������ �������� ���� �����ڱ��� ���� �Ϸ���
		while ((opnd_cnt*OPSZ + 1) > recvLen)
		{
			// �迭 opinfo 0�� �ε������� Ŭ�󿡼� �� ���ڵ�� ������ ����
			// �ǿ����� ������ ���� recv�� ������ while�� ���ǿ� ������
			// recvCnt�� ���� ���� ����Ʈ�� ���� �ϰ� recvLen�� �־� ������Ŵ 
			// �� ������Ų ���� �ε����� �Ǿ� ���� �迭���� opinfo�� ���� �����.
			recvCnt = recv(hClntSock, &opinfo[recvLen], BUF_SIZE - 1, 0);
			// ����
			recvLen += recvCnt;
		}
		// ��� ����
		result = calculate(opnd_cnt, (int*)opinfo, opinfo[recvLen - 1]);
		// ��� Ŭ�󿡰� ����
		send(hClntSock, (char*)&result, sizeof(result), 0);
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

int calculate(int opnum, int opnds[], char oprator)
{
	int result = opnds[0], i;
	switch (oprator)
	{
	case '+':
		for (i = 1; i < opnum; i++)
			result += opnds[i];
		break;
	case '-':
		for (i = 1; i < opnum; i++)
			result -= opnds[i];
		break;
	case '*':
		for (i = 1; i < opnum; i++)
			result *= opnds[i];
		break;
	default:
		break;
	}
	return result;
}
