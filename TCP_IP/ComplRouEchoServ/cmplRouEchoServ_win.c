#include<stdio.h>
#include<stdlib.h>
#include<WinSock2.h>

#define BUF_SIZE 1024

void CALLBACK ReadCompRoutine(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
void CALLBACK WriteCompRoutine(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
void ErrorHandling(char* message);

typedef struct
{
	SOCKET hClntSock;
	char buf[BUF_SIZE];
	WSABUF wsaBuf;
} PER_IO_DATA, *LPPER_IO_DATA;

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	SOCKET hLisnSock, hRecvSock;
	SOCKADDR_IN lisnAdr, recvAdr;
	// WSAOVERLAPPED�� ���������°� LPWSAOVERLAPPED �̰���
	LPWSAOVERLAPPED lpOvLp;
	DWORD recvBytes;
	LPPER_IO_DATA hbInfo;
	int mode = 1, recvAdrSz, flagInfo = 0;

	if (argc != 2) {
		printf("Usage: %s <IP> <port> \n", argv[0]);
		exit(1);
	}
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		ErrorHandling("WSAStartup() error!");
	}

	hLisnSock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	// non-blocking ���� �����
	// ioctlsocket(������, �������, ��尪);
	ioctlsocket(hLisnSock, FIONBIO, &mode);

	memset(&lisnAdr, 0, sizeof(lisnAdr));
	lisnAdr.sin_family = AF_INET;
	lisnAdr.sin_addr.s_addr = htonl(INADDR_ANY);
	lisnAdr.sin_port = htons(atoi(argv[1]));

	if (bind(hLisnSock, (SOCKADDR*)&lisnAdr, sizeof(lisnAdr)) == SOCKET_ERROR)
		ErrorHandling("bind() error");
	if (listen(hLisnSock, 5) == SOCKET_ERROR)
		ErrorHandling("listen() Error");

	recvAdrSz = sizeof(recvAdr);
	while (1)
	{
		// Completion Routine�� ȣ���ϱ� ���� alertable wait���·� ����
		// alertable wait���´� �ü���� �����ϴ� �޽����� ������ ����ϴ� �������� ������
		// �̰��� �ſ� �߿��� �۾������߿� Completion Routine�� ȣ���ع����� �߿��� �۾��� �Ϸ��� �� �����ϱ�
		// alertable wait�����϶��� Completion Routine�� ȣ���ϵ��� �������
		puts("����!");
		SleepEx(100, TRUE);
		// hLisnSock�� �ͺ��ŷ �����̹Ƿ� ���� �����Ǵ� ���� ���� ��-���ŷ �����̵ȴ�.
		hRecvSock = accept(hLisnSock, (SOCKADDR*)&recvAdr, &recvAdrSz);
		// ���� Ŭ���̾�Ʈ�� �����û�� �������� �ʴ� ���¿��� accept�Լ��� ȣ��Ǹ�
		// hRecvSock�� �ͺ������̹Ƿ� ��ٷ� INVALID_SOCKET�� ��ȯ �ȴ�.
		if (hRecvSock == INVALID_SOCKET)
		{
			// �̾ ����Ȯ�� �Լ��� ȣ���ϸ� WSAEWOULDBLOCK�� �����µ� �̷�����
			// �� while�� ó������ �ٽ� �����϶�� �Ѵ�.
			// while���� ���� �ݺ��̰� �ͺ�� �����̹Ƿ� �����û���̵� accept�Լ��� �ٷ� ���������Ƿ� �̷���
			// ����� ���� ���ϴ�.
			if (WSAGetLastError() == WSAEWOULDBLOCK)
			{
				puts("�����û������...");
				// Ŭ���̾�Ʈ�� �����û�� �������ŷα�... �ٽ� while�� ó������
				continue;
			}else
			{
				ErrorHandling("accept() error");
			}
		}
		// Ŭ���̾�Ʈ�� �����û�� �־�����
		puts("Client connected......");
		// Overlapped IO�� �ʿ���(WSASend, WSARecv�Լ��� �ʿ�) WSAOVERLAPPED����ü �޸� �Ҵ�
		// �ݺ��� �ȿ����ϴ� ������ Ŭ���̾�Ʈ �ϳ��� ���ο� WSAOVERLAPPED����ü�� �ʿ��ϱ� ����
		lpOvLp = (LPWSAOVERLAPPED)malloc(sizeof(WSAOVERLAPPED));
		// ����ü �ʱ�ȭ
		memset(lpOvLp, 0, sizeof(WSAOVERLAPPED));

		// ���������� ��� ����ü �޸� ������� (���� �����û�Ǹ� �÷������)
		// �̷��Ƿμ� ����ü �ȿ��ִ� ���ۿ� ���������� ���Ӱ� ��������� ����Ǵ� ������ ����ϰ� �ȴ�.
		// �ѹ��� PER_IO_DATA ��ŭ��
		hbInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
		// accept������ ������� �������� �ʱ�ȭ(����Ǵ� ����)
		hbInfo->hClntSock = (DWORD)hRecvSock;
		// ���� �ʱ�ȭ
		(hbInfo->wsaBuf).buf = hbInfo->buf;
		// ������ �ʱ�ȭ
		(hbInfo->wsaBuf).len = BUF_SIZE;

		// Completion Routine����� Overlapped IO������ Event������Ʈ�� ���ʿ��ϱ� ������ hEvent�� �ʿ��� �ٸ� ������ ä��
		// Completion Routine�� 3��° ���ڷκ��� ������� �Ϸ�� ����(�����ּ�,��������,ũ�� ��)�� �������
		// �̷��� ���� ���� hEvent������ �������ش�.(Event������Ʈ�� Completion Routine�� ���� ������� ���ҵ�.)
		// ���� ReadCompRoutine�� 3��° ���ڷ� ���������� ��� �ʱ�ȭ�ϴ� �� �� �� �� ���� ���̴�.
		lpOvLp->hEvent = (HANDLE)hbInfo;
		// WSARecv�� ȣ�������μ� ReadCompRoutine�Լ�(������ �Ű�����)�� Completion Routine���� ������
		// ���⼭ 6��° ���ڷ� ������ WSAOVERLAPPED����ü ������ �ּҰ��� Completion Routine�� 3��° �Ű������� ���޵�.
		// ��, ReadCompRoutine�Լ��� ����° ���ڿ� ���޵�.
		// ����  Completion Routine�Լ��� ������� �Ϸ�� ������ �ڵ�� ���ۿ� ������ �� �ִ�.
		// SleepEx(100, TRUE);�� �ݺ�ȣ�� ���ؼ� Completion Routine�� ������ �� �ִ�
		WSARecv(hRecvSock, &(hbInfo->wsaBuf), 1, &recvBytes, &flagInfo, lpOvLp, ReadCompRoutine);
	}
	closesocket(hRecvSock);
	closesocket(hLisnSock);
	WSACleanup();
	return 0;
}

