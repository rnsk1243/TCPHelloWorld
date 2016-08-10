/*
IOCP ������ ����� ����
- �ͺ��ŷ ������� IO�� ����Ǳ� ������, IO �۾����� ���� �ð��� ������ �߻����� �ʴ´�.
- IO�� �Ϸ�� �ڵ��� ã�� ���ؼ� �ݺ����� ������ �ʿ䰡 ����. // �ü���� �Ϸ�� ������ �˷���
- IO�� �������� ������ �ڵ��� �迭�� ������ ����, ������ �ʿ䰡 ����.
- IO�� ó���� ���� �������� ���� ������ �� �ִ�. ���� ������ ����� ���� ������ �������� ���� ������ �� �ִ�.
*/
#include<stdio.h>
#include<stdlib.h>
#include<process.h>
#include<WinSock2.h>
#include<Windows.h>

#define BUF_SIZE 100
#define READ 3
#define WRITE 5

// Ŭ���̾�Ʈ�� ����� ���� ������ ������� ����ü
// �� ����ü�� ���� �Ҵ�Ǿ� ���� ���̴��� �� ���ʰ�.
typedef struct
{
	// Ŭ���̾�Ʈ ����
	SOCKET hClntSock;
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

DWORD WINAPI EchoThreadMain(LPVOID CompletionPortIO);

void ErrorHandling(char* message);

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	HANDLE hComPort;
	SYSTEM_INFO sysInfo;
	LPPER_IO_DATA ioInfo;
	LPPER_HANDLE_DATA handleInfo;

	SOCKET hServSock;
	SOCKADDR_IN servAdr;
	int recvBytes, i, flags = 0;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		ErrorHandling("WSAStartup() error!");
	}
	// Completion Port ������Ʈ(���� CP������Ʈ) ���� 
	// IO�� �Ϸ� ��Ȳ�� CP������Ʈ�� ��ϵ�
	// ������ �Ű������� ���� IO�� �Ϸῡ ���� ������ �޴� �������� ������ �ǹ��Ѵ�.(0�̸� ���డ���� �ִ밹��)
	hComPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	// �ý��� ������ ������
	GetSystemInfo(&sysInfo);
	// cpu�� ������ŭ for���� �ݺ��Ͽ� �����带 ����
	for (i = 0;i < sysInfo.dwNumberOfProcessors; i++)
	{
		//printf("������ ���� = %d \n", i); 4��
		// ������ ����
		// �̶� �Ű������� CP������Ʈ�� �Ѱ������μ� �����尡 CP������Ʈ�� ���� �� �� �ִ�
		_beginthreadex(NULL, 0, EchoThreadMain, (LPVOID)hComPort, 0, NULL);
	}
	// OVERLAPPED�� ���� ����
	// �� ���ŷ ������ �ƴϴ�. �� �����û�� �������� accept���� ���ŷ ���°� �ȴ�.
	hServSock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family = AF_INET;
	servAdr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAdr.sin_port = htons(atoi(argv[1]));

	// ��������ϰ� �����ּ� ����
	bind(hServSock, (SOCKADDR*)&servAdr, sizeof(servAdr));
	// ���
	listen(hServSock, 5);

	while (1)
	{
		SOCKET hClntSock;
		SOCKADDR_IN clntAdr;
		int addrLen = sizeof(clntAdr);
		// ���� // ���� ���� ��û�� ������� ���ŷ ���°� �ȴ�.
		puts("accept.........");
		hClntSock = accept(hServSock, (SOCKADDR*)&clntAdr, &addrLen);
		// ����ü �޸� �Ҵ�(while�� �ȿ� �����Ƿ� ���ο� ������ ������������� ���Ӱ� �Ҵ��)
		handleInfo = (LPPER_HANDLE_DATA)malloc(sizeof(PER_HANDLE_DATA));
		// �޸𸮰� �Ҵ�� ����ü�� Ŭ���̾�Ʈ ���ϰ� �ּҸ� ����.
		handleInfo->hClntSock = hClntSock;
		memcpy(&(handleInfo->clntAdr), &clntAdr, addrLen);

		//accept�� ���� ���� ������� Ŭ���̾�Ʈ ���ϰ� CP������Ʈ ����
		// �̷��� �����μ� "�� ������ ������� ����Ǵ� IO�� �Ϸ� ��Ȳ�� �� CP ������Ʈ�� ����� �ּ���"�� �ȴ�.
		// ������ ������ handleInfo����ü ���� ����ϱ� ���� 3��° ���ڷ� ����ü�� �־��ش�.
		// �׸��� ������ ������ handleInfo����ü ���� ���������� GetQueuedCompletionStatus�Լ��� ȣ���Ͽ� ���´�.
		CreateIoCompletionPort((HANDLE)hClntSock, hComPort, (DWORD)handleInfo, 0);

		// WSARecv�Լ� ȣ�⿡ �ʿ��� ���� ��� ����ü �޸� ���� �Ҵ�(���� ����ø��� �þ)
		ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
		memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
		ioInfo->wsaBuf.len = BUF_SIZE;
		ioInfo->wsaBuf.buf = ioInfo->buffer;
		// IOCP�� �Է��� �Ϸ�� ����� �ϷḦ ���� �������� �ʴ´� �ٸ� �Ϸ�Ǿ��ٴ� ��Ǹ� �˷���.
		// ���� ���� �Է��� ���� ����� �������� ������ ���������Ѵ�. 
		// ������ WSARecv�� �Ұ��̹Ƿ� READ�� �����صξ���.
		ioInfo->rwMode = READ;
		// �ϰ���° ���ڷ� OVERLAPPED ����ü ������ �ּҰ��� �����Ͽ��µ� �̰��� 
		// �����忡�� GetQueuedCompletionStatus�Լ��� ȣ���Ͽ� �ٽ� ������ �����̴�.
		// ����ü ������ �ּҰ��� ù��° �ּ� ���� �����ϹǷ� 
		// LPPER_IO_DATA == &(ioInfo->overlapped)�̰��� �����Ѵ�.
		// ���� �����峻���� LPPER_IO_DATA ioInfo��� ������ �����ϰ� �Լ����ڷ� (LPOVERLAPPED*)&ioInfo��� ����
		// WSARecv�̰��� ȣ��ž� �����嵵GetQueuedCompletionStatus �Լ����� ���ŷ ������ �ʰ� �������� �Ѿ �� �ִ�.
		//puts("���ν����� WSARecv ȣ�� �Ѵ�");
		WSARecv(handleInfo->hClntSock, &(ioInfo->wsaBuf), 1, &recvBytes, &flags, &(ioInfo->overlapped), NULL);
		//printf("recvBytes = %d \n", recvBytes);
		//puts("���ν����� WSARecv ȣ�� �ߴ�");
	}
	return 0;
}

