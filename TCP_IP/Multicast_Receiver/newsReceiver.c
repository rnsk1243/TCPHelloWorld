#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<WinSock2.h>
#include<WS2tcpip.h>

#define BUF_SIZE 30

void ErrorHandling(char* message);

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	SOCKET hRecvSock;
	SOCKADDR_IN adr;
	// ��Ƽĳ��Ʈ �׷쿡 �����ϱ� ���� ���Ͽɼǿ� �� ����ü ����
	struct ip_mreq joinAdr;
	char buf[BUF_SIZE];
	int strLen;

	if (argc != 3)
	{
		exit(1);
	}
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		ErrorHandling("WSAStartup() error!");
	}
	// UDP���� ���
	hRecvSock = socket(PF_INET, SOCK_DGRAM, 0);
	// ���ϰ� ip,port ������ ���� ����ü �ʱ�ȭ
	memset(&adr, 0, sizeof(adr));
	adr.sin_family = AF_INET;
	// ��� �����̹Ƿ� ip�� �ڱ� �ڽ������Ѵ�.
	adr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	// ��Ƽĳ��Ʈ ��Ʈ(sender�� ��Ʈ��ȣ)�� ������ϹǷ� argv[2]�� �ִ´�.
	adr.sin_port = htons(atoi(argv[2]));
	if (bind(hRecvSock, (SOCKADDR*)&adr, sizeof(adr)) == SOCKET_ERROR)
	{
		ErrorHandling("bind() error");
	}
	// ��Ƽĳ��Ʈ �׷쿡 ���� ���� �ʱ�ȭ
	// ��Ƽĳ��Ʈ�� ip�ּ� �ʱ�ȭ (Sender�� ������ ip�ּ� �Է�)
	joinAdr.imr_multiaddr.S_un.S_addr = inet_addr(argv[1]);
	// ��Ƽĳ��Ʈ�� �����ϰ��� �ϴ� ���� ip�ּ� (�� �ڽ�)
	joinAdr.imr_interface.S_un.S_addr = htonl(INADDR_ANY);
	// ���Ͽɼ��� �̿��Ͽ� ����.
	if (setsockopt(hRecvSock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&joinAdr, sizeof(joinAdr)) == SOCKET_ERROR)
	{
		ErrorHandling("setsock() error");
	}
	while (1)
	{
		// ���� �ޱ� 
		// 5��° ���ڰ� NULL�� ������ ���� ������ ������ ������ �ʿ䰡 ���⶧��(���� �������� �����͸� �����ϵ� �����ϱ�)
		// �׸��� ���Ͽɼ����� �׷쿡 ���������Ƿ� ���ٸ� ���Ǿ��� �����͸� ���� �� ����
		strLen = recvfrom(hRecvSock, buf, BUF_SIZE - 1, 0, NULL, 0);
		if (strLen < 0)
		{
			break;
		}
		buf[strLen] = 0;
		fputs(buf, stdout);
	}
	closesocket(hRecvSock);
	WSACleanup();

	return 0;
}