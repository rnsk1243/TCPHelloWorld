#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<WinSock2.h>

#define BUF_SIZE 1024
void ErrorHandling(char* message);

int main(int argc, char* argv[])
{

	WSADATA wsaData;
	SOCKET hServSock, hClntSock;
	SOCKADDR_IN servAdr, clntAdr;
	// select�Լ� ȣ���Ŀ� ������ ���ŷ ������ �ʰ� �ϱ� ���� Ÿ�Ӿƿ��ð��� �־�
	// 0�� �����ϰ� ������ ���������� �ϱ�����.
	TIMEVAL timeout;
	// �б�� ����
	fd_set reads, cpyReads;

	int adrSz;
	// fdNum�� select�Լ� ȣ���Ŀ� ��ȭ�� �߻��� ������ ���� ��� ����
	// �̰��� -1���� 0���� ������������������ ���� ó���Ѵ�.
	int strLen, fdNum, i;
	char buf[BUF_SIZE];

	if (argc != 2)
	{
		printf("Usage : %s <port> \n", argv[0]);
		exit(1);
	}
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error!");

	hServSock = socket(PF_INET, SOCK_STREAM, 0);
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family = AF_INET;
	servAdr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAdr.sin_port = htons(atoi(argv[1]));

	if (bind(hServSock, (SOCKADDR*)&servAdr, sizeof(servAdr)) == SOCKET_ERROR)
		ErrorHandling("bind() Error!");
	if (listen(hServSock, 5) == SOCKET_ERROR)
		ErrorHandling("listen() error");
	// FD_ZERO(fd_set* fdset)
	// ����� �ʱ�ȭ
	FD_ZERO(&reads);
	// �������� ����� ���
	FD_SET(hServSock, &reads);

	while (1)
	{
		// ���� �Ѽ��� ���� ����
		// while���� �������� cpyReads�� �������� �ʱ�ȭ ���ش�.(select�Լ��� ȣ��Ϸ��ϸ� cpyReads���� �ٲ�Ƿ�)
		cpyReads = reads;
		// select �Լ� ȣ�� ������ �ʱ�ȭ ���־���Ѵ� select �Լ� ȣ�� ���Ŀ� �� ���� �ٽ� 5�ʷ� �ʱ�ȭ �Ǵ� ���� �ƴ϶�
		// ���� �����ִ� �ð����� �����Ǳ� ���� (�Ź� �����ϰ� 5�ʰ� ��ٸ��� �ϱ� ����)
		timeout.tv_sec = 5;
		timeout.tv_usec = 5000;
		// select�Լ� ȣ��(0, �б��, �����, ���ܼ�, Ÿ�Ӿƿ�����ü)
		// ��ȯ�� Ÿ�Ӿƿ��� 0, ������ -1, ����� ��ȭ�� �߻��� ������ ��
		if ((fdNum = select(0, &cpyReads, 0, 0, &timeout)) == SOCKET_ERROR)
		{
			break;
		}
		else if (fdNum == 0)
		{
			// Ÿ�Ӿƿ��� �߻��ϸ� �ٽ� while�� ó������ ���ư����� ��.
			continue;
		}
		else
		{
			// reads(fd_set ����ü)�� �ִ� fd_array�迭�� ������ ��µ�
			// �� �迭�� ���� �˱����� ����ü �ȿ� fd_count��� ������ �ִ�.
			// ��, for���� fd_array�迭�� ��� ��� ���Ͽ� ���Ͽ� ó���ϱ� ���ؼ�
			// ����Ѵ�.
			for (i = 0; i < (int)reads.fd_count; i++)
			{
				// ��ȭ�� ���� ������ �����ϸ�
				// ���� �迭�� ī�� �迭�� ����.
				if (FD_ISSET(reads.fd_array[i], &cpyReads))
				{
					// ��ȭ�� �߻��� ������ ���� �����̸�
					if (reads.fd_array[i] == hServSock)
					{
						adrSz = sizeof(clntAdr);
						hClntSock = accept(hServSock, (SOCKADDR*)&clntAdr, &adrSz);
						// Ŭ���̾�Ʈ������ ���� �б�¿� �߰���Ŵ
						// ���纻(cpyReads)�� �߰��ϸ� while�� ���۽� ������Ƿ� �ȵ�.
						// ����(reads)�� ����ϴ� ���� FD_�Լ���, send, recv �Լ�
						// ī���� ����ϴ� ��� closesocket(����: 
						FD_SET(hClntSock, &reads);
						printf("connected client: %d \n", hClntSock);
					}
					else // ��ȭ�� ���� ������ Ŭ���̾�Ʈ �����̸� 
					{	
						strLen = recv(reads.fd_array[i], buf, BUF_SIZE - 1, 0);
						// EOD�� �Ѿ���� (�����ȣ�� ����)
						if (strLen == 0)
						{
							// ���� �迭�� ���ϻ���
							FD_CLR(reads.fd_array[i], &reads);
							// ī�� �迭�� ���� ����.
							closesocket(cpyReads.fd_array[i]);
							printf("closed client: %d \n", cpyReads.fd_array[i]);
						}
						else
						{
							send(reads.fd_array[i], buf, strLen, 0);
						}
					}

				}
			}
		}

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
