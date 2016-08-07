#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<stdio.h>
#include<stdlib.h>
#include<WinSock2.h>
void ErrorHandling(char* msg);

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	SOCKET hSocket;
	SOCKADDR_IN sendAdr;

	WSABUF dataBuf;
	char msg[] = "NetWork is Computer!";
	int sendBytes = 0;

	WSAEVENT evObj;
	WSAOVERLAPPED overlapped;

	if (argc != 3) {
		printf("Usage: %s <IP> <port> \n", argv[0]);
		exit(1);
	}
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		ErrorHandling("WSAStartup() error!");
	}
	// WSA_FLAG_OVERLAPPED�� �����ؼ� �����Ǵ� ���Ͽ� Overlapped IO�� ������ �Ӽ��� �ο��Ͽ�
	// ������ ����
	hSocket = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	memset(&sendAdr, 0, sizeof(sendAdr));
	sendAdr.sin_family = AF_INET;
	sendAdr.sin_addr.s_addr = inet_addr(argv[1]);
	sendAdr.sin_port = htons(atoi(argv[2]));

	if (connect(hSocket, (SOCKADDR*)&sendAdr, sizeof(sendAdr)) == SOCKET_ERROR)
		ErrorHandling("connect() error!");

	// �޴����� non-signaled
	evObj = WSACreateEvent();
	memset(&overlapped, 0, sizeof(overlapped));
	// �̺�Ʈ������Ʈ�� �����ؼ� ������ ���� �Ϸ��ߴ��� Ȯ�� �뵵
	overlapped.hEvent = evObj;
	// WSADATA �����ͱ���ü�� ���� �����Ϳ� ũ�� ���� (WSASend�Լ��� 2��° ����) 
	dataBuf.len = sizeof(msg) + 1;
	dataBuf.buf = msg;
	// WSASend(����������������, ���������ͱ���ü, ����ü����, ���۵ȹ���Ʈ�� ����� �����ּ�, ����Ư��, ���ۿϷ�Ȯ�ο��̺�Ʈ��䱸��ü, Completion Routine�Լ�(����������Ȯ�ο�) �ּ�)
	if (WSASend(hSocket, &dataBuf, 1, &sendBytes, 0, &overlapped, NULL) == SOCKET_ERROR)
	{
		// ���� SOCKET_ERROR�� �߻��ϸ� ������ �� ���� �̴�.
		// ù°. WSASend�Լ��� ��ȯ�� �� �������� ����ؼ� �������� ������ �̷����� ��Ȳ
		// ��°. �׿� ����
		////////////////////////////////////////////////////////////////////////////////
		// ù°���� �ϰ��
		// WSAGetLastError�Լ�ȣ���� ���ؼ� �����ڵ� WSA_IO_PENDING�� ��ϵǾ����� Ȯ���Ѵ�.
		// �����  WSAGetLastError�Լ��� ���ϰ��� �Լ��� ȣ��� ���Ŀ� �߻��ϴ� ���� ���������� ��ȯ��.
		if (WSAGetLastError() == WSA_IO_PENDING)
		{
			// WSAGetLastError() == WSA_IO_PENDING �̰�쿡�� �������� ������ �Ϸ�����¾ʾ����� ����ؼ� �������λ���
			// ���� �̶� sendBytes�� ����� ���� �������.
			puts("Background data send");
			// WSAWaitForMultipleEvents(�̺�Ʈ������Ʈ����, �̺�Ʈ������Ʈ, ��α�ٸ�?, ����?, alertable wait���� ����?)
			// overlapped.hEvent = evObj; �̷��� ���־����Ƿ� ������ ������ �Ϸ�Ǹ� evObj�� signaled���°� �ȴ�.
			// ���� WSAWaitForMultipleEvents�Լ��� ���� evObj�� signaled���°� �ɶ����� ���� ��ٸ����� ���ش�.
			WSAWaitForMultipleEvents(1, &evObj, TRUE, WSA_INFINITE, FALSE);
			// WSAGetOverlappedResult(����, ���ۿϷ�Ȯ�ο뱸��ü, ���۵ȹ���Ʈ�� ����� �����ּ�, ���ۿϷ�ɶ����� ��ٸ���?, �μ����� �ʿ�?)
			// ������ �Ϸ�Ǿ����Ƿ� ���� sendBytes������ ���۵� ���� �����Ѵ�.
			WSAGetOverlappedResult(hSocket, &overlapped, &sendBytes, FALSE, NULL);
		}
		else
		{
				  // �׿� ����
			ErrorHandling("WSASend() error");
		}
	}
	//SOCKET_ERROR�� �߻����� ������ WSASend�Լ��� ��ȯ�� ���ÿ� ������ ������ �Ϸ�Ȱ�
	printf("Send data size: %d \n", sendBytes);
	WSACloseEvent(evObj);
	closesocket(hSocket);
	WSACleanup();
	return 0;
}

void ErrorHandling(char* msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}