#include<stdio.h>
#include<stdlib.h>
#include<process.h>
#include<WinSock2.h>
#include<Windows.h>


#define BUF_SIZE 100
#define MAX_CLNT 256
#define READ 3
#define WRITE 5

//unsigned WINAPI HandleClnt(void* arg);
//void SendMsg(char* msg, int len);
void ErrorHandling(char* message);

// Ŭ���̾�Ʈ�� ����� ���� ������ ������� ����ü
// �� ����ü�� ���� �Ҵ�Ǿ� ���� ���̴��� �� ���ʰ�.
typedef struct
{
	// Ŭ���̾�Ʈ ����
	SOCKET hClntSock[MAX_CLNT];
	// Ŭ���̾�Ʈ �ּ�
	SOCKADDR_IN clntAdr;
} PER_HANDLE_DATA, *LPPER_HANDLE_DATA;

// IO�� ���Ǵ� ���ۿ� Overlapped IO�� �ʿ��� OVERLAPPED����ü ������ ������
typedef struct
{
	// WSASend, WSARecv�� ����� ������Ʈ ����ü
	OVERLAPPED overlapped;
	// ���� �����Ϳ� ���̰� ����� ����ü
	WSABUF wsaBuf;
	// ���� ������
	char buffer[BUF_SIZE];
	// �����͸� ���´��� �޾Ҵ��� ǥ���ϱ� ����
	int rwMode;
} PER_IO_DATA, *LPPER_IO_DATA; // LP�� ������ ������ �����̶�� �� *DWORD == LPDWORD

// Ŭ���̾�Ʈ�� ����
int clntCnt = 0;
HANDLE hMutex;

DWORD WINAPI ChatThreadMain(LPVOID CompletionPortIO);

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	HANDLE hComPort;
	SYSTEM_INFO sysInfo;
	LPPER_IO_DATA ioInfo;
	LPPER_HANDLE_DATA handleInfo;
	for (int i = 0; i < 3; i++)
	{
		handleInfo = (LPPER_HANDLE_DATA)malloc(sizeof(PER_HANDLE_DATA));
	}
	
	SOCKET hServSock;
	SOCKADDR_IN servAdr;

	int recvBytes, i, flags = 0;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		ErrorHandling("WSAStartup() error!");
	}

	hMutex = CreateMutex(NULL, FALSE, NULL);

	hComPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	GetSystemInfo(&sysInfo);
	for (i = 0; i < sysInfo.dwNumberOfProcessors; i++)
	{
		_beginthreadex(NULL, 0, ChatThreadMain, (LPVOID)hComPort, 0, NULL);
	}
	hServSock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family = AF_INET;
	servAdr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAdr.sin_port = htons(atoi(argv[1]));

	bind(hServSock, (SOCKADDR*)&servAdr, sizeof(servAdr));
	listen(hServSock, 5);

	while (1)
	{
		SOCKET hClntSock;
		SOCKADDR_IN clntAdr;
		int addrLen = sizeof(clntAdr);
		puts("�����û ���ų� ������....");
		hClntSock = accept(hServSock, (SOCKADDR*)&clntAdr, &addrLen);

		
		WaitForSingleObject(hMutex, INFINITE);
		handleInfo->hClntSock[clntCnt++] = hClntSock;
		ReleaseMutex(hMutex);
		memcpy(&(handleInfo->clntAdr), &clntAdr, addrLen);

		CreateIoCompletionPort((HANDLE)hClntSock, hComPort, (DWORD)handleInfo, 0);
		puts("CP���� ��");
		ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
		memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
		ioInfo->wsaBuf.len = BUF_SIZE;
		ioInfo->wsaBuf.buf = ioInfo->buffer;

		ioInfo->rwMode = READ;

		//for (int i = 0; i < clntCnt; i++)
		//{
			printf("handleInfo->hClntSock[%d] = %d \n", clntCnt, handleInfo->hClntSock[clntCnt - 1]);
			WSARecv(handleInfo->hClntSock[clntCnt - 1], &(ioInfo->wsaBuf), 1, &recvBytes, &flags, &(ioInfo->overlapped), NULL);
		//}
	}
	return 0;
}