// GetQueuedCompletionStatus�Լ��� ȣ���ϴ� ������ �̹Ƿ� CP ������Ʈ�� �Ҵ�� �������̴�.
DWORD WINAPI EchoThreadMain(LPVOID CompletionPortIO)
{
	HANDLE hComPort = (HANDLE)CompletionPortIO;
	SOCKET sock;
	DWORD bytesTrans;
	LPPER_HANDLE_DATA handleInfo;
	LPPER_IO_DATA ioInfo;
	DWORD flags = 0;

	while (1)
	{
		// WSASend, WSARecv�� �����忡�� ȣ�� �ϱ����� �ٷ� ������ ������ �������� �ʱ�ȭ �Ѵ�.
		// �̶� �ʿ��� �Լ��� GetQueuedCompletionStatus�Լ� �̸� �� �Լ��� ���� ���� ������ �� �ִ� ������
		// ���ο��� �ι�° CreateIoCompletionPort�Լ� ȣ�⶧ 3��° �Ű������� LPPER_HANDLE_DATA�� �־��־���
		// ���� �������� WSARecv�Լ� 7��° �Ű������� overlapped������ �־��־��⶧����(== LPPER_IO_DATA����ü�� �ּ�)
		// 122,123���� �ΰ��� ����ü LPPER_HANDLE_DATA, LPPER_IO_DATA ��θ� �ʱ�ȭ �� �� �ִ�.
		// �� ���� �Լ��� ȣ���Ͽ� �ʱ�ȭ �غ���. // �̰��� �ʱ�ȭ �Ǿ� ���� �Ǵ� ���ѱ�ٸ���.
		// GetQueuedCompletionStatus�Լ��� IO�� �޷�ǰ�, �̿� ���� ������ ��ϵǾ����� ��ȯ�Ѵ�.
		puts("GetQueuedCompletionStatus.........");
		GetQueuedCompletionStatus(hComPort, &bytesTrans, (LPDWORD)&handleInfo, (LPOVERLAPPED*)&ioInfo, INFINITE);
		// WSARecv�Լ��� �Ϸ�Ǿ� GetQueuedCompletionStatus�Լ��� ���������� ������ �Ϸ��� ������ handleInfo ����ü �ȿ� �ִٴ� ��
		// ���� ���� �� �������� �����͸� ������ ��. 
		// ������� 4���̹Ƿ� �� EchoThreadMain�Լ��� 4���� ���������� �������� Ŭ���̾�Ʈ�� ����� �� �ִ� ������
		// GetQueuedCompletionStatus�Լ����� ���ŷ���ϴٰ� ������ �Ϸ��� ���Ͽ� ���ؼ��� �ü���� �˷��ֹǷ� �� ���ϸ� �������� �Ѿ���� ���ش�.
		// �׸��� ������ ������������� CreateIoCompletionPort�Լ��� ���� ���ϰ� CP�� ����ǹǷ�(���) �ü���� �� ������ IO�� �Ϸ�Ǿ����� �˷��� �� �ִ°Ŵ�.
		// ���� �ʱ�ȭ�� ����ü�� �̿��Ͽ� ������ �ʱ�ȭ �Ѵ�.
		sock = handleInfo->hClntSock;
		// ���࿡ �����Լ����� �Ѿ�� LPPER_IO_DATA ����ü �߿� rwMode���� READ�̸�
		// ��, WSARecv�Լ��� ȣ���� �ڶ�� Ŭ���̾�Ʈ�κ��� �����͸� �޾����� �״�� �ٽ� �����ش�(���ڼ����ϱ�)
		if (ioInfo->rwMode == READ)
		{
			puts("message received!");
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
			// �����͸� �ް� ���Ḧ ���� �������� �ƴ϶�� ���� �����͸� �ٽ� Ŭ���̾�Ʈ���� �������Ѵ�.
			// WSASend�Լ��� ����� OVERLAPPED ����ü�� �ٽ� ���� �ʱ�ȭ ���ش�.(������ WSARecv�� ����ϴ� �����ʹ� ������.)
			// OVERLAPPED����ü�� ����ؾ� �񵿱� IO�� �����ϹǷ�! ���� WSASend�Լ��� ����սô�.
			memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
			// ���� ������ ũ�⸦ �ٽ� �ʱ�ȭ����
			ioInfo->wsaBuf.len = bytesTrans;
			// ���� �����͸� Ŭ���̾�Ʈ�� �����Ŵϱ� �����ٴ� ǥ������.
			ioInfo->rwMode = WRITE;
			// ������ ������
			// LPPER_IO_DATA����ü�� ���� WSABUF�� OVERLAPPED ��� ������ ���·� ���ǵǾ� ���� �����Ƿ�
			// �ּҸ� �ѱ�� ���� &�� �����־���.
			WSASend(sock, &(ioInfo->wsaBuf), 1, NULL, 0, &(ioInfo->overlapped), NULL);

			puts("�����Ҳ���");
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
			WSARecv(sock, &(ioInfo->wsaBuf), 1, NULL, &flags, &(ioInfo->overlapped), NULL);
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
			free(ioInfo);
		}
	} // while�� ��
	return 0;
}

void ErrorHandling(char* msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}