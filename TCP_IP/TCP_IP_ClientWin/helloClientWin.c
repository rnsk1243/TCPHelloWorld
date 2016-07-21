#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<stdlib.h>
#include<winsock2.h>
#define BUF_SIZE 1024
#define RLT_SIZE 4
#define OPSZ 4

void ErrorHandling(char* message);

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	// ���������� �ϱ� ���� ���� ��ũ���͸� ������ ����
	SOCKET hSocket;
	// ������ ������ ������ ����ü ����
	SOCKADDR_IN servAddr;
	char opmsg[BUF_SIZE];
	int result, opnd_cnt, i;

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
	else
	{
		puts("Connected............");
	}

	// �ܼ� �ƿ�(��ġ cout�� ���� ����)
	fputs("Operand count: ", stdout);
	//c��� �Է¹ޱ� (�Է¹޾� ���� ���� �����ּ�)
	// �Է� ���� �ǿ������� ����
	scanf("%d", (char*)&opnd_cnt);
	// char������ �ٲپ� ���ۿ� ����
	opmsg[0] = (char)opnd_cnt;
	// �ǿ����� �Է� �ޱ�
	for (i = 0; i < opnd_cnt; i++)
	{
		printf("Operand %d: ", i + 1);
		//OPSZ ��ŭ�� �پ ���� char �ϳ��� ���� �ϳ��� �����ϹǷ� OPSZ �ڸ��� ��ŭ �����Ϸ��� 
		// OPSZ��ŭ �ǳʶѤӾ� �̾ �����ؾ��Ѵ�.
		// �� 123 ����� opmsg[0] = '1'; ... opmsg[2] = '3'; �̷��� �Ѵ�.
		// �׵ڿ� 456 ���� opmsg[3] ���� 4�� �����Ѵ�.  
		// 1�� ���Ѱ� ������ �ǿ������� ���� ����
		scanf("%d", (int*)&(opmsg[i*OPSZ + 1]));
	}
	// �ؿ��� scanfȣ���ϸ鼭 ���ڸ� �Է¹޴µ� �̹� ���ۿ� �����ִ� \n������ ������ ����
	fgetc(stdin);// ���ڿ� �д� �Լ�
	fputs("Operator: ", stdout);
	// ������ �Է¹ޱ� 
	scanf("%c", &opmsg[opnd_cnt*OPSZ + 1]);
	// �Է¹��� ���� ������ ������ + 2�� �ǿ����� ���� ���� 1 �׸��� ������ 1 �ؼ� 1+1 = 2�̴�. 
	send(hSocket, opmsg, opnd_cnt*OPSZ + 2, 0);
	// �������� ������ �ޱ�
	recv(hSocket, &result, RLT_SIZE, 0);

	printf("Operation result: %d \n", result);

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