DWORD WINAPI ChatThreadMain(LPVOID CompletionPortIO)
{
	HANDLE hComPort = (HANDLE)CompletionPortIO;
	SOCKET sock[MAX_CLNT];
	DWORD bytesTrans;
	LPPER_HANDLE_DATA handleInfo;
	LPPER_IO_DATA ioInfo;
	DWORD flags = 0;

	while (1)
	{
		puts("GetQueuedCompletionStatus.........");
		GetQueuedCompletionStatus(hComPort, &bytesTrans, (LPDWORD)&handleInfo, (LPOVERLAPPED*)&ioInfo, INFINITE);
		WaitForSingleObject(hMutex, INFINITE);
		for (int i = 0; i < clntCnt; i++)
		{
			sock[i] = handleInfo->hClntSock[i];
		}
		ReleaseMutex(hMutex);

		if (ioInfo->rwMode == READ)
		{
			// Ŭ���̾�Ʈ�κ��� ����(EOF)�� ���� �޾Ҵٸ�
			if (bytesTrans == 0)
			{
				// Ŭ���̾�Ʈ ���� ����, �޸� ����
				closesocket(sock);
				free(handleInfo);
				free(ioInfo);
				// �ٽ� GetQueuedCompletionStatus���� �ʱ�ȭ�ϸ鼭 �ٽ�
				continue;
			}

			memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
			// ���� ������ ũ�⸦ �ٽ� �ʱ�ȭ����
			ioInfo->wsaBuf.len = bytesTrans;
			// ���� �����͸� Ŭ���̾�Ʈ�� �����Ŵϱ� �����ٴ� ǥ������.
			ioInfo->rwMode = WRITE;
			// ������ ������
			// LPPER_IO_DATA����ü�� ���� WSABUF�� OVERLAPPED ��� ������ ���·� ���ǵǾ� ���� �����Ƿ�
			// �ּҸ� �ѱ�� ���� &�� �����־���.
			for (int i = 0; i < clntCnt; i++)
			{
				printf("sock[%d] = %d \n", i, sock[i]);
				printf("&(ioInfo->overlapped) = %d \n", &(ioInfo->overlapped));
				puts("�����Ҳ���");
				WSASend(sock[i], &(ioInfo->wsaBuf), 1, NULL, 0, &(ioInfo->overlapped), NULL);
				puts("����");
			}
			
			
			// �޽��� ������ ���Ŀ� Ŭ���̾�Ʈ�� �����ϴ� �޽����� �����ؾ��Ѵ�.
			// ������ �����͸� ������ �޸𸮸� �Ҵ��Ѵ�.
			ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
			memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
			ioInfo->wsaBuf.len = BUF_SIZE;
			ioInfo->wsaBuf.buf = ioInfo->buffer;
			// �����͸� ������ ������ ǥ��
			ioInfo->rwMode = READ;
			//puts("����ǥ�ÿϷ�");
			// ������ ����
			for (int i = 0; i < clntCnt; i++)
			{
				WSARecv(sock[i], &(ioInfo->wsaBuf), 1, NULL, &flags, &(ioInfo->overlapped), NULL);
			}
			puts("WSARecvȣ���� ������ ���� �ϷḦ ���� ������ ��");
			// Ŭ���̾�Ʈ�� �����͸� ���´� �Ⱥ��´����� WSARecv�� ȣ���ߴ�. �̰��� WSARecv�Լ��� ����ŷ �̱⶧���� �����ϴ�
			// ���� �ݺ��ϴ� while�� ���̱� ������ GetQueuedCompletionStatus�Լ��� �ٽ� �����µ� WSARecv�Լ��� ������ ������ �Ϸ��ϸ�
			// GetQueuedCompletionStatus�Լ��� �������� �� �ִ�.
			// �� Ŭ���̾�Ʈ�� �޼����� ������ GetQueuedCompletionStatus�� ���������Ƿ� 
			// ���� �ݺ����� ���� IO�� �Ϸ�� ������ ã�� �ʿ䰡 ����.

		}
		else
		{
			// WSASend�Լ� ȣ�� �� �ڶ�� (������ �����Ͱ� ���ٸ�)
			puts("message send");
			// IO���� �޸� ����
			//free(ioInfo);
		}

	}

	return 0;
}

void ErrorHandling(char* msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}