// WSARecv�� ������ ���ڷ� ��ϵ� �� �Լ��� ȣ��Ǿ��ٴ� ���� ������ �Է��� �Ϸ�Ǿ��ٴ� �� 
// ��, WSARecv�Լ��� ��� �����͸� ��׶��忡������ �ؼ� ���� �����Դٴ� ��.
void CALLBACK ReadCompRoutine(DWORD dwError, DWORD szRecvBytes, LPWSAOVERLAPPED lpOverlapped, DWORD flags)
{
	puts("ReadCompRoutine ȣ��");
	// 3��° �Ű������� ���ϰ� ���������� ����ü ��������
	//(���� �Լ����� �̸� �����صξ��⶧���� ������ �� �־���.)
	LPPER_IO_DATA hbInfo = (LPPER_IO_DATA)(lpOverlapped->hEvent);
	// ����ü���� ������ϰԲ� ���� ������
	SOCKET hSock = hbInfo->hClntSock;
	// ���۱���ü ������
	LPWSABUF bufInfo = &(hbInfo->wsaBuf);
	// ���� ����Ʈ��
	DWORD sentBytes;

	// ���� ���� ������ ���� 0�̸�(EOF�������� ���Ḧ ����)
	if (szRecvBytes == 0)
	{
		// ���� ����
		closesocket(hSock);
		// �޸� ����
		free(lpOverlapped->hEvent);
		free(lpOverlapped);
		puts("Client disconnected......");
	}
	else// ���� ������ ���� 0���� ũ�� ����
	{
		// ���� �����ͼ� �ʱ�ȭ
		bufInfo->len = szRecvBytes;
		// ����! Ŭ���̾�Ʈ�� �޽��� �����ϴµ� ������ �Ϸ�Ǹ� WriteCompRoutine�Լ��� ȣ���ض�
		WSASend(hSock, bufInfo, 1, &sentBytes, 0, lpOverlapped, WriteCompRoutine);
	}

}
// WSASend�Լ��� ��� �����͸� ���ۿϷ��ϸ� ���� �Լ��� ȣ��ȴ�. 
void CALLBACK WriteCompRoutine(DWORD dwError, DWORD szSendBytes, LPWSAOVERLAPPED lpOverlapped, DWORD flags)
{
	puts("WriteCompRoutine ȣ��");
	LPPER_IO_DATA hbInfo = (LPPER_IO_DATA)(lpOverlapped->hEvent);
	SOCKET hSock = hbInfo->hClntSock;
	LPWSABUF bufInfo = &hbInfo->wsaBuf;
	DWORD recvBytes;
	int flagInfo = 0;
	// Ŭ���̾�Ʈ�κ��� �� �޽��� ����
	WSARecv(hbInfo, bufInfo, 1, &recvBytes, &flagInfo, lpOverlapped, ReadCompRoutine);
	puts("��������������");

}

void ErrorHandling(char* msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}