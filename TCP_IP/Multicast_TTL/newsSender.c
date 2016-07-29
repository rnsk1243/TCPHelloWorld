#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<WinSock2.h>
#include<WS2tcpip.h>

// �󸶳� ���� ����͸� ����� �ǰ�?(Ŭ���� Ʈ���� ����)
#define TTL 64
#define BUF_SIZE 30

void ErrorHandling(char* message);

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	SOCKET hSendSock;
	SOCKADDR_IN mulAdr;
	int timeLive = TTL;
	FILE* fp;
	char buf[BUF_SIZE];

	if (argc != 3)
	{
		exit(1);
	}
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		ErrorHandling("WSAStartup() error!");
	}
	hSendSock = socket(PF_INET, SOCK_DGRAM, 0);
	memset(&mulAdr, 0, sizeof(mulAdr));
	mulAdr.sin_family = AF_INET;
	// ��Ƽĳ��Ʈ ���� ip�� 224.0.0.0~239 ���
	// Receiver���Ͽ��� �� ip�� �׷쿡 �����ϰ� �ȴ�.
	mulAdr.sin_addr.S_un.S_addr = inet_addr(argv[1]);
	mulAdr.sin_port = htons(atoi(argv[2]));

	// TTL �ɼ� ����
	setsockopt(hSendSock, IPPROTO_IP, IP_MULTICAST_TTL, (void*)&timeLive, sizeof(timeLive));
	if ((fp = fopen("news.txt", "r")) == NULL)
		ErrorHandling("fopen() error");
	while (!feof(fp))
	{
		fgets(buf, BUF_SIZE, fp);
		// ��Ƽĳ��Ʈ ip�� ��Ʈ��ȣ�� �������� ����ü(����������=mulAdr)�� �����͸� ������.
		// �� �����͸� �������� ������ �������� ��Ƽĳ��Ʈ ip�� ��Ʈ��ȣ�� bind�� ������ recv�ؾ��Ѵ�.
		// ��, �׷쿡 �����ϰų� ������������ bind�� �����̿��� �����͸� ���� �� �ִ�.
		sendto(hSendSock, buf, strlen(buf), 0, (SOCKADDR*)&mulAdr, sizeof(mulAdr));
		Sleep(2000);
	}
	closesocket(hSendSock);
	WSACleanup();
	return 0;
}

void ErrorHandling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}