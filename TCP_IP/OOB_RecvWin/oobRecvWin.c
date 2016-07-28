#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<stdio.h>
#include<stdlib.h>
#include<WinSock2.h>

#define BUF_SIZE 30
void ErrorHandling(char* message);

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	SOCKET hAcptSock, hRecvSock;

	SOCKADDR_IN recvAdr, sendAdr;
	int sendAdrSize, strLen;
	char buf[BUF_SIZE];
	int result;

	fd_set read, except, readCopy, exceptCopy;
	struct timeval timeout;

	if (argc != 2)
	{
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		ErrorHandling("WSAStartup() error!");
	}

	hAcptSock = socket(PF_INET, SOCK_STREAM, 0);
	memset(&recvAdr, 0, sizeof(recvAdr));
	recvAdr.sin_family = AF_INET;
	recvAdr.sin_addr.s_addr = htonl(INADDR_ANY);
	recvAdr.sin_port = htons(atoi(argv[1]));

	// �����ϱ� Ŭ���̾�Ʈ�� �ϴ� ���� �޴� recvAdr�� hAcptSock�� ������
	// Ŭ���̾�Ʈ�� �ϴ� ���� ���� hAcptSock�� ������ ����
	if (bind(hAcptSock, (SOCKADDR*)&recvAdr, sizeof(recvAdr)) == SOCKET_ERROR)
	{
		ErrorHandling("bind() error");
	}
	if (listen(hAcptSock, 5) == SOCKET_ERROR)
	{
		ErrorHandling("listen() error");
	}

	sendAdrSize = sizeof(sendAdr);
	// Ŭ���̾�Ʈ connect�κп��� sendAdr�� ���� ��û�� �ϰ�
	// ������ accept�κп��� Ŭ���̾�Ʈ���� �� sendAdr�� hAcptSock�� �����Ŵ
	hRecvSock = accept(hAcptSock, (SOCKADDR*)&sendAdr, &sendAdrSize);
	FD_ZERO(&read);
	FD_ZERO(&except);
	// ���� ������� Recv������ ����
	// ����� �������Ǻκ��̴ϱ� ��°ſ� ������ �־�
	FD_SET(hRecvSock, &read);
	FD_SET(hRecvSock, &except);
	
	while (1)
	{
		// ���� �ջ� ������
		readCopy = read;
		exceptCopy = except;
		// ���ŷ ������ ������ �ð��� ������ 0�� �����ϰ� ��������(select�Լ�����) 
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;

		result = select(0, &readCopy, 0, &exceptCopy, &timeout);
		// ������ �ִ� ������ �ִٸ�
		if (result > 0)
		{
			// Recv���Ͽ� ���ܼ��� �߻��ϸ�, 
			// MSG_OOB�� �̿��ؼ� Ŭ���̾�Ʈ�� send�� ������ ���ܼ� hRecvSock �߻�
			// ���ܼ����� ������ �����ʹ� recv�ɼ����� MSG_OOB�� ��
			if (FD_ISSET(hRecvSock, &exceptCopy))
			{
				// 
				strLen = recv(hRecvSock, buf, BUF_SIZE - 1, MSG_OOB);
				buf[strLen] = 0;
				printf("Urgent message: %s \n", buf);
			}
			// Recv���Ͽ� �б���� �߻��ϸ�
			if (FD_ISSET(hRecvSock, &readCopy))
			{
				strLen = recv(hRecvSock, buf, BUF_SIZE - 1, 0);
				// 0�̸� Ŭ���̾�Ʈ�� �����ȣ�� �������̹Ƿ�
				if (strLen == 0)
				{
					// ������
					break;
					closesocket(hRecvSock);
				}else
				{
					buf[strLen] = 0;
					puts(buf);
				}
			}
		}
	}
	closesocket(hAcptSock);
	WSACleanup();
	return 0;
}

void ErrorHandling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}